#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "osHexDump.h"
#include "osMemory.h"

#include "diaConfig.h"
#include "diaTransportIntf.h"
#include "diaConnState.h"
#include "diaConnMgr.h"
#include "diaBaseCer.h"
#include "diaBaseDwr.h"


//the number of seconds since the start of the Unix epoch at ~20200426, UTC 16:15:00
#define DIA_BASE_CER_ORIG_STATE_EPOCH_VALUE 0x5ea5cfa4      //1587924900


static uint32_t diaOrigStateId;

void dia_init(char* diaConfigFolder)
{
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    diaOrigStateId = tp.tv_sec - DIA_BASE_CER_ORIG_STATE_EPOCH_VALUE;

	diaConfig_init(diaConfigFolder);
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

		switch(pDiaDecoded->cmdCode)
		{
			case DIA_CMD_CODE_CER:
			case DIA_CMD_CODE_DWR:
			case DIA_CMD_CODE_DPR:
				break;
			default:
				//to-do, notify appliction, appId is stored in pDcb
				if(pDcb->diaNotifyApp)
				{
					pDcb->diaNotifyApp(pDiaDecoded);
				}
				break;
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
			osVPL_free(productName);
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
	}
#endif
EXIT:
	DEBUG_END
	return status;
}
