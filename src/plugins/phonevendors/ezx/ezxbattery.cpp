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
#include <linux/ezxusbd.h>
#include <sys/ioctl.h>
#include <fcntl.h>


EzxBattery::EzxBattery(QObject *parent)
: QObject(parent)
{

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT (updateMotStatus() ));


    charger = new QPowerSourceProvider(QPowerSource::Wall, "EzxCharger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "DefaultBattery", this);
    charging = false;
    usb = false;
}


void EzxBattery::updateMotStatus()
{


    POWER_IC_ATOD_REQUEST_BATT_AND_CURR_T info;
    
    int power,ret;
    bool chargerState = false;

    
    // battery info
    info.timing = POWER_IC_ATOD_TIMING_IMMEDIATE;
    power = open("/dev/power_ic",O_RDWR);
    ret = ioctl (power,POWER_IC_IOCTL_ATOD_BATT_AND_CURR,&info);
    close(power);

    
    // FIXME
    battery->setCharge( (int) ((info.batt_result - 310) / 4)  );

    

    // FIXME
    chargerState = ( !access("/tmp/charging",F_OK) );

    // with no battary it whill be 1
    if (info.batt_result == 1)
       battery->setAvailability(QPowerSource::NotAvailable);
    else
       battery->setAvailability(QPowerSource::Available); 

    // FIXME
    if (chargerState) 
            charger->setAvailability(QPowerSource::Available);
    else
            charger->setAvailability(QPowerSource::Failed);

    battery->setCharging(chargerState );

    


}


