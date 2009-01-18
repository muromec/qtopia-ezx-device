#ifndef _APM_H_
#define _APM_H_

#include <QObject>
class QSocketNotifier;

class EzxAPM: public QObject
{
  Q_OBJECT
  public:
    enum PowerPolicy
    {
      Charger,
      Battery
    };
    EzxAPM(QObject *parent = NULL);
    virtual ~EzxAPM();

    void readyRead();

    void setPowerPolicy(PowerPolicy new_policy);
    void adjustPower(int load = -1);

    void gearUp();
    void gearDown();

    void sleep();
    void startPMU();

    int timeSlept() const;
  private slots:
    void apmEvent(int fd);
  signals:
    void canSleep();
  private:
    void setPowerProfile(int n);

    QSocketNotifier *apm_noti;

    int apm_fd;

    int current_profile;
    PowerPolicy policy;
    int load_balance;

    int total_slept_time;

    int cpu_load;

    static const struct ipm_config power_profiles[];
    static const int n_profiles;
};

#endif // _APM_H_
