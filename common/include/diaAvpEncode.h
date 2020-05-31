#ifndef _DIA_AVP_ENCODE_H
#define _DIA_AVP_ENCODE_H


#include "osTypes.h"
#include "osPL.h"


struct diaEncodeAvp;

typedef struct diaEncodeAvpGroupedData {
	struct diaEncodeAvp* pDiaAvp;		//diaEncodeAvp_t
	uint8_t avpNum;
} diaEncodeAvpGroupedData_t;


typedef union {
    int32_t data32;
    int64_t data64;
    uint32_t dataU32;
    uint64_t dataU64;
    osPointerLen_t* pDataStr;
    diaEncodeAvpGroupedData_t* pDataGrouped;	//used for grouped AVP	
	void* pAvpEncodeFuncArg;	//used when user specifies a encoding function to encode a AVP
} diaEncodeAvpData_u;


typedef osStatus_e (*diaAvp_encodeGroupCallback_h) (osMBuf_t* pDiaBuf, void* pAvpEncodeFuncArg);

typedef struct diaEncodeAvp {
    uint32_t avpCode;
    diaEncodeAvpData_u avpData;
    diaAvp_encodeGroupCallback_h avpEncodeFunc;
} diaEncodeAvp_t;


void diaAvpEncode_setValue(diaEncodeAvp_t* pAvp, uint32_t avpCode, diaEncodeAvpData_u avpData, diaAvp_encodeGroupCallback_h avpEncodeFunc);
osStatus_e diaAvp_encodeSessionId(osMBuf_t* pDiaBuf, void* pData);
osStatus_e diaAvp_encodeSupportedFeature(osMBuf_t* pDiaBuf, void* pData);
osStatus_e diaAvp_encodeVendorSpecificAppId(osMBuf_t* pDiaBuf, void* pData);



#endif
