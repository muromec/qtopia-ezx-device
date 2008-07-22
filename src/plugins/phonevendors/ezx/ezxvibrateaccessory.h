#ifndef EZXVIBRATEACCESSORY_H
#define EZXVIBRATEACCESSORY_H

#define EZX_VIBRATOR_IOCTL_BASE 0xbb
#define EZX_VIBRATOR_ENABLE     _IOW (EZX_VIBRATOR_IOCTL_BASE,1,int)
#define EZX_VIBRATOR_DISABLE    _IO (EZX_VIBRATOR_IOCTL_BASE,2)


#include <qvibrateaccessory.h>
#include <QModemService>
class  EZXVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
     EZXVibrateAccessory( QModemService *service );
    ~ EZXVibrateAccessory();

public slots:
    void setVibrateNow( const bool value );
    void setVibrateOnRing( const bool value );
};


#endif
