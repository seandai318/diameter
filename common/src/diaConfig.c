#include "string.h"
#include <arpa/inet.h>
#include <endian.h>
#include <stdlib.h>

#include "osPL.h"
#include "osMemory.h"
#include "osTypes.h"
#include "osSockAddr.h"
#include "osMBuf.h"
#include "osXmlParserIntf.h"

#include "diaConfig.h"
#include "diaMsg.h"
#include "diaAvp.h"



//the order must be sorted based on the data name length.  for the data name with the same len, their orders do not matter
osXmlData_t diaConfig_xmlData[DIA_XML_MAX_DATA_NAME_NUM] = {
	{DIA_XML_HASH_SIZE,			{"DIA_HASH_SIZE", sizeof("DIA_HASH_SIZE")-1}, 			  OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_DEST_HOST,         {"DIA_DEST_HOST", sizeof("DIA_DEST_HOST")-1},             OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_IS_SERVER,         {"DIA_IS_SERVER", sizeof("DIA_IS_SERVER")-1},             OS_XML_DATA_TYPE_XS_BOOLEAN, 0, false},
    {DIA_XML_ORIG_HOST,         {"DIA_ORIG_HOST", sizeof("DIA_ORIG_HOST")-1},             OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_DEST_REALM,        {"DIA_DEST_REALM", sizeof("DIA_DEST_REALM")-1},           OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_ORIG_REALM,        {"DIA_ORIG_REALM", sizeof("DIA_ORIG_REALM")-1},           OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_PRODUCT_NAME,      {"DIA_PRODUCT_NAME", sizeof("DIA_PRODUCT_NAME")-1},       OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_CONFIG_PEER_IP,    {"DIA_CONFIG_PEER_IP", sizeof("DIA_CONFIG_PEER_IP")-1},       OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_CONFIG_LOCAL_IP,   {"DIA_CONFIG_LOCAL_IP", sizeof("DIA_CONFIG_LOCAL_IP")-1},     OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_MAX_HOST_IP_NUM,   {"DIA_MAX_HOST_IP_NUM", sizeof("DIA_MAX_HOST_IP_NUM")-1},     OS_XML_DATA_TYPE_XS_SHORT,  0, 1},
    {DIA_XML_CONFIG_PEER_PORT,  {"DIA_CONFIG_PEER_PORT", sizeof("DIA_CONFIG_PEER_PORT")-1},   OS_XML_DATA_TYPE_XS_SHORT, 0},
    {DIA_XML_MAX_SAME_AVP_NUM,  {"DIA_MAX_SAME_AVP_NUM", sizeof("DIA_MAX_SAME_AVP_NUM")-1},   OS_XML_DATA_TYPE_XS_SHORT, 0, 5},
    {DIA_XML_FIRMWARE_REVISION, {"DIA_FIRMWARE_REVISION", sizeof("DIA_FIRMWARE_REVISION")-1}, OS_XML_DATA_TYPE_XS_SHORT, 0},
    {DIA_XML_MAX_INTERFACE_NUM, {"DIA_MAX_INTERFACE_NUM", sizeof("DIA_MAX_INTERFACE_NUM")-1}, OS_XML_DATA_TYPE_XS_SHORT, 0, 5},
    {DIA_XML_CONFIG_LISTEN_PORT,        	{"DIA_CONFIG_LISTEN_PORT", sizeof("DIA_CONFIG_LISTEN_PORT")-1},       OS_XML_DATA_TYPE_XS_SHORT, 0},
    {DIA_XML_CONN_TIMER_WATCHDOG,       	{"DIA_CONN_TIMER_WATCHDOG", sizeof("DIA_CONN_TIMER_WATCHDOG")-1},     OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_CONN_TIMER_WAIT_CONN,      	{"DIA_CONN_TIMER_WAIT_CONN", sizeof("DIA_CONN_TIMER_WAIT_CONN")-1},   OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_CONN_TIMER_RETRY_CONN,     	{"DIA_CONN_TIMER_RETRY_CONN", sizeof("DIA_CONN_TIMER_RETRY_CONN")-1}, OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_REQ_WAIT_RESP_DEFAULT_TIME, 	{"DIA_REQ_WAIT_RESP_DEFAULT_TIME", sizeof("DIA_REQ_WAIT_RESP_DEFAULT_TIME")-1}, OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_CONFIG_MAX_AVP_INSIDE_GRP_AVP,     {"DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP", sizeof("DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP")-1},         OS_XML_DATA_TYPE_XS_SHORT, 0, 10},
    {DIA_XML_CONN_TIMER_TRANSMIT_WAIT_TIME,     {"DIA_CONN_TIMER_TRANSMIT_WAIT_TIME", sizeof("DIA_CONN_TIMER_TRANSMIT_WAIT_TIME")-1},         OS_XML_DATA_TYPE_XS_LONG, 0},
    {DIA_XML_CX_MAX_SERVER_CAPABILITY_ITEM,     {"DIA_CX_MAX_SERVER_CAPABILITY_ITEM", sizeof("DIA_CX_MAX_SERVER_CAPABILITY_ITEM")-1},         OS_XML_DATA_TYPE_XS_SHORT, 0, 10},
    {DIA_XML_CONFIG_TRANSPORT_TCP_BUFFER_SIZE,  {"DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE", sizeof("DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE")-1},   OS_XML_DATA_TYPE_XS_SHORT, 0}};



