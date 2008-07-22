#ifndef EZXMODEMSERVICE_H
#define EZXMODEMSERVICE_H

#include <QModemService>
class EzxModemService : public QModemService
{
    Q_OBJECT
public:
    EzxModemService( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~EzxModemService();

    void initialize();


};

#endif
