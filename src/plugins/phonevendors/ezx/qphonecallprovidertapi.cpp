#include "qphonecallprovidertapi.h"
#include "qphonecalltapi.h"

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

QPhoneCallImpl *QPhoneCallProviderTapi::create
        ( const QString& identifier, const QString& callType )
{
  // maby hack qtopia ident here?
  // ... or not. id mapping works whell
    return new QPhoneCallTapi( this, identifier, callType );
}

