#ifndef _DIA_CONFIG_H
#define _DIA_CONFIG_H

#include <sys/socket.h>
#include <netinet/in.h>
#include "string.h"

#include "osPL.h"
#include "osList.h"
#include "osTypes.h"
#include "osXmlParser.h"

#include "diaMsg.h"
#include "diaIntf.h"


#define DIA_CONFIG_MAX_FILE_NAME_SIZE	160
#define DIA_CONFIG_XSD_FILE_NAME		"diaConfig.xsd"
#define  DIA_CONFIG_XML_FILE_NAME		"diaConfig.xml"


typedef enum {
	DIA_XML_DEST_HOST,
	DIA_XML_IS_SERVER,		//if the diameter functions as a server
	DIA_XML_ORIG_HOST,      //example value: "cscf01.ims.seandai.com"
	DIA_XML_DEST_REALM,     //example value: "ims.seandai.com"
	DIA_XML_ORIG_REALM,     //example value: "ims.seandai.com"
	DIA_XML_PRODUCT_NAME,   		//example value: "Sean's Diameter Stack"
	DIA_XML_CONFIG_PEER_IP,     	//example value: "192.168.1.92"
	DIA_XML_CONFIG_LOCAL_IP,    	//example value: "192.168.1.65"
	DIA_XML_MAX_HOST_IP_NUM, 		//normally value = 1
	DIA_XML_CONFIG_PEER_PORT,   	//normally value = 3868    //this is duplicate of DIA_CONFIG_LISTEN_PORT, may be change to CLIENT_PORT and SERVER_PORT
	DIA_XML_MAX_SAME_AVP_NUM,   	//example value: 5       //how many the same name AVPs can be in a diameter message
    DIA_XML_FIRMWARE_REVISION,  	//example value: 0
	DIA_XML_MAX_INTERFACE_NUM,  	//example value: 5
	DIA_XML_CONFIG_LISTEN_PORT, 	//normally value = 3868
	DIA_XML_CONN_TIMER_WATCHDOG,             	//example value: 40000       //timer to send DWR, also be used for other DWR related timer
	DIA_XML_CONN_TIMER_WAIT_CONN,            	//example value: 60000       //started a connection, wait the conn to be established
	DIA_XML_CONN_TIMER_RETRY_CONN,           	//example value: 120000      //conn is down, retry new conn
	DIA_XML_CONFIG_MAX_AVP_INSIDE_GRP_AVP,   	//example value: 10
	DIA_XML_CONN_TIMER_TRANSMIT_WAIT_TIME,   	//example value: 5000        //wait for a msg to be delivered to the peer
	DIA_XML_CX_MAX_SERVER_CAPABILITY_ITEM,   	//example value: 10
	DIA_XML_CONFIG_TRANSPORT_TCP_BUFFER_SIZE,	//the expected maximum diameter message size
	DIA_XML_MAX_DATA_NAME_NUM,
} diaConfig_xmlDataName_e;




#define DIA_MAX_SAME_AVP_NUM    			5       //how many the same name AVPs can be in a diameter message
#define DIA_CX_MAX_SERVER_CAPABILITY_ITEM   10
#define DIA_MAX_INTERFACE_NUM   			5
#define DIA_MAX_HOST_IP_NUM 				(*(uint64_t*)diaConfig_getConfig(DIA_XML_MAX_HOST_IP_NUM))				//default 1

#define DIA_IS_SERVER       				(*(bool*)diaConfig_getConfig(DIA_XML_IS_SERVER))					//false

#define DIA_ORIG_REALM      				(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_ORIG_REALM))			//"ims.globalstar.com"
#define DIA_ORIG_HOST       				(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_ORIG_HOST))			//"cscf01-hrr.ims.globalstar.com"
#define DIA_DEST_REALM      				(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_DEST_REALM))			//"ims.globalstar.com"
#define DIA_DEST_HOST       				(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_DEST_HOST))			//0


#define DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE    (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_TRANSPORT_TCP_BUFFER_SIZE))	//5100
#define DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP       (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_MAX_AVP_INSIDE_GRP_AVP))	//10

#define DIA_CONFIG_LOCAL_IP     			(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_CONFIG_LOCAL_IP))	//"192.168.1.65"
#define DIA_CONFIG_LISTEN_PORT  			(*(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_LISTEN_PORT))		//3868
#define DIA_CONFIG_PEER_IP      			(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_CONFIG_PEER_IP))		//"192.168.1.92"
#define DIA_CONFIG_PEER_PORT    			(*(uint64_t*)diaConfig_getConfig(DIA_XML_CONFIG_PEER_PORT))			//3868, this is duplicate of DIA_CONFIG_LISTEN_PORT, may be change to CLIENT_PORT and SERVER_PORT

