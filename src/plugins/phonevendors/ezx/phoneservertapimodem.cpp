/****************************************************************************;
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

/*
*
* TODO: 
* - rename classes and add hook to phoneserver.cp... done
* - move from this file and revert original dummy modem... done
* - split file 
*
*/

#include "qnetworkregistrationtapi.h"
#include "qphonebooktapi.h"
#include "qphonecallprovidertapi.h"
#include "qphonecalltapi.h"
#include "qphonerffunctionalitytapi.h"
#include "qpinmanagertapi.h"
#include "qsiminfotapi.h"
#include "qsupplementaryservicestapi.h"
#include "qtelephonyservicetapi.h"
#include "ezxvibrateaccessory.h"

#include <qvaluespace.h>
#include <QSignalSourceProvider>
#include <qtimer.h>

#include <QSocketNotifier>

#include "tapi.h"

#include "tapiezxbattery.h"




signed int   asyncFd = -1;
int phoneFd;
unsigned short int msgId[] = { 0x200,0x800,0xa00,0xe00,0x0D00 };

// call id mapping. tapi id - array index, qtopia id - value
static QString idconv[256];



/*
 *
 * CALL
 *
 */
QPhoneCallTapi::QPhoneCallTapi
        ( QPhoneCallProvider *provider, const QString& identifier,
          const QString& callType )
    : QPhoneCallImpl( provider, identifier, callType )
{

}

QPhoneCallTapi::~QPhoneCallTapi()
{
    // Nothing to do here.
}
void QPhoneCallTapi::tapi_fd()
{
 

}

void QPhoneCallTapi::dial( const QDialOptions& options )
{

    // TODO: test tapi return value

    // set qtopia number and call state
    setNumber( options.number() );
    setState( QPhoneCall::Dialing );

    // tapi structures
    int            result_id;
    unsigned char  callId;
    unsigned char  phoneNum[42];

    // copy qtopia num to tapi num
    strcpy((char *)(phoneNum), (char *) options.number().toAscii().constData()  );

    // ask tapi for kall
    result_id = TAPI_VOICE_MakeCall( phoneNum, &callId );

    // save qtopia edent 
    idconv[callId] = identifier();


}

void QPhoneCallTapi::hangup( QPhoneCall::Scope )
{

    // qtopia asked for hungup
    printf("hangup!!!\n");

    unsigned char      callId;;

    // whe have qtopia ident, find tapi id
    // TODO: if not found - kill somewho
    for (int i=0; i<255; i++)
    {
      if  (idconv[i] == identifier()) {
        printf ("found: %d\n",i);
        callId = i;
        break;
      }

    }
    printf("dropping %d\n",callId);


    switch ( state() ){
      // if incoming call
      case QPhoneCall::Incoming:
        // ask tapi for reject
        TAPI_VOICE_RejectCall(callId, 0);
        setState (QPhoneCall::Connected); // qtopia bug? dont tuch this
        break;

      // all other call types
      default:
        // ask tapi to drop call
        TAPI_VOICE_DropCurrentCall ( callId );
        
    }
    // tell qtopia.
    // TODO: tapi return value?
    setState( QPhoneCall::HangupLocal );
}

void QPhoneCallTapi::accept()
{
    // accept incoming call 
    // call tapi id 
    unsigned char      callId;
    
    // qtopia call indent set by tapi_fd() is short
    callId = atoi(identifier().toAscii().constData());
    idconv[callId] = identifier().toAscii().constData();

    // tell tapi to accept.
    // dont tell qtopia here - it whill be done in tapi_fd() 
    // connect event
    TAPI_VOICE_AnswerCall( callId, 0 );

}

void QPhoneCallTapi::hold()
{
  // TODO: i don`t know ho it must work
    emit requestFailed( QPhoneCall::HoldFailed );
}

void QPhoneCallTapi::activate( QPhoneCall::Scope )
{
  // TODO
}

void QPhoneCallTapi::join( bool )
{
  // TODO
}

void QPhoneCallTapi::tone( const QString& )
{
  // TODO: is its DTFM?
}

void QPhoneCallTapi::transfer( const QString& )
{
  // TODO: 
}

void QPhoneCallTapi::dialTimeout()
{
  // TODO: whe need this?
  // If we are still dialing, then switch into the hangup state.
    if ( state() == QPhoneCall::Dialing )
        setState( QPhoneCall::HangupLocal );
}


/*
 *
 *
 *
 *
 *
 *
 * TELEPHONY
 *
 *
 *
 */
QTelephonyServiceTapi::QTelephonyServiceTapi
        ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
    : QModemService( service, mux, parent )
{

    // connect to tapisrv
    asyncFd = TAPI_CLIENT_Init(  msgId, sizeof(msgId) / 2 );

    // set handler for incoming data
    QSocketNotifier *sock = new QSocketNotifier(asyncFd, QSocketNotifier::Read);
    connect (sock , SIGNAL(activated (int)  ), this, SLOT(tapi_fd(int)));


    SignalStrengthUpdate();

}
// using syn call to tapi
QTelephonyServiceTapi::~QTelephonyServiceTapi()
{
}
void QTelephonyServiceTapi::SignalStrengthUpdate() {
    
    SQ sq;
    int ss;
    
    // ask tapi
    int ret = TAPI_ACCE_GetSiginalQuality( &sq );

    QSignalSourceProvider* prov = new QSignalSourceProvider( QLatin1String("modem"),  QLatin1String("modem"), this );
    // rssi - 0-31, 99 - unknown
    if (sq.rssi == 99){ // singan level unknown 
      ss = -1;
      prov->setAvailability( QSignalSource::NotAvailable ); // ??
    } else {
      ss = sq.rssi *  100 / 31;
      prov->setAvailability( QSignalSource::Available );
    }

    prov->setSignalStrength( ss );

}

