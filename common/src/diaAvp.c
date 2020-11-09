#include <time.h>
#include <endian.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "osMBuf.h"
#include "osDebug.h"
#include "osMemory.h"

#include "diaAvp.h"
#include "diaAvpEncode.h"


static osStatus_e diaAvp_encodeStr(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, void* pData);
static osStatus_e diaAvp_encodeInt32(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, uint32_t intData);
static osStatus_e diaAvp_encodeInt64(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, uint64_t intData);
static osStatus_e diaAvp_encodeGrouped(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t avpCode, uint8_t avpFlag, diaAvpVendor_e vendorId, diaEncodeAvpGroupedData_t* pGAvpData);

static osStatus_e diaAvp_decodeGroup(diaAvpGroupedData_t* pGroupedData, diaCmdCode_e diaCmd);

static void diaEncodeAvp_cleanup(void* pGrpAvp);
static void diaEncodeAvpGrp_cleanup(void* pGrpAvp);
static void diaEncodeAvpGrpData_cleanup(void* pData);


osStatus_e diaAvp_encode(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t avpCode, diaEncodeAvpData_u avpData)
{
	DEBUG_BEGIN

    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf=%p.", pDiaBuf);
        return OS_ERROR_NULL_POINTER;
    }

	uint8_t avpFlag;
	uint32_t vendorId;
	switch(diaGetAvpInfo(diaCmd, avpCode, &avpFlag, &vendorId))
	{
		case DIA_AVP_DATA_TYPE_ENUM:
		case DIA_AVP_DATA_TYPE_INT32:
		case DIA_AVP_DATA_TYPE_UINT32:
            status = diaAvp_encodeInt32(pDiaBuf, avpCode, avpFlag, vendorId, avpData.dataU32);
            break;
		case DIA_AVP_DATA_TYPE_INT64:
		case DIA_AVP_DATA_TYPE_UINT64:
            status = diaAvp_encodeInt64(pDiaBuf, avpCode, avpFlag, vendorId, avpData.dataU64);
            break;
		case DIA_AVP_DATA_TYPE_OCTET_STRING:
		case DIA_AVP_DATA_TYPE_UTF8_STRING:
		case DIA_AVP_DATA_TYPE_IP_FILTER:
		case DIA_AVP_DATA_TYPE_TIME:
		case DIA_AVP_DATA_TYPE_DIAM_IDEN:
			status = diaAvp_encodeStr(pDiaBuf, avpCode, avpFlag, vendorId, avpData.pDataStr);
			break;
		case DIA_AVP_DATA_TYPE_GROUPED:
			status = diaAvp_encodeGrouped(pDiaBuf, diaCmd, avpCode, avpFlag, vendorId, avpData.pDataGrouped);
			break;
		default:
			logError("there is no data type for avpCode(%d).", avpCode);
			status = OS_ERROR_INVALID_VALUE;
			break;
	}

EXIT:
	DEBUG_END
	return status;
}


static osStatus_e diaAvp_encodeStr(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, void* pData)
{
	osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf || !pData)
    {
        logError("null pointer, pDiaBuf=%p, pData=%p.", pDiaBuf, pData);
        return OS_ERROR_NULL_POINTER;
    }

	osPointerLen_t* str = pData;

	//avp code
    osMBuf_writeU32(pDiaBuf, htobe32(avpCode), true);

debug("to-remove, avpFlag=0x%x, avpCode=%d, vendorId=0x%x, str=%r", avpFlag, avpCode, vendorId, str);
	//avp flag + len
	osMBuf_writeU32(pDiaBuf, htobe32(avpFlag<<24 | ((avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8) + str->l)), true);

	//optional vendor-ID
	if(avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC)
	{
debug("to-remove, vendorId=0x%x", vendorId);
		osMBuf_writeU32(pDiaBuf, htobe32(vendorId), true);
	}
	
	//octetString data
	osMBuf_writePL(pDiaBuf, str, true);
	uint8_t remaining = str->l % 4;
	if(remaining)
	{
		osMBuf_setZero(pDiaBuf, 4 - remaining, true);
	}

	return status;
}



static osStatus_e diaAvp_encodeInt32(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, uint32_t intData)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf=%p.", pDiaBuf);
        return OS_ERROR_NULL_POINTER;
    }