#define DIA_CONN_TIMER_WAIT_CONN            (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONN_TIMER_WAIT_CONN))		//60000       //started a connection, wait the conn to be established
#define DIA_CONN_TIMER_RETRY_CONN           (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONN_TIMER_RETRY_CONN))	//120000      //conn is down, retry new conn
//#define DIA_CONN_TIMER_WATCHDOG           30000       //timer to send DWR, also be used for other DWR related timer
#define DIA_CONN_TIMER_WATCHDOG             (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONN_TIMER_WATCHDOG))		//40000       //timer to send DWR, also be used for other DWR related timer
#define DIA_CONN_TIMER_TRANSMIT_WAIT_TIME   (*(uint64_t*)diaConfig_getConfig(DIA_XML_CONN_TIMER_TRANSMIT_WAIT_TIME))	//5000        //wait for a msg to be delivered to the peer


#define DIA_FIRMWARE_REVISION       		(*(uint64_t*)diaConfig_getConfig(DIA_XML_FIRMWARE_REVISION))		//0
#define DIA_PRODUCT_NAME            		(*(osPointerLen_t*)diaConfig_getConfig(DIA_XML_PRODUCT_NAME))		//"Sean's Diameter Stack"


#if 0
typedef struct {
	diaConfig_xmlDataName_e eDataName;
	osPointerLen_t dataName;
	osXmlDataType_e dataType;
	union {
		bool xmlIsTrue;
		uint64_t xmlInt;
		osPointerLen_t xmlStr;
	};
} diaConfig_xmlData_t;


//the order must be sorted based on the data name length.  for the data name with the same len, their order does not matter
static diaConfig_xmlData_t diaConfig_xmlData[DIA_XML_MAX_DATA_NAME_NUM] = {
	{DIA_XML_DEST_HOST, 		{"DIA_DEST_HOST", strlen("DIA_DEST_HOST")},				OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_IS_SERVER, 		{"DIA_IS_SERVER", strlen("DIA_IS_SERVER")}, 			OS_XML_DATA_TYPE_XS_BOOLEAN, false},
    {DIA_XML_ORIG_HOST, 		{"DIA_ORIG_HOST", strlen("DIA_ORIG_HOST")}, 			OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_DEST_REALM, 		{"DIA_DEST_REALM", strlen("DIA_DEST_REALM")}, 			OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_ORIG_REALM, 		{"DIA_ORIG_REALM", strlen("DIA_ORIG_REALM")}, 			OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_PRODUCT_NAME, 		{"DIA_PRODUCT_NAME", strlen("DIA_PRODUCT_NAME")},		OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_CONFIG_PEER_IP, 	{"DIA_CONFIG_PEER_IP", strlen("DIA_CONFIG_PEER_IP")}, 		OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_CONFIG_LOCAL_IP, 	{"DIA_CONFIG_LOCAL_IP", strlen("DIA_CONFIG_LOCAL_IP")}, 	OS_XML_DATA_TYPE_XS_STRING, 0},
    {DIA_XML_MAX_HOST_IP_NUM, 	{"DIA_MAX_HOST_IP_NUM", strlen("DIA_MAX_HOST_IP_NUM")}, 	OS_XML_DATA_TYPE_XS_INTEGER,  1},
    {DIA_XML_CONFIG_PEER_PORT, 	{"DIA_CONFIG_PEER_PORT", strlen("DIA_CONFIG_PEER_PORT")}, 	OS_XML_DATA_TYPE_XS_INTEGER, 3868},
    {DIA_XML_MAX_SAME_AVP_NUM, 	{"DIA_MAX_SAME_AVP_NUM", strlen("DIA_MAX_SAME_AVP_NUM")}, 	OS_XML_DATA_TYPE_XS_INTEGER, 5},
    {DIA_XML_FIRMWARE_REVISION, {"DIA_FIRMWARE_REVISION", strlen("DIA_FIRMWARE_REVISION")},	OS_XML_DATA_TYPE_XS_INTEGER, 0},
    {DIA_XML_MAX_INTERFACE_NUM, {"DIA_MAX_INTERFACE_NUM", strlen("DIA_MAX_INTERFACE_NUM")},	DIA_XML_MAX_INTERFACE_NUM, 5},
    {DIA_XML_CONFIG_LISTEN_PORT, 		{"DIA_CONFIG_LISTEN_PORT", strlen("DIA_CONFIG_LISTEN_PORT")}, 		DIA_XML_MAX_INTERFACE_NUM, 3868},
    {DIA_XML_CONN_TIMER_WATCHDOG, 		{"DIA_CONN_TIMER_WATCHDOG", strlen("DIA_CONN_TIMER_WATCHDOG")},		OS_XML_DATA_TYPE_XS_INTEGER, 40000},
    {DIA_XML_CONN_TIMER_WAIT_CONN, 		{"DIA_CONN_TIMER_WAIT_CONN", strlen("DIA_CONN_TIMER_WAIT_CONN")}, 	OS_XML_DATA_TYPE_XS_INTEGER, 60000},
    {DIA_XML_CONN_TIMER_RETRY_CONN, 	{"DIA_CONN_TIMER_RETRY_CONN", strlen("DIA_CONN_TIMER_RETRY_CONN")},	OS_XML_DATA_TYPE_XS_INTEGER, 120000},
    {DIA_XML_CONFIG_MAX_AVP_INSIDE_GRP_AVP, 	{"DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP", strlen("DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP")}, 		OS_XML_DATA_TYPE_XS_INTEGER, 10},
    {DIA_XML_CONN_TIMER_TRANSMIT_WAIT_TIME, 	{"DIA_CONN_TIMER_TRANSMIT_WAIT_TIME", strlen("DIA_CONN_TIMER_TRANSMIT_WAIT_TIME")}, 		OS_XML_DATA_TYPE_XS_INTEGER, 5000},
    {DIA_XML_CX_MAX_SERVER_CAPABILITY_ITEM, 	{"DIA_CX_MAX_SERVER_CAPABILITY_ITEM", strlen("DIA_CX_MAX_SERVER_CAPABILITY_ITEM")}, 		OS_XML_DATA_TYPE_XS_INTEGER, 10},
    {DIA_XML_CONFIG_TRANSPORT_TCP_BUFFER_SIZE, 	{"DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE", strlen("DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE")}, 	OS_XML_DATA_TYPE_XS_INTEGER, 5100}};
