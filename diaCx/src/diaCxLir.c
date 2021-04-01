/********************************************************
 * Copyright (C) 2019, 2020, Sean Dai
 *
 * @file diaCxLir.c
 *********************************************************/

#include "osList.h"
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaCxLir.h"
#include "diaCxAvp.h" 
#include "diaConnState.h"


static osMBuf_t* diaCxLir_encode(diaCxLirParam_t* pLirParam, diaHdrSessInfo_t* pHdrSessInfo);


osStatus_e diaCx_sendLIR(diaCxLirAppInput_t* pLirInput, diaNotifyApp_h appCallback, void* pAppData)
{
	osStatus_e status = OS_STATUS_OK;
    diaHdrSessInfo_t hdrSessInfo = {};

	if(!pLirInput)
	{
		logError("null pointer, pLirInput.");
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
	}

    if(!pLirInput->pImpu)
	{
		logError("null pointer, pImpu=%p.", pLirInput->pImpu);
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
    }

    diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(DIA_INTF_TYPE_CX);
    if(!pDcb)
    {
        logError("noa ctive diameter connection exists for DIA_INTF_TYPE_CX.");
        status = OS_ERROR_SYSTEM_FAILURE;
        goto EXIT;
    }

	osVPointerLen_t pubId = {*pLirInput->pImpu, false, false};
	osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
	osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};
	diaAvp_supportedFeature_t sf;
    sf.fl[0].vendorId = DIA_AVP_VENDOR_3GPP;
    sf.fl[0].featureListId = 1;
    sf.fl[0].featureList = pLirInput->featureList;
    sf.flNum = 1;

	osMBuf_t* pMBuf = diaBuildLir(&pubId, &destHost, &destRealm, &sf, pLirInput->userAuthType, pLirInput->pExtraOptList, &hdrSessInfo);
	if(!pMBuf)
	{
		logError("fails to diaBuildLir for pubId(%r).", pLirInput->pImpu);
		status = OS_ERROR_SYSTEM_FAILURE;
		goto EXIT;
	}

	status = diaSendAppMsg(DIA_INTF_TYPE_CX, pDcb, pMBuf, &hdrSessInfo.sessionId.pl, appCallback, pAppData);        
	osMBuf_dealloc(pMBuf);

EXIT:
    if(status != OS_STATUS_OK && hdrSessInfo.sessionId.isPDynamic)
    {
        osfree((char*)hdrSessInfo.sessionId.pl.p);
    }
	return status;
}


