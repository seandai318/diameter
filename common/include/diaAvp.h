#ifndef _DIA_AVP_H
#define _DIA_AVP_H

#include "osTypes.h"
#include "osMBuf.h"
#include "osList.h"

#include "diaMsg.h"
#include "diaAvpEncode.h"
#include "diaConfig.h"


#define DIA_AVP_FLAG_NO_SPECIFY			0		//when this is used as a avp flag as a func input, the real avp flag will be filled by function itself
#define DIA_AVP_FLAG_PROTECTED			0x20
#define DIA_AVP_FLAG_MANDATORY			0x40
#define DIA_AVP_FLAG_VENDOR_SPECIFIC	0x80

#define DIA_MAX_SESSION_ID_LEN		200

typedef enum {
    DIA_AVP_DATA_TYPE_UNKNOWN,
    DIA_AVP_DATA_TYPE_ENUM,
    DIA_AVP_DATA_TYPE_INT32,
    DIA_AVP_DATA_TYPE_INT64,
    DIA_AVP_DATA_TYPE_UINT32,
    DIA_AVP_DATA_TYPE_UINT64,
    DIA_AVP_DATA_TYPE_FLOAT32,
    DIA_AVP_DATA_TYPE_FLOAT64,
    DIA_AVP_DATA_TYPE_OCTET_STRING,
    DIA_AVP_DATA_TYPE_UTF8_STRING,
    DIA_AVP_DATA_TYPE_IP_FILTER,
    DIA_AVP_DATA_TYPE_TIME,
    DIA_AVP_DATA_TYPE_DIAM_IDEN,	//for now, treat it the same as DIA_AVP_DATA_TYPE_OCTET_STRING
    DIA_AVP_DATA_TYPE_GROUPED,
} diaDataType_e;


typedef struct diaAvpGroupedData {
    osMBuf_t rawData;   //the buf refers to the original diameter message, there is no need to explicit delete buf from this data structure
    osList_t dataList;   //each element contains diaAvp_t
} diaAvpGroupedData_t;


typedef struct diaAvpData {
    diaDataType_e dataType;
    union {
        int32_t data32;
        int64_t data64;
        uint32_t dataU32;
        uint64_t dataU64;
        osVPointerLen_t dataStr;
        diaAvpGroupedData_t dataGrouped;
    };
} diaAvpData_t;


typedef struct diaAvp {
    uint32_t avpCode;			//can be diaAvpBaseCode_e, or diaCxAvpCode_e, etc.
    uint8_t avpFlag;
    uint32_t vendorId;
    diaAvpData_t avpData;
} diaAvp_t;


typedef enum {
    DIA_AVP_AUTH_SESSION_STATE_NO_STATE_MAINTAINED = 1,
} diaAvpAuthSessionState_e;


typedef enum {
	DIA_AVP_VENDOR_INVALID = 0,
    DIA_AVP_VENDOR_3GPP = 10415,
} diaAvpVendor_e;


typedef enum {
    DIA_AVP_AUTH_APP_ID_3GPP_CX = 16777216,
} diaAvpAuthAppId_e;


typedef struct diaAvp_featureListParam {
	uint32_t vendorId;
	uint32_t featureListId;
	uint32_t featureList;
} diaAvp_featureListParam_t;


typedef struct diaAvp_supportedFeature {
    diaAvp_featureListParam_t fl[DIA_MAX_SAME_AVP_NUM];
    uint8_t flNum;
} diaAvp_supportedFeature_t;


typedef struct diaAvp_vendorSpecificAppIdParam {
	diaAvpAuthAppId_e authAppId;
	diaAvpVendor_e vendor;
} diaAvp_vendorSpecificAppIdParam_t;


typedef struct diaAvp_sessionIdParam {
//	char* pHostName;
	osPointerLen_t* pHostName;
	osVPointerLen_t* pSessId;
} diaAvp_sessionIdParam_t;


osStatus_e diaAvp_decode(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t* avpCode, uint8_t* avpFlag, uint32_t* vendorId, diaAvpData_t* pAvpData);
osStatus_e diaAvp_encode(osMBuf_t* pDiaBuf, diaCmdCode_e diaCmd, uint32_t avpCode, diaEncodeAvpData_u avpData);
diaEncodeAvp_t* diaAvpGrp_create(uint32_t avpCode);
diaEncodeAvp_t* diaAvpGrp_addAvp(diaEncodeAvp_t* pGrpAvp, uint32_t avpCode, diaEncodeAvpData_u avpData, diaAvp_encodeGroupCallback_h avpEncodeFunc);
//void diaAvpGrp_cleanup(diaEncodeAvp_t* pGrpAvp);

//if encodeFunc = NULL, the standard avp_encode will be called, otherwise, encodeFunc will be called 
diaDataType_e diaGetAvpInfo(diaCmdCode_e diaCmd, uint32_t avpCode, uint8_t* avpFlag, diaAvpVendor_e* vendorId);
diaDataType_e diaGetAvpDataType(uint32_t avpCode);
//void diaEncodeAvpSetValue(diaEncodeAvp_t* pAvpInfo, diaAvpCode_e avpCode, uint8_t avpFlag, diaAvpVendor_e hdrVendorId, diaEncodeAvpData_u avpData, diaAvp_encodeGroupCallback_h encodeFunc);
bool diaListFindAvp(osListElement_t* pLE, void* arg);
diaEncodeAvp_t* diaOptListFindAndRemoveAvp(osList_t* pOptList, uint32_t avpCode);
diaEncodeAvp_t* diaOptListGetNextAndRemoveAvp(osList_t* pOptList);
void* diaAvpListLookup(osList_t* pList, uint32_t avpCode);
void diaAvp_cleanup(void* pData);

void diaWriteU16(char* pBuf, uint16_t v);
void diaWriteU32(char* pBuf, uint32_t v);


#endif
