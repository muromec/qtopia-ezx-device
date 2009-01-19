#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CONFIG_ARCH_EZX
#include <linux/apm_bios.h>

#include <QSocketNotifier>
#include <qtopiaserverapplication.h>
#include <qtopialog.h>

#include "time_utils.h"
#include "ezxapm.h"

#define GEAR_UP_THRESHOLD 70
#define GEAR_DOWN_THRESHOLD 20
#define MAX_LOAD_BALANCE 10

QTOPIA_TASK(EzxAPM, EzxAPM);

// EzxAPM power profiles; greater index gives more power
const ipm_config EzxAPM::power_profiles[] =
{
//{core_freq, core_volt, turbo_ratio, cpu_mode, fast_bus_mode, sys_bus_freq, mem_bus_freq, lcd_freq, -1}
//{13000,           900,           1,        0,             1,        13000,        13000,    26000, -1},
//{26000,           950,           1,        0,             1,        26000,        26000,    26000, -1},
  {104000,         1050,           2,        0,             1,       104000,       104000,    52000, -1},
  {208000,         1300,           2,        0,             1,       208000,       208000,   104000, -1},
  {312000,         1350,           3,        1,             1,       208000,       208000,   104000, -1},
  {416000,         1400,           4,        1,             1,       208000,       208000,   104000, -1},
  {520000,         1450,           5,        1,             1,       208000,       208000,   208000, -1},
//{520000,         1450,           5,        1,             1,       208000,       208000,   104000, -1},
//{624000,         1500,           6,        1,             1,       208000,       208000,   104000, -1}
};
//const int EzxAPM::n_profiles = 6;
const int EzxAPM::n_profiles = sizeof(power_profiles)/sizeof(ipm_config);

EzxAPM::EzxAPM(QObject *parent)
: QObject(parent),
  total_slept_time(0), cpu_load(0), load_balance(0), apm_noti(NULL)
{
  apm_fd = ::open("/dev/apm_bios", O_RDWR);
  if (apm_fd<0)
    qLog(Hardware) << "Error: could not open apm_bios";
  else
  {
    qLog(Hardware) << "APM task starting";

    // start PMU
    int n = ::ioctl(apm_fd, APM_IOC_SET_SPROF_WIN, 1);
    //dbg("starting: APM_IOC_SET_SPROF_WIN %d",n);
    startPMU();

    setPowerPolicy(Battery);
    setPowerProfile(n_profiles-1); // High performance is good at startup

    apm_noti = new QSocketNotifier(apm_fd, QSocketNotifier::Read, this);
    connect(apm_noti, SIGNAL(activated(int)), SLOT(apmEvent(int)));
  }
}

EzxAPM::~EzxAPM()
{
  if (apm_fd>=0)
  ::close(apm_fd);
}

void EzxAPM::apmEvent(int fd)
{
  apm_event_t apm_event;
  ::read(apm_fd, &apm_event, sizeof(apm_event));
  //dbg("apm event");

  switch(apm_event.type)
  {
    // device activity
    case APM_EVENT_DEVICE:
        switch(apm_event.kind)
        {
          case EVENT_DEV_TS:  break;
          case EVENT_DEV_KEY: break;
          case EVENT_DEV_FLIP:
            /*
            if (apm_event.info)
            {
              fb_on(100);
              turbo_mode();
            }
            else
            {
              fb_off();
              slow_mode();
            }
            */
            qLog(Hardware) << "flip " << apm_event.info;
            break;
          case EVENT_DEV_ACCS:
            qLog(Hardware) << "accessory";
            /*
            if (apm_event.info & 0x80000000)
              Daemon::accy()->plugCable(0x7fffffff & apm_event.info);
            else
              Daemon::accy()->unplugCable(0x7fffffff & apm_event.info);
            break;
            */

          //default:
            //dbg("apm device event kind:%x info:%x",
            //    apm_event.kind, apm_event.info);
          }
      break;

    // event from PMU (sleep, cpu load, performance)
    case APM_EVENT_PROFILER:
      switch(apm_event.kind)
      {
        // PMU tells us current CPU load value
        case EVENT_PROF_IDLE:
          //dbg("cpu load %d", apm_event.info);
          cpu_load = apm_event.info;
          adjustPower(apm_event.info);
          startPMU();

        // high cpu or mem load
        case EVENT_PROF_PERF:
          //dbg("bottlenecks: %s%s",
          //     (apm_event.info & PERF_CPU_BOUND)?"[cpu] ":"",
         //      (apm_event.info & PERF_MEM_BOUND)?"[mem]":"");
          break;

        // sleep event from PMU
        case EVENT_PROF_SLEEP:
          qLog(Hardware) << "Sleep event";
          emit canSleep();
          break;
        //default:
          //dbg("profiler %x:%x", apm_event.kind, apm_event.info);
       }
      break;
    //default:
      //dbg("apm event: %x:%x:%x", apm_event.type, apm_event.kind, apm_event.info);
  }
}

