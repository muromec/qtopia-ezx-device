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

#ifndef EZXHARDWARE_H
#define ezxHARDWARE_H

#ifdef QT_QWS_EZX

#include <QObject>
#include <QProcess>

#include <qvaluespace.h>
#include <linux/input.h>

class QBootSourceAccessoryProvider;
class QPowerSourceProvider;

class QSocketNotifier;
class QtopiaIpcAdaptor;
class QSpeakerPhoneAccessoryProvider;

class EzxHardware : public QObject
{
    Q_OBJECT

public:
    EzxHardware();
    ~EzxHardware();

private:
     QValueSpaceObject vsoPortableHandsfree;
     QValueSpaceObject vsoUsbCable;
     QValueSpaceObject vsoEzxHardware;
     QtopiaIpcAdaptor *adaptor;

     void findHardwareVersion();

     QtopiaIpcAdaptor         *audioMgr;
//     QAudioStateConfiguration *audioConf;
    
 
private slots:
     void headphonesInserted(int type);
     void cableConnected(bool);
     void shutdownRequested();
};

#endif

#endif
