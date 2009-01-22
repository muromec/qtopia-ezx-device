#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CONFIG_ARCH_EZX
#include <linux/apm_bios.h>
#include <linux/power_ic.h>

#include <QSocketNotifier>
#include <qtopiaserverapplication.h>
#include <qtopialog.h>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include "time_utils.h"
#include "ezxapm.h"

#define GEAR_UP_THRESHOLD 70
#define GEAR_DOWN_THRESHOLD 20
#define MAX_LOAD_BALANCE 10

QTOPIA_TASK(EzxAPM, EzxAPM);
QTOPIA_TASK_PROVIDES(EzxAPM, SystemSuspendHandler);

// EzxAPM power profiles; greater index gives more power
const ipm_config EzxAPM::power_profiles[] =
{
//{core_freq, core_volt, turbo_ratio, cpu_mode, fast_bus_mode, sys_bus_freq, mem_bus_freq, lcd_freq, -1}  // CLASS

//{13000,           900,           1,        0,             1,        13000,        13000,    26000, -1}, // LOW
//{26000,           950,           1,        0,             1,        26000,        26000,    26000, -1}, // LOW
  {52000,          1000,           2,        0,             1,        52000,        52000,    52000, -1}, // LOW
  {104000,         1050,           2,        0,             1,       104000,       104000,    52000, -1}, // LOW

  {208000,         1300,           2,        0,             1,       208000,       208000,   104000, -1}, // MID
  {312000,         1350,           3,        1,             1,       208000,       208000,   104000, -1}, // MID
  {416000,         1400,           4,        1,             1,       208000,       208000,   104000, -1}, // MID

  {520000,         1450,           5,        1,             1,       208000,       208000,   208000, -1}, // HIGH
//{624000,         1500,           6,        1,             1,       208000,       208000,   104000, -1}  // HIGH
};
const int EzxAPM::n_profiles = sizeof(power_profiles)/sizeof(ipm_config);
const int EzxAPM::n_profiles_low = 2; // First n_profiles_low profiles are marked as LOW performance
const int EzxAPM::n_profiles_high = 1; // Last n_profiles_high profiles are marked as HIGH performance
// Assumed that n_profiles_low<nprofiles, n_profiles_high<nprofiles
/*
  MID performance profiles are always available
  LOW performance profiles are too slow for UI rendering, thus only allowed with screensaver on
  HIGH performance profiles consume too much power, thus they are only allowed when requested
*/

EzxAPM::EzxAPM(QObject *parent)
: SystemSuspendHandler(parent),
  apm_noti(NULL), load_balance(0), total_slept_time(0), sleep_allowed(false),
  power_status(this),
  allow_low_perf(false), allow_high_perf(true),
  vso("/Hardware/EZX/APM", this)
{
  apm_fd = ::open("/dev/apm_bios", O_RDWR);
  if (apm_fd<0)
    qLog(Hardware) << "Error: could not open apm_bios";
  else
  {
    qLog(Hardware) << "APM: connected to apm_bios";

    // start PMU
    ::ioctl(apm_fd, APM_IOC_SET_SPROF_WIN, 1);
    startPMU();

    setPowerProfile(n_profiles-1); // High performance is good at startup

    apm_noti = new QSocketNotifier(apm_fd, QSocketNotifier::Read, this);
    connect(apm_noti, SIGNAL(activated(int)), SLOT(apmEvent(int)));
  }

  // Initialize ValueSpace
  vso.setAttribute("TimeSlept", total_slept_time);
  vso.setAttribute("TimeSleptString", QString("0:00"));
  // CPU/Clock is initialised in setPowerProfile()
}

EzxAPM::~EzxAPM()
{
  if (apm_fd>=0)
  ::close(apm_fd);
}

void EzxAPM::apmEvent(int)
{
  apm_event_t apm_event;
  ::read(apm_fd, &apm_event, sizeof(apm_event));

  switch(apm_event.type)
  {
    // device activity
    case APM_EVENT_DEVICE:
        switch(apm_event.kind)
        {
          case EVENT_DEV_TS:
          case EVENT_DEV_KEY:
          {
            if (current_profile<n_profiles_low)
              setPowerProfile(n_profiles_low); // First MID profile for quick wakeup
            QtopiaServiceRequest bklight("QtopiaPowerManager", "setBacklight(int)");
            bklight << -1; // Set light on
            bklight.send();
          } break;

          case EVENT_DEV_FLIP:
            qLog(Hardware) << "flip " << (apm_event.info?"opened":"closed");
            if (apm_event.info) // opened
            {
              setTSState(true);
              if (current_profile<n_profiles_low)
                setPowerProfile(n_profiles_low);
              allow_low_perf = false;
            }
            else
            {
              setTSState(false);
              allow_low_perf = true;
            }
            break;

          case EVENT_DEV_ACCS:
            break;
          }
      break;

    // event from PMU (sleep, cpu load, performance)
    case APM_EVENT_PROFILER:
      switch(apm_event.kind)
      {
        // PMU tells us current CPU load value
        case EVENT_PROF_IDLE:
          adjustPower(apm_event.info);
          startPMU();

        // high cpu or mem load
        case EVENT_PROF_PERF:
          break;

        // sleep event from PMU
        case EVENT_PROF_SLEEP:
          qLog(Hardware) << "Sleep event";
          if (sleep_allowed)
          {
            sleep();
            //sleep_allowed = false;
            //emit operationCompleted();
          }
          break;
       }
      break;
  }
}

