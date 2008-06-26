
#include <sys/ioctl.h>  
#include <fcntl.h>
#include <unistd.h>

#include "ezxvibrateaccessory.h"
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
