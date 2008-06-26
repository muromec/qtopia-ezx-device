#include "qsiminfotapi.h"

/*
 *
 *
 * SIM
 *
 */
QSimInfoTapi::QSimInfoTapi( const QString& service, QObject *parent )
    : QSimInfo( service, parent, QCommInterface::Server )
{
    setIdentity( "9876543210" );
}

QSimInfoTapi::~QSimInfoTapi()
{
}