#endif

   
#if 0
#define DIA_MAX_SAME_AVP_NUM	5		//how many the same name AVPs can be in a diameter message
#define DIA_CX_MAX_SERVER_CAPABILITY_ITEM	10
#define DIA_MAX_INTERFACE_NUM	5
#define DIA_MAX_HOST_IP_NUM	1

#define DIA_IS_SERVER		false

#define DIA_ORIG_REALM		"ims.globalstar.com"
#define DIA_ORIG_HOST		"cscf01-hrr.ims.globalstar.com"
#define DIA_DEST_REALM		"ims.globalstar.com"
#define DIA_DEST_HOST		0


#define DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE	5100
#define DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP		10

#define DIA_CONFIG_LOCAL_IP		"192.168.1.65"
#define DIA_CONFIG_LISTEN_PORT	3868
#define DIA_CONFIG_PEER_IP		"192.168.1.92"
#define DIA_CONFIG_PEER_PORT	3868	//this is duplicate of DIA_CONFIG_LISTEN_PORT, may be change to CLIENT_PORT and SERVER_PORT

#define DIA_CONN_TIMER_WAIT_CONN			60000		//started a connection, wait the conn to be established
#define DIA_CONN_TIMER_RETRY_CONN			120000		//conn is down, retry new conn
//#define DIA_CONN_TIMER_WATCHDOG				30000		//timer to send DWR, also be used for other DWR related timer
#define DIA_CONN_TIMER_WATCHDOG             40000       //timer to send DWR, also be used for other DWR related timer
#define DIA_CONN_TIMER_TRANSMIT_WAIT_TIME	5000		//wait for a msg to be delivered to the peer


#define DIA_FIRMWARE_REVISION		0
#define DIA_PRODUCT_NAME			"Sean's Diameter Stack"
#endif

#if 0
typedef struct diaXml_capConfig {
	uint8_t DIA_MAX_SAME_AVP_NUM;
	uint8_t DIA_CONFIG_MAX_AVP_INSIDE_GRP_AVP;
	uint8_t DIA_CX_MAX_SERVER_CAPABILITY_ITEM;
	uint8_t DIA_MAX_INTERFACE_NUM;
	uint8_t DIA_MAX_HOST_IP_NUM;
	int DIA_CONFIG_TRANSPORT_TCP_BUFFER_SIZE;
} diaXml_capConfig_t;


typedef struct diaXml_protocolGeneralConfig {	
	int DIA_FIRMWARE_REVISION;
	char* DIA_ORIG_REALM;
	char* DIA_ORIG_HOST;
	char* DIA_DEST_REALM;
	char* DIA_DEST_HOST;
	char* DIA_PRODUCT_NAME;
} diaXml_protocolGeneralConfig_t;


typedef struct diaXml_transportConfig {
	char* DIA_CONFIG_LOCAL_IP;
	int DIA_CONFIG_LISTEN_PORT;
	char* DIA_CONFIG_PEER_IP;
	int DIA_CONFIG_PEER_PORT;
} diaXml_transportConfig_t;


typedef struct diaXml_timerConfig {
	int DIA_CONN_TIMER_WAIT_CONN;
	int DIA_CONN_TIMER_RETRY_CONN;
	int DIA_CONN_TIMER_WATCHDOG;
	int DIA_CONN_TIMER_TRANSMIT_WAIT_TIME;
} diaXml_timerConfig_t;


typedef struct diaXml_nodeFunction {
	bool DIA_IS_SERVER;
} diaXml_nodeFunction_t;
#endif	

//per29.229, CER/CEA shall include auth-app-id in vendor-specific-application-id with vendor-id=3GPP, and application-id=3GPP-CX(16777216) 

void diaConfig_init(char* diaConfigFolder);
void* diaConfig_getConfig(diaConfig_xmlDataName_e dataName);
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