debug("to-remove, avpFlag=0x%x, avpCode=%d", avpFlag, avpCode);

    //avp code
    osMBuf_writeU32(pDiaBuf, htobe32(avpCode), true);

    //avp flag + len
    osMBuf_writeU32(pDiaBuf, htobe32(avpFlag<<24 | ((avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8) + 4)&0xffffff), true);

debug("to-remove, avpCode=%d, len=%d", avpCode, (avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8) + 4);
    //optional vendor-ID
    if(avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC)
    {
        osMBuf_writeU32(pDiaBuf, htobe32(vendorId), true);
    }

    //uint32 data
    osMBuf_writeU32(pDiaBuf, htobe32(intData), true);

    return status;
}


static osStatus_e diaAvp_encodeInt64(osMBuf_t* pDiaBuf, uint32_t avpCode, uint8_t avpFlag, uint32_t vendorId, uint64_t intData)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf=%p.", pDiaBuf);
        return OS_ERROR_NULL_POINTER;
    }
debug("to-remove, avpFlag=0x%x, avpCode=%d", avpFlag, avpCode);

    //avp code
    osMBuf_writeU32(pDiaBuf, htobe32(avpCode), true);

    //avp flag + len
    osMBuf_writeU32(pDiaBuf, htobe32(avpFlag<<24 | ((avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8) + 8)&0xffffff), true);

    //optional vendor-ID
    if(avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC)
    {
        osMBuf_writeU32(pDiaBuf, htobe32(vendorId), true);
    }

    //uint64 data
    osMBuf_writeU64(pDiaBuf, htobe64(intData), true);

    return status;
}


osStatus_e diaAvp_encodeGrouped(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t avpCode, uint8_t avpFlag, diaAvpVendor_e vendorId, diaEncodeAvpGroupedData_t* groupAvpData)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf || !groupAvpData)
    {
        logError("null pointer, pDiaBuf=%p, groupAvpData=%p.", pDiaBuf, groupAvpData);
        status = OS_ERROR_NULL_POINTER;
		goto EXIT;
    }
debug("to-remove, avpFlag=0x%x, avpCode=%d, groupAvpData=%p", avpFlag, avpCode, groupAvpData);

	size_t avpStartPos = pDiaBuf->pos;
	pDiaBuf->pos += avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8;

	for(int i=0; i<groupAvpData->avpNum; i++)
	{
		status = diaAvp_encode(pDiaBuf, diaCmd, groupAvpData->pDiaAvp[i].avpCode, groupAvpData->pDiaAvp[i].avpData);
	}

	size_t avpEndPos = pDiaBuf->pos;
	uint32_t len = avpEndPos - avpStartPos;
	pDiaBuf->pos = avpStartPos; 

debug("to-remove, len=0x%x, avpEndPos=0x%x, avpStartPos=0x%x", len, avpEndPos, avpStartPos);

    //avp code
    osMBuf_writeU32(pDiaBuf, htobe32(avpCode), true);
    //avp flag + len
    osMBuf_writeU32(pDiaBuf,  htobe32(avpFlag<<24 | len & 0xffffff), true);
    //optional vendor-ID
    if(avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC)
    {
        osMBuf_writeU32(pDiaBuf, htobe32(vendorId), true);
    }

	pDiaBuf->pos = avpEndPos;

EXIT:
    return status;
}



