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

#include <sys/ioctl.h>

QTOPIA_TASK(EzxHardware, EzxHardware);

EzxHardware::EzxHardware()
    : vsoPortableHandsfree("/Hardware/Accessories/PortableHandsfree"),
      vsoUsbCable("/Hardware/UsbGadget"),
      vsoEzxHardware("/Hardware/EZX")
{
    adaptor = new QtopiaIpcAdaptor("QPE/EzxHardware");

    qLog(Hardware) << "ezxhardware";

    vsoPortableHandsfree.setAttribute("Present", false);
    vsoPortableHandsfree.sync();

// Handle Audio State Changes
    audioMgr = new QtopiaIpcAdaptor("QPE/AudioStateManager", this);


    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(headphonesInserted(int)),
                              this, SLOT(headphonesInserted(int)));

    QtopiaIpcAdaptor::connect(adaptor, MESSAGE(cableConnected(bool)),
                              this, SLOT(cableConnected(bool)));
    findHardwareVersion();
}

EzxHardware::~EzxHardware()
{
}

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
    if (type) {
        QByteArray mode("Headphone");
        audioMgr->send("setProfile(QByteArray)", mode);
    } else {
        QByteArray mode("MediaSpeaker");
        audioMgr->send("setProfile(QByteArray)", mode);
    }


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

    QFile powerFile;
    QFile btPower;

    QtopiaServerApplication::instance()->shutdown(QtopiaServerApplication::ShutdownSystem);
}

#endif // QT_QWS_EZX

