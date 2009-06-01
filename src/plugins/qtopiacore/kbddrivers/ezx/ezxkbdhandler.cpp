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
#include <QtopiaChannel>

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
    pxaFD = ::open("/dev/input/event0", O_RDONLY | O_NDELAY, 0);
    if (pxaFD >= 0)
    {
        pxa_notify = new QSocketNotifier(pxaFD, QSocketNotifier::Read, this);
        connect(pxa_notify, SIGNAL(activated(int)), this, SLOT(readData(int)));
    }
    else
    {
        qWarning("Cannot open pxa keypad (%s)", strerror(errno));
    }


    // flip key
    gpioFD = ::open("/dev/input/event1", O_RDONLY | O_NDELAY, 0);
    if (gpioFD >= 0)
    {
        gpio_notify = new QSocketNotifier(gpioFD, QSocketNotifier::Read, this);
        connect(gpio_notify, SIGNAL(activated(int)), this, SLOT(readData(int)));
    }
    else
    {
        qWarning("Cannot open flip device (%s)", strerror(errno));
    }


    // jack event and power key
    pcapFD = ::open("/dev/input/event2", O_RDONLY | O_NDELAY, 0);
    if (pcapFD >= 0)
    {
        pcap_notify = new QSocketNotifier(pcapFD, QSocketNotifier::Read, this);
        connect(pcap_notify, SIGNAL(activated(int)), this, SLOT(readData(int)));
    }
    else
    {
        qWarning("Cannot open pcap events device (%s)", strerror(errno));
    }


}

EZXKbdHandler::~EZXKbdHandler()
{
    if (pxaFD >= 0)
        ::close(pxaFD);

    if (gpioFD >= 0)
        ::close(gpioFD);

    if (pcapFD >= 0)
        ::close(pcapFD);

}


void EZXKbdHandler::readData(int fd)
{

    struct input_event ev;
    unsigned int            qtKeyCode;
    Qt::KeyboardModifiers   modifiers = Qt::NoModifier;

    int n = ::read(fd, &ev, sizeof(struct input_event) );

    if( (n != (int)sizeof(struct input_event)) || (!ev.type) )
                  return;

    unsigned short unicode         = 0xffff;
    unsigned short isPress = controlCodeMode[ev.value].f_press;
    unsigned short isAuto  = controlCodeMode[ev.value].f_auto;

   QByteArray ipc_arg;

    switch (ev.code)
    {

        case KEY_SEND: qtKeyCode = Qt::Key_Call;   break;
        case KEY_POWER: qtKeyCode = Qt::Key_Hangup;  break;

        case KEY_UP: qtKeyCode = Qt::Key_Up; break;
        case KEY_DOWN: qtKeyCode = Qt::Key_Down; break;
        case KEY_LEFT: qtKeyCode = Qt::Key_Left; break;
        case KEY_RIGHT: qtKeyCode = Qt::Key_Right; break;
        case KEY_KPENTER: qtKeyCode = Qt::Key_Select; break;
         // Keys on left hand side of device
        case KEY_PAGEUP: qtKeyCode = Qt::Key_VolumeUp; break;
        case KEY_PAGEDOWN: qtKeyCode = Qt::Key_VolumeDown; break;
        case KEY_SELECT: qtKeyCode = Qt::Key_Select; break;
         // Keys on right hand side of device
        case KEY_CAMERA: qtKeyCode = Qt::Key_F4; break;
        case KEY_RECORD: qtKeyCode = Qt::Key_F7; break;
        // flip
        case SW_LID: qtKeyCode = Qt::Key_Flip; break;

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

        case SW_HEADPHONE_INSERT:
                   ipc_arg.setNum(ev.value);
                   QtopiaChannel::send("QPE/Jack", "plug()",ipc_arg);
                   return;
        default:
          // unknown
            printf("unknown key: %x control: %d\n",ev.code,ev.value);
            return;
     }

    processKeyEvent(unicode, qtKeyCode, modifiers, isPress, isAuto );
    printf("%x %d %d\n",qtKeyCode, isPress, isAuto );

}

