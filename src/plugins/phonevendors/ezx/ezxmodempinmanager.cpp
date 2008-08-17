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

#include <ezxmodempinmanager.h>
#include <qtopiaphonemodem/private/qmodempinmanager_p.h>
#include <qmodemservice.h>
#include <qatchat.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qatutils.h>
#include <qtopialog.h>
#include <QTimer>

class EzxModemPinManagerPrivate
{
public:
    EzxModemPinManagerPrivate( QModemService *service )
    {
        this->service = service;
        this->querying = false;
        this->simMissing = false;
    }

    QModemService *service;
    QString expectedPin;
    QString expectedAskPin;
    QString changingPin;
    QString lastSimPin;
    QString currentPin;
    bool querying;
    QList<QModemPendingPin *> pending;
    QString pendingPin;
    QTimer *lastPinTimer;
    bool simMissing;

    QModemPendingPin *findPending( const QString& type );
};

QModemPendingPin *EzxModemPinManagerPrivate::findPending( const QString& type )
{
    QList<QModemPendingPin *>::ConstIterator it;
    for ( it = pending.begin(); it != pending.end(); ++it ) {
        if ( (*it)->type() == type )
            return *it;
    }
    return 0;
}

/*!
    Create a modem pin manager for \a service.
*/
EzxModemPinManager::EzxModemPinManager( QModemService *service )
    : QModemPinManager( service )
{
    d = new EzxModemPinManagerPrivate( service );
    d->lastPinTimer = new QTimer( this );
    d->lastPinTimer->setSingleShot( true );
    connect( d->lastPinTimer, SIGNAL(timeout()), this, SLOT(lastPinTimeout()) );

    // Hook onto the "pinquery" event to allow other providers
    // to ask us to query for the pin the modem wants.  This may
    // be sent during initialization, functionality changes,
    // or when the modem reports a "pin/puk required" error.
    service->connectToPost( "pinquery", this, SLOT(sendQuery()) );
    service->connectToPost( "simnotinserted", this, SLOT(simMissing()) );
}

/*!
    Destroy this modem pin manager.
*/
EzxModemPinManager::~EzxModemPinManager()
{
    delete d;
}



void EzxModemPinManager::enterPin( const QString& type, const QString& pin )
{
    // Pending pins should be sent to the object that asked for them.
    QModemPendingPin *pending;
    bool hadPending = false;
    while ( ( pending = d->findPending( type ) ) != 0 ) {
        d->pending.removeAll( pending );
        pending->emitHavePin( pin );
        delete pending;
        hadPending = true;
    }
    if ( hadPending )
        return;

    // If this wasn't a pending pin, then we were probably processing
    // the answer to a sendQuery() request.
    if ( type != d->expectedPin ) {
        qLog( Modem ) << "Pin" << type
                      << "entered, expecting" << d->expectedPin;
        return;
    }
    d->currentPin = pin;

    d->service->chat( "AT+CPIN=1,\"" + QAtUtils::quote( pin ) + "\"",
                      this, SLOT(cpinResponse(bool,QAtResult)) );
}

void EzxModemPinManager::enterPuk
        ( const QString& type, const QString& puk, const QString& newPin )
{
    if ( ( type != "SIM PUK" && type != "SIM PUK2" ) || !d->expectedPin.isEmpty() ) {
        if ( type != d->expectedPin ) {
            qLog( Modem ) << "Puk" << type
                          << "entered, expecting" << d->expectedPin;
            return;
        }
    }

    if ( type.contains("PUK") && d->findPending( type ) != 0 ) {
        // We will need newPin later if the puk is verified.
        d->pendingPin = newPin;
        d->service->chat( "AT+CPIN=\"" + QAtUtils::quote( puk ) + "\",\"" +
                          QAtUtils::quote( newPin ) + "\"",
                          this, SLOT(cpukResponse(bool)) );
    } else {
        d->currentPin = newPin;
        d->expectedPin = type;
        d->service->chat( 
            "AT+CPUR=1;+CPIN=\"" +  puk  + "\",\"" + newPin  + "\"",
                          this, SLOT(cpinResponse(bool,QAtResult)) );
    }
}

