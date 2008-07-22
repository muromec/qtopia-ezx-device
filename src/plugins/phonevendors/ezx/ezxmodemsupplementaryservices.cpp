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

#include <ezxmodemsupplementaryservices.h>
#include <qmodemservice.h>
#include <qatutils.h>
#include <qatresult.h>

/*!
    \class EzxModemSupplementaryServices
    \mainclass
    \brief The EzxModemSupplementaryServices class provides access to structured and unstructured supplementary services for AT-based modems.
    \ingroup telephony::modem

    This class uses the \c{AT+CUSD} and \c{ATD} commands from 3GPP TS 27.007.  This class also
    processes the \c{+CSSI}, \c{+CSSU}, and \c{+CUSD} unsolicited result codes.

    EzxModemSupplementaryServices implements the QSupplementaryServices telephony interface.
    Client applications should use QSupplementaryServices instead of this class to
    access supplementary services.

    \sa QSupplementaryServices
*/

/*!
    Create an AT-based supplementary service handler for \a service.
*/
EzxModemSupplementaryServices::EzxModemSupplementaryServices
        ( QModemService *service )
    : QModemSupplementaryServices( service )
{
    this->service = service;
    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );
    service->primaryAtChat()->registerNotificationType
        ( "+CSSI:", this, SLOT(cssi(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "+CSSU:", this, SLOT(cssu(QString)) );
    service->primaryAtChat()->registerNotificationType
        ( "+CUSSD:", this, SLOT(cusd(QString)), true );
}

/*!
    Destroy this AT-based supplementary service handler.
*/
EzxModemSupplementaryServices::~EzxModemSupplementaryServices()
{
}

/*!
    \reimp
*/
void EzxModemSupplementaryServices::sendSupplementaryServiceData
        ( const QString& data )
{
    int n;
    QString tmp;
    for(n=0;n<data.length(); n++) {
      tmp.append( "00"); // FIXME
      tmp.append( QString::number( QChar(data[n]).unicode(), 16 ) );

    }
    tmp.append("0000");
    service->primaryAtChat()->chat
        ( "AT+CUSD=1,\"" + tmp + "\"", this, SLOT(atdDone(bool,QAtResult)) );

}

void EzxModemSupplementaryServices::cusd( const QString& msg )
{

    
    uint posn = 6;
    uint mflag = QAtUtils::parseNumber( msg, posn );
    QString valueHex = QAtUtils::nextString( msg, posn );
    
    // find qt functin ( in QTAtUtils ? ) or define custom
    int e = 0;
    int t = 0;

    int n;
    QString value;
    // parse modem answer 
    // TODO: use QString::toInt or not ?
    for(n=0;n<valueHex.length();n++) {
      int a;

      switch ( QChar(valueHex[n]).toAscii() ) {
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
        value.append(QChar(t));

      e++;

    }

    if (! value.isEmpty() )
      emit unstructuredNotification( (UnstructuredAction)mflag, value );
}
