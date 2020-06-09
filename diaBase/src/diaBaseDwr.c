#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "osList.h"
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaBaseDwr.h"
#include "diaConfig.h"
#include "diaMgr.h"



/*
 * rfc6733, section 5.5.1
 *
 *        <DWR>  ::= < Diameter Header: 280, REQ >
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
osMBuf_t* diaCxDwr_encode(diaBaseDwrParam_t* pDwrParam, diaCmdHdrInfo_t* pCmdHdrInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pDwrParam || !pCmdHdrInfo)
    {
        logError("null pointer, pDwrParam=%p, pCmdHdrInfo=%p.", pDwrParam, pCmdHdrInfo);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	osList_t dwrAvpList = {};		//each element contains diaEncodeAvp_t

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pDwrParam->realmHost.origHost, NULL};
	osList_append(&dwrAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pDwrParam->realmHost.origRealm, NULL};
	osList_append(&dwrAvpList, &avpOrigRealm);

    diaEncodeAvp_t* pOptAvp = NULL;
    if(osList_getCount(&pDwrParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pDwrParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&dwrAvpList, pOptAvp);
        }
    }

	if(osList_getCount(pDwrParam->pExtraOptAvpList) > 0)
	{
		 while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(pDwrParam->pExtraOptAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&dwrAvpList, pOptAvp);
        }
    }

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_DWR, 0x80, DIA_APP_ID_BASE, &dwrAvpList, pCmdHdrInfo);

EXIT:
	osList_clear(&dwrAvpList);
	return pDiaBuf;
}


/*
 * rfc6733, section 5.5.1
 *
 *        <DWR>  ::= < Diameter Header: 280, REQ >
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
//pList contains extra optional AVPs that are not explicitly specified in the rfc
osMBuf_t* diaBuildDwr(osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo)
{
	if(!pCmdHdrInfo)
	{
		logError("null pointer for mandatory parameters, pCmdHdrInfo=%p.", pCmdHdrInfo);
		return NULL;
	}

	osMBuf_t* pBuf = NULL;

	diaBaseDwrParam_t dwrParam = {};

	diaConfig_getHostRealm(&dwrParam.realmHost);

	dwrParam.pExtraOptAvpList = pExtraOptList;

	pBuf = diaCxDwr_encode(&dwrParam, pCmdHdrInfo);

EXIT:
	osList_clear(&dwrParam.optAvpList);
	return pBuf;
	
}


/*
 * rfc6733, section 5.5.2
 *
 *        <DWA>  ::= < Diameter Header: 280 >
 *                   { Result-Code }
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Error-Message ]
 *                   [ Failed-AVP ]
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
osMBuf_t* diaCxDwa_encode(diaBaseDwaParam_t* pDwaParam, diaCmdHdrInfo_t* pCmdHdrInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pDwaParam)
    {
        logError("null pointer, pDwaParam=%p.", pDwaParam);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    osList_t ceaAvpList = {};       //each element contains diaEncodeAvp_t

	//result-code
	diaEncodeAvp_t avpResultCode = {DIA_AVP_CODE_RESULT_CODE, (diaEncodeAvpData_u)pDwaParam->resultCode.resultCode, NULL};
	osList_append(&ceaAvpList, &avpResultCode);

    //orig-host
    diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pDwaParam->realmHost.origHost, NULL};
    osList_append(&ceaAvpList, &avpOrigHost);

    //orig-realm
    diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pDwaParam->realmHost.origRealm, NULL};
    osList_append(&ceaAvpList, &avpOrigRealm);

    diaEncodeAvp_t* pOptAvp = NULL;
    if(osList_getCount(&pDwaParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pDwaParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&ceaAvpList, pOptAvp);
        }
    }

    if(osList_getCount(pDwaParam->pExtraOptAvpList) > 0)
    {
         while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(pDwaParam->pExtraOptAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&ceaAvpList, pOptAvp);
        }
    }

    pDiaBuf = diaMsg_encode(DIA_CMD_CODE_DWR, 0x00, DIA_APP_ID_BASE, &ceaAvpList, pCmdHdrInfo);

EXIT:
    return pDiaBuf;
}


/*
 * rfc6733, section 5.5.2
 *
 *        <DWA>  ::= < Diameter Header: 280 >
 *                   { Result-Code }
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Error-Message ]
 *                   [ Failed-AVP ]
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
//pList contains extra optional AVPs that is not listed explicitly in the rfc
osMBuf_t* diaBuildDwa(diaResultCode_e resultCode, osPointerLen_t* errorMsg, diaEncodeAvp_t* failedEvp, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo)
{

    if(!pCmdHdrInfo)
    {
        logError("null pointer for mandatory parameters, pCmdHdrInfo=%p.", pCmdHdrInfo);
        return NULL;
    }

    diaBaseDwaParam_t ceaParam = {};

	ceaParam.resultCode.isResultCode = true;
	ceaParam.resultCode.resultCode = resultCode;

    diaConfig_getHostRealm(&ceaParam.realmHost);

    diaEncodeAvp_t avpErrorMsg = {DIA_AVP_CODE_ERROR_MESSAGE, (diaEncodeAvpData_u)errorMsg, NULL};
    if(errorMsg)
    {
		osList_append(&ceaParam.optAvpList, &avpErrorMsg);
	}

	diaEncodeAvpGroupedData_t avpFailedAvpData = {failedEvp, 1};
    diaEncodeAvp_t avpFailedAvp = {DIA_AVP_CODE_FAILED_AVP, (diaEncodeAvpData_u)&avpFailedAvpData, NULL};
	if(failedEvp)
	{
		osList_append(&ceaParam.optAvpList, &avpFailedAvp);
    }

    ceaParam.pExtraOptAvpList = pExtraOptList;

    return diaCxDwa_encode(&ceaParam, pCmdHdrInfo);
}
