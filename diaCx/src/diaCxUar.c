/********************************************************
 * Copyright (C) 2019, 2020, Sean Dai
 *
 * @file diaCxUar.c
 *********************************************************/

#include "osList.h"
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaCxUar.h"
#include "diaCxAvp.h" 
#include "diaConfig.h"
#include "diaConnState.h"



osStatus_e diaCx_sendUAR(diaCxUarAppInput_t* pUarInput, diaNotifyApp_h appCallback, void* pAppData)
{
    osStatus_e status = OS_STATUS_OK;
    diaHdrSessInfo_t hdrSessInfo = {};

    if(!pUarInput)
    {
        logError("null pointer, pUarInput.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    if(!pUarInput->pImpi || !pUarInput->pImpu)
    {
        logError("null pointer, pImpu=%p, pImpu=%p.", pUarInput->pImpu, pUarInput->pImpu);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(DIA_INTF_TYPE_CX);
    if(!pDcb)
    {
        logError("no active diameter connection exists for DIA_INTF_TYPE_CX.");
        status = OS_ERROR_SYSTEM_FAILURE;
        goto EXIT;
    }

    osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
    osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};

    osVPointerLen_t userName = {*pUarInput->pImpi, false, false};
    osVPointerLen_t pubId = {*pUarInput->pImpu, false, false};
	osVPointerLen_t visitedNWId;
	diaConfig_getHostNWId(&visitedNWId);

    osMBuf_t* pMBuf = diaBuildUar(&userName, &pubId, &visitedNWId, pUarInput->authType, &destHost, &destRealm, pUarInput->featureList, pUarInput->pExtraOptList, &hdrSessInfo);
    if(!pMBuf)
    {
        logError("fails to diaBuildUar for pubId(%r).", pUarInput->pImpu);
        status = OS_ERROR_SYSTEM_FAILURE;
        goto EXIT;
    }

    status = diaSendAppMsg(DIA_INTF_TYPE_CX, NULL, pMBuf, &hdrSessInfo.sessionId.pl, appCallback, pAppData);
    osMBuf_dealloc(pMBuf);

EXIT:
    if(status != OS_STATUS_OK && hdrSessInfo.sessionId.isPDynamic)
    {
        osfree((char*)hdrSessInfo.sessionId.pl.p);
    }
    return status;
}


/*
 * 3gpp29.229, section 6.1.1
 *
 * < User-Authorization-Request> ::=	< Diameter Header: 300, REQ, PXY, 16777216 >
 * 		< Session-Id >
 * 		[ DRMP ]
 *		{ Vendor-Specific-Application-Id }
 *		{ Auth-Session-State }
 *		{ Origin-Host }
 *		{ Origin-Realm }
 *		[ Destination-Host ]
 *		{ Destination-Realm }
 *		{ User-Name }
 *		[ OC-Supported-Features ]
 *		*[ Supported-Features ]
 *		{ Public-Identity }
 *		{ Visited-Network-Identifier }
 *		[ User-Authorization-Type ]
 *		[ UAR-Flags ]
 *		*[ AVP ]
 *		*[ Proxy-Info ]
 *		*[ Route-Record ]	
 */
osMBuf_t* diaCxUar_encode(diaCxUarParam_t* pUarParam, diaHdrSessInfo_t* pHdrSessInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pUarParam || !pHdrSessInfo)
    {
        logError("null pointer, pUarParam=%p, pHdrSessInfo=%p.", pUarParam, pHdrSessInfo);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	diaEncodeAvpData_u	avpData;

	osList_t uarAvpList = {};		//each element contains diaEncodeAvp_t
	//session-id
	diaAvp_sessionIdParam_t sessIdData = {};
	sessIdData.pSessId = &pHdrSessInfo->sessionId;	
    sessIdData.pHostName = &pUarParam->realmHost.origHost.pl;
    diaEncodeAvp_t avpSessId = {DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u)((void*) &sessIdData), diaAvp_encodeSessionId};
    osList_append(&uarAvpList, &avpSessId);

//	avpData.pAvpEncodeFuncArg = &sessIdData;
//	diaEncodeAvpSetValue(&avpList[i++], DIA_AVP_CODE_SESSION_ID, DIA_AVP_FLAG_FUNC_FILL, DIA_AVP_VENDOR_INVALID, avpData, diaAvp_encodeSessionId);

	//vendor-specific-id
	diaAvp_vendorSpecificAppIdParam_t vsAppIdData;
    vsAppIdData.authAppId = DIA_AVP_AUTH_APP_ID_3GPP_CX;
	vsAppIdData.vendor = DIA_AVP_VENDOR_ID_3GPP;
    diaEncodeAvp_t avpVsAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)((void*) &vsAppIdData), diaAvp_encodeVendorSpecificAppId};
	osList_append(&uarAvpList, &avpVsAppId);

	//auth-session-state
	diaEncodeAvp_t avpAuthSessState = {DIA_AVP_CODE_AUTH_SESSION_STATE, DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED, NULL};
	osList_append(&uarAvpList, &avpAuthSessState);

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pUarParam->realmHost.origHost, NULL};
	osList_append(&uarAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pUarParam->realmHost.origRealm, NULL};
	osList_append(&uarAvpList, &avpOrigRealm);

	//dest-host
	diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pUarParam->optAvpList, DIA_AVP_CODE_DEST_HOST);
	if(pOptAvp)
	{
		osList_append(&uarAvpList, pOptAvp);
	}

	//dest-realm
	diaEncodeAvp_t avpDestRealm = {DIA_AVP_CODE_DEST_REALM, (diaEncodeAvpData_u)&pUarParam->realmHost.destRealm, NULL};
	osList_append(&uarAvpList, &avpDestRealm);

	//user-name
	diaEncodeAvp_t avpUserName = {DIA_AVP_CODE_USER_NAME, (diaEncodeAvpData_u)&pUarParam->userName, NULL};
	osList_append(&uarAvpList, &avpUserName);

	//OC-Supported-features
	pOptAvp = diaOptListFindAndRemoveAvp(pUarParam->pExtraOptAvpList, DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES);
	if(pOptAvp)
	{
		osList_append(&uarAvpList, pOptAvp);
    }

	//supported-features
	while(1)
	{
		pOptAvp = diaOptListFindAndRemoveAvp(&pUarParam->optAvpList, DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
		if(!pOptAvp)
		{
			break;
		}
debug("to-remove, avp=%d", DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
		osList_append(&uarAvpList, pOptAvp);
	}

   	//pub-id
	diaEncodeAvp_t avpPubId = {DIA_AVP_CODE_CX_PUBLIC_ID, (diaEncodeAvpData_u)&pUarParam->pubId, NULL};
	osList_append(&uarAvpList, &avpPubId);

    //visited-network-id
	diaEncodeAvp_t avpVNId = {DIA_AVP_CODE_CX_VISITED_NW_ID, (diaEncodeAvpData_u)&pUarParam->visitedNetwork, NULL};
	osList_append(&uarAvpList, &avpVNId);

    //user-auth-type
	pOptAvp = diaOptListFindAndRemoveAvp(&pUarParam->optAvpList, DIA_AVP_CODE_CX_USER_AUTH_TYPE);
	if(pOptAvp)
    {
        osList_append(&uarAvpList, pOptAvp);
    }

	//uar-flags
	pOptAvp = diaOptListFindAndRemoveAvp(pUarParam->pExtraOptAvpList, DIA_AVP_CODE_CX_UAR_FLAGS);
	if(pOptAvp)
    {
        osList_append(&uarAvpList, pOptAvp);
    }

    //access-network-info
	pOptAvp = diaOptListFindAndRemoveAvp(&pUarParam->optAvpList, DIA_AVP_CODE_ACCESS_NW_INFO);
    if(pOptAvp)
    {
        osList_append(&uarAvpList, pOptAvp);
    }

    if(osList_getCount(&pUarParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pUarParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&uarAvpList, pOptAvp);
        }
    }

	if(osList_getCount(pUarParam->pExtraOptAvpList) > 0)
	{
		diaEncodeAvp_t *pAvpProxyInfo, *pAvpRRInfo;

		pOptAvp = diaOptListFindAndRemoveAvp(pUarParam->pExtraOptAvpList, DIA_AVP_CODE_PROXY_INFO);
		if(pOptAvp)
		{
			pAvpProxyInfo = pOptAvp;
		}
		pOptAvp = diaOptListFindAndRemoveAvp(pUarParam->pExtraOptAvpList, DIA_AVP_CODE_ROUTE_RECORD);
		if(pOptAvp)
        {
            pAvpRRInfo = pOptAvp;
        }

		while(1)
		{
			pOptAvp = diaOptListGetNextAndRemoveAvp(pUarParam->pExtraOptAvpList);
			if(!pOptAvp)
			{
				break;
			}
			osList_append(&uarAvpList, pOptAvp);
		}

		if(pAvpProxyInfo)
		{
			osList_append(&uarAvpList, pAvpProxyInfo);
		}

		if(pAvpRRInfo)
		{
			osList_append(&uarAvpList, pAvpRRInfo);
		}
	}

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_UAR, 0xc0, DIA_APP_ID_3GPP_CX, &uarAvpList, &pHdrSessInfo->hdrInfo);

