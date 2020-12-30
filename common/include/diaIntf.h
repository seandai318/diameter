#ifndef _DIA_INTF_H
#define _DIA_INTF_H


#include "osTypes.h"
#include "osSockAddr.h"

#include "diaTransportIntf.h"
#include "diaMsg.h"


typedef enum {
    DIA_INTF_TYPE_CX,
    DIA_INTF_TYPE_GX,
    DIA_INTF_TYPE_RX,
    DIA_INTF_TYPE_SH,
	DIA_INTF_TYPE_INVALID,
} diaIntfType_e;


typedef struct diaIntfInfo {
    int8_t intfId;              //if -1, means diaConn need to allocate a intfId to app, otherwise, use what app provided
    diaIntfType_e intfType;
} diaIntfInfo_t;



typedef struct diaConnProv {
	osIpPort_t peerIpPort;
	bool isPriority;	//if yes, th peer is in higher priority group
	bool isEnabled;
	bool isServer;
} diaConnProv_t;


		
//to-do to refine when working on app/diam interface
typedef void (*diaNotifyApp_h)(diaMsgDecoded_t* pDecoded, void* appData);

struct diaConnBlock;

void dia_init();
osStatus_e diaConn_initIntf(diaIntfType_e intf);
osStatus_e diaConnProv(diaIntfInfo_t* pIntfInfo, diaConnProv_t* pConnProv);
void diaMgr_onMsg(diaTransportMsg_t* pTpMsg);
//diaConnBlock_t* if pDcb is NULL, the function will try to find one based on the intfType
osStatus_e diaSendAppMsg(diaIntfType_e intfType, struct diaConnBlock* pDcb, osMBuf_t* pMBuf, osPointerLen_t* pSessId, diaNotifyApp_h appCallback, void* appData);
struct sockaddr_in* diaConnGetActiveDest(diaIntfType_e intfType);


#endif
