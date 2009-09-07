
#include <sys/ioctl.h>  
#include <fcntl.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>

#include "ezxvibrateaccessory.h"
#define VIB "/sys/bus/platform/devices/reg-virt-consumer.0/"
#define VIB_OFF "0"
#define VIB_ON "2000000"

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

    QFile *max = new QFile();
    max->setFileName(VIB"max_microvolts");
    max->open(WO);

    QTextStream o(max);
    o << VIB_ON;

    max->close();
    delete max;

    value = new QFile();
    value->setFileName(VIB"min_microvolts");
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
