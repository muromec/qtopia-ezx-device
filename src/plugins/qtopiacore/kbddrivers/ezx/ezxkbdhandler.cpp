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
#include <QFile>
#include <QTextStream>

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

    char fname[18];
    char caps[41];
    int n =0;

    while (1) {
      sprintf(fname, "/dev/input/event%d", n);
      printf("file %s\n", fname);
      if (access(fname, 0))
        break;

      // skip touchscreen
      sprintf(caps, "/sys/class/input/input%d/capabilities/abs", n);
      n++;

      QString cap_value;

      QFile cap_file;
      cap_file.setFileName(caps);
      cap_file.open( QIODevice::ReadOnly | QIODevice::Text );

      QTextStream in(&cap_file);
      in >> cap_value;

      printf("before caps\n");
      if(cap_value.toInt())
        continue;

      printf("after caps\n");

      int fd = ::open(fname, O_RDONLY | O_NDELAY, 0);

      if (fd >= 0)
      {
          printf("opened %s\n", fname);
          QSocketNotifier *notify = new QSocketNotifier(
              fd, QSocketNotifier::Read, this
          );
          connect(
              notify, SIGNAL(activated(int)), 
              this, SLOT(readData(int))
          );
          notifs.append( notify );
          fds.append( fd );
      }
      else
      {
          qWarning("Cannot open input %d (%s)", n, strerror(errno));
          break;
      }
    }


    openTty();

}

EZXKbdHandler::~EZXKbdHandler()
{
    while(! fds.empty() )
        ::close( fds.takeFirst() );

    while(! notifs.empty() )
        delete notifs.takeFirst();

    closeTty();

}

void EZXKbdHandler::openTty()
{
    printf("open fucking raw tty!\n"); 
    tcgetattr(ttyFd, &origTermData);
    struct termios termdata;
    tcgetattr(ttyFd, &termdata);

    ioctl(ttyFd, KDSKBMODE, K_RAW);

    termdata.c_iflag = (IGNPAR | IGNBRK) & (~PARMRK) & (~ISTRIP);
    termdata.c_oflag = 0;
    termdata.c_cflag = CREAD | CS8;
    termdata.c_lflag = 0;
    termdata.c_cc[VTIME]=0;
    termdata.c_cc[VMIN]=1;
    cfsetispeed(&termdata, 9600);
    cfsetospeed(&termdata, 9600);
    tcsetattr(ttyFd, TCSANOW, &termdata);

    connect(QApplication::instance(), SIGNAL(unixSignal(int)), this, SLOT(handleTtySwitch(int)));
    QApplication::instance()->watchUnixSignal(VTACQSIG, true);
    QApplication::instance()->watchUnixSignal(VTRELSIG, true);

    struct vt_mode vtMode;
    ioctl(ttyFd, VT_GETMODE, &vtMode);

    // let us control VT switching
    vtMode.mode = VT_PROCESS;
    vtMode.relsig = VTRELSIG;
    vtMode.acqsig = VTACQSIG;
    ioctl(ttyFd, VT_SETMODE, &vtMode);

    struct vt_stat vtStat;
    ioctl(ttyFd, VT_GETSTATE, &vtStat);

}

void EZXKbdHandler::closeTty()
{
    ioctl(ttyFd, KDSKBMODE, K_XLATE);
    tcsetattr(ttyFd, TCSANOW, &origTermData);
    ::close(ttyFd);

}

void EZXKbdHandler::handleTtySwitch(int sig)
{
    printf("tty switch!\n");

    if (sig == VTACQSIG) {
        if (ioctl(ttyFd, VT_RELDISP, VT_ACKACQ) == 0) {
            qwsServer->enablePainting(true);
            qt_screen->restore();
            qwsServer->resumeMouse();
            qwsServer->refresh();
        }
    } else if (sig == VTRELSIG) {
        qwsServer->enablePainting(false);
        qt_screen->save();
        if(ioctl(ttyFd, VT_RELDISP, 1) == 0) {
            qwsServer->suspendMouse();
        } else {
            qwsServer->enablePainting(true);
        }
    }
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

    // kernel bug
    if (ev.type == EV_MSC)
      return;

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
        case KEY_HP: qtKeyCode = Qt::Key_F28; break;

        /*
        case 0x2e: qtKeyCode = Qt::Key_F29; break;

        case 0x17: qtKeyCode = Qt::Key_OpenUrl; break;
        case 0x2d:*/
        case KEY_SOUND: qtKeyCode = Qt::Key_LaunchMedia;
        case KEY_PLAYPAUSE: qtKeyCode = Qt::Key_MediaPlay; break;
        case KEY_PREVIOUSSONG: qtKeyCode = Qt::Key_MediaPrevious; break;
        case KEY_NEXTSONG: qtKeyCode = Qt::Key_MediaNext ; break;

        // numpad
        case KEY_NUMERIC_0: qtKeyCode = Qt::Key_0; unicode  = 0x30; break;
        case KEY_NUMERIC_1: qtKeyCode = Qt::Key_1; unicode  = 0x31; break;
        case KEY_NUMERIC_2: qtKeyCode = Qt::Key_2; unicode  = 0x32; break;
        case KEY_NUMERIC_3: qtKeyCode = Qt::Key_3; unicode  = 0x33; break;
        case KEY_NUMERIC_4: qtKeyCode = Qt::Key_4; unicode  = 0x34; break;
        case KEY_NUMERIC_5: qtKeyCode = Qt::Key_5; unicode  = 0x35; break;
        case KEY_NUMERIC_6: qtKeyCode = Qt::Key_6; unicode  = 0x36; break;
        case KEY_NUMERIC_7: qtKeyCode = Qt::Key_7; unicode  = 0x37; break;
        case KEY_NUMERIC_8: qtKeyCode = Qt::Key_8; unicode  = 0x38; break;
        case KEY_NUMERIC_9: qtKeyCode = Qt::Key_9; unicode  = 0x39; break;

        case KEY_NUMERIC_STAR: qtKeyCode = Qt::Key_Asterisk;   unicode  = 0x2A; break;
        case KEY_NUMERIC_POUND: qtKeyCode = Qt::Key_NumberSign; unicode  = 0x23; break;

        case KEY_F2: qtKeyCode = Qt::Key_Back; break;
        case KEY_F1: qtKeyCode = Qt::Key_Context1; break;

        case KEY_BACK: qtKeyCode = Qt::Key_Clear; break;

        case SW_HEADPHONE_INSERT:
        case SW_MICROPHONE_INSERT:
                   printf("headphone detect %d, %d\n", ev.value, ev.type);
                   ipc_arg.setNum(!ev.value);
                   QtopiaChannel::send("QPE/Jack", "plug()",ipc_arg);
                   return;
        default:
          // unknown
            printf("unknown key: %x control: %d\n",ev.code,ev.value);
            return;
     }

    processKeyEvent(unicode, qtKeyCode, modifiers, isPress, isAuto );

}

