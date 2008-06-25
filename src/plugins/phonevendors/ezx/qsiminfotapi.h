#ifndef QSIMINFOTAPI_H
#define QSIMINFOTAPI_H

#include <qsiminfo.h>
class QSimInfoTapi : public QSimInfo
{
    Q_OBJECT
public:
    QSimInfoTapi( const QString& service, QObject *parent );
    ~QSimInfoTapi();
};


#endif
