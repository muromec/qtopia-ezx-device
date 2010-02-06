/****************************************************************************
**
** Copyright (C) 2009 Ilya Petrov
**
** This file may be used under the terms of the GNU General Public License
** versions 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>


#include <QtopiaIpcAdaptor>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceRequest>
#include <QPowerSource>
#include <QProcess>
#include <stdio.h>
#include <stdlib.h>
#include <QDesktopWidget>
#include <QTimer>


#include "systemsuspend.h"

#include <qvaluespace.h>

#include <qwindowsystem_qws.h>

class EzxSuspend : public SystemSuspendHandler
{
public:

  EzxSuspend();
    virtual bool canSuspend() const;
    virtual bool suspend();
    virtual bool wake();
private slots:
};

QTOPIA_DEMAND_TASK(EzxSuspend, EzxSuspend);
QTOPIA_TASK_PROVIDES(EzxSuspend, SystemSuspendHandler);

EzxSuspend::EzxSuspend()
{
}

bool EzxSuspend::canSuspend() const
{
    return true;
}

bool EzxSuspend::suspend()
{
    qWarning() << "suspend";

    int rtc = open("/dev/rtc0",O_RDWR);
    int ret = ioctl(rtc,RTC_UIE_OFF,0);
    close(rtc);

    QFile powerStateFile("/sys/power/state");
    if( !powerStateFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
        qWarning()<<"File not opened";
    } else {
        QTextStream out(&powerStateFile);
        out << "standby";
        powerStateFile.close();
    }


    return true;
}

bool EzxSuspend::wake()
{
    QWSServer::instance()->refresh();

    QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep

   //screenSaverActivate uses QTimer which depends on hwclock update
    //when the device wakes up. The clock update is another event in the
    //Qt event loop (see qeventdispatcher_unix.cpp. We have to call
    //processEvents to give Qt a chance to sync its internal timer with
    //the hw clock
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QWSServer::screenSaverActivate( false );
    {
        QtopiaIpcEnvelope("QPE/Card", "mtabChanged()" ); // might have changed while asleep
        QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
    }

    int rtc = open("/dev/rtc0",O_RDWR);
    int ret = ioctl(rtc,RTC_UIE_ON,0);
    close(rtc);

    return true;
}
