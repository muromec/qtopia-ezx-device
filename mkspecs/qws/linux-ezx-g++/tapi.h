#ifndef TAPI_H
#define TAPI_H

// battary info
typedef struct  _EZX_BATT_INF
{
    unsigned char   bcs;
    unsigned char   bcl;
} __attribute__( (packed) )  EZX_BATT_INF;

typedef struct  _SP_NAME
{
    unsigned char              b;
    unsigned short int    name[25];
} SP_NAME;


typedef struct _SQ
{
  unsigned char rssi;
  unsigned char err;
} __attribute__( (packed) )  SQ;


typedef struct _TAPI_MSG
{
  unsigned short int id;
  unsigned short int len;
  unsigned char* body;
} __attribute__( (packed) )   TAPI_MSG;

typedef struct _VOICE_CALL_STATUS
{
 unsigned char cid;
 int status;
 int pid;
} VOICE_CALL_STATUS;

typedef struct VOICE_CALL_INFO
{
  unsigned char cid;
  int type;
  unsigned char phone;
  int line;
  unsigned char a[82] ;
} VOICE_CALL_INFO;





extern "C" {

// connect to tapisrv
unsigned int TAPI_CLIENT_Init(
    const unsigned short int*     a,
    unsigned char                 b ) ;
// parse async message
int TAPI_CLIENT_ReadAndMallocMsg(signed int fd, TAPI_MSG* msg);  

// power 
int TAPI_BATTERY_GetBatteryChargeInfo (EZX_BATT_INF* a);
int TAPI_BATTERY_GetStatus (int* a);
int TAPI_BATTERY_GetChargerConnectionStatus(int* a);

// voice 
int TAPI_VOICE_MakeCall   (unsigned char num[42],  unsigned char* cid);
int TAPI_VOICE_RejectCall (unsigned char cid,  int type); 
int TAPI_VOICE_DropCurrentCall (unsigned char cid);
int TAPI_VOICE_AnswerCall (unsigned char cid,  int type); 

// net
int TAPI_NETWORK_GetServiceProviderName( SP_NAME* name);
int TAPI_NETWORK_GetCurrentNetworkId(
       signed char mcc[4],
       signed char mnc[4] ); 
// pin 
int TAPI_SECURITY_GetPin1Status( 
    const unsigned char type, unsigned char* status);

int TAPI_SECURITY_SetPin1Status(
    const signed char pass[21],
    const unsigned char   mode); 
// acce
int TAPI_ACCE_GetSiginalQuality(SQ* quality);

}


#endif
