#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "osHexDump.h"
#include "osMemory.h"
#include "osHash.h"
#include "osTimer.h"

#include "diaConfig.h"
#include "diaTransportIntf.h"
#include "diaConnState.h"
#include "diaConnMgr.h"
#include "diaBaseCer.h"
#include "diaBaseDwr.h"


//the number of seconds since the start of the Unix epoch at ~20200426, UTC 16:15:00
#define DIA_BASE_CER_ORIG_STATE_EPOCH_VALUE 0x5ea5cfa4      //1587924900


typedef struct {	
	void* appData;
	diaNotifyApp_h appCallback;
	uint64_t waitRespTimerId;
	diaConnBlock_t* pDcb;
} diaHashData_t;


static void dia_onTimeout(uint64_t timerId, void* ptr);
static void diaHashData_cleanup(void* data);

static osHash_t* diaHash;
static uint32_t diaOrigStateId;



void dia_init(char* diaConfigFolder)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    diaOrigStateId = tp.tv_sec - DIA_BASE_CER_ORIG_STATE_EPOCH_VALUE;

	diaConfig_init(diaConfigFolder);

	diaHash = osHash_create(DIA_HASH_SIZE);
    if(!diaHash)
    {
        logError("fails to create diaHash.");
        return;
    }

	diaConnMgr_init();
}


