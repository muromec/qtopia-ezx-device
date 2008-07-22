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

#ifndef _EZXBATTERY_H_
#define _EZXBATTERY_H_

#include <QObject>
#include <QSocketNotifier>

#include <QPowerSourceProvider>
class QPowerSourceProvider;


class EzxBattery : public QObject
{
Q_OBJECT
public:
    EzxBattery(QObject *parent = 0);


private:

    QPowerSourceProvider *charger;
    QPowerSourceProvider *battery;

    QSocketNotifier *powerNotify;
    bool charging;
    bool usb;

private Q_SLOTS:
    
    void updateMotStatus();
    
};

#endif // _EZXBATTERY_H_
