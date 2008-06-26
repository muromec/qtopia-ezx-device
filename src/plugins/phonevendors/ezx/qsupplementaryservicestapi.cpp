#include "qsupplementaryservicestapi.h"
#include "tapi.h"
#include "stdio.h"


/*
 * Supplementary Services
 *
 *
 */

QSupplementaryServicesTapi::QSupplementaryServicesTapi
        ( QModemService *service )
    : QSupplementaryServices( service->service(), service, QCommInterface::Server )
{
    this->service = service;
}

QSupplementaryServicesTapi::~QSupplementaryServicesTapi()
{
}




void QSupplementaryServicesTapi::sendSupplementaryServiceData 
  ( const QString& data )  
{



  int ret;
  USSD_REQUEST ussd;

  
  memset( ussd.request, 0, 400 );

  //ussd.request = data.utf16();
  ussd.len     = sizeof(unsigned short int) * (data.length() +1);
  memcpy(ussd.request, (unsigned char*)data.utf16(), ussd.len); 

  ret = TAPI_USSD_MakeRequest(&ussd);
  printf("ussd: %s, ret:%d\n",data.toAscii().constData(),ret);



}


void QSupplementaryServicesTapi::cusd ( const QString& msg )
{

  emit unstructuredNotification( 
      QSupplementaryServices::NoFurtherActionRequired,
      msg 
  );
}



