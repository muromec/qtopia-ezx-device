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

public slots:
    void sendSupplementaryServiceData( const QString& data );

private slots:
    void cusd( const QString& msg );

private:
    QModemService *service;
};


#endif
