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

#include "phoneservertapimodem.h"
#include <qvaluespace.h>
#include <QSignalSourceProvider>
#include <qtimer.h>

#include <QSocketNotifier>

#include <sys/ioctl.h>  
#include "tapi.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tapiezxbattery.h"



#include <unistd.h>

#define EZX_VIBRATOR_IOCTL_BASE 0xbb
#define EZX_VIBRATOR_ENABLE     _IOW (EZX_VIBRATOR_IOCTL_BASE,1,int)
#define EZX_VIBRATOR_DISABLE    _IO (EZX_VIBRATOR_IOCTL_BASE,2)

signed int   asyncFd = -1;
int phoneFd;
unsigned short int msgId[] = { 0x200,0x800,0xa00,0xe00 };
signed char tpin[16];

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
 * PROVIDER
 *
 *
 */
QPhoneCallProviderTapi::QPhoneCallProviderTapi
        ( const QString& service, QObject *parent )
    : QPhoneCallProvider( service, parent )
{
    setCallTypes( QStringList( "Voice" ) );
}

QPhoneCallProviderTapi::~QPhoneCallProviderTapi()
{
}

/*
 *
 *
 *
 * IMPL
 *
 *
 *
 */

QPhoneCallImpl *QPhoneCallProviderTapi::create
        ( const QString& identifier, const QString& callType )
{
  // maby hack qtopia ident here?
  // ... or not. id mapping works whell
    return new QPhoneCallTapi( this, identifier, callType );
}

/*
 *
 *
 *
 * BOOK
 *
 * TODO: now this not works at all
 */
QPhoneBookTapi::QPhoneBookTapi( const QString& service, QObject *parent )
    : QPhoneBook( service, parent, QCommInterface::Server )
{
    fixedDialingEnabled = false;
}

QPhoneBookTapi::~QPhoneBookTapi()
{
}

void QPhoneBookTapi::getEntries( const QString& store )
{
    QList<QPhoneBookEntry> list;
    if ( store == "SM" ) {
        QList<QPhoneBookEntry>::ConstIterator iter;
        for ( iter = ents.begin(); iter != ents.end(); ++iter ) {
            if ( ! (*iter).number().isEmpty() )
                list += *iter;
        }
    }
    emit entries( store, list );
}

