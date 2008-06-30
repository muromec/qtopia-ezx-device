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

#include "ezxbattery.h"




signed int   asyncFd = -1;
int phoneFd;
unsigned short int msgId[] = { 0x200,0x700,0x800,0xa00,0xe00,0x0D00 };

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

unsigned char QPhoneCallTapi::get_cid(void)
{
  for (int i=0; i<255; i++)
    {
      if  (idconv[i] == identifier()) 
        return i;

    }
  return 255;

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


    unsigned char  callId = get_cid();

    if (callId == 255)
      return ;

    // whe have qtopia ident, find tapi id
    // TODO: if not found - kill somewho
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
    unsigned char  callId = get_cid();

    TAPI_VOICE_HoldCall(callId);
}

void QPhoneCallTapi::activate( QPhoneCall::Scope )
{
    TAPI_VOICE_RetrieveCall(get_cid());
}

void QPhoneCallTapi::join( bool )
{
   //TAPI_VOICE_JoinCall(cid,hcid);
}

void QPhoneCallTapi::tone( const QString& qtone)
{
  unsigned char tone = qtone.toAscii().constData()[0];


  TAPI_VOICE_MakeDtmfTone(tone,0);
  TAPI_VOICE_MakeDtmfTone(tone,1);
}

void QPhoneCallTapi::transfer( const QString& num )
{
  unsigned char newCallId; 
  unsigned char phoneNum[42];

  // copy qtopia num to tapi num
  strcpy(
      (char *)(phoneNum), 
      (char *)num.toAscii().constData()  
  );

  TAPI_VOICE_TransferCall(get_cid(), phoneNum, &newCallId);
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
    if (asyncFd < 1) 
      printf ("ERROR: cannot connect to tapi! Fix your setup!\n");

    // cleanup
    TAPI_VOICE_DropAllCall();

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
    TAPI_ACCE_GetSiginalQuality( &sq );

    QSignalSourceProvider* prov = new QSignalSourceProvider( QLatin1String("modem"),  QLatin1String("modem"), this );
    printf("rssi: %d, %d\n",sq.rssi, (sq.rssi *  100 / 31));
    // rssi - 0-31, 99 - unknown
    if ((sq.rssi == 99) or (sq.rssi == 100) ){ // singan level unknown 
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
 * Tapi voice call status changed
 *
 */

void QTelephonyServiceTapi::voice_state(VOICE_CALL_STATUS *tapi_call) {

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
        /*
         * tapi voice call hold
         */
        case 5:
          call = callProvider()->findCall( idconv[tapi_call->cid]);
          if (call)
            call->setState (QPhoneCall::Hold);
          break;

        default:
          printf ("unhandled voice call status change: %d\n", 
              (signed int)tapi_call->status );
        
    }

}

/*
 * Incoming tapi call
 */

void QTelephonyServiceTapi::incoming(VOICE_CALL_INFO *tapi_call){
     printf("RING!\n");


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
 *  signal quality update
 */
void QTelephonyServiceTapi::signal_quality(int quality){
      
      QSignalSourceProvider* prov = new QSignalSourceProvider( QLatin1String("modem"),  QLatin1String("modem"), this );

      // set qtopia level 0-100
      prov->setSignalStrength( quality * 20 );

      // if tapi level is 0 - set NotAvailable
      if (quality)
        prov->setAvailability( QSignalSource::Available );
      else
        prov->setAvailability( QSignalSource::NotAvailable );

}
/* 
 *  ussd response
 */
void QTelephonyServiceTapi::ussd_response(USSD_RESPONSE *ussd){
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

}
/*
 *
 * this function handles incoming messages from tapi server
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
        case 0x201: voice_state( (VOICE_CALL_STATUS*)tapi_msg.body ); break;
        case 0x202: incoming(    (VOICE_CALL_INFO*)  tapi_msg.body ); break;
        case 0x701: signal_quality(*((int*)          tapi_msg.body)); break;
        case 0x702: printf("sa: %d\n",(int) *tapi_msg.body); break; //  NOP?
        case 0x703: printf("r : %d\n",(int) *tapi_msg.body); break; // QTelephony::RegistrationHome / QTelephony::RegistrationRoaming
        case 0x706: printf("searching\n"); break; // QTelephony::RegistrationSearching
        case 0x70a: printf("registered\n"); break; // set opname
        case 0x70c: printf("deregistered\n");break; // QTelephony::RegistrationNone
        
        case 0x804: printf("grps: %d\n",(int) *tapi_msg.body); break;
        case 0x80b: printf("egprs: %d\n",(int) *tapi_msg.body); break;

        // 0x702 - service availability
        // 0x703 - roaming status
        // 0x706 - searching 
        // 0x70a - registered
        // 0x70c - deregistered

        // 0x804 - gprs attach status
        // 0x80b - egprs status

        case 0xd01: ussd_response((USSD_RESPONSE*)   tapi_msg.body ); break;
        default: printf ("tapi %x body: %x\n" , (int)tapi_msg.id, tapi_msg.body) ;
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


    //TapiEzxBattery* bat;
    EzxBattery* bat;
    bat = new EzxBattery ( this  );

    

    QTelephonyService::initialize();
}