osStatus_e diaAvp_decode(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t* avpCode, uint8_t* avpFlag, uint32_t* vendorId, diaAvpData_t* pAvpData)
{
	DEBUG_BEGIN

    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf || !avpCode || !avpFlag || !vendorId || !pAvpData)
    {
        logError("null pointer, pDiaBuf=%p, avpCode=%p, avpFlag=%p, vendorId=%p, pAvpData=%p.", pDiaBuf, avpCode, avpFlag, vendorId, pAvpData);
        status = OS_ERROR_NULL_POINTER;
		goto EXIT;
    }

	if(pDiaBuf->pos % 4 != 0)
	{
		logError("pDiaBuf->pos is not multiple of 4.");
		status = OS_ERROR_INVALID_VALUE;
		goto EXIT;
	}

	debug("start avp decode, pos=%ld", pDiaBuf->pos);

	size_t startAvpPos = pDiaBuf->pos;
	size_t avpLen;

	*avpCode = htobe32(*(uint32_t*)&pDiaBuf->buf[pDiaBuf->pos]); 
	pDiaBuf->pos += 4;
	*avpFlag = pDiaBuf->buf[pDiaBuf->pos];
	avpLen = htobe32(*(uint32_t*)&pDiaBuf->buf[pDiaBuf->pos]) & 0xffffff;
    pDiaBuf->pos += 4;
	if(*avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC)
	{
		*vendorId = htobe32(*(uint32_t*)&pDiaBuf->buf[pDiaBuf->pos]);
	    pDiaBuf->pos += 4;
	}
	else
	{
		*vendorId = 0xffffffff;
	}

	switch (pAvpData->dataType = diaGetAvpInfo(diaCmd, *avpCode, NULL, NULL))
	{
		case DIA_AVP_DATA_TYPE_OCTET_STRING:
        case DIA_AVP_DATA_TYPE_UTF8_STRING:
        case DIA_AVP_DATA_TYPE_IP_FILTER:
        case DIA_AVP_DATA_TYPE_TIME:
        case DIA_AVP_DATA_TYPE_DIAM_IDEN:
			osVPL_set(&pAvpData->dataStr, &pDiaBuf->buf[pDiaBuf->pos], avpLen - (*avpCode & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8), false); 
			break;
    	case DIA_AVP_DATA_TYPE_INT32:
			pAvpData->data32 = htobe32(*(uint32_t*)&pDiaBuf->buf[pDiaBuf->pos]);
			break;	
    	case DIA_AVP_DATA_TYPE_INT64:
            pAvpData->data64 = htobe64(*(uint64_t*)&pDiaBuf->buf[pDiaBuf->pos]);
            break;
    	case DIA_AVP_DATA_TYPE_UINT32:
            pAvpData->dataU32 = htobe32(*(uint32_t*)&pDiaBuf->buf[pDiaBuf->pos]);
            break;
    	case DIA_AVP_DATA_TYPE_UINT64:
            pAvpData->dataU64 = htobe64(*(uint64_t*)&pDiaBuf->buf[pDiaBuf->pos]);
            break;
    	case DIA_AVP_DATA_TYPE_GROUPED:
			osMBuf_allocRef2(&pAvpData->dataGrouped.rawData, pDiaBuf, pDiaBuf->pos, avpLen - *avpFlag & DIA_AVP_FLAG_VENDOR_SPECIFIC ? 12 : 8);
			diaAvp_decodeGroup(&pAvpData->dataGrouped, diaCmd);
			break;
		case DIA_AVP_DATA_TYPE_UNKNOWN:
		default:
			logError("unknown data type for diameter AVP code(%d).", *avpCode);
			break;
	}

    pDiaBuf->pos = startAvpPos + avpLen + (avpLen%4 ? (4 - avpLen%4) : 0);

	debug("decoded avp code(%d), avp len=%d, vendorId=0x%x, pos=%ld.", *avpCode, avpLen, *vendorId, pDiaBuf->pos);
EXIT:
	DEBUG_END
	return status;
}


