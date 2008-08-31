/****************************************************************************
**
** Copyright (C) 2008 Roman Yepishev. All rights reserved.
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

#include "ezxbattery.h"

#include <QFileMonitor>

#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/socket.h> 
#include <sys/un.h>

#include <fcntl.h>
#include <signal.h>

#include <motod_ipc.h>


EzxBattery::EzxBattery(QObject *parent)
: QObject(parent)
{

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT (updateMotStatus() ));


    charger = new QPowerSourceProvider(QPowerSource::Wall, "Charger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "DefaultBattery", this);


}

static inline void motod_short_request(int code,  motod_request *req ) {


  struct sockaddr_un motosock; 
  memset(&motosock,0,sizeof(motosock));
  motosock.sun_family = PF_LOCAL; 

  strncpy( 
   motosock.sun_path,
   "/var/run/motod.socket", 
   sizeof(motosock.sun_path) - 1 
  ); 


  int fd = socket ( PF_LOCAL, SOCK_STREAM, 0 );

  connect(fd,(struct sockaddr*)&motosock, SUN_LEN(&motosock) ); 

  req->id = code;
  
  write(fd,req,sizeof(req)) ;
  read(fd,req,40);
  close(fd);

}

void EzxBattery::updateMotStatus()
{

  motod_request req; 

  // battery info
  motod_short_request(MOTOD_IPC_SUMMARY,&req);

  battery->setCharge( req.data[0] );

  if (req.data[0] == 1)
     battery->setAvailability(QPowerSource::NotAvailable);
  else
     battery->setAvailability(QPowerSource::Available);

  // charging state

  if (req.data[1]) 
          charger->setAvailability(QPowerSource::Available);
  else
          charger->setAvailability(QPowerSource::Failed);
  
  battery->setCharging( req.data[2] );
}


