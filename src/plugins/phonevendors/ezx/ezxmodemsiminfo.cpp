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

#include <ezxmodemsiminfo.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <QTimer>

// Number of milliseconds between polling attempts on AT+CIMI command.
#ifndef CIMI_TIMEOUT
#define CIMI_TIMEOUT    2000
#endif

/*!
    \class EzxModemSimInfo
    \mainclass
    \brief The EzxModemSimInfo class provides SIM identity information for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CIMI} command from 3GPP TS 27.007.

    EzxModemSimInfo implements the QSimInfo telephony interface.  Client
    applications should use QSimInfo instead of this class to
    access the modem's SIM identity information.

    \sa QSimInfo
*/

class EzxModemSimInfoPrivate
{
public:
    QModemService *service;
    QTimer *checkTimer;
    int count;
    bool simPinRequired;
};

/*!
    Construct an AT-based SIM information object for \a service.
*/
EzxModemSimInfo::EzxModemSimInfo( QModemService *service )
    : QModemSimInfo(  service )
{
}

/*!
    Destroy this AT-based SIM information object.
*/
EzxModemSimInfo::~EzxModemSimInfo()
{
}



void EzxModemSimInfo::cimi( bool ok, const QAtResult& result )
{

    QAtResultParser parser( result );
    parser.next( "+CIMI:" );
    QString id =  parser.readString();
    if ( ok && !id.isEmpty() ) {
        // We have a valid SIM identity.
        setIdentity( id );
    } else {
        // No SIM identity, so poll again in a few seconds for the first two minutes.
        setIdentity( QString() );

        if ( d->count < 120000/CIMI_TIMEOUT ) {
            d->checkTimer->start( CIMI_TIMEOUT );
            d->count++;
        } else {
            d->count = 0;
            // If not waiting for SIM pin to be entered by the user
            if ( !d->simPinRequired ) {
                // post a message to modem service to stop SIM PIN polling
                d->service->post( "simnotinserted" );
                emit notInserted();
            }
        }
        // If we got a definite "not inserted" error, then emit notInserted().
        if ( result.resultCode() == QAtResult::SimNotInserted )
            emit notInserted();
    }
}


