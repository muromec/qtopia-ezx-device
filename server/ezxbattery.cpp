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
#include "qtopiaserverapplication.h"

#include <QPowerSourceProvider>
#include <QTimer>
#include <QFileMonitor>

#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "tapi.h"

EzxBattery::EzxBattery(QObject *parent)
: QObject(parent), charger(0), battery(0)
{
    qWarning()<<"EzxBattery::EzxBattery";

    /* needed to register new tapi client */
    unsigned short int  regMsgId[]   = { 0x0700  };
    signed int               asyncFd = TAPI_CLIENT_Init( regMsgId, 1 );

    if ( (asyncFd == -1) || (asyncFd == 0) )
    {
        qWarning()<<"TAPI client init failed";
        return;
    }
    
    charger = new QPowerSourceProvider(QPowerSource::Wall, "EzxCharger", this);
    battery = new QPowerSourceProvider(QPowerSource::Battery, "EzxBattery", this);

    QFileMonitor *fileMon;
    fileMon = new QFileMonitor( "/proc/apm", QFileMonitor::Auto, this);
    connect(fileMon, SIGNAL( fileChanged ( const QString & )),
            this,SLOT(apmFileChanged(const QString &)));

    startTimer(60 * 1000);
    QTimer::singleShot( 10 * 1000, this, SLOT(updateMotStatus()));
}

void EzxBattery::apmFileChanged(const QString & file)
{
    updateMotStatus();
}

void EzxBattery::updateMotStatus()
{

    int   ret;

    unsigned char batteryMode;
    unsigned char batteryCapacity;

    int batteryStatus;
    int chargerStatus;

    EZX_BATT_INF batteryInfo; 

    ret = TAPI_BATTERY_GetBatteryChargeInfo(&batteryInfo);
    if (ret) {
        qLog(PowerManagement) << "Error getting Battery Charge Info" << ret;
        return;
    }
    ret = TAPI_BATTERY_GetStatus(&batteryStatus);
    if (ret) {
        qLog(PowerManagement) << "Error getting Battery status" << ret;
        return;
    }
    ret = TAPI_BATTERY_GetChargerConnectionStatus( &chargerStatus );
    if (ret) {
        qLog(PowerManagement) << "Error getting Charger status" << ret;
        return;
    }

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

    battery->setCharge(batteryInfo.bcl);

    switch(batteryStatus) {
        case 0:
            battery->setAvailability(QPowerSource::NotAvailable);
            break;
        case 1:
            battery->setAvailability(QPowerSource::Failed);
            break;
        case 2:
            battery->setAvailability(QPowerSource::Available);
            break;
        case 3:
            battery->setAvailability(QPowerSource::Failed);
            break;
        default:
            battery->setAvailability(QPowerSource::Failed);
    }

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

/*! \internal */
void EzxBattery::timerEvent(QTimerEvent *)
{
    updateMotStatus();
}

QTOPIA_TASK(EzxBattery, EzxBattery);
