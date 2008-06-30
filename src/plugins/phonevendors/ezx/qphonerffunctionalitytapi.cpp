#include "qphonerffunctionalitytapi.h"
#include "tapi.h"

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
    // force pin request here?
    int level;
    TAPI_ACCE_GetRfMode(&level);
    if (level)
      setValue( "level", qVariantFromValue( DisableTransmitAndReceive ) );
    else
      setValue( "level", qVariantFromValue( Full ) );

    emit levelChanged();
}

void QPhoneRfFunctionalityTapi::setLevel( QPhoneRfFunctionality::Level level )
{
  if (level == 1)
    TAPI_ACCE_SetRfMode(0);
  else
    TAPI_ACCE_SetRfMode(1);
}

