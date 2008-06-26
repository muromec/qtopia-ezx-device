#include "qphonerffunctionalitytapi.h"
#include "stdio.h"

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

