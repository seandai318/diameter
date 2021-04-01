/********************************************************
 * Copyright (C) 2019, 2020, Sean Dai
 *
 * @file diaCxSar.c
 *********************************************************/

#include "osList.h"
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaCxSar.h"
#include "diaCxAvp.h" 
#include "diaConnState.h"


static osStatus_e diaCxSar_encodeRestInfo(osMBuf_t* pDiaBuf, void* pData);
static osMBuf_t* diaCxSar_encode(diaCxSarParam_t* pSarParam, diaHdrSessInfo_t* pHdrSessInfo);


osStatus_e diaCx_sendSAR(diaCxSarAppInput_t* pSarInput, diaNotifyApp_h appCallback, void* pAppData)
{
	osStatus_e status = OS_STATUS_OK;
    diaHdrSessInfo_t hdrSessInfo = {};

	if(!pSarInput)
	{
		logError("null pointer, pSarInput.");
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
	}

    if(!pSarInput->pImpu || !pSarInput->pServerName || !pSarInput->pSarInfo)
	{
		logError("null pointer, pImpu=%p, pServerName=%p, pSarInfo=%p.", pSarInput->pImpu, pSarInput->pServerName, pSarInput->pSarInfo);
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

	osVPointerLen_t userName = {};
	if(pSarInput->pImpi)
	{
		userName.pl = *pSarInput->pImpi;
	}

	osVPointerLen_t pubId = {*pSarInput->pImpu, false, false};
	osVPointerLen_t serverName = {*pSarInput->pServerName, false, false};
	osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
	osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};
	diaAvp_supportedFeature_t sf;
    sf.fl[0].vendorId = DIA_AVP_VENDOR_3GPP;
    sf.fl[0].featureListId = 1;
    sf.fl[0].featureList = pSarInput->featureList;
    sf.flNum = 1;

	osMBuf_t* pMBuf = diaBuildSar(pSarInput->pImpi ? &userName : NULL, &pubId, &serverName, &destHost, &destRealm, pSarInput->pSarInfo, &sf, pSarInput->pExtraOptList, &hdrSessInfo);
	if(!pMBuf)
	{
		logError("fails to diaBuildSar for pubId(%r).", pSarInput->pImpu);
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
	<Server-Assignment-Request> ::=	< Diameter Header: 301, REQ, PXY, 16777216 >
	< Session-Id >
	[ DRMP ]
	{ Vendor-Specific-Application-Id }
	{ Auth-Session-State }
	{ Origin-Host }
	{ Origin-Realm }
	[ Destination-Host ]
	{ Destination-Realm }
	[ User-Name ] 
	[ OC-Supported-Features ]
	*[ Supported-Features ]
	*[ Public-Identity ]
	[ Wildcarded-Public-Identity ]
	{ Server-Name }
	{ Server-Assignment-Type }
	{ User-Data-Already-Available }
	[ SCSCF-Restoration-Info ]
	[ Multiple-Registration-Indication ] 
	[ Session-Priority ] 
	[ SAR-Flags ]
	*[ AVP ]
	*[ Proxy-Info ]
	*[ Route-Record ]
*/
static osMBuf_t* diaCxSar_encode(diaCxSarParam_t* pSarParam, diaHdrSessInfo_t* pHdrSessInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pSarParam || !pHdrSessInfo)
    {
        logError("null pointer, pSarParam=%p, pHdrSessInfo=%p.", pSarParam, pHdrSessInfo);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	diaEncodeAvpData_u	avpData;

	osList_t sarAvpList = {};		//each element contains diaEncodeAvp_t
	//session-id
	diaAvp_sessionIdParam_t sessIdData;
    sessIdData.pSessId = &pHdrSessInfo->sessionId;
    sessIdData.pHostName = &pSarParam->realmHost.origHost.pl;
    diaEncodeAvp_t avpSessId = {DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u)((void*) &sessIdData), diaAvp_encodeSessionId};
    osList_append(&sarAvpList, &avpSessId);

	//vendor-specific-id
	diaAvp_vendorSpecificAppIdParam_t vsAppIdData;
    vsAppIdData.authAppId = DIA_AVP_AUTH_APP_ID_3GPP_CX;
	vsAppIdData.vendor = DIA_AVP_VENDOR_ID_3GPP;
    diaEncodeAvp_t avpVsAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)((void*) &vsAppIdData), diaAvp_encodeVendorSpecificAppId};
	osList_append(&sarAvpList, &avpVsAppId);

	//auth-session-state
	diaEncodeAvp_t avpAuthSessState = {DIA_AVP_CODE_AUTH_SESSION_STATE, DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED, NULL};
	osList_append(&sarAvpList, &avpAuthSessState);

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pSarParam->realmHost.origHost, NULL};
	osList_append(&sarAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pSarParam->realmHost.origRealm, NULL};
	osList_append(&sarAvpList, &avpOrigRealm);

	//dest-host
	diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pSarParam->optAvpList, DIA_AVP_CODE_DEST_HOST);
	if(pOptAvp)
	{
		osList_append(&sarAvpList, pOptAvp);
	}

	//dest-realm
