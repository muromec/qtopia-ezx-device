#ifndef QNETWORKREGISTRATIONTAPI_H
#define QNETWORKREGISTRATIONTAPI_H

#include <qnetworkregistration.h>
class QNetworkRegistrationTapi : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    QNetworkRegistrationTapi( const QString& service, QObject *parent );
    ~QNetworkRegistrationTapi();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

private slots:
    void searching();
    void home();
    void initDone();
};



#endif
