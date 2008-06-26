#ifndef QPINMANAGERTAPI_H
#define QPINMANAGERTAPI_H

#include <qpinmanager.h>
class QPinManagerTapi : public QPinManager
{
    Q_OBJECT
public:
    QPinManagerTapi( const QString& service, QObject *parent );
    ~QPinManagerTapi();
private:
    signed char tpin[16];

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );
};


#endif
