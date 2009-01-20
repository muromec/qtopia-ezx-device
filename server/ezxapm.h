#ifndef _APM_H_
#define _APM_H_

#include <QObject>
#include <QValueSpaceObject>
#include <QPowerStatus>
#include <systemsuspend.h>
class QSocketNotifier;

class EzxAPM: public SystemSuspendHandler
{
  Q_OBJECT
  public:
    EzxAPM(QObject *parent = NULL);
    virtual ~EzxAPM();

    void startPMU();

    // SystemSuspendHandler implementation
    bool canSuspend() const;
    bool suspend();
    bool wake();

  private slots:
    void apmEvent(int fd);

  private:
    void adjustPower(int load = -1);
    void gearUp();
    void gearDown();
    void setPowerProfile(int n);
    void sleep();
    void setTSState(bool st);

    int apm_fd;
    QSocketNotifier *apm_noti;

    int current_profile;
    int load_balance;

    int total_slept_time;
    bool sleep_allowed;

    QPowerStatus power_status;

    bool allow_low_perf;
    bool allow_high_perf;

    QValueSpaceObject vso;

    static const struct ipm_config power_profiles[];
    static const int n_profiles;
    static const int n_profiles_low;
    static const int n_profiles_high;
};

#endif // _APM_H_