EXIT:
	return pDiaBuf;
}


osVPointerLen_t* diaCxUar_getServerName(diaMsgDecoded_t* pMsgDecoded)
{
	osVPointerLen_t* pServerName = NULL;

    if(!pMsgDecoded)
    {
        logError("null pointer, pMsgDecoded.");
        goto EXIT;
    }

	uint32_t avpCode = DIA_AVP_CODE_CX_SERVER_NAME;
    osListElement_t* pLE = osList_lookup(&pMsgDecoded->avpList, true, diaListFindAvp, &avpCode);
    if(pLE)
    {
		pServerName = &((diaAvp_t*)pLE->data)->avpData.dataStr;
	}

EXIT:
	return pServerName;
}


osStatus_e diaCxUar_getServerCap(diaMsgDecoded_t* pMsgDecoded, diaCxServerCap_t* pServerCap)
{
	osStatus_e status = OS_STATUS_OK;
	
    if(!pMsgDecoded || !pServerCap)
    {
        logError("null pointer, pMsgDecoded=%p, pServerCap=%p.", pMsgDecoded, pServerCap);
        goto EXIT;
    }

	osListPlus_init(&pServerCap->manCap, false);
	osListPlus_init(&pServerCap->optCap, false);
	osListPlus_init(&pServerCap->serverName, false);

	uint32_t avpCode = DIA_AVP_CODE_CX_SERVER_CAPABILITY;
	osListElement_t* pLE = osList_lookup(&pMsgDecoded->avpList, true, diaListFindAvp, &avpCode);
    if(pLE)
    {
		if(((diaAvp_t*)pLE->data)->avpData.dataType != DIA_AVP_DATA_TYPE_GROUPED)
        {
            logError("experimentalResult AVP is not a grouped data type.");
            goto EXIT;
        }

        diaAvpGroupedData_t* pAvpData = &((diaAvp_t*)pLE->data)->avpData.dataGrouped;
		osListElement_t* pLE1 = pAvpData->dataList.head;
		while(pLE1)
		{
			switch(((diaAvp_t*)pLE->data)->avpCode)
			{
				case DIA_AVP_CODE_CX_MANDATORY_CAPABILITY:
					osListPlus_append(&pServerCap->manCap, &((diaAvp_t*)pLE->data)->avpData.dataU32);
					break;
				case DIA_AVP_CODE_CX_OPTIONAL_CAPABILITY:
					osListPlus_append(&pServerCap->optCap, &((diaAvp_t*)pLE->data)->avpData.dataU32);
					break;
				case DIA_AVP_CODE_CX_SERVER_NAME:
					osListPlus_append(&pServerCap->serverName, &((diaAvp_t*)pLE->data)->avpData.dataStr);
					break;
				default:
					logInfo("UAR server capability contains avp(%d), ignore.", ((diaAvp_t*)pLE->data)->avpCode);
					break;
			}
			
			pLE1 = pLE1->next;
		}
	}

EXIT:
	return status;
}