//static osVPointerLen_t gPlDiaHostIp[DIA_MAX_HOST_IP_NUM];
static osVPointerLen_t* gPlDiaHostIp = NULL;
static uint32_t diaHostNum;


void diaConfig_init(char* configFolder)
{
	osXmlDataCallbackInfo_t cbInfo={true, true, false, NULL, diaConfig_xmlData, DIA_XML_MAX_DATA_NAME_NUM};
    if(osXml_getLeafValue(configFolder, DIA_CONFIG_XSD_FILE_NAME, DIA_CONFIG_XML_FILE_NAME, &cbInfo) != OS_STATUS_OK)
    {
        logError("fails to diaConfig_getXmlConfig.");
		exit(EXIT_FAILURE);
        return;
    }

	gPlDiaHostIp = oszalloc(sizeof(osVPointerLen_t)* *(uint64_t*)diaConfig_getConfig(DIA_XML_MAX_HOST_IP_NUM), NULL);
	if(!gPlDiaHostIp)
	{
		logError("fails to oszalloc for gPlDiaHostIp.");
		return;
	}

    //for now, only support 1 host IP.
	char* pHostIp = osmalloc(6, NULL);
	diaWriteU16(pHostIp, htobe16(OS_AF_IPv4));

    struct sockaddr_in saddr;
	osIpPort_t ipPort = {{*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_CONFIG_LOCAL_IP), false, false}};
	osConvertPLton(&ipPort, false, &saddr);
   // inet_pton(AF_INET, DIA_CONFIG_LOCAL_IP, &saddr.sin_addr.s_addr);

	//saddr.sin_addr.s_addr already in big enden order
    diaWriteU32(&pHostIp[2], saddr.sin_addr.s_addr);	

	//for now, only support 1 host IP.
	gPlDiaHostIp[0].pl.p = pHostIp;
	gPlDiaHostIp[0].pl.l = 6;
	gPlDiaHostIp[0].isPDynamic = false;		//though pHostIp is dynamically allocated, this is intended to be kept forever
	gPlDiaHostIp[0].isVPLDynamic = false;

	diaHostNum = 1;
}
		

