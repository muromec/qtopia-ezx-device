#ifndef _EZXHARDWARE_H_
#define _EZXHARDWARE_H_

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
    int batteryRaw();
    int batteryPercent(int raw);

    int accy_fd;

    QValueSpaceObject vsoPortableHandsfree;
    QValueSpaceObject vsoEzxHardware;

    QPowerSourceProvider charger;
    QPowerSourceProvider battery;

    QFileMonitor fileMon;
    QSocketNotifier *accy_noti;
};

#endif // _EZXHARDWARE_H_
