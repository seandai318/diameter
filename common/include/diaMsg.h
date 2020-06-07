/* based on 29.229 v16.1.0, 29.230 v16.2.0, 29.329 v15.2.0, 29.214 v16.2.0, 29.212 v16.3.0
 * supports Cx, Sh, Gx, Rx
 */

#ifndef _DIA_MSG_H
#define _DIA_MSG_H


#include "osTypes.h"
#include "osMBuf.h"
#include "osList.h"
#include "osPL.h"

#include "diaBaseAvp.h"
#include "diaAvpEncode.h"



#define DIA_MSG_AVP_START   20
#define DIA_MSG_MAX_SIZE	4000
#define DIA_AVP_VENDOR_ID_3GPP  10415
#define DIA_AVP_AUTH_APP_ID     16777216

typedef enum {
    DIA_VENDOR_ID_RESERVED = 0,
    DIA_VENDOR_ID_ERICSSON = 193,
    DIA_VENDOR_ID_ETSI = 13019,
    DIA_VENDOR_ID_3GPP = 10415,
} diaVendorId_e;


typedef enum {
    DIA_APP_ID_BASE = 0,
    DIA_APP_ID_3GPP_CX = 16777216,
    DIA_APP_ID_3GPP_SH = 16777217,
    DIA_APP_ID_3GPP_ZH = 16777221,
    DIA_APP_ID_3GPP_SLH = 16777291,
    DIA_APP_ID_3GPP_RX = 16777236,
    DIA_APP_ID_3GPP_GX = 16777238,
} diaAppId_e;

typedef enum {
	DIA_CMD_FLAG_REQUEST 			= 0x80,
	DIA_CMD_FLAG_PROXYABLE 			= 0x40,
	DIA_CMD_FLAG_ERROR 				= 0x20,
	DIA_CMD_FLAG_T_RETRANSMISSION 	= 0x10,
} diaCmdFlag_e;


typedef enum diaCmdCode {
	DIA_CMD_CODE_BASE = 0,		//no particular cmd code, used to get base avp info
	DIA_CMD_CODE_ACR = 271,		//accounting-request, rfc6733
	DIA_CMD_CODE_ASR = 274,		//Abort-session-request, rf6733
	DIA_CMD_CODE_BIR = 310,		//boostrapping-info-request, 3gpp 29.109
    DIA_CMD_CODE_CER = 257,     //capabilities-exchange-request, rfc6733
    DIA_CMD_CODE_DPR = 282,     //disconnect-peer-request, rfc6733
    DIA_CMD_CODE_DWR = 280,     //device-watchdog-request, rfc6733
	DIA_CMD_CODE_GPR = 312,		//GBAPush-info-request, 3gpp 29.109
	DIA_CMD_CODE_LIR = 302,		//Location-info-request, 3gpp 29.229
	DIA_CMD_CODE_MAR = 303,		//3gpp 29.229
	DIA_CMD_CODE_MPR = 311,		//message-process-request, 3gpp 29.140
	DIA_CMD_CODE_PNR = 309,		//push-notification-request, 3gpp 29.329
	DIA_CMD_CODE_PPR = 305,		//push-profile-request, 3gpp 29.229
	DIA_CMD_CODE_PUR = 307,		//profile-update-request, 3gpp 29.329
    DIA_CMD_CODE_RAR = 258,     //re-auth-request, rfc6733
	DIA_CMD_CODE_RTR = 304,		//registration-termination-request, 3gpp 29.229
    DIA_CMD_CODE_SAR = 301,     //3gpp 29.229
	DIA_CMD_CODE_SNR = 308,		//subscribe-notification-request, 3gpp 29.329
	DIA_CMD_CODE_STR = 275,		//session-termination-request, rfc6733
	DIA_CMD_CODE_UAR = 300,		//3gpp 29.229
	DIA_CMD_CODE_UDR = 306,		//user-data-request, 3gpp 29.329
} diaCmdCode_e;


typedef struct diaMsgDecoded {
    diaCmdCode_e cmdCode;
    uint8_t cmdFlag;
    uint32_t len;
    uint32_t appId;
    uint32_t h2hId;
    uint32_t e2eId;
    osList_t avpList;       //each element contains diaAvp_t
} diaMsgDecoded_t;


typedef struct diaRealmHost {
    osPointerLen_t origHost;
    osPointerLen_t origRealm;
    osPointerLen_t destHost;
    osPointerLen_t destRealm;
} diaRealmHost_t;


typedef struct diaResultCode {
	bool isResultCode;
	union {
        uint32_t resultCode;
        uint32_t expCode;
    };
} diaResultCode_t;


typedef struct diaCmdHdrInfo {
	uint32_t h2hId;
	uint32_t e2eId;
} diaCmdHdrInfo_t;


typedef struct diaHdrSessInfo {
	diaCmdHdrInfo_t hdrInfo;
//    osPointerLen_t h2hId;
//    osPointerLen_t e2eId;
    osPointerLen_t sessionId;
} diaHdrSessInfo_t;


diaMsgDecoded_t* diaMsg_decode(osMBuf_t* pDiaBuf);
//pAvpList contains list of diaEncodeAvp_t
osMBuf_t* diaMsg_encode(diaCmdCode_e cmdCode, uint8_t cmdFlag, uint32_t appId, osList_t* pAvpList, diaCmdHdrInfo_t* pHdrInfo);
diaResultCode_e dia_GetResultCode(diaMsgDecoded_t* pMsgDecoded);
osStatus_e dia_encodeHdr(osMBuf_t* pDiaBuf, uint8_t cmdFlag, uint32_t cmd, uint32_t len, uint32_t appId, uint32_t* pH2hId, uint32_t* pE2eId);
osStatus_e diaMsg_updateCmdFlagR(osMBuf_t* pDiaBuf, bool isSet);
osStatus_e diaMsg_updateCmdFlagP(osMBuf_t* pDiaBuf, bool isSet);
osStatus_e diaMsg_updateCmdFlagE(osMBuf_t* pDiaBuf, bool isSet);
osStatus_e diaMsg_updateCmdFlagT(osMBuf_t* pDiaBuf, bool isSet);
osStatus_e diaMsg_updateCmdFlag(osMBuf_t* pDiaBuf, uint8_t flag);



#endif