//pList contains extra optional AVPs
//osMBuf_t* diaBuildUar(osVPointerLen_t* userName, osVPointerLen_t* pubId, osVPointerLen_t* visitedNWId, DiaCxUarAuthType_e authType, diaAvp_supportedFeature_t* pSF, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo)
osMBuf_t* diaBuildUar(osVPointerLen_t* userName, osVPointerLen_t* pubId, osVPointerLen_t* visitedNWId, DiaCxUarAuthType_e authType, osVPointerLen_t* pDestHost, osVPointerLen_t* pDestRealm, uint32_t featureList, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo)
{

	if(!userName || !pubId || !visitedNWId || !pHdrSessInfo || !pDestRealm)
	{
		logError("null pointer for mandatory parameters, userName=%p, pubId=%p, visitedNWId=%p, pHdrSessInfo=%p, pDestRealm=%p.", userName, pubId, visitedNWId, pHdrSessInfo, pDestRealm);
		return NULL;
	}

	diaCxUarParam_t uarParam = {};

	diaConfig_getHostRealm(&uarParam.realmHost);
	diaEncodeAvp_t destHost = {DIA_AVP_CODE_DEST_HOST, (diaEncodeAvpData_u)pDestHost, NULL};
	if(pDestHost)
	{
		osList_append(&uarParam.optAvpList, &destHost);
	}

	uarParam.realmHost.destRealm.pl = pDestRealm->pl;

	uarParam.userName = *userName;
	uarParam.pubId = *pubId;
	uarParam.visitedNetwork = *visitedNWId;

	diaAvp_supportedFeature_t sf;
    diaEncodeAvp_t sfavp = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)&sf, diaAvp_encodeSupportedFeature};

	if(featureList)
	{
	    sf.fl[0].vendorId = DIA_AVP_VENDOR_3GPP;
    	sf.fl[0].featureListId = 1;
    	sf.fl[0].featureList = featureList;
    	sf.flNum = 1;

		diaEncodeAvp_t sf = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)&sf, diaAvp_encodeSupportedFeature};
