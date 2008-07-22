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

#ifndef EZXMODEMPINMANAGER_H
#define EZXMODEMPINMANAGER_H

#include <qpinmanager.h>
#include <qmodempinmanager.h>


class QModemService;
class QAtResult;
class EzxModemPinManagerPrivate;

class QTOPIAPHONEMODEM_EXPORT EzxModemPinManager : public QModemPinManager
{
    Q_OBJECT
public:
    explicit EzxModemPinManager( QModemService *service );
    ~EzxModemPinManager();


public slots:
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void enterPin( const QString& type, const QString& pin ) ;

private slots:
    void cpinQuery( bool ok, const QAtResult& result );
    void cpinResponse( bool ok, const QAtResult& result );
    void sendQuery();



private:
    EzxModemPinManagerPrivate *d;
};

#endif /* EZXMODEMPINMANAGER_H */
