#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "osList.h"
#include "osMemory.h"
#include "osPL.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaAvpEncode.h"
#include "diaBaseCer.h"
#include "diaConfig.h"
#include "diaMgr.h"



//the number of seconds since the start of the Unix epoch at ~20200426, UTC 16:15:00
#define DIA_BASE_CER_ORIG_STATE_EPOCH_VALUE	0x5ea5cfa4		//1587924900
/*
 * rfc6733, section 5.3.1
 *
 * <CER> ::= < Diameter Header: 257, REQ >
 *                  { Origin-Host }
 *                  { Origin-Realm }
 *               1* { Host-IP-Address }
 *                  { Vendor-Id }
 *                  { Product-Name }
 *                  [ Origin-State-Id ]
 *                * [ Supported-Vendor-Id ]
 *                * [ Auth-Application-Id ]
 *                * [ Inband-Security-Id ]
 *                * [ Acct-Application-Id ]
 *                * [ Vendor-Specific-Application-Id ]
 *                  [ Firmware-Revision ]
 *                * [ AVP ]
 */
osMBuf_t* diaCxCer_encode(diaBaseCerParam_t* pCerParam, diaCmdHdrInfo_t* pCmdHdrInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;
    osList_t cerAvpList = {};       //each element contains diaEncodeAvp_t

    if(!pCerParam || !pCmdHdrInfo)
    {
        logError("null pointer, pCerParam=%p, pCmdHdrInfo=%p.", pCerParam, pCmdHdrInfo);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	//orig-host
	diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pCerParam->realmHost.origHost, NULL};
	osList_append(&cerAvpList, &avpOrigHost);

	//orig-realm
	diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pCerParam->realmHost.origRealm, NULL};
	osList_append(&cerAvpList, &avpOrigRealm);

	//host-ip-address
	if(pCerParam->hostIpList->num < 1)
	{
		logError("there shall be at least 1 host IP.");
		status = OS_ERROR_INVALID_VALUE;
		goto EXIT;
	}

    diaEncodeAvp_t avpHostIp[DIA_MAX_SAME_AVP_NUM];
    diaAvpEncode_setValue(&avpHostIp[0], DIA_AVP_CODE_HOST_IP_ADDRESS, (diaEncodeAvpData_u)pCerParam->hostIpList->first, NULL);
	osList_append(&cerAvpList, &avpHostIp[0]);
	if(pCerParam->hostIpList->num > 1)
	{
		osListElement_t* pLE = pCerParam->hostIpList->more.head;
		int i=1;
		while(pLE)
		{
			diaAvpEncode_setValue(&avpHostIp[i], DIA_AVP_CODE_HOST_IP_ADDRESS, (diaEncodeAvpData_u)pLE->data, NULL);
			osList_append(&cerAvpList, &avpHostIp[i]);
			pLE = pLE->next;
			if(++i >= DIA_MAX_SAME_AVP_NUM)
			{
				logError("avpHostIp number(%d) exceeds DIA_MAX_SAME_AVP_NUM(%d).", i, DIA_MAX_SAME_AVP_NUM);
				break;
			}
		}
	}

	//vendor-id
	diaEncodeAvp_t avpVendorId = {DIA_AVP_CODE_VENDOR_ID, (diaEncodeAvpData_u)pCerParam->vendorId, NULL};
	osList_append(&cerAvpList, &avpVendorId);

	//product-name
	diaEncodeAvp_t avpProductName = {DIA_AVP_CODE_PRODUCT_NAME, (diaEncodeAvpData_u)pCerParam->productName, NULL};
	osList_append(&cerAvpList, &avpProductName);
		
	//orig-state-id
    diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_ORIG_STATE_ID); 
    if(pOptAvp)
    {
        osList_append(&cerAvpList, pOptAvp);
    }

	//supported-vendor-id
	while(1)
	{
		pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_SUPPORTED_VENDOR_ID);
		if(!pOptAvp)
		{
			break;
		}

		osList_append(&cerAvpList, pOptAvp);
	}

	//Auth-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_AUTH_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&cerAvpList, pOptAvp);
    }

	//Acct-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_ACCT_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&cerAvpList, pOptAvp);
    }

	//Vendor-Specific-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&cerAvpList, pOptAvp);
    }

	//firmware-revision
	pOptAvp = diaOptListFindAndRemoveAvp(&pCerParam->optAvpList, DIA_AVP_CODE_FIRMWARE_REV);
	if(pOptAvp)
    {
        osList_append(&cerAvpList, pOptAvp);
    }
	

    if(osList_getCount(&pCerParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pCerParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&cerAvpList, pOptAvp);
        }
    }

	if(osList_getCount(pCerParam->pExtraOptAvpList) > 0)
	{
		 while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(pCerParam->pExtraOptAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&cerAvpList, pOptAvp);
        }
    }

	pDiaBuf = diaMsg_encode(DIA_CMD_CODE_CER, 0x80, DIA_APP_ID_BASE, &cerAvpList, pCmdHdrInfo);

