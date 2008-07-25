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

#ifndef EZXMODEMSMSSENDER_H
#define EZXMODEMSMSSENDER_H

#include <qsmssender.h>

class QModemService;
class EzxModemSMSSenderPrivate;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT EzxModemSMSSender : public QSMSSender
{
    Q_OBJECT
public:
    explicit EzxModemSMSSender( QModemService *service );
    ~EzxModemSMSSender();

public slots:
    void send( const QString& id, const QSMSMessage& msg );

private slots:
    void smsReady();
    void smsReadyTimeout();
    void sendNext();
    void transmitDone( bool ok, const QAtResult& result );

private:
    EzxModemSMSSenderPrivate *d;
};

#endif /* EZXMODEMSMSSENDER_H */
