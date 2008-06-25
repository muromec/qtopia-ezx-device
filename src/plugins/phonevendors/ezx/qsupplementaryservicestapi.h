#ifndef QSUPPLEMENTARYSERVICESTAPI_H
#define QSUPPLEMENTARYSERVICESTAPI_H

#include <qsupplementaryservices.h>
#include <QModemService>


class QSupplementaryServicesTapi
            : public QSupplementaryServices
{
    Q_OBJECT
public:
    explicit QSupplementaryServicesTapi( QModemService *service );
    ~QSupplementaryServicesTapi();
    void cusd( const QString& msg );

public slots:
    void sendSupplementaryServiceData( const QString& data );


private:
    QModemService *service;
};


#endif
