#include "ezxvibrateaccessory.h"
#include "ezxmodemservice.h"
#include "ezxmodempinmanager.h"
#include "ezxmodemnetworkregistration.h"
#include "ezxmodemsiminfo.h"
#include "ezxmodemsupplementaryservices.h"
#include "ezxmodemsmsreader.h"
#include "ezxmodemsmssender.h"
#include "ezxmodemcallprovider.h"

#include "QSerialPort"  
#include <qslotinvoker.h>
#include <qtopiaipcadaptor.h>
#include <qmodemindicators.h>


class EzxModemServicePrivate
{
public:
    EzxModemServicePrivate()
    {
        smsRetryCount = 0;
        provider = 0;
        firstInitDone = false;
    }

    QSerialIODeviceMultiplexer *mux;

    QAtChat *primaryAtChat;
    QAtChat *secondaryAtChat;
    QAtChat *smsAtChat;

    int smsRetryCount;
    QStringList pending;
    QMultiMap< QString, QSlotInvoker * > invokers;
    QModemCallProvider *provider;
    QModemIndicators *indicators;
    bool firstInitDone;
};


EzxModemService::EzxModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
    : QModemService( service, mux, parent )
{
  init(mux);

}
EzxModemService::~EzxModemService()
{
}

void EzxModemService::init( QSerialIODeviceMultiplexer *mux )
{
    d = new EzxModemServicePrivate();
    d->mux = mux;
    d->primaryAtChat = mux->channel( "primary" )->atchat();
    d->secondaryAtChat = mux->channel( "secondary" )->atchat();
    d->smsAtChat = mux->channel( "sms" )->atchat();

    // Make sure that the primary command channel is open and valid.
    QSerialIODevice *primary = d->mux->channel( "primary" );
    if ( !primary->isOpen() )
        primary->open( QIODevice::ReadWrite );

    // Set a slightly different debug mode for the secondary channel,
    // to make it easier to see which command goes where.
    if ( d->primaryAtChat != d->secondaryAtChat )
        d->secondaryAtChat->setDebugChars( 'f', 't', 'n', '?' );

    connectToPost( "needsms", this, SLOT(needSms()) );
    connectToPost( "simready", this, SLOT(firstTimeInit()) );

    d->indicators = new QModemIndicators( this );

    // Listen for suspend() and wake() messages on QPE/ModemSuspend.
    QtopiaIpcAdaptor *suspend =
        new QtopiaIpcAdaptor( "QPE/ModemSuspend", this );
    QtopiaIpcAdaptor::connect( suspend, MESSAGE(suspend()),
                               this, SLOT(suspend()) );
    QtopiaIpcAdaptor::connect( suspend, MESSAGE(wake()),
                               this, SLOT(wake()) );

    primaryAtChat()->chat( "ATE1" );
    secondaryAtChat()->chat( "ATE1" );
    smsAtChat()->chat( "ATE1" );

    primaryAtChat()->setRetryOnNonEcho( 30000 );
    secondaryAtChat()->setRetryOnNonEcho( 30000 );
    smsAtChat()->setRetryOnNonEcho( 30000 );

}

QAtChat *EzxModemService::smsAtChat() const
{
  return  d->smsAtChat;
}

void EzxModemService::initialize()
{


    if ( !supports<QVibrateAccessory>() )
        addInterface( new EZXVibrateAccessory( this ) );
    
    if ( !supports<QPinManager>() )
              addInterface( new EzxModemPinManager( this ) );

    if ( !supports<QNetworkRegistration>() )
          addInterface( new EzxModemNetworkRegistration( this ) );
    if ( !supports<QSimInfo>() )
          addInterface( new EzxModemSimInfo( this ) );
    if ( !supports<QSupplementaryServices>() )
          addInterface( new  EzxModemSupplementaryServices( this ) );

    if ( !supports<QSMSSender>() )
        addInterface( new EzxModemSMSSender( this ) );

    if ( !supports<QSMSReader>() )
        addInterface( new EzxModemSMSReader( this ) );

    if ( !callProvider() )
              setCallProvider( new EzxModemCallProvider( this ) );

    QModemService::initialize();
}


void EzxModemService::suspend()
{
    qWarning() << "modem suspend";
    primaryAtChat()->chat( "AT+CMER=0,0,0,0,0" );

    suspendDone();
}

void EzxModemService::wake()
{
    qWarning() << "modem wake";
    primaryAtChat()->chat( "AT+CMER=3,0,0,2,0" );

    wakeDone();
}
