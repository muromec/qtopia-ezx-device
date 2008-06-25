#ifndef QPHONECALLTAPI_H
#define QPHONECALLTAPI_H

#include <QPhoneCallImpl>

class QPhoneCallTapi : public QPhoneCallImpl
{
    Q_OBJECT
public:
    QPhoneCallTapi( QPhoneCallProvider *provider, const QString& identifier,
                     const QString& callType );
    ~QPhoneCallTapi();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private slots:
    void dialTimeout();
    void tapi_fd();
};


#endif
