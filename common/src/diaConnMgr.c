#include "osTypes.h"
#include "osTimer.h"
#include "osMemory.h"
#include "osSockAddr.h"

#include "transportIntf.h"

#include "diaMsg.h"
#include "diaConnState.h"
#include "diaConfig.h"
#include "diaConnMgr.h"
#include "diaIntf.h"



/* each program instance supports up to DIA_MAX_INTERFACE_NUM interfaces.  each interface supports active/stby peer groups.  Each active or stby group can have multiple peers.  Only when no active peer in a active group, the stby group is used.  As soon as at least one peer in a active group becomes active, any newly created session shall start to use the active peer in the active group. */


static osStatus_e diaConnAddNewPeer(diaConnIntf_t* pConnIntf, diaIntfInfo_t* intfInfo, diaConnProv_t* pConnProv);
static void diaConnTimerFunc(uint64_t timerId, void* ptr);



static diaConnIntf_t gDiaConnIntf[DIA_MAX_INTERFACE_NUM];
static uint8_t intfNum;


osStatus_e diaConnMgr_init()
{
	osStatus_e status = OS_STATUS_OK;

	intfNum = 0;

	for(int i=0; i<DIA_MAX_INTERFACE_NUM; i++)
	{
		osList_init(&gDiaConnIntf[i].priorityGroup.activePeerList);
		osList_init(&gDiaConnIntf[i].priorityGroup.waitingPeerList);
        osList_init(&gDiaConnIntf[i].stbyGroup.activePeerList);
        osList_init(&gDiaConnIntf[i].stbyGroup.waitingPeerList);
		gDiaConnIntf[i].priorityGroup.isPriorityGrp = true;
		gDiaConnIntf[i].priorityGroup.pNextPeer = gDiaConnIntf[i].priorityGroup.activePeerList.head;
		gDiaConnIntf[i].priorityGroup.pConnIntf = &gDiaConnIntf[i];
		gDiaConnIntf[i].stbyGroup.isPriorityGrp = false;
		gDiaConnIntf[i].stbyGroup.pNextPeer = gDiaConnIntf[i].stbyGroup.activePeerList.head;
		gDiaConnIntf[i].stbyGroup.pConnIntf = &gDiaConnIntf[i];
		gDiaConnIntf[i].intfState = DIA_CONN_INTF_STATE_NONE;
	}

	diaIntfInfo_t cxIfInfo = {-1, DIA_INTF_TYPE_CX};
	diaConnProv_t cxProv;
	diaConfig_getPeer(DIA_INTF_TYPE_CX, &cxProv.peerIpPort);
	cxProv.isPriority = true;
	cxProv.isEnabled = true;
	cxProv.isServer = true;
	diaConnProv(&cxIfInfo, &cxProv);

EXIT:
	return status;
}


