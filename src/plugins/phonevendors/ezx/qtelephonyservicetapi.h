#ifndef QTELEPHONYSERVICETAPI_H
#define QTELEPHONYSERVICETAPI_H

#include <QModemService>
#include <qsupplementaryservicestapi.h>
class QTelephonyServiceTapi : public QModemService
{
    Q_OBJECT
public:
    QTelephonyServiceTapi( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~QTelephonyServiceTapi();

    void initialize();

private:
    QSupplementaryServicesTapi *supp;

public slots:
    void tapi_fd(int n);
    void SignalStrengthUpdate();
};

#endif
