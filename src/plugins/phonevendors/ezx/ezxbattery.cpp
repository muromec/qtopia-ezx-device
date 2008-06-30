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
#include <unistd.h>

#include "stdio.h"
#include <linux/power_ic.h>
#include <linux/moto_accy.h>
#include <sys/ioctl.h>
#include <fcntl.h>


// move this to header
typedef struct 
{
  int time;
  int timeout;
  int direction;
  int bat;
  int cur;
} BAT_INFO;

typedef struct
{
  int channel;
  int result;
} CHANNEL_REQUEST;


EzxBattery::EzxBattery(QObject *parent)
: QObject(parent)
{

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT (updateMotStatus() ));


    charger = new QPowerSourceProvider(QPowerSource::Wall, "EzxCharger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "DefaultBattery", this);
}


void EzxBattery::updateMotStatus()
{


    BAT_INFO info;
    
    int fd,ret;
    unsigned long int devs;
    bool chargerState = false;

    unsigned char battaryVoltage = 0;
    signed short int battaryCurrent = 0;


    
    // battary info
    fd = open("/dev/power_ic",O_RDWR);
    info.time = 0;
    ret = ioctl (fd,POWER_IC_IOCTL_ATOD_BATT_AND_CURR,&info);

    battaryVoltage = info.bat >> 2;
    battaryCurrent = info.cur;
    
    // FIXME
    battery->setCharge(info.bat * 100 / 700);
    printf("bat: %d, %d\n", info.bat * 100 / 700, info.bat);
    printf("voltage: %d, curr: %d\n", battaryVoltage, battaryCurrent);
    close(fd);

    // cable
    fd = open("/dev/accy", O_RDWR);  
    ioctl(fd, MOTO_ACCY_IOCTL_GET_ALL_DEVICES, &devs);
    close(fd);

    // connected cables 3-11 are chargers
    while (devs){
        ret = generic_ffs((devs)) - 1;

        if ((ret > 2) and (ret < 12))
          chargerState = true;

        devs &= ~(1 << (ret));
    }


    battery->setAvailability(QPowerSource::Available); // FIXME
    
    if (chargerState) 
            charger->setAvailability(QPowerSource::Available);
    else
            charger->setAvailability(QPowerSource::Failed);

    if (charger->availability() == QPowerSource::Available
            && battery->availability() == QPowerSource::Available
            && info.bat < 700) {
        battery->setCharging(true);
    }
    else {
        battery->setCharging(false);
    }


}


