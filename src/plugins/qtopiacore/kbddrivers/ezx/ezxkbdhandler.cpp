/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
** Copyright (C) 2008 Ilya Petrov <ilya.muromec@gmail.com>
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

#include "ezxkbdhandler.h"

#include <QScreen>
#include <QSocketNotifier>

#include "qscreen_qws.h"
#include "qwindowsystem_qws.h"
#include "qapplication.h"
#include "qnamespace.h"
#include <qtopialog.h>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/vt.h>
#include <sys/kd.h>

#define VTACQSIG SIGUSR1
#define VTRELSIG SIGUSR2

static int vtQws = 0;

EZXKbdHandler::EZXKbdHandler()
{
    qLog(Input) << "Loaded EZX keypad plugin";

    setObjectName( "EZX Keypad Handler" );

    kbdFD = ::open("/dev/keypad0", O_RDONLY | O_NDELAY, 0);
    if (kbdFD >= 0)
    {
        qLog(Input) << "Opened /dev/keypad0 as keypad input";
        m_notify = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(m_notify, SIGNAL(activated(int)), this, SLOT(readKbdData()));
    }
    else
    {
        qWarning("Cannot open /dev/keypad0 for keypad (%s)", strerror(errno));
        return;
    }

    tcgetattr(kbdFD, &origTermData);
    struct termios termdata;
    tcgetattr(kbdFD, &termdata);

    ioctl(kbdFD, KDSKBMODE, K_RAW);

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(kbdFD, TCSANOW, &termdata);

    connect(QApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(handleTtySwitch(int)));
    QApplication::instance()->watchUnixSignal(VTACQSIG, true);
    QApplication::instance()->watchUnixSignal(VTRELSIG, true);

    struct vt_mode vtMode;
    ioctl(kbdFD, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTRELSIG;
    vtMode.acqsig = VTACQSIG;
    ioctl(kbdFD, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(kbdFD, VT_GETSTATE, &vtStat);
    vtQws = vtStat.v_active;

}

EZXKbdHandler::~EZXKbdHandler()
{
    if (kbdFD >= 0)
    {
        ioctl(kbdFD, KDSKBMODE, K_XLATE);
        tcsetattr(kbdFD, TCSANOW, &origTermData);
        ::close(kbdFD);
    }
}

void EZXKbdHandler::handleTtySwitch(int sig)
{
    if (sig == VTACQSIG) {
        if (ioctl(kbdFD, VT_RELDISP, VT_ACKACQ) == 0) {
            qwsServer->enablePainting(true);
            qt_screen->restore();
            qwsServer->resumeMouse();
            qwsServer->refresh();
        }
    } else if (sig == VTRELSIG) {
        qwsServer->enablePainting(false);
        qt_screen->save();
        if(ioctl(kbdFD, VT_RELDISP, 1) == 0) {
            qwsServer->suspendMouse();
        } else {
            qwsServer->enablePainting(true);
        }
    }
}

void EZXKbdHandler::readKbdData()
{
    /*
     * Call:
     * 1c - green
     * 1e - red
     *
     * Joystick:
     * 0e - left
     * 0f - right 
     * 0d - down
     * 0c - up
     *
     * Left side:
     * 25 - +
     * 26 - -
     * 27 - select
     *
     * Right side:
     * 19: camera
     * 20: voice
     *
     * Flip:
     * 1b: flip
    */
    unsigned char           buf[2];
    unsigned int            qtKeyCode;
    Qt::KeyboardModifiers   modifiers = Qt::NoModifier;

    int n = ::read(kbdFD, buf, 2);

    unsigned short driverKeyCode   = (unsigned short)buf[0];
    unsigned short controlCode     = (unsigned short)buf[1];
    unsigned short unicode         = 0xffff;

    switch (driverKeyCode)
    {
        
        // Navigation+
        case 0x1c: qtKeyCode = Qt::Key_Call; break;
        case 0x1e: qtKeyCode = Qt::Key_Hangup; break;

        case 0x0c: qtKeyCode = Qt::Key_Up; break;
        case 0x0d: qtKeyCode = Qt::Key_Down; break;
        case 0x0e: qtKeyCode = Qt::Key_Left; break;
        case 0x0f: qtKeyCode = Qt::Key_Right; break;
        case 0x10: qtKeyCode = Qt::Key_Select; break;

        // Keys on left hand side of device
        case 0x25: qtKeyCode = Qt::Key_VolumeUp; break;
        case 0x26: qtKeyCode = Qt::Key_VolumeDown; break;
        case 0x27: qtKeyCode = Qt::Key_Select; break;

        // Keys on right hand side of device
        case 0x19: qtKeyCode = Qt::Key_F4; break;
        case 0x20: qtKeyCode = Qt::Key_F7; break;   // Key +

        // flip
        case 0x1b: qtKeyCode = Qt::Key_Flip; break;

        case 0x29: qtKeyCode = Qt::Key_F28; break;

        case 0x2e: qtKeyCode = Qt::Key_F29; break;
        
        case 0x17: qtKeyCode = Qt::Key_OpenUrl; break;
        case 0x2f: qtKeyCode = Qt::Key_LaunchMedia;
        case 0x2c: qtKeyCode = Qt::Key_MediaPlay; break;
        case 0x2a: qtKeyCode = Qt::Key_MediaPrevious; break;
        case 0x2b: qtKeyCode = Qt::Key_MediaNext ; break;

        // unknown
        default: printf("unknown key: %x, control: %d\n",driverKeyCode, controlCode);

    }

    processKeyEvent(unicode, qtKeyCode, modifiers, controlCode, false);


}