void diaMgr_onMsg(diaTransportMsg_t* pTpMsg)
{
	osStatus_e status = OS_STATUS_OK;
    diaMsgDecoded_t* pDiaDecoded = NULL;

	if(!pTpMsg)
    {
        logError("null pointer, pTpMsg.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	//diaId is for the connection status.  For msg from peer, diaId will be 0  
    diaConnBlock_t* pDcb = pTpMsg->diaId ? pTpMsg->diaId : diaConnMgr_getDcb(&pTpMsg->peer);
    if(!pDcb)
    {
        logError("null pDcb, pTpMsg->diaId=%p.", pTpMsg->diaId);
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

	logInfo("received a pTpMsg, msgType=%d, pDcb=%p.", pTpMsg->tpMsgType, pDcb);

	if(pTpMsg->tpMsgType == DIA_TRANSPORT_MSG_TYPE_PEER_MSG)
    {
        pDiaDecoded = diaMsg_decode(pTpMsg->peerMsg.pDiaBuf);
        if(!pDiaDecoded)
        {
            logError("fails to diaMsg_decode a diameter message.");
            status = OS_ERROR_INVALID_VALUE;
            goto EXIT;
        }

		logInfo("msg cmdCode=%d.", pDiaDecoded->cmdCode); 
		switch(pDiaDecoded->cmdCode)
		{
			case DIA_CMD_CODE_CER:
			case DIA_CMD_CODE_DWR:
			case DIA_CMD_CODE_DPR:
				break;
			default:
			{
				//notify appliction, appId is stored in pDcb
				if(pDiaDecoded->cmdFlag & DIA_CMD_FLAG_REQUEST)
				{
					pDcb->diaNotifyApp(pDiaDecoded, NULL);
					break;
				}
				else
				{
					osPointerLen_t* pSessId = diaMsg_getSessId(pDiaDecoded);
    				if(!pSessId)
    				{
        				logInfo("fails to get dia sessionId.");
        				status = OS_ERROR_NETWORK_FAILURE;
        				break;
    				}
	
					osListElement_t* pHashLE = osPlHash_getElement(diaHash, pSessId, true);
					if(!pHashLE)
					{
						logError("received a dia response for cmd(%d), but there is no associated request record, ignore.", pDiaDecoded->cmdCode);
						goto EXIT;
					}

				    diaHashData_t* pDiaHashData = pHashLE->data;
    				if(pDiaHashData->waitRespTimerId)
					{
						pDiaHashData->waitRespTimerId = osStopTimer(pDiaHashData->waitRespTimerId);
					}

					diaNotifyApp_h notifyApp = pDiaHashData->appCallback ? pDiaHashData->appCallback : pDcb->diaNotifyApp;
					notifyApp(pDiaDecoded, pDiaHashData->appData);

				    osHash_deleteNode(pHashLE, OS_HASH_DEL_NODE_TYPE_ALL);
				}
				break;
			}
		}
	}

	if(diaConnMgr_onMsg(pDcb, pTpMsg, pDiaDecoded) != OS_STATUS_OK)
	{
		logError("fails to diaConnMgr_onMsg for pDcb(%p).", pDcb);
		goto EXIT;
	}

EXIT:
	if(pTpMsg)
	{
		if(pTpMsg->tpMsgType == DIA_TRANSPORT_MSG_TYPE_PEER_MSG)
		{
			osMBuf_dealloc(pTpMsg->peerMsg.pDiaBuf);
			osfree(pDiaDecoded);
		}

		osfree(pTpMsg);
	}

	return;
}

uint32_t diaMgr_getInitH2hId()
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    srand(tp.tv_nsec);
    uint32_t h2hId=rand();
}


uint32_t diaGetOrigStateId()
{
	return diaOrigStateId;
}


#if 1	//to-remove, uar test
extern osMBuf_t* testDiaUar();
extern osMBuf_t* testDiaMar();
extern osMBuf_t* testDiaSar();
static uint32_t runNum = 0;
#endif

osStatus_e diaSendCommonMsg(diaIntfType_e intfType, diaConnBlock_t* pDcb, diaCmdCode_e cmd, bool isReq, diaResultCode_e resultCode) 
{
	DEBUG_BEGIN

	osStatus_e status = OS_STATUS_OK;

	if(!pDcb)
	{
		logError("null pointer, pDcb.");
	}

	osMBuf_t* pBuf = NULL;
	switch(cmd)
	{
		case DIA_CMD_CODE_CER:
		{
            osListPlus_t* pHostIpList = diaConfig_getHostIpList(intfType, NULL);
            osVPointerLen_t* productName = diaConfig_getProductName(intfType, NULL);

            uint32_t vendorId = diaConfig_getDiaVendorId();
            osList_t* pSupportedVendorId = diaConfig_getSupportedVendorId(intfType, NULL);
            osList_t* pAuthAppId = diaConfig_getAuthAppId(intfType, NULL);
            osList_t* pAcctAppId = diaConfig_getAcctAppId(intfType, NULL);
            osList_t* pVendorSpecificAppId = diaConfig_getVendorSpecificAppId(intfType, NULL);
			uint32_t firmwareRev = 0;
			bool isFWRevExist = diaConfig_getFirmwareRev(&firmwareRev);

			if(isReq)
			{
				pBuf = diaBuildCer(pHostIpList, vendorId, productName, pSupportedVendorId, pAuthAppId, pAcctAppId, pVendorSpecificAppId, isFWRevExist ? &firmwareRev : NULL, NULL, &pDcb->cmdHdrInfo);
			}
			else
			{
				pBuf = diaBuildCea(resultCode, pHostIpList, vendorId, productName, pSupportedVendorId, pAuthAppId, pAcctAppId, pVendorSpecificAppId, isFWRevExist ? &firmwareRev : NULL, NULL, &pDcb->cmdHdrInfo);
			}

			osListPlus_free(pHostIpList);
			osVPL_free(productName, true);
			osList_free(pSupportedVendorId);
			osList_free(pAuthAppId);
			osList_free(pAcctAppId);
			osList_free(pVendorSpecificAppId);
			break;
		}
		case DIA_CMD_CODE_DWR:
            if(isReq)
            {
                diaCmdHdrInfo_t cmdHdrInfo;
                pBuf = diaBuildDwr(NULL, &pDcb->cmdHdrInfo);
            }
            else
            {
                pBuf = diaBuildDwa(resultCode, NULL, NULL, NULL, &pDcb->cmdHdrInfo);
            }
		case DIA_CMD_CODE_DPR:	
			break;
	}

	debug("send diameter common message(cmd=%d, isReq=%d) for pDcb(%p), tcpFd=%d, message=", cmd, isReq, pDcb, pDcb->tcpFd);
	hexdump(stdout, pBuf->buf, pBuf->end);

	transportStatus_e tpStatus = transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, pBuf, NULL);
	osMBuf_dealloc(pBuf);
	
	if(tpStatus == TRANSPORT_STATUS_TCP_FAIL || tpStatus == TRANSPORT_STATUS_FAIL)
	{
		//take no further action, expect transport to send a seperate notify message
		logInfo("fails to transport_send(%d).", tpStatus);
		status = OS_ERROR_NETWORK_FAILURE;
	}

#if 1	//to-remove, for testing
	if(cmd == DIA_CMD_CODE_DWR && runNum <= 3)
	{
		runNum++;
		if(runNum == 1)
		{
debug("to-remove, send UAR");
			pBuf = testDiaUar();
			tpStatus = transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, pBuf, NULL);
    		osMBuf_dealloc(pBuf);

    		if(tpStatus == TRANSPORT_STATUS_TCP_FAIL || tpStatus == TRANSPORT_STATUS_FAIL)
    		{
        		//take no further action, expect transport to send a seperate notify message
        		logInfo("fails to transport_send(%d) for uar.", tpStatus);
        		status = OS_ERROR_NETWORK_FAILURE;
    		}
		}
	}

	if(runNum == 2)
	{
debug("to-remove, send MAR");		
        pBuf = testDiaMar();
        tpStatus = transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, pBuf, NULL);
        osMBuf_dealloc(pBuf);

        if(tpStatus == TRANSPORT_STATUS_TCP_FAIL || tpStatus == TRANSPORT_STATUS_FAIL)
        {
            logInfo("fails to transport_send(%d) for mar.", tpStatus);
            status = OS_ERROR_NETWORK_FAILURE;
        }
	}
    if(runNum == 3)
    {
debug("to-remove, send SAR");
        pBuf = testDiaSar();
        tpStatus = transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, pBuf, NULL);
        osMBuf_dealloc(pBuf);

        if(tpStatus == TRANSPORT_STATUS_TCP_FAIL || tpStatus == TRANSPORT_STATUS_FAIL)
        {
            logInfo("fails to transport_send(%d) for sar.", tpStatus);
            status = OS_ERROR_NETWORK_FAILURE;
        }
    }