debug("to-remove, insert avp=%d", DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
    	osList_append(&uarParam.optAvpList, &sfavp);
	}

	diaEncodeAvp_t authTypeAvp = {DIA_AVP_CODE_CX_USER_AUTH_TYPE, (diaEncodeAvpData_u)authType, NULL};
	if(authType != DIA_CX_AUTH_TYPE_NONE)
	{
		osList_append(&uarParam.optAvpList, &authTypeAvp);
	}

	uarParam.pExtraOptAvpList = pExtraOptList;
	return diaCxUar_encode(&uarParam, pHdrSessInfo);
}


/*
 * 3gpp29.229, section 6.1.2
 *
 *
 *< User-Authorization-Answer> ::=	< Diameter Header: 300, PXY, 16777216 >
 *		< Session-Id >
 *		[ DRMP ]
 *		{ Vendor-Specific-Application-Id }
 *		[ Result-Code ]
 *		[ Experimental-Result ]
 *		{ Auth-Session-State }
 *		{ Origin-Host }
 *		{ Origin-Realm }
 *		[ OC-Supported-Features ]
 *		[ OC-OLR ] 
 *		*[ Load ]
 *		*[ Supported-Features ]
 *		[ Server-Name ]
 *		[ Server-Capabilities ]
 *		*[ AVP ]
 *		[ Failed-AVP ]
 *		*[ Proxy-Info ]
 *		*[ Route-Record ]
 */
