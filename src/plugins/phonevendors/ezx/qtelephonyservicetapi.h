#ifndef QTELEPHONYSERVICETAPI_H
#define QTELEPHONYSERVICETAPI_H

#include <QModemService>
class QTelephonyServiceTapi : public QModemService
{
    Q_OBJECT
public:
    QTelephonyServiceTapi( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~QTelephonyServiceTapi();

    void initialize();
public slots:
    void tapi_fd(int n);
    void SignalStrengthUpdate();
};

#endif
