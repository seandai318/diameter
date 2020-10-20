#include "osList.h"
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaCxMar.h"
#include "diaCxAvp.h" 


static osStatus_e diaCxMar_encodeAuthData(osMBuf_t* pDiaBuf, void* pData);


/*
 * 3gpp29.229, section 6.1.7
 *
     < Multimedia-Auth-Request > ::=  < Diameter Header: 303, REQ, PXY, 16777216 >
    < Session-Id >
    [ DRMP ]
    { Vendor-Specific-Application-Id }
    { Auth-Session-State }
    { Origin-Host }
    { Origin-Realm }
    { Destination-Realm }
    [ Destination-Host ]
    { User-Name }
    [ OC-Supported-Features ]
    *[ Supported-Features ]
    { Public-Identity }
    { SIP-Auth-Data-Item }
    { SIP-Number-Auth-Items }
    { Server-Name }
    *[ AVP ]
    *[ Proxy-Info ]
    *[ Route-Record ]
*/
osMBuf_t* diaCxMar_encode(diaCxMarParam_t* pMarParam, diaHdrSessInfo_t* pHdrSessInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;

    if(!pMarParam)
    {
        logError("null pointer, pMarParam=%p.", pMarParam);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	diaEncodeAvpData_u	avpData;

	osList_t marAvpList = {};		//each element contains diaEncodeAvp_t
	//session-id
	diaAvp_sessionIdParam_t sessIdData;
    sessIdData.pSessId = &pHdrSessInfo->sessionId;
    sessIdData.pHostName = &pMarParam->realmHost.origHost.pl;
    diaEncodeAvp_t avpSessId = {DIA_AVP_CODE_SESSION_ID, (diaEncodeAvpData_u)((void*) &sessIdData), diaAvp_encodeSessionId};
    osList_append(&marAvpList, &avpSessId);

	//vendor-specific-id
	diaAvp_vendorSpecificAppIdParam_t vsAppIdData;
    vsAppIdData.authAppId = DIA_AVP_AUTH_APP_ID_3GPP_CX;
	vsAppIdData.vendor = DIA_AVP_VENDOR_ID_3GPP;
    diaEncodeAvp_t avpVsAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)((void*) &vsAppIdData), diaAvp_encodeVendorSpecificAppId};
	osList_append(&marAvpList, &avpVsAppId);

	//auth-session-state
	diaEncodeAvp_t avpAuthSessState = {DIA_AVP_CODE_AUTH_SESSION_STATE, DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED, NULL};
	osList_append(&marAvpList, &avpAuthSessState);

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pMarParam->realmHost.origHost, NULL};
	osList_append(&marAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pMarParam->realmHost.origRealm, NULL};
	osList_append(&marAvpList, &avpOrigRealm);

	//dest-host
	diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pMarParam->optAvpList, DIA_AVP_CODE_DEST_HOST);
	if(pOptAvp)
	{
		osList_append(&marAvpList, pOptAvp);
	}

	//dest-realm
	diaEncodeAvp_t avpDestRealm = {DIA_AVP_CODE_DEST_REALM, (diaEncodeAvpData_u)&pMarParam->realmHost.destRealm, NULL};
	osList_append(&marAvpList, &avpDestRealm);

	//user-name
	diaEncodeAvp_t avpUserName = {DIA_AVP_CODE_USER_NAME, (diaEncodeAvpData_u)&pMarParam->userName, NULL};
	osList_append(&marAvpList, &avpUserName);

	//OC-Supported-features
	pOptAvp = diaOptListFindAndRemoveAvp(pMarParam->pExtraOptAvpList, DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES);
	if(pOptAvp)
	{
		osList_append(&marAvpList, pOptAvp);
    }

	//supported-features
	while(1)
	{
		pOptAvp = diaOptListFindAndRemoveAvp(&pMarParam->optAvpList, DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
		if(!pOptAvp)
		{
			break;
		}
		osList_append(&marAvpList, pOptAvp);
	}

   	//pub-id
	diaEncodeAvp_t avpPubId = {DIA_AVP_CODE_CX_PUBLIC_ID, (diaEncodeAvpData_u)&pMarParam->pubId, NULL};
	osList_append(&marAvpList, &avpPubId);

    //sip-auth-data-item
	diaEncodeAvp_t avpAuthData = {DIA_AVP_CODE_CX_SIP_AUTH_DATA_ITEM, (diaEncodeAvpData_u)(void*)pMarParam->authData, diaCxMar_encodeAuthData};
	osList_append(&marAvpList, &avpAuthData);

	//sip-number-auth-items
	diaEncodeAvp_t avpAuthItem = {DIA_AVP_CODE_CX_SIP_NUM_AUTH_ITEMS, (diaEncodeAvpData_u)pMarParam->authItem, NULL};
	osList_append(&marAvpList, &avpAuthItem);

	//server-name
	diaEncodeAvp_t avpServerName = {DIA_AVP_CODE_CX_SERVER_NAME, (diaEncodeAvpData_u)&pMarParam->serverName, NULL};
	osList_append(&marAvpList, &avpServerName);

	if(osList_getCount(&pMarParam->optAvpList) > 0)
	{
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pMarParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&marAvpList, pOptAvp);
        }
	}
			
	if(osList_getCount(pMarParam->pExtraOptAvpList) > 0)
	{
		diaEncodeAvp_t *pAvpProxyInfo, *pAvpRRInfo;

		pOptAvp = diaOptListFindAndRemoveAvp(pMarParam->pExtraOptAvpList, DIA_AVP_CODE_PROXY_INFO);
		if(pOptAvp)
		{
			pAvpProxyInfo = pOptAvp;
		}
		pOptAvp = diaOptListFindAndRemoveAvp(pMarParam->pExtraOptAvpList, DIA_AVP_CODE_ROUTE_RECORD);
		if(pOptAvp)
        {
            pAvpRRInfo = pOptAvp;
        }

		while(1)
		{
			pOptAvp = diaOptListGetNextAndRemoveAvp(pMarParam->pExtraOptAvpList);
			if(!pOptAvp)
			{
				break;
			}
			osList_append(&marAvpList, pOptAvp);
		}

		if(pAvpProxyInfo)
		{
			osList_append(&marAvpList, pAvpProxyInfo);
		}

		if(pAvpRRInfo)
		{
			osList_append(&marAvpList, pAvpRRInfo);
		}
	}

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_MAR, 0xc0, DIA_APP_ID_3GPP_CX, &marAvpList, &pHdrSessInfo->hdrInfo);

