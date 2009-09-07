#ifndef EZXVIBRATEACCESSORY_H
#define EZXVIBRATEACCESSORY_H

#include <qvibrateaccessory.h>
#include <QModemService>
#include <QFile>

class  EZXVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
     EZXVibrateAccessory( QModemService *service );
    ~ EZXVibrateAccessory();

public slots:
    void setVibrateNow( const bool value );
    void setVibrateOnRing( const bool value );

private:
    QFile *value;
};


#endif