void* diaConfig_getConfig(diaConfig_xmlDataName_e dataName)
{
	switch(dataName)
	{
		case DIA_XML_DEST_HOST:
		case DIA_XML_ORIG_HOST:
		case DIA_XML_DEST_REALM:
		case DIA_XML_ORIG_REALM:
		case DIA_XML_PRODUCT_NAME:
		case DIA_XML_CONFIG_PEER_IP:
		case DIA_XML_CONFIG_LOCAL_IP:
			return &diaConfig_xmlData[dataName].xmlStr;
			break;
		case DIA_XML_IS_SERVER:
			return &diaConfig_xmlData[dataName].xmlIsTrue;
			break;
		case DIA_XML_HASH_SIZE:
		case DIA_XML_MAX_HOST_IP_NUM:
		case DIA_XML_CONFIG_PEER_PORT:
		case DIA_XML_MAX_SAME_AVP_NUM:
		case DIA_XML_FIRMWARE_REVISION:
		case DIA_XML_MAX_INTERFACE_NUM:
		case DIA_XML_CONFIG_LISTEN_PORT:
		case DIA_XML_CONN_TIMER_WATCHDOG:
		case DIA_XML_CONN_TIMER_WAIT_CONN:
		case DIA_XML_CONN_TIMER_RETRY_CONN:
		case DIA_XML_REQ_WAIT_RESP_DEFAULT_TIME:
		case DIA_XML_CONFIG_MAX_AVP_INSIDE_GRP_AVP:
		case DIA_XML_CONN_TIMER_TRANSMIT_WAIT_TIME:
		case DIA_XML_CX_MAX_SERVER_CAPABILITY_ITEM:
		case DIA_XML_CONFIG_TRANSPORT_TCP_BUFFER_SIZE:
			return &diaConfig_xmlData[dataName].xmlInt;
			break;
		default:
			logError("dataName is not defined(%d).", dataName);
			break;
	}

	return NULL;
}

 
void diaConfig_getHostRealm(diaRealmHost_t* pRealmHost)
{
	if(!pRealmHost)
	{
		logError("null pointer, pRealmHost.");
		return;
	}

    osVPL_init(&pRealmHost->origHost, false);
    osVPL_init(&pRealmHost->origRealm, false);
    osVPL_init(&pRealmHost->destHost, false);
    osVPL_init(&pRealmHost->destRealm, false);

	osVPL_setPL(&pRealmHost->origRealm, diaConfig_getConfig(DIA_XML_ORIG_REALM), false);
	osVPL_setPL(&pRealmHost->origHost, diaConfig_getConfig(DIA_XML_ORIG_HOST), false); 
	osVPL_setPL(&pRealmHost->destRealm, diaConfig_getConfig(DIA_XML_DEST_REALM), false);
} 


void diaConfig_getHost(osPointerLen_t* host, int* port)
{
	*host = *(osPointerLen_t*) diaConfig_getConfig(DIA_XML_CONFIG_LOCAL_IP);
	*port = *(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_LISTEN_PORT);	
}


void diaConfig_getHost1(struct sockaddr_in* pHost)
{
    struct sockaddr_in saddr;
    osIpPort_t ipPort = {{*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_CONFIG_LOCAL_IP), false, false}, *(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_LISTEN_PORT)};
    osConvertPLton(&ipPort, true, pHost);
}


void diaConfig_getPeer(diaIntfType_e intfType, osIpPort_t* pPeer)
{
	if(!pPeer)
	{
		logError("null pointer, pPeer.");
		return;
	}

	pPeer->ip.pl = *(osPointerLen_t*)diaConfig_getConfig(DIA_XML_CONFIG_PEER_IP);
    pPeer->ip.isPDynamic = false;
	pPeer->port = *(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_PEER_PORT);
	return;
}


bool diaConfig_isServer()
{
	return *(bool*) diaConfig_getConfig(DIA_XML_IS_SERVER);
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
		pProductName->isVPLDynamic = true;
	}

	pProductName->pl = *(osPointerLen_t*)diaConfig_getConfig(DIA_XML_PRODUCT_NAME);
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

	uint32_t* pVendorId1 = osmalloc(sizeof(uint32_t), NULL);
	uint32_t* pVendorId2 = osmalloc(sizeof(uint32_t), NULL);
	*pVendorId1 = DIA_VENDOR_ID_3GPP;
	*pVendorId2 = DIA_VENDOR_ID_ETSI;
	osList_append(pSupportedVendorId, pVendorId1);
	osList_append(pSupportedVendorId, pVendorId2);

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
