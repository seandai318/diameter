#ifndef _DIA_INTF_H
#define _DIA_INTF_H


#include "osTypes.h"
#include "osSockAddr.h"

#include "diaTransportIntf.h"


typedef enum {
    DIA_INTF_TYPE_CX,
    DIA_INTF_TYPE_GX,
    DIA_INTF_TYPE_RX,
    DIA_INTF_TYPE_SH,
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



void dia_init();
osStatus_e diaConnProv(diaIntfInfo_t* pIntfInfo, diaConnProv_t* pConnProv);
void diaMgr_onMsg(diaTransportMsg_t* pTpMsg);


#endif
