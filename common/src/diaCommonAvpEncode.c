#include <pthread.h>
#include <stdio.h>

#include "osMemory.h"
#include "osPrintf.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaCxAvp.h"


osStatus_e diaAvp_encodeSessionId(osMBuf_t* pDiaBuf, void* pData)
{
	osStatus_e status = OS_STATUS_OK;
	static pthread_mutex_t ovMutex = PTHREAD_MUTEX_INITIALIZER;
	static uint32_t optionalValue = 0;

    if(!pDiaBuf || !pData)
    {
        logError("null pointer, pDiaBuf=%p, pData=%p.", pDiaBuf, pData);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	if(pDiaBuf->pos % 4)
    {
        logError("pDiaBuf does not start from 4 byte boundary, pDiaBuf->pos=%ld.", pDiaBuf->pos);
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

	diaAvp_sessionIdParam_t* pSessData = pData;
	osPointerLen_t* pHostName = pSessData->pHostName;
	if(!pHostName)
	{
        logError("null pointer, pHostName.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);

	char* sessIdBuf = osmalloc(DIA_MAX_SESSION_ID_LEN, NULL);	
//	osPointerLen_t optionalPL = {pSessData->pSessId, 0};
	uint32_t value;
	pthread_mutex_lock(&ovMutex);
	value = optionalValue++;
	pthread_mutex_unlock(&ovMutex);
	uint32_t len = osPrintf_buffer(sessIdBuf, DIA_MAX_SESSION_ID_LEN, "%r;%lu;%lu;%u", pHostName, tp.tv_sec, tp.tv_nsec, optionalValue);

	if(len >= DIA_MAX_SESSION_ID_LEN)
	{
		logError("dia_session_id exceeds maximum size(%d), session_id=%r;%lu;%lu;%u.", DIA_MAX_SESSION_ID_LEN, pHostName, tp.tv_sec, tp.tv_nsec, optionalValue);
		osfree(sessIdBuf);
		status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

	osVPL_set(pSessData->pSessId, sessIdBuf, len, true);

	status = diaAvp_encode(pDiaBuf, DIA_CMD_CODE_BASE, DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u) pSessData->pSessId);

EXIT:
	return status;
}


osStatus_e diaAvp_encodeSupportedFeature(osMBuf_t* pDiaBuf, void* pData)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf || !pData)
    {
        logError("null pointer, pDiaBuf=%p, pData=%p.", pDiaBuf, pData);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    if(pDiaBuf->pos % 4)
    {
        logError("pDiaBuf does not start from 4 byte boundary.");
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

    diaAvp_supportedFeature_t* pSF = pData;

	diaEncodeAvpGroupedData_t grpAvpData;
	diaEncodeAvp_t avp[3];
	grpAvpData.pDiaAvp = avp;
	grpAvpData.avpNum = 3;
	for(int i=0; i<pSF->flNum; i++)
	{
		diaAvpEncode_setValue(&avp[0], DIA_AVP_CODE_VENDOR_ID, (diaEncodeAvpData_u) pSF->fl[i].vendorId, NULL);
		diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_FEATURE_LIST_ID, (diaEncodeAvpData_u) pSF->fl[i].featureListId, NULL);
		diaAvpEncode_setValue(&avp[2], DIA_AVP_CODE_CX_FEATURE_LIST, (diaEncodeAvpData_u) pSF->fl[i].featureList, NULL);

		//use any Cx cmd.
    	status = diaAvp_encode(pDiaBuf, DIA_CMD_CODE_UAR, DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)&grpAvpData);
	}

EXIT:
	return status;
}


osStatus_e diaAvp_encodeVendorSpecificAppId(osMBuf_t* pDiaBuf, void* pData)
{
    osStatus_e status = OS_STATUS_OK;

    if(!pDiaBuf)
    {
        logError("null pointer, pDiaBuf=%p.", pDiaBuf);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    if(pDiaBuf->pos % 4)
    {
        logError("pDiaBuf does not start from 4 byte boundary.");
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

	diaAvpVendor_e vendor = ((diaAvp_vendorSpecificAppIdParam_t*)pData)->vendor;  

    diaAvp_supportedFeature_t* pSF = pData;

    diaEncodeAvpGroupedData_t grpAvpData;
    diaEncodeAvp_t avp[2];
    grpAvpData.pDiaAvp = avp;
    grpAvpData.avpNum = 2;
        
	diaAvpEncode_setValue(&avp[0], DIA_AVP_CODE_AUTH_APP_ID, (diaEncodeAvpData_u) ((diaAvp_vendorSpecificAppIdParam_t*)pData)->authAppId, NULL);
	diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_VENDOR_ID, (diaEncodeAvpData_u) ((diaAvp_vendorSpecificAppIdParam_t*)pData)->vendor, NULL);

    status = diaAvp_encode(pDiaBuf, DIA_CMD_CODE_BASE, DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)&grpAvpData);

EXIT:
    return status;
}
