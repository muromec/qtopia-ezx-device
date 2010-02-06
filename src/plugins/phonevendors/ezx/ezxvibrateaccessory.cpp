
#include <sys/ioctl.h>  
#include <fcntl.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>

#include "ezxvibrateaccessory.h"
#define VIB "/sys/class/leds/a1200::vibrator/brightness"
#define VIB_OFF "0"
#define VIB_ON "2"

#define WO (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)

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

    value = new QFile();
    value->setFileName(VIB);
    value->open(WO);

}

EZXVibrateAccessory::~EZXVibrateAccessory()
{
    value->close();
    delete value;
}

void EZXVibrateAccessory::setVibrateOnRing( const bool on )
{
    setVibrateNow(on);
}

void EZXVibrateAccessory::setVibrateNow( const bool on )
{

  QTextStream o(value);

  if(on)
    o << VIB_ON;
  else
    o << VIB_OFF;

  QVibrateAccessoryProvider::setVibrateNow( value );
}
