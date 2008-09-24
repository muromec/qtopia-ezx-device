/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#ifdef QT_QWS_EZX

#include "ezxhardware.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QLabel>
#include <QDesktopWidget>
#include <QProcess>
#include <QtopiaIpcAdaptor>

#include <qcontentset.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>

#include <qbootsourceaccessory.h>
#include <qtopiaipcenvelope.h>

#include <qtopiaserverapplication.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/moto_accy.h>

#include <sys/ioctl.h>

QTOPIA_TASK(EzxHardware, EzxHardware);

EzxHardware::EzxHardware()
    : vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
      vsoUsbCable("/Hardware/UsbGadget"),
      vsoEzxHardware("/Hardware/EZX")
{

    vsoPortableHandsfree.setAttribute("Present", false);
    vsoUsbCable.setAttribute("Present", false);

    vsoPortableHandsfree.sync();
    vsoUsbCable.sync();


    unsigned long int connected_cables;

    accyFd = ::open("/dev/accy", O_RDWR);
    if (accyFd>=0) {
      qLog(Hardware)<<"Opened /dev/accy";
        accyNotify = new QSocketNotifier(accyFd, QSocketNotifier::Read, this);
        connect(accyNotify, SIGNAL(activated(int)), 
            this, SLOT(plug()));

    } else {
      qLog(Hardware)<<"Cannot open /dev/accy";
    }



    ioctl(accyFd, MOTO_ACCY_IOCTL_GET_ALL_DEVICES, &connected_cables);

    while (connected_cables){
      int n = generic_ffs(connected_cables) - 1;

      if ((n>13) && (n<20)) { // headset
        headphonesInserted(n);
      } else if (n==11) {      // usb
        cableConnected(true);
      }

      connected_cables &= ~(1 << (n));
    }


// Handle Audio State Changes
    //audioMgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);


    findHardwareVersion();
}

EzxHardware::~EzxHardware()
{
}
void EzxHardware::plug() 
{
  read(accyFd,&accyEvent,40);

  if (accyEvent[1]) { //plug
    qLog(Hardware) << "plug cable";

    if (accyEvent[0]>14) // headphone
      headphonesInserted(accyEvent[0]);
    else if  (accyEvent[0]=11) // usb
      cableConnected(true);
  }  else { // unplug
    qLog(Hardware) << "unplug cable"; 

    if (accyEvent[0]>14) // headphone
      headphonesInserted(0);
    else if  (accyEvent[0]=11) // usb
      cableConnected(false);

  }


}
// TODO: find phone model
void EzxHardware::findHardwareVersion()
{
    vsoEzxHardware.setAttribute("Device", "EZX");
    vsoEzxHardware.sync();
}

void EzxHardware::headphonesInserted(int  type)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << type;
    vsoPortableHandsfree.setAttribute("Present", type);
    vsoPortableHandsfree.sync();
}

void EzxHardware::cableConnected(bool b)
{
    qLog(Hardware)<< __PRETTY_FUNCTION__ << b;
    vsoUsbCable.setAttribute("cableConnected", b);
    vsoUsbCable.sync();
}

void EzxHardware::shutdownRequested()
{
    qLog(PowerManagement)<< __PRETTY_FUNCTION__;

    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

#endif // QT_QWS_EZX

