#ifndef _DIA_CONFIG_H
#define _DIA_CONFIG_H

#include <sys/socket.h>
#include <netinet/in.h>

#include "osPL.h"
#include "osList.h"

#include "osTypes.h"
#include "diaMsg.h"
#include "diaIntf.h"


#define DIA_MAX_SAME_AVP_NUM	5		//how many the same name AVPs can be in a diameter message
#define DIA_CX_MAX_SERVER_CAPABILITY_ITEM	10
#define DIA_MAX_INTERFACE_NUM	5
#define DIA_MAX_HOST_IP_NUM	1

#define DIA_IS_SERVER		true

#define DIA_ORIG_REALM		"ims.globalstar.com"
#define DIA_ORIG_HOST		"cscf01-hrr.ims.globalstar.com"
#define DIA_DEST_REALM		"ims.globalstar.com"
#define DIA_DEST_HOST		0

#define DIA_CONFIG_LOCAL_IP		"192.168.1.86"
#define DIA_CONFIG_LISTEN_PORT	3868
#define DIA_CONFIG_PEER_IP		"192.168.1.65"
#define DIA_CONFIG_PEER_PORT	0

#define DIA_CONN_TIMER_WAIT_CONN			60000		//started a connection, wait the conn to be established
#define DIA_CONN_TIMER_RETRY_CONN			120000		//conn is down, retry new conn
#define DIA_CONN_TIMER_WATCHDOG				30000		//timer to send DWR, also be used for other DWR related timer
#define DIA_CONN_TIMER_TRANSMIT_WAIT_TIME	5000		//wait for a msg to be delivered to the peer


#define DIA_FIRMWARE_REVISION		0
#define DIA_PRODUCT_NAME			"Sean's Diameter Stack"




//per29.229, CER/CEA shall include auth-app-id in vendor-specific-application-id with vendor-id=3GPP, and application-id=3GPP-CX(16777216) 

void diaConfig_init();
void diaConfig_getHostRealm(diaRealmHost_t* pRealmHost);
bool diaConfig_isServer();
void diaConfig_getHost(osPointerLen_t* host, int* port);
void diaConfig_getHost1(struct sockaddr_in* pHost);
void diaConfig_getPeer(diaIntfType_e intfType, osIpPort_t* pPeer);


osListPlus_t* diaConfig_getHostIpList(diaIntfType_e intfType, osListPlus_t* pList);
osVPointerLen_t* diaConfig_getProductName(diaIntfType_e intfType, osVPointerLen_t* pProductName);
uint32_t diaConfig_getDiaVendorId();
osList_t* diaConfig_getSupportedVendorId(diaIntfType_e intfType, osList_t* pSupportedVendorId);
osList_t* diaConfig_getAuthAppId(diaIntfType_e intfType, osList_t* pAuthAppId);
osList_t* diaConfig_getAcctAppId(diaIntfType_e intfType, osList_t* pAcctAppId);
osList_t* diaConfig_getVendorSpecificAppId(diaIntfType_e intfType, osList_t* pVSAppId);
bool diaConfig_getFirmwareRev(uint32_t* pFWRev);


#endif
