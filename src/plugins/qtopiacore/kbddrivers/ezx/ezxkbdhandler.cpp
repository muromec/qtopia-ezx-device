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
static unsigned int lastKey = -1;

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

    bool repeate = false;
    bool releaseOnly = false;

    switch (driverKeyCode)
    {
        
        // Navigation+
        case 0x1c: qtKeyCode = Qt::Key_Call;   repeate = true; break;
        case 0x1e: qtKeyCode = Qt::Key_Hangup; repeate = true; break;

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

        // headphone 
        case 0x28: qtKeyCode = Qt::Key_F28; repeate = true; releaseOnly = true; break;

        case 0x2e: qtKeyCode = Qt::Key_F29; break;
        
        case 0x17: qtKeyCode = Qt::Key_OpenUrl; break;
        case 0x2d:
        case 0x2f: qtKeyCode = Qt::Key_LaunchMedia;
        case 0x2c: qtKeyCode = Qt::Key_MediaPlay; break;
        case 0x2a: qtKeyCode = Qt::Key_MediaPrevious; break;
        case 0x2b: qtKeyCode = Qt::Key_MediaNext ; break;

        // numpad
        case 0x00: qtKeyCode = Qt::Key_0; unicode  = 0x30; break;
        case 0x01: qtKeyCode = Qt::Key_1; unicode  = 0x31; break;
        case 0x02: qtKeyCode = Qt::Key_2; unicode  = 0x32; break;
        case 0x03: qtKeyCode = Qt::Key_3; unicode  = 0x33; break;
        case 0x04: qtKeyCode = Qt::Key_4; unicode  = 0x34; break;
        case 0x05: qtKeyCode = Qt::Key_5; unicode  = 0x35; break;
        case 0x06: qtKeyCode = Qt::Key_6; unicode  = 0x36; break;
        case 0x07: qtKeyCode = Qt::Key_7; unicode  = 0x37; break;
        case 0x08: qtKeyCode = Qt::Key_8; unicode  = 0x38; break;
        case 0x09: qtKeyCode = Qt::Key_9; unicode  = 0x39; break;

        case 0x0b: qtKeyCode = Qt::Key_Asterisk;   unicode  = 0x2A; break;
        case 0x0a: qtKeyCode = Qt::Key_NumberSign; unicode  = 0x23; break;

        case 0x13: qtKeyCode = Qt::Key_Back; break;
        case 0x12: qtKeyCode = Qt::Key_Context1; break;

        case 0x16: qtKeyCode = Qt::Key_Clear; break;


        // unknown
        printf("unknown key: %x control: %d\n",driverKeyCode,controlCode);

    }


    // key is pressed
    if (controlCode) {
      if (repeate) {
        // autorepeate handled by qtopia, start it
        processKeyEvent(unicode, qtKeyCode, modifiers, true, false);
        beginAutoRepeat(unicode, qtKeyCode, modifiers);
        // save keycode in variable
        lastKey = qtKeyCode;
      } else if  (qtKeyCode != lastKey) {
        // autorepeate handled by kernel. press
        processKeyEvent(unicode, qtKeyCode, modifiers, true, false);
        lastKey = qtKeyCode;
      } else {
        // autorepeate handled by kernel. hold
        processKeyEvent(unicode, qtKeyCode, modifiers, true, true);
      }
    } else {
      if (releaseOnly && (qtKeyCode != lastKey) ) 
         // key sends only release event
        processKeyEvent(unicode, qtKeyCode, modifiers, true, false);

      // stop autorepeate, send release event and erase saved code
      endAutoRepeat();
      processKeyEvent(unicode, qtKeyCode, modifiers, false, false);

      lastKey = -1;
    }

}