osMBuf_t* diaCxUaa_encode(diaCxUaaParam_t* pUaaParam, diaHdrSessInfo_t* pHdrSessInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pUaaParam)
    {
        logError("null pointer, pUaaParam=%p.", pUaaParam);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	diaEncodeAvp_t* pOptAvp;
    diaEncodeAvpData_u avpData;

    osList_t uaaAvpList = {};       //each element contains diaEncodeAvp_t
    //session-id
    diaAvp_sessionIdParam_t sessIdData;
    sessIdData.pHostName = &pUaaParam->realmHost.origHost.pl;
    diaEncodeAvp_t avpSessId = {DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u)&pHdrSessInfo->sessionId, NULL};
    osList_append(&uaaAvpList, &avpSessId);

    //vendor-specific-id
    diaAvp_vendorSpecificAppIdParam_t vsAppIdData;
    vsAppIdData.authAppId = DIA_AVP_AUTH_APP_ID_3GPP_CX;
    vsAppIdData.vendor = DIA_AVP_VENDOR_ID_3GPP;
    diaEncodeAvp_t avpVsAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)((void*) &vsAppIdData), diaAvp_encodeVendorSpecificAppId};
    osList_append(&uaaAvpList, &avpVsAppId);

	//result code or experimental code
	diaEncodeAvp_t avpResultCode;	
	diaEncodeAvp_t expCode;
	diaEncodeAvpGroupedData_t gResultCode = {&expCode, 1};
	if(pUaaParam->resultCode.isResultCode)
	{
		diaAvpEncode_setValue(&avpResultCode, DIA_AVP_CODE_RESULT_CODE, (diaEncodeAvpData_u)pUaaParam->resultCode.resultCode, NULL);
	}
	else
	{
		diaAvpEncode_setValue(&expCode, DIA_AVP_CODE_EXPERIMENTAL_RESULT_CODE, (diaEncodeAvpData_u)pUaaParam->resultCode.expCode, NULL);
        diaAvpEncode_setValue(&avpResultCode, DIA_AVP_CODE_EXPERIMENTAL_RESULT, (diaEncodeAvpData_u)&gResultCode, NULL);
    }
	osList_append(&uaaAvpList, &avpResultCode);

    //auth-session-state
    diaEncodeAvp_t avpAuthSessState = {DIA_AVP_CODE_AUTH_SESSION_STATE, DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED, NULL};
    osList_append(&uaaAvpList, &avpAuthSessState);

    //orig-host
    diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pUaaParam->realmHost.origHost, NULL};
    osList_append(&uaaAvpList, &avpOrigHost);

    //orig-realm
    diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pUaaParam->realmHost.origRealm, NULL};
    osList_append(&uaaAvpList, &avpOrigRealm);

    //OC-Supported-features
    pOptAvp = diaOptListFindAndRemoveAvp(pUaaParam->pExtraOptAvpList, DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES);
    if(pOptAvp)
    {
        osList_append(&uaaAvpList, pOptAvp);
    }

    //supported-features
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pUaaParam->optAvpList, DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
        if(!pOptAvp)
        {
            break;
        }
        osList_append(&uaaAvpList, pOptAvp);
    }

	//server-name
    pOptAvp = diaOptListFindAndRemoveAvp(&pUaaParam->optAvpList, DIA_AVP_CODE_CX_SERVER_NAME);
    if(pOptAvp)
    {
        osList_append(&uaaAvpList, pOptAvp);
    }

	//server-capability
    pOptAvp = diaOptListFindAndRemoveAvp(&pUaaParam->optAvpList, DIA_AVP_CODE_CX_SERVER_CAPABILITY);
    if(pOptAvp)
    {
        osList_append(&uaaAvpList, pOptAvp);
    }

    if(osList_getCount(&pUaaParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pUaaParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&uaaAvpList, pOptAvp);
        }
    }

    if(osList_getCount(pUaaParam->pExtraOptAvpList) > 0)
    {
        diaEncodeAvp_t *pAvpProxyInfo, *pAvpRRInfo;

        pOptAvp = diaOptListFindAndRemoveAvp(pUaaParam->pExtraOptAvpList, DIA_AVP_CODE_PROXY_INFO);
        if(pOptAvp)
        {
            pAvpProxyInfo = pOptAvp;
        }
        pOptAvp = diaOptListFindAndRemoveAvp(pUaaParam->pExtraOptAvpList, DIA_AVP_CODE_ROUTE_RECORD);
        if(pOptAvp)
        {
            pAvpRRInfo = pOptAvp;
        }

        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(pUaaParam->pExtraOptAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&uaaAvpList, pOptAvp);
        }

        if(pAvpProxyInfo)
        {
            osList_append(&uaaAvpList, pAvpProxyInfo);
        }

        if(pAvpRRInfo)
        {
            osList_append(&uaaAvpList, pAvpRRInfo);
        }
    }

    pDiaBuf = diaMsg_encode(DIA_CMD_CODE_UAR, 0xc0, DIA_APP_ID_3GPP_CX, &uaaAvpList, &pHdrSessInfo->hdrInfo);

