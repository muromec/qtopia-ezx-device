#ifndef _EZXHARDWARE_H_
#define _EZXHARDWARE_H_

#include <linux/power_ic.h>

#include <QObject>
#include <QValueSpaceObject>
#include <QFileMonitor>
#include <QPowerSourceProvider>
#include <QSocketNotifier>

class EzxHardware: public QObject
{
  Q_OBJECT
  public:
    EzxHardware(QObject *parent = NULL);
    virtual ~EzxHardware();

    void plugAccesory(int type);
    void unplugAccesory(int type);
    void checkAccesories();
  private slots:
    void accyEvent(int);
    void chargeUpdated();
  private:
    int batteryPercent();

    int accy_fd;
    int power_fd;
    bool charging_state;
    POWER_IC_ATOD_REQUEST_BATT_AND_CURR_T power_info;

    QValueSpaceObject vsoPortableHandsfree;
    QValueSpaceObject vsoEzxHardware;

    QPowerSourceProvider charger;
    QPowerSourceProvider battery;

    QFileMonitor fileMon;
    QSocketNotifier *accy_noti;
};

#endif // _EZXHARDWARE_H_
