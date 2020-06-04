#include "string.h"
#include <arpa/inet.h>
#include <endian.h>

#include "osPL.h"
#include "osMemory.h"
#include "osTypes.h"
#include "osSockAddr.h"

#include "diaConfig.h"
#include "diaMsg.h"
#include "diaAvp.h"


static osPointerLen_t gPlDiaHostIp[DIA_MAX_HOST_IP_NUM];
static uint32_t diaHostNum;


void diaConfig_init()
{
    //for now, only support 1 host IP.
	char* pHostIp = osmalloc(6, NULL);
	diaWriteU16(pHostIp, htobe16(OS_AF_IPv4));

    struct sockaddr_in saddr;
    inet_pton(AF_INET, DIA_CONFIG_LOCAL_IP, &saddr.sin_addr.s_addr);

	//saddr.sin_addr.s_addr already in big enden order
    diaWriteU32(&pHostIp[2], saddr.sin_addr.s_addr);	

	//for now, only support 1 host IP.
	gPlDiaHostIp[0].p = pHostIp;
	gPlDiaHostIp[0].l = 6;
	diaHostNum = 1;
}
		

void diaConfig_getHostRealm(diaRealmHost_t* pRealmHost)
{
	if(!pRealmHost)
	{
		logError("null pointer, pRealmHost.");
		return;
	}

	osPL_setStr(&pRealmHost->origRealm, DIA_ORIG_REALM, strlen(DIA_ORIG_REALM));  
	osPL_setStr(&pRealmHost->origHost, DIA_ORIG_HOST, strlen(DIA_ORIG_HOST));
	osPL_setStr(&pRealmHost->destRealm, DIA_DEST_REALM, strlen(DIA_DEST_REALM));
	osPL_init(&pRealmHost->destHost);
} 


void diaConfig_getHost(osPointerLen_t* host, int* port)
{
    host->p = DIA_CONFIG_LOCAL_IP;
    host->l = strlen(DIA_CONFIG_LOCAL_IP);
    *port = DIA_CONFIG_LISTEN_PORT;
}


void diaConfig_getHost1(struct sockaddr_in* pHost)
{
    pHost->sin_family = AF_INET;
    inet_pton(AF_INET, DIA_CONFIG_LOCAL_IP, &pHost->sin_addr);
    pHost->sin_port = htons(DIA_CONFIG_LISTEN_PORT);
}


void diaConfig_getPeer(diaIntfType_e intfType, osIpPort_t* pPeer)
{
	if(!pPeer)
	{
		logError("null pointer, pPeer.");
		return;
	}

	pPeer->ip.p = DIA_CONFIG_PEER_IP;
	pPeer->ip.l = strlen(DIA_CONFIG_PEER_IP);
	pPeer->port = DIA_CONFIG_PEER_PORT;

	return;
}


bool diaConfig_isServer()
{
	return DIA_IS_SERVER;
}


osListPlus_t* diaConfig_getHostIpList(diaIntfType_e intfType, osListPlus_t* pList)
{
	if(!pList)
	{
		pList = oszalloc(sizeof(osListPlus_t), NULL);
		if(!pList)
		{
			logError("null pointer, pList.");
			return NULL;
		}
	}

	pList->num = 1;
	pList->first = &gPlDiaHostIp[0];
	return pList;
}


osVPointerLen_t* diaConfig_getProductName(diaIntfType_e intfType, osVPointerLen_t* pProductName)
{
	if(!pProductName)
	{
		pProductName = osmalloc(sizeof(osVPointerLen_t), NULL);
		if(!pProductName)
		{
			logError("null pointer, pProductName.");
			return NULL;
		}
	}

	pProductName->pl.p = DIA_PRODUCT_NAME;
	pProductName->pl.l = strlen(DIA_PRODUCT_NAME);
	pProductName->isPDynamic = false;

	return pProductName;
}


//there is no assigned vendor id for sean's dia stack, uses reserved for now
uint32_t diaConfig_getDiaVendorId()
{
	return DIA_VENDOR_ID_RESERVED;
}


