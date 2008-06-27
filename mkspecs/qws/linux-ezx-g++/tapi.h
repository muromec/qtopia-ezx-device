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
  unsigned char phone[42];
  int line;
  unsigned char a[82] ;
} VOICE_CALL_INFO;


typedef struct _USSD_REQUEST 
{
  unsigned char len;
  unsigned char request[400];
} USSD_REQUEST;

typedef struct _USSD_RESPONSE
{
  unsigned short int id;
  int type;
  unsigned short int len;
  unsigned char text[364];
} USSD_RESPONSE;

typedef struct _PHONEBOOK_ENTRY
{
  unsigned short int index;
  unsigned char number[42];
  unsigned char x[82]; // FIXME wtf is this?
  unsigned char type; 
} __attribute__( (packed) )  PHONEBOOK_ENTRY;



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
int TAPI_VOICE_HoldCall (unsigned char cid); 
int TAPI_VOICE_JoinCall (
    unsigned char cid,
    unsigned char cid);
int TAPI_VOICE_RetrieveCall (unsigned char cid);
int TAPI_VOICE_MakeDtmfTone (
    unsigned char dtfmChar,
    int state);
int TAPI_VOICE_TransferCall (
    unsigned char cid,
    unsigned char num[42],
    unsigned char* newCid);
int TAPI_VOICE_DropAllCall(void);
      


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

// ussd FIXME
int TAPI_USSD_MakeRequest ( USSD_REQUEST* msg);

// phonebook
int TAPI_PHONEBOOK_GetEntryList ( 
    unsigned short int start,
    unsigned short int end,
    PHONEBOOK_ENTRY* book );


}


#endif