static osStatus_e diaAvp_decodeGroup(diaAvpGroupedData_t* pGroupedData, diaCmdCode_e diaCmd)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pGroupedData)
    {
        logError("null pointer, pGroupedData=%p.", pGroupedData);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	osMBuf_t* pDiaGrpBuf = &pGroupedData->rawData;
    if(pDiaGrpBuf->pos % 4 != 0)
    {
        logError("pDiaGrpBuf->pos is not multiple of 4.");
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

    while(pDiaGrpBuf->pos < pDiaGrpBuf->end)
	{
		diaAvp_t* pAvp = oszalloc(sizeof(diaAvp_t), diaAvp_cleanup);
		status = diaAvp_decode(pDiaGrpBuf, diaCmd, &pAvp->avpCode, &pAvp->avpFlag, &pAvp->vendorId, &pAvp->avpData);
		if(pAvp->avpData.dataType == DIA_AVP_DATA_TYPE_GROUPED)
		{
			diaAvp_decodeGroup(&pAvp->avpData.dataGrouped, diaCmd);
		}

		osList_append(&pGroupedData->dataList, pAvp);
	}

EXIT:
	return status;
}


//this function is the first step to construct a group AVP
diaEncodeAvp_t* diaAvpGrp_create(uint32_t avpCode)
{

    diaEncodeAvp_t* pGrpAvp = oszalloc(sizeof(diaEncodeAvp_t), diaEncodeAvpGrp_cleanup);
    if(!pGrpAvp)
    {
        logError("fail to allocate pGrpAvp.");
        return NULL;
    }

	//can not use diaEncodeAvp_cleanup() here as the allocation is for an array of diaEncodeAvp_t.  The clean up of individual avp memory will be called inside diaEncodeAvpGrpData_cleanup().
    diaEncodeAvp_t* pAvp = oszalloc(sizeof(diaEncodeAvp_t)*DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP, NULL);
    if(!pAvp)
    {
        logError("fail to allocate pAvp.");

        osfree(pGrpAvp);
        return NULL;
    }

    diaEncodeAvpGroupedData_t* pGrpAvpData = osmalloc(sizeof(diaEncodeAvpGroupedData_t), diaEncodeAvpGrpData_cleanup);
    if(!pGrpAvpData)
    {
        logError("fail to allocate pGrpAvpData.");

        osfree(pGrpAvp);
        osfree(pAvp);
        return NULL;
    }

    pGrpAvpData->pDiaAvp = pAvp;
    pGrpAvpData->avpNum = 0;

    pGrpAvp->avpCode = avpCode;
    pGrpAvp->avpData = (diaEncodeAvpData_u) pGrpAvpData;
    pGrpAvp->avpEncodeFunc = NULL;

    return pGrpAvp;
}


//add sub AVP to a group AVP
diaEncodeAvp_t* diaAvpGrp_addAvp(diaEncodeAvp_t* pGrpAvp, uint32_t avpCode, diaEncodeAvpData_u avpData, diaAvp_encodeGroupCallback_h avpEncodeFunc)
{
    diaEncodeAvp_t* pAvp = NULL;

    if(!pGrpAvp)
    {
        logError("null pointer, pGrpAvp");
        return NULL;
    }

	diaEncodeAvpGroupedData_t* pGrpAvpData = pGrpAvp->avpData.pDataGrouped;
    if(pGrpAvpData->avpNum < DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP -1)
    {
        pAvp = &pGrpAvpData->pDiaAvp[pGrpAvpData->avpNum++];
        diaAvpEncode_setValue(pAvp, avpCode, avpData, avpEncodeFunc);
    }

    return pAvp;
}

#if 0
//use a group AVP has been used (diameter message has been constructed), this function shall be called to reclaim used memories
void diaAvpGrp_cleanup(void* pGrpAvp)
{
    if(!pGrpAvp)
    {
        return;
    }

    osfree(((diaEncodeAvp_t*)pGrpAvp)->avpData.pDataGrouped->pDiaAvp);
    osfree(((diaEncodeAvp_t*)pGrpAvp)->avpData.pDataGrouped);
    osfree(pGrpAvp);
}
#endif


void diaAvp_cleanup(void* pData)
{
    if(!pData)
    {
        return;
    }

    diaAvp_t* pAvp = pData;

    if(pAvp->avpData.dataType == DIA_AVP_DATA_TYPE_GROUPED)
    {
        osList_cleanup(&pAvp->avpData.dataGrouped.dataList);
    }
}


static void diaEncodeAvp_cleanup(void* pData)
{
    if(!pData)
    {
        return;
    }

    diaEncodeAvp_t* pAvp = pData;

    switch(diaGetAvpEncodeDataType(pAvp->avpCode))
	{
		case DIA_AVP_ENCODE_DATA_TYPE_GROUP:
        	osfree(pAvp->avpData.pDataGrouped);
			break;
		case DIA_AVP_ENCODE_DATA_TYPE_STR:
			osVPL_free(pAvp->avpData.pDataStr, true);
			break;
		default:
			break;
    }
}


//use a group AVP has been used (diameter message has been constructed), this function shall be called to reclaim used memories
static void diaEncodeAvpGrp_cleanup(void* pGrpAvp)
{
    if(!pGrpAvp)
    {
        return;
    }

    osfree(((diaEncodeAvp_t*)pGrpAvp)->avpData.pDataGrouped);
}

//for cleanup() function, the own data structure does not need to be freed, as calling of the osfree() that triggers the cleanup() will free it
static void diaEncodeAvpGrpData_cleanup(void* pData)
{
    if(!pData)
    {
        return;
    }

	diaEncodeAvpGroupedData_t* pDataGrouped = pData;

	//cleanup the avpData for each avp.  avp itself will be freed as part of osfree(pDataGrouped->pDiaAvp).  when pDataGrouped->pDiaAvp was created, its cleanup function shall be NULL
	for(int i=0; i<pDataGrouped->avpNum; i++)
	{
		diaEncodeAvp_cleanup(&pDataGrouped->pDiaAvp[i]);
	}
	
	osfree(pDataGrouped->pDiaAvp);
}