osStatus_e diaConnMgr_onMsg(diaConnBlock_t* pDcb, diaTransportMsg_t* pTpMsg, diaMsgDecoded_t* pDiaDecoded)
{
	osStatus_e status = OS_STATUS_OK;
	if(!pTpMsg || !pDcb)
	{
		logError("null pointer, pTpMsg=%p, pDcb=%p.", pTpMsg, pDcb);
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
	}

    diaConnMsgType_e msgType = DIA_CONN_MSG_TYPE_NONE;
	if(pTpMsg->tpMsgType == DIA_TRANSPORT_MSG_TYPE_PEER_MSG)
	{
		debug("received peerMsg, cmdCode=%d, for pDcb(%p).", pDiaDecoded->cmdCode, pDcb);
		if(!pDiaDecoded)
		{
			logError("pDiaDecoded is NULL for DIA_TRANSPORT_MSG_TYPE_PEER_MSG.");
			status = OS_ERROR_INVALID_VALUE;
            goto EXIT;
        }

		switch(pDiaDecoded->cmdCode)
		{
			case DIA_CMD_CODE_CER:
				if(pDiaDecoded->cmdFlag & DIA_CMD_FLAG_REQUEST)
				{
					msgType = DIA_CONN_MSG_TYPE_RCV_CER;
				}
				else
				{
					if(pDiaDecoded->cmdFlag & DIA_CMD_FLAG_ERROR)
					{
						msgType = DIA_CONN_MSG_TYPE_RCV_ERROR_CEA;
					}
					else
					{
						msgType = DIA_CONN_MSG_TYPE_RCV_CEA;
					}
				}
				break;
			case DIA_CMD_CODE_DWR:
                if(pDiaDecoded->cmdFlag & DIA_CMD_FLAG_REQUEST)
                {
                    msgType = DIA_CONN_MSG_TYPE_RCV_DWR;
                }
                else
                {
 					msgType = DIA_CONN_MSG_TYPE_RCV_DWA;
				}
				break;
			case DIA_CMD_CODE_DPR:
                if(pDiaDecoded->cmdFlag & DIA_CMD_FLAG_REQUEST)
                {
                    msgType = DIA_CONN_MSG_TYPE_RCV_DPR;
                }
                else
                {
                    msgType = DIA_CONN_MSG_TYPE_RCV_DPA;
                }
                break;
			default:
				msgType = DIA_CONN_MSG_TYPE_RCV_TRAFFIC_MSG;
				break;
		}
	}
	else
	{
		debug("received connStatus(%d) for pDcb(%p).", pTpMsg->connStatusMsg.connStatus, pDcb);
		switch(pTpMsg->connStatusMsg.connStatus)
		{
			case TRANSPORT_STATUS_TCP_SERVER_OK:
			case TRANSPORT_STATUS_TCP_OK:
				msgType = DIA_CONN_MSG_TYPE_TRANSPORT_READY;
				pDcb->tcpFd = pTpMsg->connStatusMsg.fd;
				pDcb->tpInfo.tcpFd = pTpMsg->connStatusMsg.fd;
				break;
			case TRANSPORT_STATUS_TCP_FAIL:
    		case TRANSPORT_STATUS_FAIL:
				msgType = DIA_CONN_MSG_TYPE_TRANSPORT_DOWN;
				break;
			default:
				logInfo("received connStatus(%d), ignore.", pTpMsg->connStatusMsg.connStatus);
				break;
		}
	}

	diaConnState_onMsg(msgType, pDcb, pDiaDecoded, 0);

EXIT:
	return status;
}


uint64_t diaStartTimer(time_t msec, diaConnBlock_t* pDcb)
{
	return osStartTimer(msec, diaConnTimerFunc, pDcb);
}

 
osStatus_e diaConnProv(diaIntfInfo_t* pIntfInfo, diaConnProv_t* pConnProv)
{
	DEBUG_BEGIN
	osStatus_e status = OS_STATUS_OK;

	if(!pIntfInfo || !pConnProv)
	{
		logError("null pointer, pIntfInfo=%p, pConnProv=%p.", pIntfInfo, pConnProv);
		return OS_ERROR_NULL_POINTER;
	}

	if(pIntfInfo->intfId < 0)
	{
		if(intfNum >= DIA_MAX_INTERFACE_NUM)
		{
			logError("all diameter interfaces have been used up, max number=%d.", DIA_MAX_INTERFACE_NUM);
			status = OS_ERROR_INVALID_VALUE;
			goto EXIT;
		}

		pIntfInfo->intfId = intfNum++;
		diaConnAddNewPeer(&gDiaConnIntf[pIntfInfo->intfId], pIntfInfo, pConnProv);
	}
	else
	{
		if(pIntfInfo->intfId >= intfNum)
		{
			logError("app provided intfId(%d) has never been assigned.", pIntfInfo->intfId);
			status = OS_ERROR_INVALID_VALUE;
            goto EXIT;
        }

		//to-do, perform necessary sanity check to make sure nee peer matches with the existing peer
		diaConnAddNewPeer(&gDiaConnIntf[pIntfInfo->intfId], pIntfInfo, pConnProv);
	}

EXIT:
	DEBUG_END
	return status;
}


