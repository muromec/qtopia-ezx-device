#ifndef QPHONERFFUNCTIONALITYTAPI_H
#define QPHONERFFUNCTIONALITYTAPI_H

#include <qphonerffunctionality.h>
class QPhoneRfFunctionalityTapi : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    QPhoneRfFunctionalityTapi( const QString& service, QObject *parent );
    ~QPhoneRfFunctionalityTapi();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );
};

#endif
