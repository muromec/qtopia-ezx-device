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
#define SUPPLY "/sys/class/power_supply/"
#define REGULATOR "/sys/class/regulator/regulator.1/"

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


}

EzxHardware::~EzxHardware()
{
}

void EzxHardware::chargeUpdated()
{
  
  int charge_raw = batteryRaw();
  int charge_percent = 90;//batteryPercent(charge_raw);
  vsoEzxHardware.setAttribute("Battery/Raw", charge_raw);
  qLog(Hardware) << "Charge: raw =" << charge_raw << "; percent =" << charge_percent;

  battery.setCharge(charge_percent);
  battery.setCharging(regulator());

  vsoEzxHardware.setAttribute("Cable/Connected", cable() );

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

  QStringList cableNames;
  cableNames << "ac" << "usb";

  for (int i = 0; i < cableNames.size(); ++i) {

    QString cableName = QString(SUPPLY "%1/online")
      .arg(cableNames.at(i));

    QFile cableFile;
    cableFile.setFileName(cableName);

    QString cableState;

    if(!cableFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning()<<"cant open cable state file";
    } else {
      QTextStream in(&cableFile);
      in >> cableState;
      cableFile.close();

    }

    if (cableState.toInt())
      return true;

  }

  return false;

}

bool EzxHardware::regulator() {

  QFile regulatorFile;
  regulatorFile.setFileName(REGULATOR "state");

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
  return (raw - 490 ) * 10 / 22;
}

void EzxHardware::ipcEvent(const QString &msg, const QByteArray &arg)
{

  if (msg == "plug()") {
    vsoPortableHandsfree.setAttribute("Present", QString(arg).toInt());
  }
}

