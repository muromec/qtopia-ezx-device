#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define CONFIG_ARCH_EZX
#include <linux/moto_accy.h>

#include <QPowerSourceProvider>
#include <QSocketNotifier>
#include <QFileMonitor>
#include <qtopiaserverapplication.h>
#include <qtopialog.h>

#include "ezxhardware.h"

QTOPIA_TASK(EzxHardware, EzxHardware);

EzxHardware::EzxHardware(QObject *parent)
  : QObject(parent),
    fileMon("/proc/apm", QFileMonitor::Auto, this),
    battery(QPowerSource::Battery, "DefaultBattery", this),
    charger(QPowerSource::Wall, "Charger", this),
    vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
    vsoEzxHardware("/Hardware/EZX")
{

  power_fd = open("/dev/power_ic", O_RDWR);
  if (power_fd < 0)
    qLog(Hardware) << "Error: could not open power ic";

  accy_fd = open("/dev/accy", O_RDWR);
  if (accy_fd>=0)
  {
    checkAccesories();
    accy_noti = new QSocketNotifier(accy_fd, QSocketNotifier::Read, this);
    connect(accy_noti, SIGNAL(activated(int)), SLOT(accyEvent(int)));
  }
  else
    qLog(Hardware) << "Error: could not open accy";

  connect(&fileMon, SIGNAL(fileChanged(QString)), SLOT(chargeUpdated()));

  vsoEzxHardware.setAttribute("Device", "EZX");
  charging_state = false;

}

EzxHardware::~EzxHardware()
{
  if (accy_fd>=0)
    close(accy_fd);
}

void EzxHardware::accyEvent(int)
{
  unsigned long int event;
  read(accy_fd, &event, 4);
  qLog(Hardware) << "EzxAccessory:" << (0x7fffffff&event) << ((event & 0x80000000)?"plugged":"unplugged");
  if (event & 0x80000000)
    plugAccesory  (0x7fffffff & event);
  else
    unplugAccesory(0x7fffffff & event);
}

void EzxHardware::plugAccesory(int type)
{
  switch (type)
  {
    case MOTO_ACCY_TYPE_CARKIT_MID:
    case MOTO_ACCY_TYPE_CHARGER_MID_MPX:
    case MOTO_ACCY_TYPE_CHARGER_MID:
    case MOTO_ACCY_TYPE_CABLE_USB:
      charger.setAvailability(QPowerSource::Available);
      charging_state = true;
      vsoEzxHardware.setAttribute("Cable/Connected", true);
      break;

    // headsets handled by script and qtopia
    case MOTO_ACCY_TYPE_HEADSET_MONO:
    case MOTO_ACCY_TYPE_HEADSET_STEREO:
    case MOTO_ACCY_TYPE_HEADSET_EMU_MONO:
    case MOTO_ACCY_TYPE_HEADSET_EMU_STEREO:
    case MOTO_ACCY_TYPE_3MM5_HEADSET_STEREO:
    case MOTO_ACCY_TYPE_3MM5_HEADSET_STEREO_MIC:
      vsoPortableHandsfree.setAttribute("Present", type);
      break;
    //default:
        //dbg("attached cable %d",type);
    }
}

void EzxHardware::unplugAccesory(int type)
{
  switch (type)
  {
    case MOTO_ACCY_TYPE_CARKIT_MID:
    case MOTO_ACCY_TYPE_CHARGER_MID_MPX:
    case MOTO_ACCY_TYPE_CHARGER_MID:
    case MOTO_ACCY_TYPE_CABLE_USB:
      charger.setAvailability(QPowerSource::NotAvailable);
      battery.setCharging(false);
      charging_state = false;
      vsoEzxHardware.setAttribute("Cable/Connected", false);
      break;

    case MOTO_ACCY_TYPE_HEADSET_MONO:
    case MOTO_ACCY_TYPE_HEADSET_STEREO:
    case MOTO_ACCY_TYPE_HEADSET_EMU_MONO:
    case MOTO_ACCY_TYPE_HEADSET_EMU_STEREO:
    case MOTO_ACCY_TYPE_3MM5_HEADSET_STEREO:
    case MOTO_ACCY_TYPE_3MM5_HEADSET_STEREO_MIC:
      vsoPortableHandsfree.setAttribute("Present", 0);
      break;

    //default:
       // dbg("detached cable %d",type);
    }
}

void EzxHardware::checkAccesories()
{
  unsigned long int accys;
  ioctl(accy_fd, MOTO_ACCY_IOCTL_GET_ALL_DEVICES, &accys);
  while (accys)
  {
    int n = generic_ffs(accys) - 1;
    plugAccesory(n);

    accys &= ~(1 << (n));
  }
}

void EzxHardware::chargeUpdated()
{
  power_info.timing = POWER_IC_ATOD_TIMING_IMMEDIATE;
  ioctl(power_fd, POWER_IC_IOCTL_ATOD_BATT_AND_CURR, &power_info);

  int charge_percent = batteryPercent();
  vsoEzxHardware.setAttribute("Battery/Raw", power_info.batt_result);
  vsoEzxHardware.setAttribute("Current/Raw", power_info.curr_result);
  //qLog(Hardware) << "Charge: raw =" << charge_raw << "; percent =" << charge_percent;
  battery.setCharge(charge_percent);
  battery.setCharging( charging_state && (power_info.curr_result > 100) );
}

int EzxHardware::batteryPercent()
{
  return (power_info.batt_result - 490 ) * 10 / 22;
}

