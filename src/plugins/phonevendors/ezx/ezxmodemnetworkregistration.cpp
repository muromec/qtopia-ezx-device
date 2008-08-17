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

#include <ezxmodemnetworkregistration.h>
#include <qmodemservice.h>
#include <qatchat.h>
#include <qatutils.h>
#include <qatresult.h>
#include <qatresultparser.h>

/*!
    \class EzxModemNetworkRegistration
    \mainclass
    \brief The EzxModemNetworkRegistration class provides access to network registration features of AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+COPS} and \c{AT+CREG} commands from 3GPP TS 27.007.

    EzxModemNetworkRegistration implements the QNetworkRegistration telephony interface.
    Client applications should use QNetworkRegistration instead of this class to
    access the modem's network registration settings.

    \sa QNetworkRegistration
*/

class EzxModemNetworkRegistrationPrivate
{
public:
    EzxModemNetworkRegistrationPrivate( QModemService *service )
    {
        this->service = service;
        supportsOperatorTechnology = false;
        operatorId = -1;
    }

    QModemService *service;
    bool supportsOperatorTechnology;
    int operatorId;
    QString operatorName;
};

/*!
    Construct an AT-based modem network registration object for \a service.
*/
EzxModemNetworkRegistration::EzxModemNetworkRegistration( QModemService *service )
    : QModemNetworkRegistration( service )
{

    d = new EzxModemNetworkRegistrationPrivate( service );
    service->primaryAtChat()->registerNotificationType
        ( "+CREG:", this, SLOT(cregNotify(QString)), true );
    service->connectToPost( "cfunDone", this, SLOT(cfunDone()) );
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );
    queryCurrentOperator();
}

/*!
    Destroy this AT-based modem network registration object.
*/
EzxModemNetworkRegistration::~EzxModemNetworkRegistration()
{
  delete d;
}




void EzxModemNetworkRegistration::copsDone( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    parser.next( "+COPS:" );
    uint mode = parser.readNumeric();
    uint format = parser.readNumeric();
    QString id;
   
    id = parser.readString();
    
    QTelephony::OperatorMode mmode;
    switch ( mode ) {
        default: case 0: mmode = QTelephony::OperatorModeAutomatic; break;
        case 1: mmode = QTelephony::OperatorModeManual; break;
        case 2: mmode = QTelephony::OperatorModeDeregister; break;
        case 4: mmode = QTelephony::OperatorModeManualAutomatic; break;
    }
    
    updateCurrentOperator( mmode,id,d->operatorName, "GSM" );

}

void EzxModemNetworkRegistration::espnDone( bool, const QAtResult& result )
{
    QAtResultParser parser( result );
    parser.next( "+ESPN:" );
    uint a = parser.readNumeric();
    QString name, nameHex;
    name = "";

    // string is hex encoded utf16
    nameHex = parser.readString();

    int e = 0;
    int t = 0;

    int n;
    // parse modem answer 
    // TODO: use QString::toInt or not ?
    for(n=0;n<nameHex.length();n++) {
      int a;

      switch ( QChar(nameHex[n]).toAscii() ) {
        case '0': a=0;  break;
        case '1': a=1;  break;
        case '2': a=2;  break;
        case '3': a=3;  break;
        case '4': a=4;  break;
        case '5': a=5;  break;
        case '6': a=6;  break;
        case '7': a=7;  break;
        case '8': a=8;  break;
        case '9': a=9;  break;
        case 'A': a=10; break;
        case 'B': a=11; break;
        case 'C': a=12; break;
        case 'D': a=13; break;
        case 'E': a=14; break;
        case 'F': a=15; break;
      }

      if (e == 4) {
        t = 0;
        e = 0;
      }
      
      t += (a  << ( (3-e) * 4) );


      if (e == 3) 
        name.append(QChar(t));

      e++;

    }

    d->operatorName = name;

        

}

void EzxModemNetworkRegistration::cregNotify( const QString& msg )
{
    uint posn = 6;
    uint stat = QAtUtils::parseNumber( msg, posn );
    QString lac = QAtUtils::nextString( msg, posn );
    QString ci = QAtUtils::nextString( msg, posn );
    if ( !lac.isEmpty() && !ci.isEmpty() ) {
        // We have location information after the state value.
        updateRegistrationState( (QTelephony::RegistrationState)stat,
                                 lac.toInt( 0, 16 ), ci.toInt( 0, 16 ) );
    } else {
        // We don't have any location information.
        updateRegistrationState( (QTelephony::RegistrationState)stat );
    }

    // Query for the operator name if home or roaming.
    if ( stat == 1 || stat == 5 )
        queryCurrentOperator();
}





void EzxModemNetworkRegistration::queryCurrentOperator()
{
    d->operatorName = "";

    d->service->secondaryAtChat()->chat
        ( "AT+ESPN?", this, SLOT(espnDone(bool,QAtResult)) );

    d->service->secondaryAtChat()->chat( "AT+COPS=3,2" );
    d->service->secondaryAtChat()->chat
        ( "AT+COPS?", this, SLOT(copsDone(bool,QAtResult)) );
}