//a conn is down
osStatus_e diaconnMgr_notifyFailover(diaConnBlock_t* pDcb)
{
	osStatus_e status = OS_STATUS_OK;

	if(!pDcb)
	{
		logError("null pointer, pDcb.");
		return OS_ERROR_NULL_POINTER;
	}

	//remove from the original list, which must be in activePeerList of either priorityGroup or stbyGroup	
	osList_unlinkElement(pDcb->listInfo.pLE);
	//add to the waitingPeerList of either priorityGroup or stbyGroup
	osList_appendLE(&pDcb->listInfo.pPeerGrp->waitingPeerList, pDcb->listInfo.pLE, NULL);

   	if(pDcb->listInfo.pPeerGrp->isPriorityGrp)
    {
		if(!pDcb->listInfo.pPeerGrp->pNextPeer && pDcb->listInfo.pPeerGrp->pConnIntf->intfState == DIA_CONN_INTF_STATE_USE_PRIORITY)
		{
			pDcb->listInfo.pPeerGrp->pConnIntf->intfState = DIA_CONN_INTF_STATE_USE_STBY;
		}
    }
    else
    {
		if(!pDcb->listInfo.pPeerGrp->pNextPeer && pDcb->listInfo.pPeerGrp->pConnIntf->intfState == DIA_CONN_INTF_STATE_USE_STBY)
        {
           	pDcb->listInfo.pPeerGrp->pConnIntf->intfState = DIA_CONN_INTF_STATE_NONE;
        }
    }
}


//a conn is established, ready to take traffic.
//be noted "ready to take traffic" barely means it can be moved out of waiting peerList. does not mean it will take traffic.  the other criteria is priority/stby.
osStatus_e diaconnMgr_notifyFailback(diaConnBlock_t* pDcb)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        return OS_ERROR_NULL_POINTER;
    }

    //remove from the original list, which must be in waitingPeerList of either priorityGroup or stbyGroup
    osList_unlinkElement(pDcb->listInfo.pLE);
    //add to the activePeerList of either priorityGroup or stbyGroup
    osList_appendLE(&pDcb->listInfo.pPeerGrp->activePeerList, pDcb->listInfo.pLE, NULL);

	if(pDcb->listInfo.pPeerGrp->isPriorityGrp)
	{
		pDcb->listInfo.pPeerGrp->pConnIntf->intfState = DIA_CONN_INTF_STATE_USE_PRIORITY;
	}
	else
	{
		if(pDcb->listInfo.pPeerGrp->pConnIntf->intfState == DIA_CONN_INTF_STATE_NONE)	
		{
			pDcb->listInfo.pPeerGrp->pConnIntf->intfState = DIA_CONN_INTF_STATE_USE_STBY;
		}
	}

	return status;
}


transportStatus_e diaConnMgr_startConn(struct diaConnBlock* pDcb)
{
	if(!pDcb)
	{
		return TRANSPORT_STATUS_FAIL;
	}

	
	pDcb->tpInfo.isCom = true;
	pDcb->tpInfo.tpType = TRANSPORT_TYPE_TCP;
	pDcb->tpInfo.tcpFd = -1;
#if 0
	char ip[INET_ADDRSTRLEN];
	if(osConvertntoPL(&pDcb->peer, (osIpPort_t*)&tpInfo.peer, ip) != OS_STATUS_OK)
	{
		logError("fails to osConvertntoPL for peer(%r:%d).", &tpInfo.peer.ip, tpInfo.peer.port);
		return TRANSPORT_STATUS_FAIL;
	}
#endif
	diaConfig_getHost1(&pDcb->tpInfo.local); 
	pDcb->tpInfo.protocolUpdatePos = 0;
 
	return transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, NULL, &pDcb->tcpFd);
}	


osStatus_e diaConnMgr_stopConn(diaConnBlock_t* pDcb)
{
    if(!pDcb || pDcb->tcpFd == -1)
    {
        return TRANSPORT_STATUS_FAIL;
    }

	return transport_closeTcpConn(pDcb->tcpFd, true);
}	
	

