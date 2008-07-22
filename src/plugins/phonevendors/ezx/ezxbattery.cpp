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

static inline void set_usb_mode(int mode)
{

    int usbc = open("/dev/motusbd", O_RDWR);
    ioctl(usbc,MOTUSBD_IOCTL_SET_USBMODE,&mode);
    close(usbc);

}

static inline void set_charge_mode(int fd, int mode)
{

    ioctl (fd, POWER_IC_IOCTL_CHARGER_SET_CHARGE_CURRENT, mode); 

}


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
    charging = false;
    usb = false;
}


void EzxBattery::updateMotStatus()
{


    // TODO: 
    // * check if battery connected
    // * check charging status, not connected cables
    // * replace emud? 

    BAT_INFO info;
    
    int power,accy,ret;
    unsigned long int devs;
    bool chargerState = false;

    unsigned char batteryVoltage = 0;
    signed short int batteryCurrent = 0;

    
    // battery info
    power = open("/dev/power_ic",O_RDWR);
    info.time = 0;
    ret = ioctl (power,POWER_IC_IOCTL_ATOD_BATT_AND_CURR,&info);

    batteryVoltage = info.bat >> 2;
    batteryCurrent = info.cur;
    
    // FIXME
    battery->setCharge( (int) ((info.bat - 310) / 4)  );

    // get attached cables list
    accy = open("/dev/accy", O_RDWR);  
    ioctl(accy, MOTO_ACCY_IOCTL_GET_ALL_DEVICES, &devs);

    // if no cable connected - no charging

    // need to uninitialise usb and charging
    if (devs == 0)  {
      if (charging) { 
        set_charge_mode(power,0);
        charging = false;
      }
      if (usb) {
        set_usb_mode(0);
        usb = false;
      }
      chargerState = false;
    }


    while (devs){
        ret = generic_ffs((devs)) - 1;


        switch (ret) {
          case 1:
            printf("charger error!\n");
            break;

          // charger attached
          case 3:  
            chargerState = true;
            if  (info.bat < 710) {

              if (! charging ) {

                set_charge_mode(power,13);
                charging = true;
              }
            } else {
                set_charge_mode(power,0);
                charging = false;
            }

            break;

          case 11:
            // set mode to usbnet
            if (! usb ) {
              set_usb_mode(1) ; // FIXME: usbnet hardcoded
              usb = true;
            }
            break;
          default:

            if (usb) {
              set_usb_mode(0);
              usb = false;
            }

            if (charging) {

              set_charge_mode(power,0);
              charging = false;
            }
            chargerState = false;

        }

        devs &= ~(1 << (ret));
    }


    // with no battary it whill be 1
    if (info.bat == 1)
       battery->setAvailability(QPowerSource::NotAvailable);
    else
       battery->setAvailability(QPowerSource::Available); 

    if (chargerState) 
            charger->setAvailability(QPowerSource::Available);
    else
            charger->setAvailability(QPowerSource::Failed);

    battery->setCharging(charging);
    
    close(power);
    close(accy);


}


