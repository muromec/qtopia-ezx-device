#include "qsiminfotapi.h"
#include "tapi.h"

/*
 *
 *
 * SIM
 *
 */
QSimInfoTapi::QSimInfoTapi( const QString& service, QObject *parent )
    : QSimInfo( service, parent, QCommInterface::Server )
{

    // get sim info from tapi

    unsigned char imssi[16];


    if ( TAPI_ACCE_GetImsi(imssi) )
    {
      emit notInserted ();
    }
    else
    {
      setIdentity( (char*)imssi );
      emit inserted();
    }
}

QSimInfoTapi::~QSimInfoTapi()
{
}

