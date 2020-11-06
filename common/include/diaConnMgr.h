#ifndef _DIA_CONN_MGR_H
#define _DIA_CONN_MGR_H


#include <netinet/in.h>

#include "osTypes.h"

#include "transportIntf.h"
#include "diaTransportIntf.h"

#include "diaMsg.h"
#include "diaIntf.h"



//a interface may have up to 2 peer list/group: priority group and stby group. 
//within a group, some peers are connected(activePeerList), some peers are not connected(waitingPeerList),
//traffic always uses priority group connection unless no active connection in the group, then the stby group connection will be used
typedef struct diaPeerGroup {
	bool isPriorityGrp;			
	osList_t activePeerList;	//each element contains pDcb that can take traffic
	osList_t waitingPeerList;	//each element contains pDcb that does not have conn up to take traffic
	osListElement_t* pNextPeer;	//the next peer to send a new session request
	struct diaConnIntf* pConnIntf;
} diaPeerGroup_t;


typedef enum diaConnIntfState {
	DIA_CONN_INTF_STATE_NONE,
	DIA_CONN_INTF_STATE_USE_PRIORITY,
	DIA_CONN_INTF_STATE_USE_STBY,
} diaConnIntfState_e;



typedef struct diaConnIntf {
	diaPeerGroup_t priorityGroup;
	diaPeerGroup_t stbyGroup;
	diaIntfInfo_t intfInfo;
	diaConnIntfState_e intfState;
	bool isServer;
} diaConnIntf_t;



struct diaConnBlock;


transportStatus_e diaConnMgr_startConn(struct diaConnBlock* pDcb);
osStatus_e diaSendCommonMsg(diaIntfType_e intfType, struct diaConnBlock* pDcb, diaCmdCode_e cmd, bool isReq, diaResultCode_e resultCode);
//osStatus_e diaSendAppMsg(diaIntfType_e intfType, osMBuf_t* pBuf);
osStatus_e diaConnMgr_stopConn(struct diaConnBlock* pDcb);  //close the current connection
osStatus_e diaconnMgr_notifyFailover(struct diaConnBlock* pDcb);
osStatus_e diaconnMgr_notifyFailback(struct diaConnBlock* pDcb);
osStatus_e diaConnMgr_onMsg(struct diaConnBlock* pDcb, diaTransportMsg_t* pTpMsg, diaMsgDecoded_t* pDiaDecoded);
uint64_t diaStartTimer(time_t msec, struct diaConnBlock* pDcb);
struct diaConnBlock* diaConnGetActiveDcbByIntf(diaIntfType_e intfType);
struct diaConnBlock* diaConnMgr_getDcb(struct sockaddr_in* peer);
osStatus_e diaConnMgr_init();

#endif
