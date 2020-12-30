#ifndef _DIA_CX_SAR_H
#define _DIA_CX_SAR_H

#include "osList.h"
#include "osMBuf.h"

#include "diaAvp.h"


#define DIA_CX_SAR_OPTION_AVP_DEST_HOST					0x1
#define DIA_CX_SAR_OPTION_AVP_MULTI_REG_INDICATION		0x2
#define DIA_CX_SAR_OPTION_AVP_OC_SUPPORTED_FEATURES		0x4
#define DIA_CX_SAR_OPTION_AVP_PROXY_INFO				0x8
#define DIA_CX_SAR_OPTION_AVP_PUB_ID					0x10
#define DIA_CX_SAR_OPTION_AVP_RESTORATION_INFO			0x20
#define DIA_CX_SAR_OPTION_AVP_ROUTE_RECORD				0x40
#define DIA_CX_SAR_OPTION_AVP_SAR_FLAGS					0x80
#define DIA_CX_SAR_OPTION_AVP_SESSION_PRIORITY			0x100
#define DIA_CX_SAR_OPTION_AVP_SUPPORTED_FEATURE			0x200
#define DIA_CX_SAR_OPTION_AVP_USER_NAME					0x400
#define DIA_CX_SAR_OPTION_AVP_WILDCARDED_PUB_ID			0x800



typedef enum {
	DIA_3GPP_CX_NO_ASSIGNMENT = 0,
	DIA_3GPP_CX_REGISTRATION = 1,
	DIA_3GPP_CX_RE_REGISTRATION = 2,
	DIA_3GPP_CX_UNREGISTERED_USER = 3,
	DIA_3GPP_CX_TIMEOUT_DEREGISTRATION = 4,
	DIA_3GPP_CX_USER_DEREGISTRATION = 5,
	DIA_3GPP_CX_TIMEOUT_DEREGISTRATION_STORE_SERVER_NAME = 6,
	DIA_3GPP_CX_USER_DEREGISTRATION_STORE_SERVER_NAME = 7,
	DIA_3GPP_CX_ADMINISTRATIVE_DEREGISTRATION = 8,
	DIA_3GPP_CX_AUTHENTICATION_FAILURE = 9,
	DIA_3GPP_CX_AUTHENTICATION_TIMEOUT = 10,
	DIA_3GPP_CX_DEREGISTRATION_TOO_MUCH_DATA = 11,
	DIA_3GPP_CWX_AAA_USER_DATA_REQUEST = 12,
	DIA_3GPP_CWX_PGW_UPDATE = 13,
	DIA_3GPP_CWX_RESTORATION = 14,
} dia3gppServerAssignmentType_e;



typedef enum {
	DIA_3GPP_CX_USER_DATA_NOT_AVAILABLE = 0,
	DIA_3GPP_CX_USER_DATA_ALREADY_AVAILABLE = 1,
} diaCxUserDataAvailable_e;


typedef enum {
	DIA_3GPP_CX_MULTI_REG_IND_NO_EXIST = -1,
	DIA_3GPP_CX_MULTI_REG_IND_NOT_MULTI_REG = 0,
	DIA_3GPP_CX_MULTI_REG_IND_MULTI_REG = 1,
} diaCxMultiRegInd_e;


typedef struct {
	osVPointerLen_t sipCallIdHdr;
	osVPointerLen_t sipFromHdr;
	osVPointerLen_t sipToHdr;
	osVPointerLen_t sipRRHdr;
	osVPointerLen_t contact;
	osList_t 		extraInfoList;		//each entry contains diaEncodeAvp_t, any extra AVP
} diaCxSubscriptionInfo_t;


typedef struct {
	osVPointerLen_t 		path;
	osVPointerLen_t 		contact;
	uint32_t 				initialCSeqNum;
	osVPointerLen_t 		sipCallIdHdr;
	diaCxSubscriptionInfo_t subscriptionInfo;
    osList_t 				extraInfoList;     //each entry contains diaEncodeAvp_t, any extra AVP
} diaCxRestorationInfo_t;
		
typedef struct {
    dia3gppServerAssignmentType_e 	serverAssignmentType;
    diaCxUserDataAvailable_e 		userDataAvailable;
    diaCxMultiRegInd_e				multiRegInd;
    diaCxRestorationInfo_t* 		pRestorationInfo;
} diaCxSarInfo_t;


typedef struct diaCxSarParam {
	diaRealmHost_t realmHost;
    osVPointerLen_t userName;
    osVPointerLen_t pubId;
    osVPointerLen_t serverName;
	diaCxSarInfo_t* pSarInfo;
    osList_t optAvpList;            //contains list of diaEncodeAvp_t for destHost, supported-feature, etc.
    osList_t* pExtraOptAvpList;     //extra optional AVPs, passed from user directly
} diaCxSarParam_t;


//a group of input data provided by application that calls SAR
typedef struct {
    osPointerLen_t* pImpi;
    osPointerLen_t* pImpu;
    osPointerLen_t* pServerName;
    diaCxSarInfo_t* pSarInfo;
    uint32_t featureList;
    osList_t* pExtraOptList;
}diaCxSarAppInput_t;


//API for application to send a SAR requet
osStatus_e diaCx_sendSAR(diaCxSarAppInput_t* pSarInput, diaNotifyApp_h appCallback, void* pAppData);
osStatus_e diaSar_encode(osMBuf_t* pDiaBuf, diaCxSarParam_t* pSarParam);
osMBuf_t* diaBuildSar(osVPointerLen_t* userName, osVPointerLen_t* pubId, osVPointerLen_t* serverName, osVPointerLen_t* pDestHost, osVPointerLen_t* pDestRealm, diaCxSarInfo_t* pSarInfo, diaAvp_supportedFeature_t* pSF, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo);


#endif