/*
 *
 * this function handles incoming messages from tapi server
 * TODO: split this
 *
 */
void QTelephonyServiceTapi::tapi_fd(int n)
{

	    
      // get tapi message from socket fd
      TAPI_MSG  tapi_msg;
      memset( &tapi_msg, 0, sizeof(TAPI_MSG) );
      TAPI_CLIENT_ReadAndMallocMsg( asyncFd, &tapi_msg );


      /*
       *  handle type of message
       */
      switch ( (unsigned int)tapi_msg.id )
      {
        /* 
         * status of voice call changed
         */
        case 513:
        { 
            printf ("voice notify from tapi\n");
            // get call info
            VOICE_CALL_STATUS*    tapi_call;
            tapi_call = (VOICE_CALL_STATUS*)tapi_msg.body;
            
            char id[200]; // for qtopia call id
            sprintf(id,"%d",tapi_call->cid); // copy tapi id (short) to qtopia id (long)
            QPhoneCallImpl *call; 

            switch ( (signed int)tapi_call->status )
            {   
                /* 
                 * tapi voice call connected. setting qtopia state
                 */
                case 0:
                  printf("CONNECT\n");

                    /* find call in qtopia pool by qtopia id
                     * tapi id used as array index
                     * if found set state as connected
                     */
                    call = callProvider()->findCall( idconv[tapi_call->cid]);
                    if (call )
                     call->setState (QPhoneCall::Connected);


                    break;
                
                /*
                 * tapi voice call disconnected. 
                 */
                case 1:
                    printf ("DISCONNECT.\n");
                    
                    /* try to find call by id */
                    call = callProvider()->findCall( idconv[tapi_call->cid] ) ;

                    if (call) // if found.. 
                    {
                     printf("state: %d\n",call->state());

                     call->setState (QPhoneCall::HangupRemote);
                     printf("n: remote hangup\n");

                    } else { // if not... kill soomewho
                      printf ("Hmmmm...\n");
                    }

                    // FIXME: clean id mapping?

                    break;
 
                default:
                  printf ("unhandled voice call status change: %d\n", 
                      (signed int)tapi_call->status );
                
            }
            break;
          }
          /*
           * incoming call event 
           */
          case 514:
          {
             printf("RING!\n");

             // get tapi call info
             VOICE_CALL_INFO*  tapi_call;
             tapi_call = (VOICE_CALL_INFO*)tapi_msg.body;

             // set mapping between qtopia call id and tapi call id
             char id[50];
             sprintf(id,"%d",tapi_call->cid);
             idconv[tapi_call->cid] = QString(id);

             // if call with tapi id not found qtopia pool - add
             if ( !callProvider()->findCall(id)) 
             {
               QPhoneCallTapi * call = new QPhoneCallTapi(callProvider(),id,"Voice");
               call->setNumber( (char *) tapi_call->phone );
               call->setState (QPhoneCall::Incoming);
             } 

          }
          /*
           * signal quality change event
           * async call from tapi
           */

          case 1793:
          {
              QSignalSourceProvider* prov = new QSignalSourceProvider( QLatin1String("modem"),  QLatin1String("modem"), this );

              // get level tapi value 0-5
              int sq;
              sq = *( (int*)tapi_msg.body ); 

              // set qtopia level 0-100
              prov->setSignalStrength( sq * 20 );

              // if tapi level is 0 - set NotAvailable
              if (sq)
                prov->setAvailability( QSignalSource::Available );
              else
                prov->setAvailability( QSignalSource::NotAvailable );

              break;
          }

          case 3329:
          {
            USSD_RESPONSE* ussd;
            ussd = (USSD_RESPONSE*) tapi_msg.body;

            unsigned char  text[364];
            memset(( char*)text,0,ussd->len);

            // invert 
            for(int i=0; i<sizeof(ussd->text); (i = i+2))
            {
                text[i]   = ussd->text[i+1];
                text[i+1] = ussd->text[i];
            }

            // only type==0
            if (! ussd->type)
              supp->cusd( 
                  QString::fromUtf16(
                    (unsigned short int *)text,
                    ussd->len/2
                  )   
              );
            break;
          }


         
          // some other event
           default:
              printf ("unhandled tapi msg. id: %d\n" , (int)tapi_msg.id );


         }

  

}
void QTelephonyServiceTapi::initialize()
{


  printf ("OK, intializing tapi bridge plugin..\n");
    // initialising sersises
    if ( !supports<QSimInfo>() )
        addInterface( new QSimInfoTapi( service(), this ) );

    if ( !supports<QPhoneBook>() )
        addInterface( new QPhoneBookTapi( service(), this ) );

    if ( !supports<QNetworkRegistration>() )
        addInterface( new QNetworkRegistrationTapi( service(), this ) );

    if ( !supports<QPhoneRfFunctionality>() )
        addInterface( new QPhoneRfFunctionalityTapi( service(), this ) );

    if ( !supports<QPinManager>() )
        addInterface( new QPinManagerTapi( service(), this ) );

    if ( !callProvider() )
        setCallProvider( new QPhoneCallProviderTapi( service(), this ) );
    
    if ( !supports<QVibrateAccessory>() )
        addInterface( new EZXVibrateAccessory( this ) );

    if ( !supports<QSupplementaryServices>() ) 
    {
              supp = new QSupplementaryServicesTapi( this );
              addInterface( supp );
    }


    TapiEzxBattery* bat;
    bat = new TapiEzxBattery ( this  );

    

    QTelephonyService::initialize();
}