EXIT:
	return pDiaBuf;
}


//pList contains extra optional AVPs
osMBuf_t* diaBuildMar(osVPointerLen_t* userName, osVPointerLen_t* pubId, diaCxMarSipAuthDataItem_t* pAuthData, osVPointerLen_t* serverName, osVPointerLen_t* pDestHost, diaAvp_supportedFeature_t* pSF, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo)
{

	if(!userName || !pubId || !serverName || !pAuthData)
	{
		logError("null pointer for mandatory parameters, userName=%p, pubId=%p, serverName=%p, pAuthData=%p.", userName, pubId, serverName, pAuthData);
		return NULL;
	}

	diaCxMarParam_t marParam = {};

	diaConfig_getHostRealm(&marParam.realmHost);
	diaEncodeAvp_t destHost = {DIA_AVP_CODE_DEST_HOST, (diaEncodeAvpData_u)pDestHost, NULL};
	if(pDestHost)
	{
		osList_append(&marParam.optAvpList, &destHost);
	}

	marParam.userName = *userName;
	marParam.pubId = *pubId;
	marParam.serverName = *serverName;
    marParam.authItem = 1;
	marParam.authData = pAuthData;
	if(pAuthData->sipAuthScheme == DIA_CX_AUTH_SCHEME_DIGEST_AKA)
	{
		marParam.authItem = pAuthData->authItem;
	}

	if(pSF)
	{
#if 1
#if 0
		diaAvp_supportedFeature_t sfData;
		sf.flNum = 2;
		sf.fl[0].vendorId = DIA_AVP_VENDOR_ID_3GPP;
		sf.fl[0].featureListId = 1;
		sf.fl[0].featureList = 0x2000;
    	sf.fl[1].vendorId = DIA_AVP_VENDOR_ID_3GPP;
    	sf.fl[1].featureListId = 2;
    	sf.fl[1].featureList = 0x3000;
#endif
		diaEncodeAvp_t sf = {DIA_AVP_CODE_CX_SUPPORTED_FEATURE, (diaEncodeAvpData_u)(void*)pSF, diaAvp_encodeSupportedFeature};
debug("to-remove, insert avp=%d", DIA_AVP_CODE_CX_SUPPORTED_FEATURE);
    	osList_append(&marParam.optAvpList, &sf);
#else
		diaEncodeAvpGroupedData_t gData[DIA_MAX_SAME_AVP_NUM];
		diaEncodeAvpData_u sfData[DIA_MAX_SAME_AVP_NUM];
	 	diaEncodeAvp_t sf[DIA_MAX_SAME_AVP_NUM];
		for(int i=0; i<pSF->flNum; i++)
		{
    		gData[i].avpNum = 3;
			gData[i].pDiaAvp = osmalloc(sizeof(diaEncodeAvp_t)*3, NULL);
			diaAvpEncode_setValue(&gData[i].pDiaAvp[0], DIA_AVP_CODE_VENDOR_ID, DIA_AVP_VENDOR_ID_3GPP, NULL);
			diaAvpEncode_setValue(&gData[i].pDiaAvp[1], DIA_AVP_CODE_CX_FEATURE_LIST_ID, pSF->featureListId, NULL);
			diaAvpEncode_setValue(&gData[i].pDiaAvp[2], DIA_AVP_CODE_CX_FEATURE_LIST, pSF->featureList, NULL);
			sfData[i].pDataGrouped = &gData[i]; 
    		diaAvpEncode_setValue(&sf[i], DIA_AVP_CODE_CX_SUPPORTED_FEATURE, &sfData[i], NULL);
    		osList_append(&marParam.optAvpList, &sf[i]);
		}
#endif
	}

	marParam.pExtraOptAvpList = pExtraOptList;
	return diaCxMar_encode(&marParam, pHdrSessInfo);
}


