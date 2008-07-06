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

#include <qsmssendertapi.h>
#include <QTimer>
#include <cstdio>

class QSMSSenderTapiPending
{
public:
    QString id;
    QList<QSMSMessage> msgs;
    int posn;
    QSMSSenderTapiPending *next;
    QTelephony::Result result;
    int retries;
};

class QSMSSenderTapiPrivate
{
public:
    QSMSSenderTapiPrivate()
    {
        this->first = 0;
        this->last = 0;
        this->sending = false;
    }
    QSMSSenderTapiPending *first;
    QSMSSenderTapiPending *last;
    //QTimer *needTimeout;
    bool sending;
};

QSMSSenderTapi::QSMSSenderTapi( const QString & service, QObject * parent )
    : QSMSSender( service, parent, QCommInterface::Server )
{
    d = new QSMSSenderTapiPrivate();
    connect(parent, SIGNAL(smsSendResponse(SMS_SEND_RESP *)), this, SLOT(sendResponse(SMS_SEND_RESP *)));
}

QSMSSenderTapi::~QSMSSenderTapi()
{
    delete d;
}

void QSMSSenderTapi::send( const QString& id, const QSMSMessage& msg )
{
    // Create the message block and add it to the pending list.
    QSMSSenderTapiPending *pending = new QSMSSenderTapiPending();
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
        sendNext();
    }
}

void QSMSSenderTapi::sendNext()
{
    // We are currently sending.
    d->sending = true;

    // Emit a finished signal if the current message is done.
    QSMSSenderTapiPending *current = d->first;
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


    // Transmit the next message in the queue.
    QSMSMessage m = current->msgs[ (current->posn)++ ];

    // Convert the message into a hexadecimal PDU string.
    QByteArray pdu = m.toPdu();

    // Prepare a SMS_PDU struct for Tapi
    SMS_PDU pdu_s;
    pdu_s.pdu_len = pdu.length();
    pdu_s.tpdu_len = pdu.length() - QSMSMessage::pduAddressLength(pdu);
    memcpy(pdu_s.pdu_buf, pdu.constData(), pdu.length());
    unsigned int seqn;
    int ret = TAPI_SMS_SendPduUnblockMode(&pdu_s, &seqn);
    id_map.insert(seqn, current);
    printf("Trying to send SMS %u: result 0x%03X\n", seqn, ret);

    d->sending = false;
}

void QSMSSenderTapi::sendResponse(SMS_SEND_RESP *resp)
{
  printf("SMS send response for PDU %u: result 0x%03X\n", resp->seq_n, resp->result);
  QSMSSenderTapiPending *pmsg = id_map[resp->seq_n];
  id_map.remove(resp->seq_n);
  if (!pmsg)
    return;
  if (resp->result!=0)
  {
    pmsg->posn = pmsg->msgs.size(); // Will cause sendNext() to stop sending
    pmsg->result = QTelephony::UnknownError;
  }
  else
    pmsg->result = QTelephony::OK;
  sendNext();
}