#endif
EXIT:
	DEBUG_END
	return status;
}



osStatus_e diaSendAppMsg(diaIntfType_e intfType, osMBuf_t* pMBuf, osPointerLen_t* pSessId, diaNotifyApp_h appCallback, void* appData)
{
	osStatus_e status = OS_STATUS_OK;
	diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(intfType);
	if(!pDcb)
	{
		logError("noa ctive diameter connection exists for intfType(%d).", intfType);
		status = OS_ERROR_SYSTEM_FAILURE;
		goto EXIT;
	}

	transportStatus_e tpStatus = transport_send(TRANSPORT_APP_TYPE_DIAMETER, pDcb, &pDcb->tpInfo, pMBuf, NULL);
    if(tpStatus != TRANSPORT_STATUS_TCP_OK)
    {
        logInfo("fails to transport_send for intfType(%d), tpStatus=%d.", intfType, tpStatus);
        status = OS_ERROR_NETWORK_FAILURE;
		goto EXIT;
    }

	//if pSessId != NULL, indicates it is a request and needs to be hashed for response, otherwise, the appMsg is a response, no need to be hashed
	if(pSessId)
	{
		diaHashData_t* pDiaHashData = osmalloc(sizeof(diaHashData_t), diaHashData_cleanup);
		pDiaHashData->appData = appData;
		pDiaHashData->appCallback = appCallback;
		pDiaHashData->pDcb = osmemref(pDcb);
		osListElement_t* pHashLE = osPlHash_addUserData(diaHash, pSessId, true, pDiaHashData);
		pDiaHashData->waitRespTimerId = osStartTimer(DIA_REQ_WAIT_RESP_DEFAULT_TIME, dia_onTimeout, pHashLE);
	}	

EXIT:
	return status;
}


static void dia_onTimeout(uint64_t timerId, void* ptr)
{
	if(!ptr)
	{
		logError("null pointer, ptr.");
		return;
	}

	diaHashData_t* pDiaHashData = ((osListElement_t*)ptr)->data;
	if(timerId != pDiaHashData->waitRespTimerId)
	{
		logError("received timeout id(%ld) does not match with received(%ld).", timerId, pDiaHashData->waitRespTimerId);
		return;
	}

	diaNotifyApp_h notifyApp = pDiaHashData->appCallback ? pDiaHashData->appCallback : pDiaHashData->pDcb->diaNotifyApp;
	if(!notifyApp)
	{
		logError("notifyApp is empty.");
		return;
	}

	notifyApp(NULL, pDiaHashData->appData);

    osHash_deleteNode(ptr, OS_HASH_DEL_NODE_TYPE_ALL);
}


static void diaHashData_cleanup(void* data)
{
	if(!data)
	{
		return;
	}

	osfree(((diaHashData_t*)data)->pDcb);
}