EXIT:
    return pDiaBuf;
}


//pList contains extra optional AVPs
osMBuf_t* diaBuildUaa(diaHdrSessInfo_t* pHdrSessInfo, diaResultCode_t* pResultCode, diaAvp_supportedFeature_t* pSF, osVPointerLen_t* serverName,diaCxUarServerCap_t* pCap, osList_t* pExtraOptList)
{
	osMBuf_t* pDiaBuf = NULL;

    if(!pHdrSessInfo || !pResultCode)
    {
        logError("null pointer for mandatory parameters, pHdrSessInfo=%p, pResultCode=%p.", pHdrSessInfo, pResultCode);
        return NULL;
    }

    diaCxUaaParam_t uaaParam = {};

    diaConfig_getHostRealm(&uaaParam.realmHost);

	uaaParam.resultCode = *pResultCode;

	diaEncodeAvp_t sfAvp = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)pSF, diaAvp_encodeSupportedFeature};
    if(pSF)
    {
        osList_append(&uaaParam.optAvpList, &sfAvp);
	}

	//server-name
	diaEncodeAvp_t snAvp = {DIA_AVP_CODE_CX_SERVER_NAME, (diaEncodeAvpData_u)serverName, NULL};
	if(serverName)
	{
		osList_append(&uaaParam.optAvpList, &snAvp);
	}

	//server-capability
	if(pCap)
	{
		diaEncodeAvpGroupedData_t gScData;
		diaEncodeAvp_t gScAvp = {DIA_AVP_CODE_CX_SERVER_CAPABILITY, (diaEncodeAvpData_u)&gScData, NULL};
		diaEncodeAvp_t scAvp[DIA_CX_MAX_SERVER_CAPABILITY_ITEM];
		uint8_t i=0;
		for(int j=0; j<pCap->mscNum, i<DIA_CX_MAX_SERVER_CAPABILITY_ITEM; j++)
		{
        	diaAvpEncode_setValue(&scAvp[i++], DIA_AVP_CODE_CX_MANDATORY_CAPABILITY, (diaEncodeAvpData_u)pCap->msc[j], NULL);
		}
        for(int j=0; j<pCap->oscNum, i<DIA_CX_MAX_SERVER_CAPABILITY_ITEM; j++)
        {
            diaAvpEncode_setValue(&scAvp[i++], DIA_AVP_CODE_CX_OPTIONAL_CAPABILITY, (diaEncodeAvpData_u)pCap->osc[j], NULL);
        }
        for(int j=0; j<pCap->serverNameNum, i<DIA_CX_MAX_SERVER_CAPABILITY_ITEM; j++)
        {
            diaAvpEncode_setValue(&scAvp[i++], DIA_AVP_CODE_CX_SERVER_NAME, (diaEncodeAvpData_u)&pCap->serverName[j], NULL);
        }
		
		gScData.pDiaAvp = scAvp;
		gScData.avpNum = i;

		osList_append(&uaaParam.optAvpList, &gScAvp);
	}

	pDiaBuf = diaCxUaa_encode(&uaaParam, pHdrSessInfo);		

EXIT:
	return pDiaBuf;
}	