void EzxAPM::sleep()
{
  qLog(Hardware) << "Going to sleep";

  int sleep_time = Time::currentStamp();

  Time::save();
  int ret = ioctl(apm_fd, APM_IOC_SLEEP, 0);
  Time::load();

  sleep_time = Time::currentStamp()-sleep_time;

  total_slept_time += sleep_time;

  qLog(Hardware) << QString("APM_IOC_SLEEP(%1): %2 seconds now, %3 minutes all").
      arg(ret).arg(sleep_time).arg(total_slept_time/60);

  // Publish sleep stats to ValueSpace
  vso.setAttribute("TimeSlept", total_slept_time);
  char tmp[16];
  sprintf(tmp, "%d:%02d", total_slept_time/60, total_slept_time%60);
  vso.setAttribute("TimeSleptString", QString(tmp));
}

void EzxAPM::startPMU()
{
  ioctl(apm_fd, APM_IOC_STARTPMU, NULL);
}

// CPU power profiles management
void EzxAPM::gearUp()
{
  if (current_profile<n_profiles-1 && (allow_high_perf || current_profile<n_profiles-1-n_profiles_high))
    setPowerProfile(current_profile+1);
}

void EzxAPM::gearDown()
{
  if (current_profile>0 && (allow_low_perf || current_profile>n_profiles_low))
    setPowerProfile(current_profile-1);
}

void EzxAPM::setPowerProfile(int n)
{
  ipm_config cconf;
  int ret = ::ioctl(apm_fd, APM_IOC_GET_IPM_CONFIG, &cconf);
  ret = ::ioctl(apm_fd, APM_IOC_SET_IPM_CONFIG, &power_profiles[n]);
  qLog(Hardware) << "CPU clock:" << cconf.core_freq/1000 << "MHz ->" << power_profiles[n].core_freq/1000 << "MHz";

  // Publish actual clock to ValueSpace
  ret = ::ioctl(apm_fd, APM_IOC_GET_IPM_CONFIG, &cconf);
  vso.setAttribute("CPU/Clock", cconf.core_freq/1000);


  current_profile = n;
  load_balance = 0; // reset it to avoid confusing logic
}

void EzxAPM::adjustPower(int load)
{
  switch (power_status.wallStatus())
  {
    case QPowerStatus::Available: // Charger is plugged, we may use as much power as we can
      if (allow_high_perf)
      {
        if (current_profile<n_profiles-1)
          setPowerProfile(n_profiles-1);
      }
      else
      {
        if (current_profile<n_profiles_high-1)
          setPowerProfile(n_profiles-n_profiles_high-1);
      }
      break;

    case QPowerStatus::NotAvailable: // No charger available, so think before switching
    case QPowerStatus::NoWallSource:
      if (load<0)
        setPowerProfile(n_profiles_low);

      else if (load > GEAR_UP_THRESHOLD)
        load_balance++;
      else if (load < GEAR_DOWN_THRESHOLD)
        load_balance--;

      if (load_balance > MAX_LOAD_BALANCE)
        gearUp();
      else if (load_balance < -MAX_LOAD_BALANCE)
        gearDown();
  }
}

// SystemSuspendHandler

bool EzxAPM::canSuspend() const
{
  qLog(PowerManagement)<<"EzxAPM::canSuspend()";
  if (QValueSpaceItem("/Hardware/EZX/Cable/Connected").value().toBool())
  {
    qLog(PowerManagement)<<"EzxAPM::canSuspend(): cable detected, refusing";
    return false;
  }

  return true;
}

bool EzxAPM::suspend()
{
  qLog(PowerManagement)<<"EzxAPM::suspend()";
  qLog(PowerManagement)<<"Starting asychronous suspend";

  sleep_allowed = true;

  return false; // enter asynchronous mode
}

bool EzxAPM::wake()
{
  qLog(PowerManagement)<<"EzxAPM::wake()";
  sleep_allowed = false;
  QWSServer::instance()->refresh();

#ifdef Q_WS_QWS
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  /*QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
  e << -3; // Force on
  e.send();*/
  QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
#endif
  return true;
}

void EzxAPM::setTSState(bool st)
{
  /* // Cannot be compiled as c++ because of sizeof(void)
  qLog(PowerManagement) << "EzxAPM::setTSState" << st;
  int power_fd = open("/dev/power_ic", O_RDWR);
  if (power_fd>=0)
  {
    if (st) // Turn on
    {
      int ret = ioctl(power_fd, POWER_IC_IOCTL_TS_ENABLE, NULL);
      qLog(PowerManagement) << "POWER_IC_IOCTL_TS_ENABLE:" << ret;
    }
    else // Turn off
    {
      int ret = ioctl(power_fd, POWER_IC_IOCTL_TS_DISABLE);
      qLog(PowerManagement) << "POWER_IC_IOCTL_TS_DISABLE:" << ret;
    }

    close(power_fd);
  }
  else
  {
    qLog(PowerManagement) << "Failed to open power_ic";
  }
  */
}

