#ifndef EZXMODEMSERVICE_H
#define EZXMODEMSERVICE_H

#include <QModemService>
class EzxModemServicePrivate;
class EzxModemService : public QModemService
{
    Q_OBJECT
public:
    EzxModemService( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~EzxModemService();
    QAtChat *smsAtChat() const;

    void initialize();
private slots:
    void init( QSerialIODeviceMultiplexer *mux ); 

protected slots:
    virtual void suspend();
    virtual void wake();

private:
    EzxModemServicePrivate *d;
};

#endif
