#ifndef _APM_H_
#define _APM_H_

#include <QObject>
#include <QValueSpaceObject>
#include <QPowerStatus>
#include <systemsuspend.h>
#include <QtopiaAbstractService>
#include <QSet>
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

    void lockHighPerformance(const QString &id);
    void releaseHighPerformance(const QString &id);
    void requestHighPerformance(const QString &reason);

    QSet<QString> perf_locks;

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
    friend class EzxAPMService;
};

class EzxAPMService: public QtopiaAbstractService
{
  Q_OBJECT
  public slots:
    // Only one lock is available: a single call to release() releases the lock
    void lockHighPerformance(const QString &id); // Force the CPU to enter High Performance mode
    void releaseHighPerformance(const QString &id); // Let the CPU leave High Performance mode

    void requestHighPerformance(const QString &reason); // Make the CPU enter High Performance mode without keeping it
  private:
    EzxAPMService(EzxAPM *task);
    virtual ~EzxAPMService();

    EzxAPM *m_task;

  friend class EzxAPM;
};

#endif // _APM_H_