static osStatus_e diaCxMar_encodeAuthData(osMBuf_t* pDiaBuf, void* pData)
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
        logError("pDiaBuf does not start from 4 byte boundary, pDiaBuf->pos=%ld.", pDiaBuf->pos);
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }

	diaCxMarSipAuthDataItem_t* pAuthData = pData;

    diaEncodeAvpGroupedData_t gAvpData;
    osVPointerLen_t schemeAvpData;
	osVPL_init(&schemeAvpData, false);

    //for aka, there may have 5 sub avps, to allocate 6 avp space first just in case.
    diaEncodeAvp_t avp[6];
    gAvpData.pDiaAvp = avp;
	diaAvpEncode_setValue(&avp[0], DIA_AVP_CODE_CX_SIP_AUTH_SCHEME, (diaEncodeAvpData_u)&schemeAvpData, NULL);
	if(pAuthData->isReq)
	{
	    gAvpData.avpNum = 1;

		switch(pAuthData->sipAuthScheme)
		{
			case DIA_CX_AUTH_SCHEME_UNKNOWN:		//"Unknown"
                osVPL_setStr(&schemeAvpData, "Unknown", 7, false);
                break;
    		case DIA_CX_AUTH_SCHEME_DIGEST_AKA:		//"Digest-AKAv1-MD5"
                osVPL_setStr(&schemeAvpData, "Digest-AKAv1-MD5", 16, false);
    			//aka auth and contains auth-info
				if(pAuthData->reqData.sipAuthorization_onlyAka.pl.l)
				{
					gAvpData.avpNum = 2;
    	            diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_SIP_AUTHORIZATION, (diaEncodeAvpData_u)&pAuthData->reqData.sipAuthorization_onlyAka, NULL);
				}
                break;
    		case DIA_CX_AUTH_SCHEME_SIP_DIGEST:     //"SIP Digest"
				osVPL_setStr(&schemeAvpData, "SIP Digest", 10, false);	
                break;
    		case DIA_CX_AUTH_SCHEME_EARLY_IMS:      //"Early IMS Security"
				osVPL_setStr(&schemeAvpData, "Early IMS Security", 18, false);
				break; 
            case DIA_CX_AUTH_SCHEME_NASS_BUNDLED:   //"NASS-Bundled"
			default:
				logError("the auth scheme(%d) is not supported.", pAuthData->sipAuthScheme);
				goto EXIT;
                break;
		}
	}
	else
	{
        switch(pAuthData->sipAuthScheme)
        {
            case DIA_CX_AUTH_SCHEME_DIGEST_AKA:     //"Digest-AKAv1-MD5"
                osVPL_setStr(&schemeAvpData, "Digest-AKAv1-MD5", 16, false);
                gAvpData.avpNum = 6;
                diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_SIP_ITEM_NUM, (diaEncodeAvpData_u) pAuthData->respData.authAka.itemOrder, NULL);
                diaAvpEncode_setValue(&avp[2], DIA_AVP_CODE_CX_SIP_AUTHENTICATE, (diaEncodeAvpData_u)&pAuthData->respData.authAka.sipAuthenticate, NULL);
                diaAvpEncode_setValue(&avp[3], DIA_AVP_CODE_CX_SIP_AUTHORIZATION, (diaEncodeAvpData_u)&pAuthData->respData.authAka.sipAuthorization, NULL);
                diaAvpEncode_setValue(&avp[4], DIA_AVP_CODE_CX_CONFIDENTIALITY_KEY, (diaEncodeAvpData_u)&pAuthData->respData.authAka.ck, NULL);
                diaAvpEncode_setValue(&avp[5], DIA_AVP_CODE_CX_INTEGRITY_KEY, (diaEncodeAvpData_u)&pAuthData->respData.authAka.ik, NULL);
                break;
            case DIA_CX_AUTH_SCHEME_SIP_DIGEST:     //"SIP Digest"
                osVPL_setStr(&schemeAvpData, "SIP Digest", 10, false);
                gAvpData.avpNum = 4;
                diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_DIGEST_REALM, (diaEncodeAvpData_u)&pAuthData->respData.authDigest.digestRealm, NULL);
                diaAvpEncode_setValue(&avp[2], DIA_AVP_CODE_CX_DIGEST_QOP, (diaEncodeAvpData_u)&pAuthData->respData.authDigest.digestQop, NULL);
                diaAvpEncode_setValue(&avp[3], DIA_AVP_CODE_CX_DIGEST_HA1, (diaEncodeAvpData_u)&pAuthData->respData.authDigest.digestHa1, NULL);
                break;
            case DIA_CX_AUTH_SCHEME_EARLY_IMS:      //"Early IMS Security"
                osVPL_setStr(&schemeAvpData, "Early IMS Security", 18, false);
                gAvpData.avpNum = 2;
				if(pAuthData->respData.authGiba.isIPv4)
				{
                    diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_REUSE_FRAMED_IP_ADDRESS, (diaEncodeAvpData_u)&pAuthData->respData.authGiba.framedIP, NULL);
				}
				else
				{
			    	diaAvpEncode_setValue(&avp[1], DIA_AVP_CODE_CX_REUSE_FRAMED_IPV6_PREFIX, (diaEncodeAvpData_u)&pAuthData->respData.authGiba.framedIPv6Prefix, NULL);
				}
                break;
            case DIA_CX_AUTH_SCHEME_UNKNOWN:        //"Unknown"
            case DIA_CX_AUTH_SCHEME_NASS_BUNDLED:   //"NASS-Bundled"
            default:
                logError("the auth scheme(%d) is not supported.", pAuthData->sipAuthScheme);
                goto EXIT;
                break;
        }

	}

	status = diaAvp_encode(pDiaBuf, DIA_CMD_CODE_MAR, DIA_AVP_CODE_CX_SIP_AUTH_DATA_ITEM, (diaEncodeAvpData_u) &gAvpData);

