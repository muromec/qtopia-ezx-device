#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <QPowerSourceProvider>
#include <QSocketNotifier>
#include <QFileMonitor>
#include <qtopiaserverapplication.h>
#include <qtopialog.h>

#include "ezxhardware.h"
#define ADC "/sys/devices/platform/pxa2xx-spi.1/spi1.0/"
#define CABLE "/sys/bus/platform/devices/eoc_charger/cable"
#define REGULATOR "/sys/class/regulator/regulator.%1/"

QTOPIA_TASK(EzxHardware, EzxHardware);

EzxHardware::EzxHardware(QObject *parent)
  : QObject(parent),
    battery(QPowerSource::Battery, "DefaultBattery", this),
    charger(QPowerSource::Wall, "Charger", this),
    vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
    vsoEzxHardware("/Hardware/EZX")
{

  btimer = new QTimer(this);
  connect(btimer, SIGNAL(timeout()), this, SLOT(chargeUpdated()));
  btimer->start(3000);

  vsoEzxHardware.setAttribute("Device", "EZX");

  ipc = new QtopiaChannel("QPE/Jack", this);
  connect(
    ipc,  SIGNAL(received(QString,QByteArray)),
    this, SLOT(ipcEvent(QString,QByteArray))
  );

  regulator_num = -1;
  int n;

  for (n=0;regulator_num == -1;n++) {

    QFile regulator_file;
    QString name = QString(REGULATOR"name").arg(n);

    qWarning() << name;

    regulator_file.setFileName(name);

    if(! regulator_file.open(QIODevice::ReadOnly | QIODevice::Text))
      break;

    QTextStream in(&regulator_file);
    QString reg_name;
    in >> reg_name;

    qWarning() << reg_name;

    if (reg_name == "eoc_charger")
      regulator_num = n;

    regulator_file.close();

  }

}

EzxHardware::~EzxHardware()
{
}

void EzxHardware::chargeUpdated()
{
  
  int charge_raw = batteryRaw();
  int charge_percent = batteryPercent(charge_raw);
  vsoEzxHardware.setAttribute("Battery/Raw", charge_raw);
  qLog(Hardware) << "Charge: raw =" << charge_raw << "; percent =" << charge_percent;

  bool cablePlugged = cable();

  if (cablePlugged)
    charger.setAvailability(QPowerSource::Available);
  else
    charger.setAvailability(QPowerSource::NotAvailable);

  battery.setCharge(charge_percent);
  battery.setCharging(cablePlugged && regulator() );

  vsoEzxHardware.setAttribute("Cable/Connected", cablePlugged );

  btimer->start(3000);
 
}

int EzxHardware::batteryRaw()
{

  QString strvalue;
  QFile batt;
  batt.setFileName("/sys/class/power_supply/main-battery/voltage_now");

  if(!batt.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"voltage file not opened";
  } else {
        QTextStream in(&batt);
        in >> strvalue;
        batt.close();
  }
  return  strvalue.toInt();

  
}

bool EzxHardware::cable() {

  QFile cableFile;
  cableFile.setFileName(CABLE);

  QString cableState;

  if(!cableFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning()<<"cant open cable state file";
  } else {
    QTextStream in(&cableFile);
    in >> cableState;
    cableFile.close();

  }

  return (bool)cableState.toInt();

}

bool EzxHardware::regulator() {

  QFile regulatorFile;
  QString filename = QString(REGULATOR "state").arg(regulator_num);
  regulatorFile.setFileName(filename);

  QString regulatorState;

  if(!regulatorFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning()<<"cant open regulator file";
  } else {
    QTextStream in(&regulatorFile);
    in >> regulatorState;
    regulatorFile.close();

    return (regulatorState == "enabled");

  }

  return false;


}

int EzxHardware::batteryPercent(int raw)
{
  return (raw-2000)*100/2100;
}

void EzxHardware::ipcEvent(const QString &msg, const QByteArray &arg)
{

  if (msg == "plug()") {
    vsoPortableHandsfree.setAttribute("Present", QString(arg).toInt());
  }
}

