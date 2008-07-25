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

#include <ezxmodemsmssender.h>
#include <qmodemservice.h>
#include <qatresult.h>
#include <QTimer>

/*!
    \class EzxModemSMSSender
    \mainclass
    \brief The EzxModemSMSSender class provides SMS sending facilities for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CMGS} and \c{AT+CMMS} commands from
    3GPP TS 27.005.

    EzxModemSMSSender implements the QSMSSender telephony interface.  Client
    applications should use QSMSSender instead of this class to
    access the modem's SMS sending facilities.

    \sa QSMSSender
*/

class EzxModemSMSSenderPending
{
public:
    QString id;
    QList<QSMSMessage> msgs;
    int posn;
    EzxModemSMSSenderPending *next;
    QTelephony::Result result;
    int retries;
};

class EzxModemSMSSenderPrivate
{
public:
    EzxModemSMSSenderPrivate( QModemService *service )
    {
        this->service = service;
        this->first = 0;
        this->last = 0;
        this->sending = false;
    }

    QModemService *service;
    EzxModemSMSSenderPending *first;
    EzxModemSMSSenderPending *last;
    QTimer *needTimeout;
    bool sending;
};

/*!
    Create a new AT-based SMS sending object for for \a service.
*/
EzxModemSMSSender::EzxModemSMSSender( QModemService *service )
    : QSMSSender( service->service(), service, QCommInterface::Server )
{
    d = new EzxModemSMSSenderPrivate( service );
    service->connectToPost( "smsready", this, SLOT(smsReady()) );

    d->needTimeout = new QTimer( this );
    d->needTimeout->setSingleShot( true );
    connect( d->needTimeout, SIGNAL(timeout()), this, SLOT(smsReadyTimeout()) );
}

/*!
    Destroy this SMS sending object.
*/
EzxModemSMSSender::~EzxModemSMSSender()
{
    delete d;
}

/*!
    \reimp
*/
void EzxModemSMSSender::send( const QString& id, const QSMSMessage& msg )
{
    // Create the message block and add it to the pending list.
    EzxModemSMSSenderPending *pending = new EzxModemSMSSenderPending();
    pending->id = id;
    pending->posn = 0;
    pending->result = QTelephony::OK;
    pending->retries = 5;
    pending->msgs = msg.split();    // Split into GSM-sized message fragments.
    pending->next = 0;
    if ( d->last )
        d->last->next = pending;
    else
        d->first = pending;
    d->last = pending;

    // If this was the first message, then start the transmission process.
    if ( d->first == pending ) {
        d->needTimeout->start( 15 * 1000 );
        d->service->post( "needsms" );
    }
}

void EzxModemSMSSender::smsReady()
{
    // Stop the "need SMS" timeout if it was active.
    d->needTimeout->stop();

    // Bail out if no messages to send or we are already sending.
    // QModemSMSReader may have caused the "smsReady()" signal to
    // be emitted, so this is a false positive.
    if ( !d->first || d->sending )
        return;

    // Transmit the next message in the queue.
    sendNext();
}

void EzxModemSMSSender::smsReadyTimeout()
{
    // Fail all of the pending requests as the SMS system is not available.
    EzxModemSMSSenderPending *current = d->first;
    EzxModemSMSSenderPending *next;
    while ( current != 0 ) {
        next = current->next;
        emit finished( current->id, QTelephony::SMSOperationNotAllowed );
        delete current;
        current = next;
    }
    d->first = 0;
    d->last = 0;
}

void EzxModemSMSSender::sendNext()
{
    // We are currently sending.
    d->sending = true;

    // Emit a finished signal if the current message is done.
    EzxModemSMSSenderPending *current = d->first;
    if ( !d->first ) {
        d->sending = false;
        return;
    }
    if ( current->posn >= current->msgs.size() ) {
        emit finished( current->id, current->result );
        d->first = current->next;
        if ( !d->first )
            d->last = 0;
        delete current;
        if ( !d->first ) {
            d->sending = false;
            return;
        }
        current = d->first;
    }

    // If this is the first message in a multi-part, send AT+CMMS=1.
    if ( current->posn == 0 && current->msgs.size() > 1 ) {
        d->service->secondaryAtChat()->chat( "AT+CMMS=1" );
    }

    // Transmit the next message in the queue.
    QSMSMessage m = current->msgs[ (current->posn)++ ];

    // Convert the message into a hexadecimal PDU string.
    QByteArray pdu = m.toPdu();

    // Get the length of the data portion of the PDU,
    // excluding the service centre address.
    uint pdulen = pdu.length() - QSMSMessage::pduAddressLength( pdu );

    // Build and deliver the send command.  Note: "\032" == CTRL-Z.
    QString cmd = "AT+CMGS=" + QString::number( pdulen );
    d->service->secondaryAtChat()->chatPDU
        ( cmd, pdu, this, SLOT(transmitDone(bool,QAtResult)) );
}

void EzxModemSMSSender::transmitDone( bool ok, const QAtResult& result )
{
    // If the send was successful, then move on to the next message to be sent.
    if ( ok ) {
        sendNext();
        return;
    }

    // If this is the first or only message in a multi-part,
    // then retry after a second.  The system may not be ready.
    EzxModemSMSSenderPending *current = d->first;
    if ( current->posn == 1 ) {
        if ( --(current->retries) > 0 ) {
            --(current->posn);
            QTimer::singleShot( 1000, this, SLOT(sendNext()) );
            return;
        }
    }

    // Record the error and then move on to the next message.
    current->result = (QTelephony::Result)( result.resultCode() );
    sendNext();
}
