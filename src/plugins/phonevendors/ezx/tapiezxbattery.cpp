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

#include "tapiezxbattery.h"
//#include "qtopiaserverapplication.h"

#include <QFileMonitor>

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "tapi.h"
#include "stdio.h"

TapiEzxBattery::TapiEzxBattery(QObject *parent)
: QObject(parent)
{

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT (updateMotStatus() ));


    charger = new QPowerSourceProvider(QPowerSource::Wall, "EzxCharger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "DefaultBattery", this);
}


void TapiEzxBattery::updateMotStatus()
{


    int chargerStatus;

    EZX_BATT_INF batteryInfo; 

    TAPI_BATTERY_GetBatteryChargeInfo(&batteryInfo);
    TAPI_BATTERY_GetChargerConnectionStatus( &chargerStatus );
    
    battery->setCharge(batteryInfo.bcl);
    
    switch(batteryInfo.bcs) {
        case 0: // present, 
        case 1:
            battery->setAvailability(QPowerSource::Available);
            break;
        case 2:
            battery->setAvailability(QPowerSource::NotAvailable);
            break;
        case 3:
            battery->setAvailability(QPowerSource::Failed);

    };

    switch(chargerStatus) {
        case 0:
            charger->setAvailability(QPowerSource::Available);
            break;
        case 1:
            charger->setAvailability(QPowerSource::NotAvailable);
            break;
        default:
            charger->setAvailability(QPowerSource::Failed);
    }

    if (charger->availability() == QPowerSource::Available
            && battery->availability() == QPowerSource::Available
            && batteryInfo.bcl < 100) {
        battery->setCharging(true);
    }
    else {
        battery->setCharging(false);
    }


}


