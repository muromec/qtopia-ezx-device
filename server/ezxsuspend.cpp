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
#include <QtopiaIpcEnvelope>
#include <qtopiaserverapplication.h>

#ifdef Q_WS_QWS
#include <qwindowsystem_qws.h>
#endif

#include "ezxapm.h"
#include "ezxsuspend.h"

QTOPIA_DEMAND_TASK(EzxSuspend, EzxSuspend);
QTOPIA_TASK_PROVIDES(EzxSuspend, SystemSuspendHandler);

EzxSuspend::EzxSuspend()
  : SystemSuspendHandler(), sleep_requested(false)
{
  //EzxAPM *apm_task = qtopiaTask<EzxAPM>();
  EzxAPM *apm_task = qobject_cast<EzxAPM *>(QtopiaServerApplication::qtopiaTask("EzxAPM"));
  if (!apm_task)
    qLog(PowerManagement) << "EzxSuspend: cannot get EzxAPM instance";
  else
  {
    connect(apm_task, SIGNAL(canSleep()), this, SLOT(canSleep()));
    qLog(PowerManagement) << "EzxSuspend: connected to EzxAPM";
  }
}

bool EzxSuspend::canSuspend() const
{
  qLog(PowerManagement)<<"EzxSuspend::canSuspend()";

  return true;
}

bool EzxSuspend::suspend()
{
  qLog(PowerManagement)<<"EzxSuspend::suspend()";
  qLog(PowerManagement)<<"Starting asychronous suspend";

  sleep_requested = true;

  return false; // enter asynchronous mode
}

bool EzxSuspend::wake()
{
  sleep_requested = false;
  QWSServer::instance()->refresh();

#ifdef Q_WS_QWS
  QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
  /*QtopiaServiceRequest e("QtopiaPowerManager", "setBacklight(int)");
  e << -3; // Force on
  e.send();*/
  QtopiaIpcEnvelope("QPE/NetworkState", "updateNetwork()"); //might have changed
#endif
  return true;
}

void EzxSuspend::canSleep()
{
  if (!sleep_requested)
    return; // not asked to sleep
//   EzxAPM *apm_task = qtopiaTask<EzxAPM>();
  EzxAPM *apm_task = qobject_cast<EzxAPM *>(QtopiaServerApplication::qtopiaTask("EzxAPM"));
  if (!apm_task)
    qLog(PowerManagement) << "EzxSuspend: cannot get EzxAPM instance: sleep failed";
  else
  {
    qLog(PowerManagement) << "EzxSuspend: requesting sleep...";
    apm_task->sleep();
    qLog(PowerManagement) << "EzxSuspend: woke up";
  }
}

