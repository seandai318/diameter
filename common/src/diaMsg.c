#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "osMBuf.h"
#include "osDebug.h"
#include "osMemory.h"
#include "osList.h"

#include "diaMgr.h"
#include "diaMsg.h"
#include "diaAvp.h"



osStatus_e diaMsg_decodeHdr(osMBuf_t* pDiaBuf, uint32_t* cmd, uint8_t* cmdFlag, uint32_t* len, uint32_t* appId, uint32_t* h2hId, uint32_t* e2eId);
static void diaMsgDecoded_cleanup(void* pData);



diaMsgDecoded_t* diaMsg_decode(osMBuf_t* pDiaBuf)
{
    osStatus_e status = OS_STATUS_OK;
    diaMsgDecoded_t* pMsgDecoded = NULL;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf=%p.", pDiaBuf);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    pMsgDecoded = oszalloc(sizeof(diaMsgDecoded_t), diaMsgDecoded_cleanup);

    pDiaBuf->pos = 0;

    status = diaMsg_decodeHdr(pDiaBuf, &pMsgDecoded->cmdCode, &pMsgDecoded->cmdFlag, &pMsgDecoded->len, &pMsgDecoded->appId, &pMsgDecoded->h2hId, &pMsgDecoded->e2eId);
    if(status != OS_STATUS_OK)
    {
        logError("fails to parse a received diameter message header.");
        osfree(pDiaBuf);
        goto EXIT;
    }

logError("to-remove, cmdCode=%d, cmdFlag=0x%x, len=%d, pos=%d.", pMsgDecoded->cmdCode, pMsgDecoded->cmdFlag, pMsgDecoded->len, pDiaBuf->pos);

    while(pDiaBuf->pos < pDiaBuf->end)
    {
        diaAvp_t* pAvp = oszalloc(sizeof(diaAvp_t), diaAvp_cleanup);

        status = diaAvp_decode(pDiaBuf, pMsgDecoded->cmdCode, &pAvp->avpCode, &pAvp->avpFlag, &pAvp->vendorId, &pAvp->avpData);
        if(status != OS_STATUS_OK)
        {
            logError("fails to parse a AVP for a received diameter message (cmdCode=%d).", pMsgDecoded->cmdCode);
            osfree(pDiaBuf);
            goto EXIT;
        }
        osList_append(&pMsgDecoded->avpList, pAvp);
    }

EXIT:
    if(status != OS_STATUS_OK)
    {
        osfree(pMsgDecoded);
        pMsgDecoded = NULL;
    }

    return pMsgDecoded;
}


osMBuf_t* diaMsg_encode(diaCmdCode_e cmdCode, uint8_t cmdFlag, uint32_t appId, osList_t* pAvpList, diaCmdHdrInfo_t* pHdrInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaMsg = osMBuf_alloc(DIA_MSG_MAX_SIZE);

    if(!pDiaMsg ||!pAvpList || !pHdrInfo)
    {
        logError("null pointer, fails to osMBuf_alloc for pDiaMsg (%p) or pAvpList=%p, or pHdrInfo=%p.", pDiaMsg, pAvpList, pHdrInfo);
        goto EXIT;
    }

    pDiaMsg->pos = DIA_MSG_AVP_START;

	osListElement_t* pLE = pAvpList->head;
	while(pLE)
	{
		diaEncodeAvp_t* pAvp = pLE->data;
debug("to-remove, pos=%ld, avp=%d", pDiaMsg->pos, pAvp->avpCode);

        if(pAvp->avpEncodeFunc)
        {
            status = pAvp->avpEncodeFunc(pDiaMsg, pAvp->avpData.pAvpEncodeFuncArg);
			if(status != OS_STATUS_OK)
			{
				logError("fails to encode avp(%d) using avp specific func", pAvp->avpCode); 
				goto EXIT;
			}
        }
        else
        {
            status = diaAvp_encode(pDiaMsg, cmdCode, pAvp->avpCode, pAvp->avpData);
			if(status != OS_STATUS_OK)
            {
                logError("fails to encode avp(%d)", pAvp->avpCode);
                goto EXIT;
            }
        }

		pLE = pLE->next;
    }

	dia_encodeHdr(pDiaMsg, cmdFlag, cmdCode, pDiaMsg->pos, appId, &pHdrInfo->h2hId, &pHdrInfo->e2eId);

EXIT:
    if(status != OS_STATUS_OK)
    {
        osfree(pDiaMsg);
        pDiaMsg = NULL;
    }

    return pDiaMsg;
}


diaResultCode_e dia_GetResultCode(diaMsgDecoded_t* pMsgDecoded)
{
    diaResultCode_e resultCode = DIA_RESULT_CODE_NONE;

    if(!pMsgDecoded)
    {
        logError("null pointer, pMsgDecoded.");
        goto EXIT;
    }

	uint32_t lookupArg = DIA_AVP_CODE_RESULT_CODE;
    osListElement_t* pLE = osList_lookup(&pMsgDecoded->avpList, true, diaListFindAvp, &lookupArg);
    if(pLE)
    {
        resultCode = ((diaAvp_t*)pLE->data)->avpData.dataU32;
    }

EXIT:
    return resultCode;
}