osList_t* diaConfig_getSupportedVendorId(diaIntfType_e intfType, osList_t* pSupportedVendorId)
{
	if(!pSupportedVendorId)
	{
		pSupportedVendorId = oszalloc(sizeof(osList_t), NULL);
		if(!pSupportedVendorId)
		{
			logError("null pointer, pSupportedVendorId.");
            return NULL;
        }
    }

	uint32_t* pVendorId = osmalloc(sizeof(uint32_t)*2, NULL);
	pVendorId[0] = DIA_VENDOR_ID_3GPP;
	pVendorId[1] = DIA_VENDOR_ID_ETSI;
	osList_append(pSupportedVendorId, &pVendorId[0]);
	osList_append(pSupportedVendorId, &pVendorId[1]);

	return pSupportedVendorId;	
}


osList_t* diaConfig_getAuthAppId(diaIntfType_e intfType, osList_t* pAuthAppIdList)
{
	osList_t* pAuthAppId = pAuthAppIdList;
	if(!pAuthAppId)
    {
        pAuthAppId = oszalloc(sizeof(osList_t), NULL);
        if(!pAuthAppId)
        {
            logError("null pointer, pAuthAppId.");
            return NULL;
        }
    }

    uint32_t* pAuthId = osmalloc(sizeof(uint32_t), NULL);
	switch(intfType)
	{
		case DIA_INTF_TYPE_CX:
			*pAuthId = DIA_APP_ID_3GPP_CX;
			break;
		case DIA_INTF_TYPE_GX:
            *pAuthId = DIA_APP_ID_3GPP_GX;
			break;			
    	case DIA_INTF_TYPE_RX:
            *pAuthId = DIA_APP_ID_3GPP_RX;
			break;
    	case DIA_INTF_TYPE_SH:
            *pAuthId = DIA_APP_ID_3GPP_SH;
			break;
		default:
			if(!pAuthAppIdList)
			{
				osfree(pAuthAppId);
			}
			osfree(pAuthId);
			return NULL;
			break;
	}

    osList_append(pAuthAppId, pAuthId);

	return pAuthAppId;
}


osList_t* diaConfig_getAcctAppId(diaIntfType_e intfType, osList_t* pAcctAppId)
{
	//for now do not advertise acct support
	return NULL;
}


osList_t* diaConfig_getVendorSpecificAppId(diaIntfType_e intfType, osList_t* pVSAppIdList)
{
	osList_t* pVSAppId = pVSAppIdList; 
    if(!pVSAppId)
    {
        pVSAppId = oszalloc(sizeof(osList_t), NULL);
        if(!pVSAppId)
        {
            logError("null pointer, pVSAppId.");
            return NULL;
        }
    }

	diaEncodeAvpData_u vendorAvpData;
	diaEncodeAvpData_u authAvpData;
    switch(intfType)
    {
        case DIA_INTF_TYPE_CX:
			vendorAvpData = (diaEncodeAvpData_u) DIA_VENDOR_ID_3GPP;
			authAvpData = (diaEncodeAvpData_u) DIA_APP_ID_3GPP_CX;
            break;
        case DIA_INTF_TYPE_GX:
			vendorAvpData = (diaEncodeAvpData_u) DIA_VENDOR_ID_3GPP;
            authAvpData = (diaEncodeAvpData_u) DIA_APP_ID_3GPP_GX;
            break;
        case DIA_INTF_TYPE_RX:
            vendorAvpData = (diaEncodeAvpData_u) DIA_VENDOR_ID_3GPP;
            authAvpData = (diaEncodeAvpData_u) DIA_APP_ID_3GPP_RX;
            break;
        case DIA_INTF_TYPE_SH:
            vendorAvpData = (diaEncodeAvpData_u) DIA_VENDOR_ID_3GPP;
            authAvpData = (diaEncodeAvpData_u) DIA_APP_ID_3GPP_SH;
            break;
        default:
			if(!pVSAppIdList)
			{
            	osfree(pVSAppId);
			}
            return NULL;
            break;
    }

	diaEncodeAvp_t* pGrpAvp = diaAvpGrp_create(DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID);
	diaAvpGrp_addAvp(pGrpAvp, DIA_AVP_CODE_VENDOR_ID, vendorAvpData, NULL);
	diaAvpGrp_addAvp(pGrpAvp, DIA_AVP_CODE_AUTH_APP_ID, authAvpData, NULL); 
    osList_append(pVSAppId, pGrpAvp);

    return pVSAppId;
}


bool diaConfig_getFirmwareRev(uint32_t* pFWRev)
{
	*pFWRev = DIA_FIRMWARE_REVISION;
	return true;
}