EXIT:
	osList_clear(&cerAvpList);
	return pDiaBuf;
}



//pList contains extra optional AVPs
//firmware revision is a pointer so that we can base on the pointer value to determine if the avp exists
osMBuf_t* diaBuildCer(osListPlus_t* pHostIpList, uint32_t vendorId, osVPointerLen_t* productName, osList_t* pSupportedVendorId, osList_t* pAuthAppId, osList_t* pAcctAppId, osList_t* pVendorSpecificAppId, uint32_t* firmwareRev, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo)
{
    osMBuf_t* pBuf = NULL;
    diaBaseCerParam_t cerParam = {};

	if(!pHostIpList || !productName || !pCmdHdrInfo)
	{
		logError("null pointer for mandatory parameters, pHostIpList=%p, productName=%p, pCmdHdrInfo=%p.", pHostIpList, productName, pCmdHdrInfo);
		return NULL;
	}

	diaConfig_getHostRealm(&cerParam.realmHost);

	cerParam.hostIpList = pHostIpList;
	cerParam.vendorId = vendorId;
	cerParam.productName = productName;

	//orig-state-id
	diaEncodeAvp_t avpVendorId = {DIA_AVP_CODE_ORIG_STATE_ID, (diaEncodeAvpData_u)diaGetOrigStateId(), NULL};
	osList_append(&cerParam.optAvpList, &avpVendorId);

	diaEncodeAvp_t supportedVendorIdAvp[DIA_MAX_SAME_AVP_NUM];
	if(pSupportedVendorId)
	{
		int i=0;
		osListElement_t* pLE = pSupportedVendorId->head;
		while(pLE)
		{
			diaAvpEncode_setValue(&supportedVendorIdAvp[i], DIA_AVP_CODE_SUPPORTED_VENDOR_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL);
//			diaEncodeAvp_t avpSupportedId = {DIA_AVP_CODE_SUPPORTED_VENDOR_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL};
			osList_append(&cerParam.optAvpList, &supportedVendorIdAvp[i]);
			pLE = pLE->next;
			if(++i >= DIA_MAX_SAME_AVP_NUM)
			{
				logError("avp DIA_AVP_CODE_SUPPORTED_VENDOR_ID exceeds DIA_MAX_SAME_AVP_NUM(%d).", DIA_MAX_SAME_AVP_NUM);
				break;
			}
		}
	}

	diaEncodeAvp_t authAppIdAvp[DIA_MAX_SAME_AVP_NUM];
    if(pAuthAppId)
    {
		int i=0;
        osListElement_t* pLE = pAuthAppId->head;
        while(pLE)
        {
			diaAvpEncode_setValue(&authAppIdAvp[i], DIA_AVP_CODE_AUTH_APP_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL); 
            //diaEncodeAvp_t avpAuthAppId = {DIA_AVP_CODE_AUTH_APP_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL};
            osList_append(&cerParam.optAvpList, &authAppIdAvp[i]);
            pLE = pLE->next;
            if(++i >= DIA_MAX_SAME_AVP_NUM)
            {
                logError("avp DIA_AVP_CODE_AUTH_APP_ID exceeds DIA_MAX_SAME_AVP_NUM(%d).", DIA_MAX_SAME_AVP_NUM);
                break;
            }
        }
    }

	diaEncodeAvp_t acctAppIdAvp[DIA_MAX_SAME_AVP_NUM];
    if(pAcctAppId)
    {
		int i=0;
        osListElement_t* pLE = pAcctAppId->head;
        while(pLE)
        {
			diaAvpEncode_setValue(&acctAppIdAvp[i], DIA_AVP_CODE_ACCT_APP_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL);
            //diaEncodeAvp_t avpAcctAppId = {DIA_AVP_CODE_ACCT_APP_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL};
            osList_append(&cerParam.optAvpList, &acctAppIdAvp[i]);
            pLE = pLE->next;
            if(++i >= DIA_MAX_SAME_AVP_NUM)
            {
                logError("avp DIA_AVP_CODE_ACCT_APP_ID exceeds DIA_MAX_SAME_AVP_NUM(%d).", DIA_MAX_SAME_AVP_NUM);
                break;
            }
        }
    }

    if(pVendorSpecificAppId)
    {
        osListElement_t* pLE = pVendorSpecificAppId->head;
        while(pLE)
        {
            //diaEncodeAvp_t avpVSAppId = {DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL};
            osList_append(&cerParam.optAvpList, pLE->data);
            pLE = pLE->next;
        }
    }

	diaEncodeAvp_t avpFWRev;
	if(firmwareRev)
	{
		diaAvpEncode_setValue(&avpFWRev, DIA_AVP_CODE_FIRMWARE_REV, (diaEncodeAvpData_u)*firmwareRev, NULL);
		osList_append(&cerParam.optAvpList, &avpFWRev);
	}

	cerParam.pExtraOptAvpList = pExtraOptList;

	pBuf = diaCxCer_encode(&cerParam, pCmdHdrInfo);

EXIT:
	osList_clear(&cerParam.optAvpList);
	return pBuf;
	
}