//by default, flag P is set, E, T has to be set explicitly
osStatus_e dia_encodeHdr(osMBuf_t* pDiaBuf, uint8_t cmdFlag, uint32_t cmd, uint32_t len, uint32_t appId, uint32_t* pH2hId, uint32_t* pE2eId)
{
	static uint32_t h2hId;
    static pthread_mutex_t h2hMutex = PTHREAD_MUTEX_INITIALIZER;

	bool isReq = cmdFlag & DIA_CMD_FLAG_REQUEST;
	uint32_t h2hIdValue = 0;
	uint32_t e2eIdValue = 0;
	osStatus_e status = OS_STATUS_OK;

	if(!pDiaBuf)
	{
		logError("null pointer, pDiaBuf.");
		return OS_ERROR_NULL_POINTER;
	}

	if(isReq && (!pH2hId || !pE2eId))
	{
		logError("isResponse, but null pointer, pH2hId=%p, pE2eId=%p.", pH2hId, pE2eId);
		return OS_ERROR_NULL_POINTER;
    }

	size_t origPos = pDiaBuf->pos;
	pDiaBuf->pos = 0;

	osMBuf_writeU32(pDiaBuf, htobe32(1<<24 | len & 0xffffff), true);
	osMBuf_writeU32(pDiaBuf, htobe32(cmdFlag <<24 | cmd & 0xffffff), true);
//	osMBuf_writeU32(pDiaBuf, isReq ? htobe32(0xc0 <<24 | cmd & 0xffffff) : htobe32(0x80 <<24 | cmd & 0xffffff), true);
	osMBuf_writeU32(pDiaBuf, htobe32(appId), true);

	if(isReq)
	{
		pthread_mutex_lock(&h2hMutex);	
		if(!h2hId)
		{
			h2hId = diaMgr_getInitH2hId();
		}
		h2hIdValue = ++h2hId;
		pthread_mutex_unlock(&h2hMutex);

    	struct timespec tp;
    	clock_gettime(CLOCK_REALTIME, &tp);
    	srand(tp.tv_nsec);
    	int randValue=rand();
		e2eIdValue = (tp.tv_sec & 0xfff)<<20 | (randValue & 0xfffff);
	}
	else
	{
        h2hIdValue = *pH2hId;
		e2eIdValue = *pE2eId;
    }

	osMBuf_writeU32(pDiaBuf, htobe32(h2hIdValue), true);
	osMBuf_writeU32(pDiaBuf, htobe32(e2eIdValue), true);

    if(isReq)
    {
		*pH2hId = h2hIdValue;
		*pE2eId = e2eIdValue;
	}

EXIT:
	pDiaBuf->pos = origPos;	
	return status;
}


osStatus_e diaMsg_decodeHdr(osMBuf_t* pDiaBuf, uint32_t* cmd, uint8_t* cmdFlag, uint32_t* len, uint32_t* appId, uint32_t* h2hId, uint32_t* e2eId)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

    *len =  htobe32(*(uint32_t*)pDiaBuf->buf) & 0xffffff;
	*cmd = htobe32(*(uint32_t*)&pDiaBuf->buf[4]) & 0xffffff;
	*cmdFlag = pDiaBuf->buf[4];
	*appId = htobe32(*(uint32_t*)&pDiaBuf->buf[8]);
	*h2hId = htobe32(*(uint32_t*)&pDiaBuf->buf[12]);
	*e2eId = htobe32(*(uint32_t*)&pDiaBuf->buf[16]);

	pDiaBuf->pos = 20;

	return status;
}


osStatus_e diaMsg_updateCmdFlagR(osMBuf_t* pDiaBuf, bool isSet)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

	uint8_t flag = pDiaBuf->buf[4];
	flag = isSet? flag | 0x80 : flag & 0x7f;
	pDiaBuf->buf[4] = flag;
}


osStatus_e diaMsg_updateCmdFlagP(osMBuf_t* pDiaBuf, bool isSet)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

    uint8_t flag = pDiaBuf->buf[4];
    flag = isSet? flag | 0x40 : flag & 0xbf;
    pDiaBuf->buf[4] = flag;
}


osStatus_e diaMsg_updateCmdFlagE(osMBuf_t* pDiaBuf, bool isSet)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

    uint8_t flag = pDiaBuf->buf[4];
    flag = isSet? flag | 0x20 : flag & 0xdf;
    pDiaBuf->buf[4] = flag;
}


osStatus_e diaMsg_updateCmdFlagT(osMBuf_t* pDiaBuf, bool isSet)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

    uint8_t flag = pDiaBuf->buf[4];
    flag = isSet? flag | 0x10 : flag & 0xef;
    pDiaBuf->buf[4] = flag;
}


osStatus_e diaMsg_updateCmdFlag(osMBuf_t* pDiaBuf, uint8_t flag)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf.");
        return OS_ERROR_NULL_POINTER;
    }

    pDiaBuf->buf[4] = flag;
}	



#if 0
diaAvp_encodeInfo_t avpList[5];
diaAvpSetGroupAvpInfo(&avpList[i], DIA_AVP_CODE_VENDOR_ID, 0x40, DIA_AVP_VENDOR_INVALID, &vendor);
diaAvpEncode_t avpList[0] = {DIA_AVP_CODE_VENDOR_ID, 0x40, DIA_AVP_VENDOR_INVALID, 12}
diaAvpEncode_t avpList[1] = {CODE_2, 0x40, DIA_AVP_VENDOR_INVALID, &pl}

diaAvpEncode_t gavpList[2];
diaAvpEncode_t gavpList[0] = {gcode_3, 0x80, DIA_AVP_VENDOR_INVALID, 10}
diaAvpEncode_t gavpList[1] = {gcode_4, 0x80, DIA_AVP_VENDOR_INVALID, 11}
diaAvpGroupedDataEncode_t gavp={gavpList, 2}
diaAvpEncode_t avpList[2] = {GCODE_3, 0x80, DIA_AVP_VENDOR_INVALID, &gavp}
#endif


static void diaMsgDecoded_cleanup(void* pData)
{
	if(!pData)
	{
		return;
	}

    osList_cleanup(&((diaMsgDecoded_t*)pData)->avpList);
}
