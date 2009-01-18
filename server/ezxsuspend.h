#ifndef _EZXSUSPEND_H_
#define _EZXSUSPEND_H_

#include "systemsuspend.h"

class EzxSuspend: public SystemSuspendHandler
{
  Q_OBJECT
  public:
    EzxSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
  private slots:
    void canSleep();
  private:
    bool sleep_requested;
};

#endif // _EZXSUSPEND_H_