/*
 * rfc6733, section 5.3.2
 * <CEA> ::= < Diameter Header: 257 >
 *                  { Result-Code }
 *                  { Origin-Host }
 *                  { Origin-Realm }
 *               1* { Host-IP-Address }
 *                  { Vendor-Id }
 *                  { Product-Name }
 *                  [ Origin-State-Id ]
 *                  [ Error-Message ]
 *                  [ Failed-AVP ]
 *                * [ Supported-Vendor-Id ]
 *                * [ Auth-Application-Id ]
 *                * [ Inband-Security-Id ]
 *                * [ Acct-Application-Id ]
 *                * [ Vendor-Specific-Application-Id ]
 *                  [ Firmware-Revision ]
 *                * [ AVP ]
 *
 */
osMBuf_t* diaCxCea_encode(diaBaseCeaParam_t* pCeaParam, diaCmdHdrInfo_t* pCmdHdrInfo)
{
    osStatus_e status = OS_STATUS_OK;
    osMBuf_t* pDiaBuf = NULL;
    osList_t ceaAvpList = {};       //each element contains diaEncodeAvp_t

    if(!pCeaParam)
    {
        logError("null pointer, pCeaParam=%p.", pCeaParam);
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	//result-code
	diaEncodeAvp_t avpResultCode = {DIA_AVP_CODE_RESULT_CODE, (diaEncodeAvpData_u)pCeaParam->resultCode.resultCode, NULL};
	osList_append(&ceaAvpList, &avpResultCode);

    //orig-host
    diaEncodeAvp_t avpOrigHost = {DIA_AVP_CODE_ORIG_HOST, (diaEncodeAvpData_u)&pCeaParam->realmHost.origHost, NULL};
    osList_append(&ceaAvpList, &avpOrigHost);

    //orig-realm
    diaEncodeAvp_t avpOrigRealm = {DIA_AVP_CODE_ORIG_REALM, (diaEncodeAvpData_u)&pCeaParam->realmHost.origRealm, NULL};
    osList_append(&ceaAvpList, &avpOrigRealm);

    //host-ip-address
    if(pCeaParam->hostIpList->num < 1)
    {
        logError("there shall be at least 1 host IP.");
        status = OS_ERROR_INVALID_VALUE;
        goto EXIT;
    }
	diaEncodeAvp_t avpHostIp[DIA_MAX_SAME_AVP_NUM];
    diaAvpEncode_setValue(&avpHostIp[0], DIA_AVP_CODE_HOST_IP_ADDRESS, (diaEncodeAvpData_u)pCeaParam->hostIpList->first, NULL);
    osList_append(&ceaAvpList, &avpHostIp[0]);
    if(pCeaParam->hostIpList->num > 1)
    {
        osListElement_t* pLE = pCeaParam->hostIpList->more.head;
		int i=1;
        while(pLE)
        {
			diaAvpEncode_setValue(&avpHostIp[i], DIA_AVP_CODE_HOST_IP_ADDRESS, (diaEncodeAvpData_u)pLE->data, NULL);
            osList_append(&ceaAvpList, &avpHostIp[i]);
            pLE = pLE->next;
            if(++i >= DIA_MAX_SAME_AVP_NUM)
            {
                logError("avpHostIp number(%d) exceeds DIA_MAX_SAME_AVP_NUM(%d).", i, DIA_MAX_SAME_AVP_NUM);
                break;
            }
        }
    }

    //vendor-id
    diaEncodeAvp_t avpVendorId = {DIA_AVP_CODE_VENDOR_ID, (diaEncodeAvpData_u)pCeaParam->vendorId, NULL};
    osList_append(&ceaAvpList, &avpVendorId);

    //product-name
    diaEncodeAvp_t avpProductName = {DIA_AVP_CODE_PRODUCT_NAME, (diaEncodeAvpData_u)pCeaParam->productName, NULL};
    osList_append(&ceaAvpList, &avpProductName);

    //orig-state-id
    diaEncodeAvp_t* pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_ORIG_STATE_ID);
    if(pOptAvp)
    {
        osList_append(&ceaAvpList, pOptAvp);
    }

    //supported-vendor-id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_SUPPORTED_VENDOR_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&ceaAvpList, pOptAvp);
    }

    //Auth-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_AUTH_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&ceaAvpList, pOptAvp);
    }

    //Acct-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_ACCT_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&ceaAvpList, pOptAvp);
    }

    //Vendor-Specific-Application-Id
    while(1)
    {
        pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID);
        if(!pOptAvp)
        {
            break;
        }

        osList_append(&ceaAvpList, pOptAvp);
    }

    //firmware-revision
    pOptAvp = diaOptListFindAndRemoveAvp(&pCeaParam->optAvpList, DIA_AVP_CODE_FIRMWARE_REV);
    if(pOptAvp)
    {
        osList_append(&ceaAvpList, pOptAvp);
    }


    if(osList_getCount(&pCeaParam->optAvpList) > 0)
    {
        while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(&pCeaParam->optAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&ceaAvpList, pOptAvp);
        }
    }

    if(osList_getCount(pCeaParam->pExtraOptAvpList) > 0)
    {
         while(1)
        {
            pOptAvp = diaOptListGetNextAndRemoveAvp(pCeaParam->pExtraOptAvpList);
            if(!pOptAvp)
            {
                break;
            }
            osList_append(&ceaAvpList, pOptAvp);
        }
    }

    pDiaBuf = diaMsg_encode(DIA_CMD_CODE_CER, 0x00, DIA_APP_ID_BASE, &ceaAvpList, pCmdHdrInfo);

