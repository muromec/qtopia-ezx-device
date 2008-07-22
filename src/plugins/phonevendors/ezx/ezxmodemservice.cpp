#include "ezxvibrateaccessory.h"
#include "ezxmodemservice.h"
#include "ezxbattery.h"
#include "ezxmodempinmanager.h"
#include "ezxmodemnetworkregistration.h"
#include "ezxmodemsiminfo.h"
#include "ezxmodemsupplementaryservices.h"
#include "QSerialPort"  



EzxModemService::EzxModemService
        ( const QString& service, QSerialIODeviceMultiplexer *mux, QObject *parent )
    : QModemService( service, mux, parent )
{


}
EzxModemService::~EzxModemService()
{
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

    EzxBattery* bat;
    bat = new EzxBattery ( this  );

    QModemService::initialize();
}



