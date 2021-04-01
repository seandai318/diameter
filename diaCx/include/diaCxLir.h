#ifndef _DIA_CX_LIR_H
#define _DIA_CX_LIR_H

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
	DIA_3GPP_CX_USER_AUTH_TYPE_REGISTRATION = 0,
	DIA_3GPP_CX_USER_AUTH_TYPE_DE_REGISTRATION = 1,
	DIA_3GPP_CX_USER_AUTH_TYPE_REG_AND_CAP = 2,
} dia3gppUserAuthType_e;


//this structure is used by diaCx_sendLIR() to pass information to diaLir_encode()
typedef struct diaCxSarParam {
	diaRealmHost_t realmHost;
    osVPointerLen_t pubId;
	dia3gppUserAuthType_e userAuthType;
    osList_t optAvpList;            //contains list of diaEncodeAvp_t for destHost, supported-feature, etc.
    osList_t* pExtraOptAvpList;     //extra optional AVPs, passed from user directly
} diaCxLirParam_t;


//a group of input data provided by application that calls SAR
typedef struct {
    osPointerLen_t* pImpu;
	dia3gppUserAuthType_e userAuthType;
    uint32_t featureList;
    osList_t* pExtraOptList;
}diaCxLirAppInput_t;


//API for application to send a SAR requet
osStatus_e diaCx_sendLIR(diaCxLirAppInput_t* pLirInput, diaNotifyApp_h appCallback, void* pAppData);
osMBuf_t* diaBuildLir(osVPointerLen_t* pubId, osVPointerLen_t* pDestHost, osVPointerLen_t* pDestRealm, diaAvp_supportedFeature_t* pSF, dia3gppUserAuthType_e uat, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo);


#endif
