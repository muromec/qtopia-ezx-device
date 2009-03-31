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

#include <linux/input.h>

#define VTACQSIG SIGUSR1
#define VTRELSIG SIGUSR2

static int vtQws = 0;
static unsigned int lastKey = -1;

struct press_mode {
  bool f_press;
  bool f_auto;
};

static struct press_mode controlCodeMode[] = { {0,0},{1,0},{1,1} };

EZXKbdHandler::EZXKbdHandler()
{
    qLog(Input) << "Loaded EZX keypad plugin";

    setObjectName( "EZX Keypad Handler" );

    // normal pxa270 keyboard
    kbdFD = ::open("/dev/input/event0", O_RDONLY | O_NDELAY, 0);
    if (kbdFD >= 0)
    {
        qLog(Input) << "Opened /dev/input/event0 as keypad input";
        k_notify = new QSocketNotifier(kbdFD, QSocketNotifier::Read, this);
        connect(k_notify, SIGNAL(activated(int)), this, SLOT(readKbd()));
    }
    else
    {
        qWarning("Cannot open keypad (%s)", strerror(errno));
    }

    // separate device for red key handled via PCAP
    pwrFD = ::open("/dev/input/event2", O_RDONLY | O_NDELAY, 0); 
    if (pwrFD >= 0)
    {
      qLog(Input) << "Opened /dev/input/event2 as powerkey input"; 
       p_notify = new QSocketNotifier(pwrFD, QSocketNotifier::Read, this);
       connect(p_notify, SIGNAL(activated(int)), this, SLOT(readPwr()));  
    }


}

EZXKbdHandler::~EZXKbdHandler()
{
    if (kbdFD >= 0)
        ::close(kbdFD);

    if (pwrFD >= 0)
        ::close(pwrFD);
}


void EZXKbdHandler::readData(int fd)
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
    //unsigned char           buf[64];
    struct input_event ev;
    unsigned int            qtKeyCode;
    Qt::KeyboardModifiers   modifiers = Qt::NoModifier;

    int n = ::read(fd, &ev, sizeof(struct input_event) );
    
    if( (n != (int)sizeof(struct input_event)) || (!ev.type) )
                  return;

    unsigned short unicode         = 0xffff;
    unsigned short isPress = controlCodeMode[ev.value].f_press;
    unsigned short isAuto  = controlCodeMode[ev.value].f_auto;
    
    bool repeate;

    switch (ev.code)
    {
        
        // Navigation+
        case 0x66: qtKeyCode = Qt::Key_Call;   break; 
        case 0x74: qtKeyCode = Qt::Key_Hangup;  break;

        case 0x67: qtKeyCode = Qt::Key_Up; break;
        case 0x6c: qtKeyCode = Qt::Key_Down; break;
        case 0x69: qtKeyCode = Qt::Key_Left; break;
        case 0x6a: qtKeyCode = Qt::Key_Right; break;
        case 0x60: qtKeyCode = Qt::Key_Select; break;

        // Keys on left hand side of device
        case 0x68: qtKeyCode = Qt::Key_VolumeUp; break;
        case 0x6d: qtKeyCode = Qt::Key_VolumeDown; break;
        case 0x1c: qtKeyCode = Qt::Key_Select; break;

        // Keys on right hand side of device
        //case 0x00: qtKeyCode = Qt::Key_F4; break; // FIX KERNEL
        case 0xa7: qtKeyCode = Qt::Key_F7; break;   // Key +

        // flip
        //case 0x1b: qtKeyCode = Qt::Key_Flip; break; // FIX KERNEL

        // headphone FIX KERNEL for E2/E6
        /*
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

        case 0x16: qtKeyCode = Qt::Key_Clear; break;*/


        // unknown
        printf("unknown key: %x control: %d\n",ev.code,ev.value);

    }

    processKeyEvent(unicode, qtKeyCode, modifiers, isPress, isAuto );
    printf("%x %d %d\n",qtKeyCode, isPress, isAuto );

}

void EZXKbdHandler::readKbd() {
  readData(kbdFD);
}

void EZXKbdHandler::readPwr() {
  readData(pwrFD);
}