EXIT:
	osList_clear(&ceaAvpList);
    return pDiaBuf;
}



//pList contains extra optional AVPs
osMBuf_t* diaBuildCea(diaResultCode_e resultCode, osListPlus_t* pHostIpList, uint32_t vendorId, osVPointerLen_t* productName, osList_t* pSupportedVendorId, osList_t* pAuthAppId, osList_t* pAcctAppId, osList_t* pVendorSpecificAppId, uint32_t* firmwareRev, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo)
{
	osMBuf_t* pBuf;
    diaBaseCeaParam_t ceaParam = {};

    if(!pHostIpList || !productName || !pCmdHdrInfo)
    {
        logError("null pointer for mandatory parameters, pHostIpList=%p, productName=%p, pCmdHdrInfo=%p.", pHostIpList, productName, pCmdHdrInfo);
        return NULL;
    }

	ceaParam.resultCode.isResultCode = true;
	ceaParam.resultCode.resultCode = resultCode;

    diaConfig_getHostRealm(&ceaParam.realmHost);

    ceaParam.hostIpList = pHostIpList;
    ceaParam.vendorId = vendorId;
    ceaParam.productName = productName;

    //orig-state-id
    diaEncodeAvp_t avpVendorId = {DIA_AVP_CODE_ORIG_STATE_ID, (diaEncodeAvpData_u)diaGetOrigStateId(), NULL};
    osList_append(&ceaParam.optAvpList, &avpVendorId);

    diaEncodeAvp_t avpSupportedId[DIA_MAX_SAME_AVP_NUM];
    if(pSupportedVendorId)
    {
		diaAvpAddList(&ceaParam.optAvpList, pSupportedVendorId, DIA_AVP_CODE_SUPPORTED_VENDOR_ID, DIA_AVP_ENCODE_DATA_TYPE_U32, avpSupportedId);
    }

    diaEncodeAvp_t avpAuthAppId[DIA_MAX_SAME_AVP_NUM];
    if(pAuthAppId)
    {
		diaAvpAddList(&ceaParam.optAvpList, pAuthAppId, DIA_AVP_CODE_AUTH_APP_ID, DIA_AVP_ENCODE_DATA_TYPE_U32, avpAuthAppId);
    }

    diaEncodeAvp_t avpAcctAppId[DIA_MAX_SAME_AVP_NUM];
    if(pAcctAppId)
    {
		diaAvpAddList(&ceaParam.optAvpList, pAcctAppId, DIA_AVP_CODE_ACCT_APP_ID, DIA_AVP_ENCODE_DATA_TYPE_U32, avpAcctAppId);
    }

    diaEncodeAvp_t avpVSAppId[DIA_MAX_SAME_AVP_NUM];
    if(pVendorSpecificAppId)
    {
		diaAvpAddList(&ceaParam.optAvpList, pVendorSpecificAppId, DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID, DIA_AVP_ENCODE_DATA_TYPE_GROUP, avpVSAppId);
    }

    if(firmwareRev)
    {
        diaEncodeAvp_t avpFWRev = {DIA_AVP_CODE_FIRMWARE_REV, (diaEncodeAvpData_u)*firmwareRev, NULL};
        osList_append(&ceaParam.optAvpList, &avpFWRev);
    }

    ceaParam.pExtraOptAvpList = pExtraOptList;

    pBuf = diaCxCea_encode(&ceaParam, pCmdHdrInfo);

EXIT:
	osList_clear(&ceaParam.optAvpList);
	return pBuf;
}


