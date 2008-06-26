#include "qnetworkregistrationtapi.h"
#include "QTimer"
#include "stdio.h"
#include "tapi.h"

/*
 *
 *
 *
 * NETWORK
 *
 *
 */
QNetworkRegistrationTapi::QNetworkRegistrationTapi
        ( const QString& service, QObject *parent )
    : QNetworkRegistrationServer( service, parent )
{
  // TODO: find matching tapi calls?
    QTimer::singleShot( 500, this, SLOT(searching()) );
    QTimer::singleShot( 5000, this, SLOT(home()) );
    QTimer::singleShot( 200, this, SLOT(initDone()) );

    printf("registration: none\n");
    updateRegistrationState( QTelephony::RegistrationNone );
}

QNetworkRegistrationTapi::~QNetworkRegistrationTapi()
{
}

void QNetworkRegistrationTapi::setCurrentOperator
        ( QTelephony::OperatorMode, const QString&, const QString&)
{
  // TODO: somewho uses this?
    emit setCurrentOperatorResult( QTelephony::OK );
}

/*
 * Are next 4 methods hacks for dummy modem or not?
 */
void QNetworkRegistrationTapi::requestAvailableOperators()
{
  //  TODO: call TAPI_NETWORK_GetAvailableNetwork() here
  //  and catch fucking async answer in tapi_fd()

    QList<QNetworkRegistration::AvailableOperator> opers;
    QNetworkRegistration::AvailableOperator oper;
    oper.availability = QTelephony::OperatorAvailable;
    oper.name = "Qtopia";       // No tr
    oper.id = "0Qtopia";       // No tr
    oper.technology = "GSM";       // No tr
    opers.append( oper );

    emit availableOperators( opers );
}

void QNetworkRegistrationTapi::searching()
{
    printf("registration: search\n");

    updateRegistrationState( QTelephony::RegistrationSearching );
}

void QNetworkRegistrationTapi::home()
{
    SP_NAME opName;
    signed char   mcc[4];
    signed char   mnc[4];

    char netid[8];


    // get opname and network codes
    TAPI_NETWORK_GetServiceProviderName( &opName );
    TAPI_NETWORK_GetCurrentNetworkId( mcc, mnc );

    // formar network id
    sprintf(netid,"%s-%s",mcc,mnc);
        
    // decode opname
    unsigned short int* p;
    p = (unsigned short int*)&(opName.name);
    char opname[25];
    
    for(int i = 0; i < 25; i++)
    {
       /*if(*p == 0)
               break;*/
       opname[i] = ((int)*p);
       p++;
    }      


    // FIXME: no test conditions
    updateRegistrationState( QTelephony::RegistrationHome );

    // set discovered data
    updateCurrentOperator( QTelephony::OperatorModeAutomatic,
                           netid, opname, "GSM" );    
    printf("registration: home: %s, %s-%s\n", opname, mcc, mnc);
}

void QNetworkRegistrationTapi::initDone()
{
    updateInitialized( true );
}