// Process the response to a AT+CPIN=[puk,]pin command.
void EzxModemPinManager::cpinResponse( bool ok, const QAtResult& result )
{

    d->lastSimPin = QString();
    if ( ok ) {

        // The pin was entered correctly.
        QPinOptions options;
        options.setMaxLength( pinMaximum() );
        emit pinStatus( d->expectedPin, QPinManager::Valid, options );
        if ( d->expectedPin.contains( "PUK" ) ) {
            // If we just successfully sent the PUK, then the PIN
            // version is also successfull.
            QString pin = d->expectedPin.replace( "PUK", "PIN" );
            emit pinStatus( pin, QPinManager::Valid, options );
        }
        if ( d->expectedPin == "SIM PIN" ) {
            // Cache the last valid SIM PIN value because some modems will ask
            // for it again immediately even when they accept it first time.
            // 3GPP TS 27.007 requires this behaviour.
            d->lastSimPin = d->currentPin;
            d->lastPinTimer->start( 5000 ); // Clear it after 5 seconds.
        }
        d->expectedPin = QString();
        d->service->post( "simpinentered" );
    }
    d->currentPin = QString();

    // If the result includes a "+CPIN" line, then the modem may
    // be asking for a new pin/puk already.  Otherwise send AT+CPIN?
    // to ask whether the modem is "READY" or needs another pin/puk.
    QAtResultParser parser( result );
    if ( parser.next( "+CPIN:" ) ) {
        d->querying = true;
        cpinQuery( true, result );
    } else {
        sendQuery();
    }
}

void EzxModemPinManager::sendQuery()
{
    if ( !d->querying ) {
        d->querying = true;
        if ( d->service->multiplexer()->channel( "primary" )->isValid() ) {
            d->service->chat
                ( "AT+CPIN?", this, SLOT(cpinQuery(bool,QAtResult)) );
        } else {
            // The underlying serial device could not be opened,
            // so we are not talking to a modem.  Fake sim ready.
            QAtResult result;
            result.setResultCode( QAtResult::OK );
            result.setContent( "+CPIN: READY" );
            cpinQuery( true, result );
        }
    }
}


// Process the response to a AT+CPIN? command.
void EzxModemPinManager::cpinQuery( bool ok, const QAtResult& result )
{

    if ( !ok ) {

        // The AT+CPIN? request failed.  The SIM may not be ready.
        // Wait three seconds and resend.
        if ( !d->simMissing )
            QTimer::singleShot( 3000, this, SLOT(sendQueryAgain()) );

    } else {

        // No longer querying for the pin.
        d->querying = false;

        // Extract the required pin from the returned result.
        QAtResultParser parser( result );
        parser.next( "+CPIN:" );
        QString pin = parser.line().trimmed().remove("\"");
        if ( pin == "READY" || ( pin.isEmpty() && emptyPinIsReady() ) ) {

            // No more PIN's are required, so the sim is ready.
            d->expectedPin = QString();
            d->service->post( "simready" );

            // Notify the application level that the sim is ready.
            emit pinStatus( "READY", QPinManager::Valid, QPinOptions() );

        } else if ( pin == "SIM PIN" && !d->lastSimPin.isEmpty() ) {

            // We have a cached version of the primary "SIM PIN"
            // that we know is valid.  Re-send it immediately.
            // Some modems ask for the SIM PIN multiple times.
            // 3GPP TS 27.007 requires this behaviour.
            d->expectedPin = pin;
            d->currentPin = d->lastSimPin;
            d->service->chat( "AT+CPIN=1,\"" + d->lastSimPin  +
                              "\"", this, SLOT(cpinResponse(bool,QAtResult)) );

        } else {

            // Ask that the pin be supplied by the user.
            d->expectedPin = pin;
            QPinManager::Status status;
            if ( pin.contains( "PUK" ) )
                status = QPinManager::NeedPuk;
            else {
                status = QPinManager::NeedPin;
                d->service->post( "simpinrequired" );
            }
            QPinOptions options;
            options.setMaxLength( pinMaximum() );
            emit pinStatus( d->expectedPin, status, options );

        }
    }
}

