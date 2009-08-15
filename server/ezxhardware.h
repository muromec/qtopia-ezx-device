#ifndef _EZXHARDWARE_H_
#define _EZXHARDWARE_H_

#include <QObject>
#include <QValueSpaceObject>
#include <QTimer>
#include <QPowerSourceProvider>
#include <QtopiaChannel>

class EzxHardware: public QObject
{
  Q_OBJECT
  public:
    EzxHardware(QObject *parent = NULL);
    virtual ~EzxHardware();

  private slots:
    void chargeUpdated();
    void ipcEvent(const QString &msg, const QByteArray &arg);

  private:
    int batteryRaw();
    int batteryPercent(int raw);
    bool cable();
    bool regulator();

    int regulator_num;
    QValueSpaceObject vsoPortableHandsfree;
    QValueSpaceObject vsoEzxHardware;
    QtopiaChannel *ipc;

    QPowerSourceProvider charger;
    QPowerSourceProvider battery;

    QTimer *btimer;
};

#endif // _EZXHARDWARE_H_
