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

#ifndef EZXKBDHANDLER_H
#define EZXKBDHANDLER_H


#include <QObject>
#include <QWSKeyboardHandler>

#include <termios.h>
#include <linux/kd.h>

class QSocketNotifier;

class EZXKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    EZXKbdHandler();
    ~EZXKbdHandler();

private:

    QList<int> fds;
    QList<QSocketNotifier *> notifs;

    int ttyFd;

    void openTty();
    void closeTty();
    struct termios origTermData;


private Q_SLOTS:
    void readData(int fd);
    void handleTtySwitch(int sig);

};


#endif // EZXKBDHANDLER_H
