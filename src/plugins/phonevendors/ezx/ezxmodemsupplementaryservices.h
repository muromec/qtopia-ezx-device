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

#ifndef EZXMODEMSUPPLEMENTARYSERVICES_H
#define EZXMODEMSUPPLEMENTARYSERVICES_H

#include <qsupplementaryservices.h>
#include <qmodemsupplementaryservices.h>

class QModemService;
class QAtResult;

class QTOPIAPHONEMODEM_EXPORT EzxModemSupplementaryServices
            : public QModemSupplementaryServices
{
    Q_OBJECT
public:
    explicit EzxModemSupplementaryServices( QModemService *service );
    ~EzxModemSupplementaryServices();

public slots:
    void sendSupplementaryServiceData( const QString& data );

private slots:
    void cusd( const QString& msg );

private:
    QModemService *service;
};

#endif 
