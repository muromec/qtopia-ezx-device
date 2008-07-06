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

#ifndef QSMSSENDERTAPI_H
#define QSMSSENDERTAPI_H

#include <qsmssender.h>
#include <QMap>
#include "tapi.h"

class QTelephonyServiceTapi;
class QSMSSenderTapiPrivate;
class QSMSSenderTapiPending;

class QTOPIAPHONEMODEM_EXPORT QSMSSenderTapi : public QSMSSender
{
    Q_OBJECT
public:
    explicit QSMSSenderTapi( const QString & service = QString(), QObject * parent = 0 );
    ~QSMSSenderTapi();

public slots:
    void send( const QString& id, const QSMSMessage& msg );
    void sendResponse(SMS_SEND_RESP *resp);

private slots:
    void sendNext();

private:
    QSMSSenderTapiPrivate *d;
    QMap<unsigned int, QSMSSenderTapiPending *> id_map; //Sequence Numbers -> IDs
};

#endif /* QMODEMSMSSENDER_H */
