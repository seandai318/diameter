#ifndef _DIA_CONN_STATE_H
#define _DIA_CONN_STATE_H


#include <netinet/in.h>

#include "osTypes.h"

#include "transportIntf.h"

#include "diaMsg.h"
#include "diaConnMgr.h"
#include "diaIntf.h"
 

typedef enum {
	DIA_CONN_MSG_TYPE_NONE,
	DIA_CONN_MSG_TYPE_TIMEOUT,
	DIA_CONN_MSG_TYPE_TRANSPORT_READY,
	DIA_CONN_MSG_TYPE_TRANSPORT_DOWN,
	DIA_CONN_MSG_TYPE_FORCE_CONN,	//user enable connection
	DIA_CONN_MSG_TYPE_STOP_CONN,	//user disable connection
	DIA_CONN_MSG_TYPE_RCV_CER,
	DIA_CONN_MSG_TYPE_RCV_ERROR_CEA,
	DIA_CONN_MSG_TYPE_RCV_CEA,
	DIA_CONN_MSG_TYPE_RCV_DWR,
	DIA_CONN_MSG_TYPE_RCV_DWA,
	DIA_CONN_MSG_TYPE_RCV_DPR,
	DIA_CONN_MSG_TYPE_RCV_DPA,
	DIA_CONN_MSG_TYPE_RCV_TRAFFIC_MSG,
} diaConnMsgType_e;


typedef enum {
    DIA_CONN_STATE_CLIENT_CLOSED,       //only for client SM
    DIA_CONN_STATE_CLIENT_WAIT_CEA,     //only for client SM
    DIA_CONN_STATE_SERVER_CLOSED,
    DIA_CONN_STATE_OPEN,
    DIA_CONN_STATE_SUSPECT,
    DIA_CONN_STATE_DOWN,
    DIA_CONN_STATE_REOPEN,
    DIA_CONN_STATE_CLOSING,
} diaConnState_e;



typedef struct diaConnBlock {
    diaConnState_e connState;
	diaIntfType_e ifType;
	int tcpFd;
	int8_t  numDwa;					//how many continuous DWA has been received
	bool isWaitDwa;					//is waiting for a DWA due to the same timer is used for sending DWR an wait for DWA
	bool isServer;					//if function as diam server, or client
	diaNotifyApp_h diaNotifyApp;	//notify app when receiving peer diameter message, or status
	struct {
		diaPeerGroup_t* pPeerGrp;	//link to the pPeerGrp in diaConnIntf_t
		osListElement_t* pLE;		//pLE in the peerGroup list
	} listInfo;
	transportInfo_t tpInfo;
	osVPointerLen_t peerHost;		//filled when getting a successful CEA
	osVPointerLen_t peerRealm;		//filled when getting a successful CEA
//	struct sockaddr_in peer;
	diaCmdHdrInfo_t cmdHdrInfo;
//	transportIpPort_t peerIpPort;
	uint64_t timerId_tpWaitConn; 	//issue a conn command to TP, wait connection status response
	uint64_t timerId_tpRetryConn;	//timer to retry next conn
	uint64_t timerId_watchdog;		//timer to start DWR, and wait for DWA
	uint64_t timerId_twt;			//timer to allow delivery of a message to a peer
} diaConnBlock_t;



typedef struct diaConnMsg {
    diaCmdCode_e diaMsgType;
    diaMsgDecoded_t diaDecodedMsg;
    diaConnBlock_t* pDcb;
} diaConnMsg_t;




osStatus_e diaConnState_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
void diaConnState_init(diaConnBlock_t* pDcb, bool isServer, bool isEnabled);


#endif