#if 1
    diaEncodeAvp_t* pDestRealm = diaOptListFindAndRemoveAvp(&pSarParam->optAvpList, DIA_AVP_CODE_DEST_REALM);
    if(pDestRealm)
    {
        osList_append(&sarAvpList, pDestRealm);
    }
#else
	diaEncodeAvp_t avpDestRealm = {DIA_AVP_CODE_DEST_REALM, (diaEncodeAvpData_u)&pSarParam->realmHost.destRealm, NULL};
	osList_append(&sarAvpList, &avpDestRealm);
#endif

	//user-name
	if(osPL_isset(&pSarParam->userName.pl))
	{
		diaEncodeAvp_t avpUserName = {DIA_AVP_CODE_USER_NAME, (diaEncodeAvpData_u)&pSarParam->userName, NULL};
		osList_append(&sarAvpList, &avpUserName);
	}

	//OC-Supported-features
	pOptAvp = diaOptListFindAndRemoveAvp(pSarParam->pExtraOptAvpList, DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES);
	if(pOptAvp)
	{
		osList_append(&sarAvpList, pOptAvp);
    }

	//supported-features
	while(1)
	{
		pOptAvp = diaOptListFindAndRemoveAvp(&pSarParam->optAvpList, DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
		if(!pOptAvp)
		{
			break;
		}
		osList_append(&sarAvpList, pOptAvp);
	}

   	//pub-id
	diaEncodeAvp_t avpPubId = {DIA_AVP_CODE_CX_PUBLIC_ID, (diaEncodeAvpData_u)&pSarParam->pubId, NULL};
	osList_append(&sarAvpList, &avpPubId);

	//server-name
	diaEncodeAvp_t avpServerName = {DIA_AVP_CODE_CX_SERVER_NAME, (diaEncodeAvpData_u)&pSarParam->serverName, NULL};
	osList_append(&sarAvpList, &avpServerName);

	if(osList_getCount(&pSarParam->optAvpList) > 0)
	{
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pSarParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&sarAvpList, pOptAvp);
        }
	}

	//server-assignment-type
	diaEncodeAvp_t sat = {DIA_AVP_CODE_CX_SERVER_ASSIGNMENT_TYPE, (diaEncodeAvpData_u)pSarParam->pSarInfo->serverAssignmentType, NULL};
	osList_append(&sarAvpList, &sat);	
	
	//user-data-already-available
	diaEncodeAvp_t userDataAvail = {DIA_AVP_CODE_CX_USER_DATA_ALREADY_AVAILABLE, (diaEncodeAvpData_u)pSarParam->pSarInfo->userDataAvailable, NULL};
	osList_append(&sarAvpList, &userDataAvail);

	//restoration-info
    diaEncodeAvp_t restInfo = {DIA_AVP_CODE_CX_RESTORATION_INFO, (diaEncodeAvpData_u)(void*)pSarParam->pSarInfo->pRestorationInfo, diaCxSar_encodeRestInfo};
	if(pSarParam->pSarInfo->pRestorationInfo)
	{
#if 0	//to enable when scscf supports restoration info
    	osList_append(&sarAvpList, &restInfo);
#endif
	}
			
	//multiple-registration-indication
	diaEncodeAvp_t multiRegInd = {DIA_AVP_CODE_CX_MULTI_REG_INDICATION, (diaEncodeAvpData_u)pSarParam->pSarInfo->multiRegInd};
	if(pSarParam->pSarInfo->pRestorationInfo && pSarParam->pSarInfo->multiRegInd != DIA_3GPP_CX_MULTI_REG_IND_NO_EXIST)
	{
		osList_append(&sarAvpList, &multiRegInd);
	}	

	if(osList_getCount(pSarParam->pExtraOptAvpList) > 0)
	{
		diaEncodeAvp_t *pAvpProxyInfo, *pAvpRRInfo;

		pOptAvp = diaOptListFindAndRemoveAvp(pSarParam->pExtraOptAvpList, DIA_AVP_CODE_PROXY_INFO);
		if(pOptAvp)
		{
			pAvpProxyInfo = pOptAvp;
		}
		pOptAvp = diaOptListFindAndRemoveAvp(pSarParam->pExtraOptAvpList, DIA_AVP_CODE_ROUTE_RECORD);
		if(pOptAvp)
        {
            pAvpRRInfo = pOptAvp;
        }

		while(1)
		{
			pOptAvp = diaOptListGetNextAndRemoveAvp(pSarParam->pExtraOptAvpList);
			if(!pOptAvp)
			{
				break;
			}
			osList_append(&sarAvpList, pOptAvp);
		}

		if(pAvpProxyInfo)
		{
			osList_append(&sarAvpList, pAvpProxyInfo);
		}

		if(pAvpRRInfo)
		{
			osList_append(&sarAvpList, pAvpRRInfo);
		}
	}

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_SAR, 0xc0, DIA_APP_ID_3GPP_CX, &sarAvpList, &pHdrSessInfo->hdrInfo);

