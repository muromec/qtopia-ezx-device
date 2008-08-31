/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
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

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <linux/apm_bios.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

#include <motod_ipc.h>
#include <sys/socket.h> 
#include <sys/un.h>



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
  qLog(PowerManagement)<<"EzxSuspend::suspend()";
  printf("suspend\n");


  motod_request req;

  struct sockaddr_un motosock; 
  memset(&motosock,0,sizeof(motosock));
  motosock.sun_family = PF_LOCAL; 

  strncpy( 
   motosock.sun_path,
   "/var/run/motod.socket", 
   sizeof(motosock.sun_path) - 1 
  ); 


  int fd = socket ( PF_LOCAL, SOCK_STREAM, 0 );

  int ret = ::connect(fd,(struct sockaddr*)&motosock, SUN_LEN(&motosock) ); 

  req.id = MOTOD_IPC_SUSPEND;
  req.data[0] = 1;
  
  write(fd,&req,sizeof(req)) ;
  read(fd,&req,40);

  close(fd);


  return true;
}

bool EzxSuspend::wake()
{
    printf("wake\n");
    QWSServer::instance()->refresh();


#ifdef Q_WS_QWS
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QWSServer::screenSaverActivate( false );
    {
        /*QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
        e << -3; // Force on
        e.send();*/
        QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
    }
#endif
    return true;
}
