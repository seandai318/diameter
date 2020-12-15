#ifndef _DIA_CX_UAR_H
#define _DIA_CX_UAR_H


#include "osPL.h"
#include "osMBuf.h"
#include "osList.h"

#include "diaAvp.h"
#include "diaMsg.h"


typedef enum {
    DIA_CX_AUTH_TYPE_REGISTRATION = 0,
	DIA_CX_AUTH_TYPE_DE_REGISTRATION = 1,
	DIA_CX_AUTH_TYPE_REGISTRATION_AND_CAPABILITIES = 2,
	DIA_CX_AUTH_TYPE_NONE,
} DiaCxUarAuthType_e;
	

typedef struct diaCxUarServerCap {
	uint32_t msc[DIA_CX_MAX_SERVER_CAPABILITY_ITEM];		//mandatory server capabilities
	uint8_t mscNum;
	uint32_t osc[DIA_CX_MAX_SERVER_CAPABILITY_ITEM];		//optional server capabilities
	uint8_t oscNum;
	osVPointerLen_t serverName[DIA_CX_MAX_SERVER_CAPABILITY_ITEM];	//server name
	uint8_t serverNameNum;
} diaCxUarServerCap_t;


typedef struct diaCxUarParam {
	diaRealmHost_t realmHost;
    osVPointerLen_t userName;
    osVPointerLen_t pubId;
    osVPointerLen_t visitedNetwork;      //visited network id
	osList_t optAvpList;			//contains list of diaEncodeAvp_t for destHost, supported-feature, userAuthType, etc.
	osList_t* pExtraOptAvpList;		//extra optional AVPs, passed from user directly
} diaCxUarParam_t;


typedef struct diaCxUaaParam {
    diaRealmHost_t realmHost;
	diaResultCode_t resultCode;
    osList_t optAvpList;            //contains list of diaEncodeAvp_t for server-name, server-capability, supported-feature, etc.
    osList_t* pExtraOptAvpList;     //extra optional AVPs, passed from user directly
} diaCxUaaParam_t;



//a group of input data provided by application that calls UAR
typedef struct {
	DiaCxUarAuthType_e authType;
    osPointerLen_t* pImpi;
    osPointerLen_t* pImpu;
    uint32_t featureList;
    osList_t* pExtraOptList;
}diaCxUarAppInput_t;


osStatus_e diaCx_sendUAR(diaCxUarAppInput_t* pUarInput, diaNotifyApp_h appCallback, void* pAppData);
osMBuf_t* diaBuildUar(osVPointerLen_t* userName, osVPointerLen_t* pubId, osVPointerLen_t* visitedNWId, DiaCxUarAuthType_e authType, uint32_t featureList, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo);
osMBuf_t* diaCxUar_encode(diaCxUarParam_t* pUarParam, diaHdrSessInfo_t* pHdrSessInfo);

//pList contains extra optional AVPs
osMBuf_t* diaBuildUaa(diaHdrSessInfo_t* pHdrSessInfo, diaResultCode_t* pResultCode, diaAvp_supportedFeature_t* pSF, osVPointerLen_t* serverName,diaCxUarServerCap_t* pCap, osList_t* pExtraOptList);
osMBuf_t* diaCxUaa_encode(diaCxUaaParam_t* pUaaParam, diaHdrSessInfo_t* pHdrSessInfo);



#endif
