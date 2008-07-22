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

#ifndef EZXMODEMNETWORKREGISTRATION_H
#define EZXMODEMNETWORKREGISTRATION_H

#include <qnetworkregistration.h>
#include <qmodemnetworkregistration.h>
#include <qatresultparser.h>

class QModemService;
class EzxModemNetworkRegistrationPrivate;

class QTOPIAPHONEMODEM_EXPORT EzxModemNetworkRegistration
        : public QModemNetworkRegistration
{
    Q_OBJECT
public:
    explicit EzxModemNetworkRegistration( QModemService *service );
    ~EzxModemNetworkRegistration();

    void setSupportsOperatorTechnology( bool value );



private slots:
    void copsDone( bool ok, const QAtResult& result );
    void espnDone( bool ok, const QAtResult& result ); 
    void cregNotify( const QString& msg );


private:
    EzxModemNetworkRegistrationPrivate *d;

    void queryCurrentOperator();
};

#endif /* EZXMODEMNETWORKREGISTRATION_H */