void QPhoneBookTapi::add( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index;
    for ( index = 0; index < ents.size(); ++index ) {
        if ( ents[index].number().isEmpty() )
            break;
    }

    QPhoneBookEntry newEntry( entry );
    newEntry.setIndex( (uint)index );

    if ( index < ents.size() ) {
        ents[index] = newEntry;
    } else {
        ents += newEntry;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::remove( uint index, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    if ( ((int)index) < ents.size() ) {
        ents[(int)index].setNumber( "" );
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::update( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index = (int)entry.index();
    if ( index < ents.size() ) {
        ents[index] = entry;
    } else {
        add( entry, store, flush );
        return;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::flush( const QString& store )
{
    getEntries( store );
}

void QPhoneBookTapi::setPassword( const QString&, const QString& )
{
    // Nothing to do here.
}

void QPhoneBookTapi::clearPassword( const QString& )
{
    // Nothing to do here.
}

void QPhoneBookTapi::requestLimits( const QString& store )
{
    QPhoneBookLimits l;
    l.setNumberLength( 20 );
    l.setTextLength( 18 );
    l.setFirstIndex( 1 );
    l.setLastIndex( 150 );
    emit limits( store, l );
}

void QPhoneBookTapi::requestFixedDialingState()
{
    emit fixedDialingState( fixedDialingEnabled );
}

void QPhoneBookTapi::setFixedDialingState( bool enabled, const QString& )
{
    fixedDialingEnabled = enabled;
    emit setFixedDialingStateResult( QTelephony::OK );
}

/*
 *
 *
 *
 * NETWORK
 *
 *
 */
QNetworkRegistrationTapi::QNetworkRegistrationTapi
        ( const QString& service, QObject *parent )
    : QNetworkRegistrationServer( service, parent )
{
  // TODO: find matching tapi calls?
    QTimer::singleShot( 500, this, SLOT(searching()) );
    QTimer::singleShot( 5000, this, SLOT(home()) );
    QTimer::singleShot( 200, this, SLOT(initDone()) );

    printf("registration: none\n");
    updateRegistrationState( QTelephony::RegistrationNone );
}

QNetworkRegistrationTapi::~QNetworkRegistrationTapi()
{
}

void QNetworkRegistrationTapi::setCurrentOperator
        ( QTelephony::OperatorMode, const QString&, const QString&)
{
  // TODO: somewho uses this?
    emit setCurrentOperatorResult( QTelephony::OK );
}

/*
 * Are next 4 methods hacks for dummy modem or not?
 */
void QNetworkRegistrationTapi::requestAvailableOperators()
{
  //  TODO: call TAPI_NETWORK_GetAvailableNetwork() here
  //  and catch fucking async answer in tapi_fd()

    QList<QNetworkRegistration::AvailableOperator> opers;
    QNetworkRegistration::AvailableOperator oper;
    oper.availability = QTelephony::OperatorAvailable;
    oper.name = "Qtopia";       // No tr
    oper.id = "0Qtopia";       // No tr
    oper.technology = "GSM";       // No tr
    opers.append( oper );

    emit availableOperators( opers );
}

void QNetworkRegistrationTapi::searching()
{
    printf("registration: search\n");

    updateRegistrationState( QTelephony::RegistrationSearching );
}

void QNetworkRegistrationTapi::home()
{
    SP_NAME opName;
    signed char   mcc[4];
    signed char   mnc[4];

    char netid[8];


    // get opname and network codes
    TAPI_NETWORK_GetServiceProviderName( &opName );
    TAPI_NETWORK_GetCurrentNetworkId( mcc, mnc );

    // formar network id
    sprintf(netid,"%s-%s",mcc,mnc);
        
    // decode opname
    unsigned short int* p;
    p = (unsigned short int*)&(opName.name);
    char opname[25];
    
    for(int i = 0; i < 25; i++)
    {
       /*if(*p == 0)
               break;*/
       opname[i] = ((int)*p);
       p++;
    }      


    // FIXME: no test conditions
    updateRegistrationState( QTelephony::RegistrationHome );

    // set discovered data
    updateCurrentOperator( QTelephony::OperatorModeAutomatic,
                           netid, opname, "GSM" );    
    printf("registration: home: %s, %s-%s\n", opname, mcc, mnc);
}

void QNetworkRegistrationTapi::initDone()
{
    updateInitialized( true );
}
/*
 *
 *
 * SIM
 *
 */
QSimInfoTapi::QSimInfoTapi( const QString& service, QObject *parent )
    : QSimInfo( service, parent, QCommInterface::Server )
{
    setIdentity( "9876543210" );
}

QSimInfoTapi::~QSimInfoTapi()
{
}

/* 
 *
 *  RF
 *
 */ 
QPhoneRfFunctionalityTapi::QPhoneRfFunctionalityTapi
        ( const QString& service, QObject *parent )
    : QPhoneRfFunctionality( service, parent, QCommInterface::Server )
{
}

QPhoneRfFunctionalityTapi::~QPhoneRfFunctionalityTapi()
{
}

void QPhoneRfFunctionalityTapi::forceLevelRequest()
{
    setValue( "level", qVariantFromValue( Full ) );
    printf("level..\n");
    emit levelChanged();
}

void QPhoneRfFunctionalityTapi::setLevel( QPhoneRfFunctionality::Level level )
{
    Q_UNUSED(level);
}
/*
 *
 *
 * PIN
 *
 */
QPinManagerTapi::QPinManagerTapi( const QString& service, QObject *parent )
    : QPinManager( service, parent, Server )
{
}

QPinManagerTapi::~QPinManagerTapi()
{
}

void QPinManagerTapi::querySimPinStatus()
{
      // FIXME: check ret

      int ret;
      unsigned char              state;

      ret = TAPI_SECURITY_GetPin1Status( 1, &state);

      printf("pin: %d\n",state);

      if (state)
        emit pinStatus( "SIM PIN", NeedPin, QPinOptions() );
      else
        emit pinStatus( "SIM PIN", Valid,   QPinOptions() );

}

void QPinManagerTapi::enterPin( const QString& type, const QString& pin )
{
  int ret;
  strcpy((char *)(tpin), (char *) pin.toAscii().constData()  );

  // disable pin request at all. FIXME: onetime pin enter
  ret = TAPI_SECURITY_SetPin1Status( tpin, 0 );
    

  printf("enter pin: %d\n",ret);
  
  if (ret)
      emit pinStatus( "SIM PIN", NeedPin, QPinOptions() ); // if not ok - try again
  else
      emit pinStatus( type, Valid, QPinOptions() ); // all ok
}

void QPinManagerTapi::enterPuk
        ( const QString& type, const QString& puk, const QString& newPin )
{
   // TODO:
    Q_UNUSED(puk);
    Q_UNUSED(newPin);
    emit pinStatus( type, Valid, QPinOptions() );
}

void QPinManagerTapi::cancelPin( const QString& type )
{
   // TODO: wtf is this?
    Q_UNUSED(type);
    printf("cancel pin\n");
}

void QPinManagerTapi::changePin
        ( const QString& type, const QString& oldPin, const QString& newPin )
{
   // TODO
    Q_UNUSED(type);
    Q_UNUSED(oldPin);
    Q_UNUSED(newPin);
    emit changePinResult( type, true );
}

/*
 *
 * Vibro
 *
 */
EZXVibrateAccessory::EZXVibrateAccessory
        ( QModemService *service )
    : QVibrateAccessoryProvider( service->service(), service )
{
    setSupportsVibrateOnRing( true );
    setSupportsVibrateNow( false );
}

EZXVibrateAccessory::~EZXVibrateAccessory()
{
}

void EZXVibrateAccessory::setVibrateOnRing( const bool value )
{
    setVibrateNow(value);
}

void EZXVibrateAccessory::setVibrateNow( const bool value )
{


  int level = 3; // TODO: is there vibro level in qtopia?

  // open vibrator device and set mode
  int vibro = open("/dev/vibrator", O_RDWR);
  if (value)
      ioctl(vibro, EZX_VIBRATOR_ENABLE,  &level);
  else
      ioctl(vibro, EZX_VIBRATOR_DISABLE, &level);
  close(vibro);



  QVibrateAccessoryProvider::setVibrateNow( value );
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

    TapiEzxBattery* bat;
    bat = new TapiEzxBattery ( this  );

    

    QTelephonyService::initialize();
}