osStatus_e diaConnProcessCea(diaMsgDecoded_t* pDiaDecoded, struct diaConnBlock* pDcb)
{
    osStatus_e status = OS_STATUS_OK;

	if(!pDiaDecoded || !pDcb)
	{
		logError("null pointer, pDiaDecoded=%p, pDcb=%p.", pDiaDecoded, pDcb);
		return OS_ERROR_NULL_POINTER;
	}

    //starts from avp after sessionId
    osListElement_t* pLE = pDiaDecoded->avpList.head->next;
    while(pLE)
    {
        diaAvp_t* pAvp = pLE->data;
        switch(pAvp->avpCode)
        {
            case DIA_AVP_CODE_RESULT_CODE:
				if(pAvp->avpData.data32 >= 3000 || pAvp->avpData.data32 < 2000)
				{
					logInfo("CEA result code=%d, CER fails.", pAvp->avpData.data32);
					status = OS_ERROR_EVENT_FAILURE;
					goto EXIT;
				}
				break;
            case DIA_AVP_CODE_EXPERIMENTAL_RESULT_CODE:
                if(pAvp->avpData.data32 >= 3000 || pAvp->avpData.data32 < 2000)
                {
                    logInfo("CEA experimental result=%d, CER fails.", pAvp->avpData.data32);
                    status = OS_ERROR_EVENT_FAILURE;
                    goto EXIT;
                }
                break;
			case DIA_AVP_CODE_ORIG_HOST:
				osVPL_copyVPL(&pDcb->peerHost, &pAvp->avpData.dataStr);
                break;
			case DIA_AVP_CODE_ORIG_REALM:
				osVPL_copyVPL(&pDcb->peerRealm, &pAvp->avpData.dataStr);
				break;
			default:
				break;
		}

		pLE = pLE->next;
	}

EXIT:
	return status;
}