diaConnBlock_t* diaConnGetActiveDcb(diaIntfInfo_t intfInfo)
{
	diaConnBlock_t* pDcb = NULL;

	if(intfInfo.intfId >= intfNum)
	{
		logError("app provided intfId(%d) is not assigned by connMgr.", intfInfo.intfId);
		goto EXIT;
	}

	diaConnIntf_t* pDiaConnIntf	= &gDiaConnIntf[intfInfo.intfId];
	switch(pDiaConnIntf->intfState)
	{
		case DIA_CONN_INTF_STATE_NONE:
			goto EXIT;
    	case DIA_CONN_INTF_STATE_USE_PRIORITY:
			if(pDiaConnIntf->priorityGroup.pNextPeer)
			{
				pDcb = pDiaConnIntf->priorityGroup.pNextPeer->data;
				pDiaConnIntf->priorityGroup.pNextPeer = osList_getNextElement(pDiaConnIntf->priorityGroup.pNextPeer);
			}
			break;
    	case DIA_CONN_INTF_STATE_USE_STBY:
			if(pDiaConnIntf->stbyGroup.pNextPeer)
			{
            	pDcb = pDiaConnIntf->stbyGroup.pNextPeer->data;
            	pDiaConnIntf->stbyGroup.pNextPeer = osList_getNextElement(pDiaConnIntf->stbyGroup.pNextPeer);
			}
            break;
		default:
			logError("intfState(%d) not handled.", pDiaConnIntf->intfState);
			break;
	}

EXIT:
	return pDcb;
}



diaConnBlock_t* diaConnMgr_getDcb(struct sockaddr_in* peer)
{
	diaPeerGroup_t* pPeerGrp;
	osListElement_t* pLE;
	diaConnBlock_t* pDcb = NULL;
	for(int i=0; i<intfNum; i++)
	{
		for(int j=0; j<2; j++)
		{
			//check priorityGroup and stbyGrp in turn, check priorityGrp first
			pPeerGrp = (j==0 ? &gDiaConnIntf[i].priorityGroup : &gDiaConnIntf[i].stbyGroup);

			for(int k=0; k<2; k++)
			{	
				//within peerGrp, check waitingPeer first, then activePeer
				pLE = (k==0 ? pPeerGrp->waitingPeerList.head : pPeerGrp->activePeerList.head);

				if(pLE)
				{
					pDcb = pLE->data;
					if(pDcb)
					{
						if(osIsSameSA(&pDcb->tpInfo.peer, peer))
						{
							goto EXIT;
						}
					}

					pLE = pLE->next;
				} 
			} //for(int k=0; k<2; k++)
		} //for(int j=0; j<2; j++)
	} //for(int i=0; i<intfNum; i++)

EXIT:
	return pDcb;
}


static diaConnIntf_t gDiaConnIntf[DIA_MAX_INTERFACE_NUM];
static uint8_t intfNum;
	

static osStatus_e diaConnAddNewPeer(diaConnIntf_t* pConnIntf, diaIntfInfo_t* pIntfInfo, diaConnProv_t* pConnProv)
{
	osStatus_e status = OS_STATUS_OK;
	if(!pConnIntf || !pIntfInfo || !pConnProv)
    {
        logError("null pointer, pConnIntf=%p, pIntfInfo=%p, pConnProv=%p.", pConnIntf, pIntfInfo, pConnProv);
        return OS_ERROR_NULL_POINTER;
    }

	pConnIntf->intfInfo = *pIntfInfo;
	pConnIntf->isServer = pConnProv->isServer;

	diaPeerGroup_t* pPeerGrp = pConnProv->isPriority ? &pConnIntf->priorityGroup : &pConnIntf->stbyGroup;		
	diaConnBlock_t* pDcb = osmalloc(sizeof(diaConnBlock_t), NULL);
	if(!pDcb)
	{
		logError("fails to osmalloc for pDcb.");
		return OS_ERROR_SYSTEM_FAILURE;
	}

	pDcb->ifType = pIntfInfo->intfType;
	osConvertPLton(&pConnProv->peerIpPort, true, &pDcb->tpInfo.peer);
	diaConfig_getHost1(&pDcb->tpInfo.local);
//	pDcb->peerIpPort = pConnProv->peerIpPort;
	pDcb->listInfo.pLE = osList_append(&pPeerGrp->waitingPeerList, pDcb);
	pDcb->listInfo.pPeerGrp = pPeerGrp;

	diaConnState_init(pDcb, pConnProv->isServer, pConnProv->isEnabled);

EXIT:
	return status;
}	
		
			
static void diaConnTimerFunc(uint64_t timerId, void* ptr)
{
    diaConnState_onMsg(DIA_CONN_MSG_TYPE_TIMEOUT, (diaConnBlock_t*)ptr, NULL, timerId);
}