/*
 * 3gpp29.229, section 6.1.3
 *
   <Location-Info-Request> ::=	< Diameter Header: 302, REQ, PXY, 16777216 >
	< Session-Id >
    [ DRMP ]
	{ Vendor-Specific-Application-Id }
    { Auth-Session-State }
    { Origin-Host }
    { Origin-Realm }
    [ Destination-Host ]
	{ Destination-Realm }
    [ Originating-Request ] 
    [ OC-Supported-Features ]
    *[ Supported-Features ]
    { Public-Identity }
    [ User-Authorization-Type ] 
    [ Session-Priority ]
	*[ AVP ]
	*[ Proxy-Info ]
	*[ Route-Record ]
*/
static osMBuf_t* diaCxLir_encode(diaCxLirParam_t* pLirParam, diaHdrSessInfo_t* pHdrSessInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pLirParam || !pHdrSessInfo)
    {
        logError("null pointer, pLirParam=%p, pHdrSessInfo=%p.", pLirParam, pHdrSessInfo);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	diaEncodeAvpData_u	avpData;

	osList_t lirAvpList = {};		//each element contains diaEncodeAvp_t
	//session-id
	diaAvp_sessionIdParam_t sessIdData;
    sessIdData.pSessId = &pHdrSessInfo->sessionId;
    sessIdData.pHostName = &pLirParam->realmHost.origHost.pl;
    diaEncodeAvp_t avpSessId = {DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u)((void*) &sessIdData), diaAvp_encodeSessionId};
    osList_append(&lirAvpList, &avpSessId);

	//vendor-specific-id
	diaAvp_vendorSpecificAppIdParam_t vsAppIdData;
    vsAppIdData.authAppId = DIA_AVP_AUTH_APP_ID_3GPP_CX;
	vsAppIdData.vendor = DIA_AVP_VENDOR_ID_3GPP;
    diaEncodeAvp_t avpVsAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)((void*) &vsAppIdData), diaAvp_encodeVendorSpecificAppId};
	osList_append(&lirAvpList, &avpVsAppId);

	//auth-session-state
	diaEncodeAvp_t avpAuthSessState = {DIA_AVP_CODE_AUTH_SESSION_STATE, DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED, NULL};
	osList_append(&lirAvpList, &avpAuthSessState);

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pLirParam->realmHost.origHost, NULL};
	osList_append(&lirAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pLirParam->realmHost.origRealm, NULL};
	osList_append(&lirAvpList, &avpOrigRealm);

	//dest-host
	diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pLirParam->optAvpList, DIA_AVP_CODE_DEST_HOST);
	if(pOptAvp)
	{
		osList_append(&lirAvpList, pOptAvp);
	}

	//dest-realm
    diaEncodeAvp_t* pDestRealm = diaOptListFindAndRemoveAvp(&pLirParam->optAvpList, DIA_AVP_CODE_DEST_REALM);
    if(pDestRealm)
    {
        osList_append(&lirAvpList, pDestRealm);
    }

	//OC-Supported-features
	pOptAvp = diaOptListFindAndRemoveAvp(pLirParam->pExtraOptAvpList, DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES);
	if(pOptAvp)
	{
		osList_append(&lirAvpList, pOptAvp);
    }

	//supported-features
	while(1)
	{
		pOptAvp = diaOptListFindAndRemoveAvp(&pLirParam->optAvpList, DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
		if(!pOptAvp)
		{
			break;
		}
		osList_append(&lirAvpList, pOptAvp);
	}

   	//pub-id
	diaEncodeAvp_t avpPubId = {DIA_AVP_CODE_CX_PUBLIC_ID, (diaEncodeAvpData_u)&pLirParam->pubId, NULL};
	osList_append(&lirAvpList, &avpPubId);

	if(osList_getCount(&pLirParam->optAvpList) > 0)
	{
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pLirParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&lirAvpList, pOptAvp);
        }
	}

	//user-auth-type, only include in the request if the type is DIA_3GPP_CX_USER_AUTH_TYPE_REG_AND_CAP
	diaEncodeAvp_t uat = {DIA_AVP_CODE_CX_USER_AUTH_TYPE, (diaEncodeAvpData_u)pLirParam->userAuthType, NULL};
	if(pLirParam->userAuthType == DIA_3GPP_CX_USER_AUTH_TYPE_REG_AND_CAP)
	{
		osList_append(&lirAvpList, &uat);	
	}
		
	if(osList_getCount(pLirParam->pExtraOptAvpList) > 0)
	{
		diaEncodeAvp_t *pAvpProxyInfo, *pAvpRRInfo;

		pOptAvp = diaOptListFindAndRemoveAvp(pLirParam->pExtraOptAvpList, DIA_AVP_CODE_PROXY_INFO);
		if(pOptAvp)
		{
			pAvpProxyInfo = pOptAvp;
		}
		pOptAvp = diaOptListFindAndRemoveAvp(pLirParam->pExtraOptAvpList, DIA_AVP_CODE_ROUTE_RECORD);
		if(pOptAvp)
        {
            pAvpRRInfo = pOptAvp;
        }

		while(1)
		{
			pOptAvp = diaOptListGetNextAndRemoveAvp(pLirParam->pExtraOptAvpList);
			if(!pOptAvp)
			{
				break;
			}
			osList_append(&lirAvpList, pOptAvp);
		}

		if(pAvpProxyInfo)
		{
			osList_append(&lirAvpList, pAvpProxyInfo);
		}

		if(pAvpRRInfo)
		{
			osList_append(&lirAvpList, pAvpRRInfo);
		}
	}

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_LIR, 0xc0, DIA_APP_ID_3GPP_CX, &lirAvpList, &pHdrSessInfo->hdrInfo);

EXIT:
	return pDiaBuf;
}


//pList contains extra optional AVPs
osMBuf_t* diaBuildLir(osVPointerLen_t* pubId, osVPointerLen_t* pDestHost, osVPointerLen_t* pDestRealm, diaAvp_supportedFeature_t* pSF, dia3gppUserAuthType_e uat, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo)
{

	if(!pubId)
	{
		logError("null pointer for mandatory parameters, pubId=%p.", pubId);
		return NULL;
	}

	diaCxLirParam_t lirParam = {};

	diaConfig_getHostRealm(&lirParam.realmHost);
	diaEncodeAvp_t destHost = {DIA_AVP_CODE_DEST_HOST, (diaEncodeAvpData_u)pDestHost, NULL};
	if(pDestHost)
	{
		osList_append(&lirParam.optAvpList, &destHost);
	}

    diaEncodeAvp_t destRealm = {DIA_AVP_CODE_DEST_REALM, (diaEncodeAvpData_u)pDestRealm, NULL};
    if(pDestRealm)
    {
        osList_append(&lirParam.optAvpList, &destRealm);
    }

	lirParam.pubId = *pubId;

	lirParam.userAuthType = uat;

	if(pSF)
	{
		diaEncodeAvp_t sf = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)pSF, diaAvp_encodeSupportedFeature};
debug("to-remove, insert avp=%d", DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
    	osList_append(&lirParam.optAvpList, &sf);
	}

	lirParam.pExtraOptAvpList = pExtraOptList;
	return diaCxLir_encode(&lirParam, pHdrSessInfo);
}