EXIT:
	return pDiaBuf;
}


//pList contains extra optional AVPs
osMBuf_t* diaBuildSar(osVPointerLen_t* userName, osVPointerLen_t* pubId, osVPointerLen_t* serverName, osVPointerLen_t* pDestHost, osVPointerLen_t* pDestRealm, diaCxSarInfo_t* pSarInfo, diaAvp_supportedFeature_t* pSF, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo)
{

	if(!pubId || !serverName || !pSarInfo)
	{
		logError("null pointer for mandatory parameters, pubId=%p, serverName=%p, pSarInfo=%p.", pubId, serverName, pSarInfo);
		return NULL;
	}

	diaCxSarParam_t sarParam = {};

	diaConfig_getHostRealm(&sarParam.realmHost);
	diaEncodeAvp_t destHost = {DIA_AVP_CODE_DEST_HOST, (diaEncodeAvpData_u)pDestHost, NULL};
	if(pDestHost)
	{
		osList_append(&sarParam.optAvpList, &destHost);
	}

    diaEncodeAvp_t destRealm = {DIA_AVP_CODE_DEST_REALM, (diaEncodeAvpData_u)pDestRealm, NULL};
    if(pDestRealm)
    {
        osList_append(&sarParam.optAvpList, &destRealm);
    }

	if(userName)
	{
		sarParam.userName = *userName;
	}
	sarParam.pubId = *pubId;
	sarParam.serverName = *serverName;

	sarParam.pSarInfo = pSarInfo;

	if(pSF)
	{
		diaEncodeAvp_t sf = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)pSF, diaAvp_encodeSupportedFeature};
debug("to-remove, insert avp=%d", DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
    	osList_append(&sarParam.optAvpList, &sf);
	}

	sarParam.pExtraOptAvpList = pExtraOptList;
	return diaCxSar_encode(&sarParam, pHdrSessInfo);
}


//a place holder, to add implementation when restorationInfo is supported
static osStatus_e diaCxSar_encodeRestInfo(osMBuf_t* pDiaBuf, void* pData)
{
	osStatus_e status = OS_STATUS_OK;
	return status;
}