// CPU power profiles management
void EzxAPM::gearUp()
{
  if (current_profile<n_profiles-1)
    setPowerProfile(current_profile+1);
  load_balance = 0;
}

void EzxAPM::gearDown()
{
  if (current_profile>0)
    setPowerProfile(current_profile-1);
  load_balance = 0;
}

void EzxAPM::setPowerProfile(int n)
{
  ipm_config cconf;
  int ret = ::ioctl(apm_fd, APM_IOC_GET_IPM_CONFIG, &cconf);
  qLog(Hardware) << "Current cpu freq:" << cconf.core_freq/1000 << "MHz";
  ret = ::ioctl(apm_fd, APM_IOC_SET_IPM_CONFIG, &power_profiles[n]);
  qLog(Hardware) << "New cpu freq:" << power_profiles[n].core_freq/1000 << "MHz";

  current_profile = n;
}

void EzxAPM::sleep()
{
  qLog(Hardware) << "Going to sleep";
  //dbg("battery level: %d", Daemon::power()->battery());

  Time::save();
  int sleep_time = Time::currentStamp();

  int ret = ioctl(apm_fd, APM_IOC_SLEEP, 1);

  Time::load();
  sleep_time = Time::currentStamp()-sleep_time;

  total_slept_time += sleep_time;

  //dbg("APM_IOC_SLEEP(%d): %d seconds now, %d minutes all, battery level: %d",
  //    ret, sleep_time, total_slept_time/60, Daemon::power()->battery());
  qLog(Hardware) << QString("APM_IOC_SLEEP(%1): %2 seconds now, %3 minutes all").
      arg(ret).arg(sleep_time).arg(total_slept_time/60);
}

void EzxAPM::startPMU()
{
  ioctl(apm_fd, APM_IOC_STARTPMU, NULL);
  //dbg("Starting PMU");
}

int EzxAPM::timeSlept() const
{
  return total_slept_time;
}

void EzxAPM::setPowerPolicy(PowerPolicy new_policy)
{
  switch (new_policy)
  {
    case Battery:
      qLog(Hardware) << "Power management policy: on battery";
      break;
    case Charger:
      qLog(Hardware) << "Power management policy: on charger";
  }
  policy = new_policy;
  adjustPower();
}

void EzxAPM::adjustPower(int load)
{
  switch (policy)
  {
    case Battery:
      if (load<0)
        setPowerProfile(0);
      else if (load > GEAR_UP_THRESHOLD)
        load_balance++;
      else if (load < GEAR_DOWN_THRESHOLD)
        load_balance--;

      //dbg("Load balance %d", load_balance);

      if (load_balance > MAX_LOAD_BALANCE)
        gearUp();
      else if (load_balance < -MAX_LOAD_BALANCE)
        gearDown();
      break;
    case Charger:
      if (load<0) // Call only on policy change
        setPowerProfile(n_profiles-1); // Always maximum
  }
}
