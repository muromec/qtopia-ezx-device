#include "qpinmanagertapi.h"
#include "tapi.h"
#include "stdio.h"

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

