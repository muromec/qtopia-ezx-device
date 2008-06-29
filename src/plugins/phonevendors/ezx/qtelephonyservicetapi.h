#ifndef QTELEPHONYSERVICETAPI_H
#define QTELEPHONYSERVICETAPI_H

#include <QModemService>
#include <qsupplementaryservicestapi.h>
#include "tapi.h"
class QTelephonyServiceTapi : public QModemService
{
    Q_OBJECT
public:
    QTelephonyServiceTapi( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~QTelephonyServiceTapi();

    void initialize();

private:
    QSupplementaryServicesTapi *supp;
    void voice_state( VOICE_CALL_STATUS *tapi_call);
    void incoming   ( VOICE_CALL_INFO   *tapi_call);
    void signal_quality(int quality);
    void ussd_response(USSD_RESPONSE *ussd);

public slots:
    void tapi_fd(int n);
    void SignalStrengthUpdate();
};

#endif
