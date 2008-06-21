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
: QObject(parent), charger(0), battery(0)
{

    charger = new QPowerSourceProvider(QPowerSource::Wall, "TapiEzxCharger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "TapiEzxBattery", this);

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT (updateMotStatus() ));


    adaptor = new QtopiaIpcAdaptor("QPE/TAPI/Battary");


}


void TapiEzxBattery::updateMotStatus()
{


    int chargerStatus;

    EZX_BATT_INF batteryInfo; 

    TAPI_BATTERY_GetBatteryChargeInfo(&batteryInfo);
    TAPI_BATTERY_GetChargerConnectionStatus( &chargerStatus );
    
    adaptor->send(MESSAGE(battary(int)),  batteryInfo.bcs);
    adaptor->send(MESSAGE(battary_charge  (int)), batteryInfo.bcl);
    adaptor->send(MESSAGE(charger (int)), chargerStatus);





}


