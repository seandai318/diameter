#ifndef _DIA_CX_MAR_H
#define _DIA_CX_MAR_H


#include "osPL.h"
#include "osMBuf.h"
#include "osList.h"

#include "diaAvp.h"
#include "diaMsg.h"


typedef enum {
	DIA_CX_AUTH_SCHEME_UNKNOWN,			//"Unknown"
	DIA_CX_AUTH_SCHEME_DIGEST_AKA,		//"Digest-AKAv1-MD5"
	DIA_CX_AUTH_SCHEME_SIP_DIGEST,		//"SIP Digest"
	DIA_CX_AUTH_SCHEME_NASS_BUNDLED,	//"NASS-Bundled"
	DIA_CX_AUTH_SCHEME_EARLY_IMS,		//"Early IMS Security"	
} diaCxAuthScheme_e;



typedef union {
    osVPointerLen_t sipAuthContext_noAka;		//3gpp29.228, table 6.3.2, authentication Context (see 7.9.7) 
    osVPointerLen_t sipAuthorization_onlyAka;	//3gpp29.228, table 6.3.2, Authorization Information (see 7.9.4)
} diaCxMar_ReqAuthData_u;

typedef struct diaCxMar_authAka {
	uint32_t itemOrder;		//sip-item-num, the order of each item
    osVPointerLen_t sipAuthenticate;
    osVPointerLen_t sipAuthorization;
    osVPointerLen_t ck;
    osVPointerLen_t ik;
} diaCxMar_authAka_t;


typedef struct diaCxMar_authDigest {
	osVPointerLen_t digestRealm;
	osVPointerLen_t digestQop;
	osVPointerLen_t digestHa1;
} diaCxMar_authDigest_t;

typedef struct diaCxMar_authGiba {
	bool isIPv4;
	union {
		osVPointerLen_t framedIP;
		osVPointerLen_t framedIPv6Prefix;
	};
} diaCxMar_authGiba_t;


typedef union {
	diaCxMar_authAka_t authAka;
	diaCxMar_authDigest_t authDigest;
	diaCxMar_authGiba_t authGiba;
} diaCxMar_RespAuthData_u;


typedef struct diaCxMarSipAuthDataItem {
	bool isReq;
	diaCxAuthScheme_e sipAuthScheme;
	uint8_t authItem;
	union {
		diaCxMar_ReqAuthData_u reqData;
		diaCxMar_RespAuthData_u respData;
	};
    osList_t extraAvp;
} diaCxMarSipAuthDataItem_t;

	
typedef struct diaCxMarParam {
    diaRealmHost_t realmHost;
    osVPointerLen_t userName;
    osVPointerLen_t pubId;
	diaCxMarSipAuthDataItem_t* authData;
	uint32_t authItem;
	osVPointerLen_t serverName;
    osList_t optAvpList;            //contains list of diaEncodeAvp_t for destHost, supported-feature, etc.
    osList_t* pExtraOptAvpList;     //extra optional AVPs, passed from user directly
} diaCxMarParam_t;
	

//pList contains extra optional AVPs
osMBuf_t* diaBuildMar(osVPointerLen_t* userName, osVPointerLen_t* pubId, diaCxMarSipAuthDataItem_t* pAuthData, osVPointerLen_t* serverName, osVPointerLen_t* pDestHost, diaAvp_supportedFeature_t* pSF, osList_t* pExtraOptList, diaHdrSessInfo_t* pHdrSessInfo);

osStatus_e diaMar_encode(osMBuf_t* pDiaBuf, diaCxMarParam_t* pMarParam, diaHdrSessInfo_t* pHdrSessInfo);


#endif