EXIT:
	return status;
}


#if 0
diaEncodeAvp_t* pGrpAvp = diaAvpGrp_create(DIA_AVP_CODE_CX_SERVER_CAPABILITY);
for(int j=0; j<pCap->mscNum; j++)
{
	if(!diaAvpGrp_addAvp(pGrpAvp, DIA_AVP_CODE_CX_MANDATORY_CAPABILITY, (diaEncodeAvpData_u)pCap->msc[j], NULL))
	{
		logError("fails to diaAvpGrp_addAvp, pGrpAvp->avpNum=%d.", pGrpAvp->avpNum);
		diaAvpGrp_cleanup(pGrpAvp);
	}
}

for(int j=0; j<pCap->oscNum; j++) 
{
	if(!diaAvpGrp_addAvp(pGrpAvp, DIA_AVP_CODE_CX_OPTIONAL_CAPABILITY, (diaEncodeAvpData_u)pCap->osc[j], NULL))
	{
        logError("fails to diaAvpGrp_addAvp, pGrpAvp->avpNum=%d.", pGrpAvp->avpNum);
        diaAvpGrp_cleanup(pGrpAvp);
    }
}

for(int j=0; j<pCap->serverNameNum; j++)
{
	if(!diaAvpGrp_addAvp(pGrpAvp, DIA_AVP_CODE_CX_SERVER_NAME, (diaEncodeAvpData_u)&pCap->serverName[j], NULL))
	{
		logError("fails to diaAvpGrp_addAvp, pGrpAvp->avpNum=%d.", pGrpAvp->avpNum);
        diaAvpGrp_cleanup(pGrpAvp);
    }
}

//after the group AVP has been encoded into a diameter message, need to clean up the allocated memory
diaAvpGrp_cleanup(pGrpAvp);
#endif
