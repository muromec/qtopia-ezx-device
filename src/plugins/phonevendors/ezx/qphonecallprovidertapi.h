#ifndef QPHONECALLPROVIDERTAPI_H
#define QPHONECALLPROVIDERTAPI_H

#include <QPhoneCallProvider>

class QPhoneCallProviderTapi : public QPhoneCallProvider
{
    Q_OBJECT
public:
    QPhoneCallProviderTapi( const QString& service, QObject *parent );
    ~QPhoneCallProviderTapi();

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
};


#endif
