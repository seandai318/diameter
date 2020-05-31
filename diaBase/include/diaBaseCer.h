#ifndef _DIA_BASE_CER_H
#define _DIA_BASE_CER_H

#include "osList.h"
#include "osPL.h"
#include "osMBuf.h"

#include "diaAvp.h"
#include "diaBaseAvp.h"
#include "diaConnState.h"
#include "diaMsg.h"

/*
 * rfc6733, section 5.3.1
 *
 * <CER> ::= < Diameter Header: 257, REQ >
 *                  { Origin-Host }
 *                  { Origin-Realm }
 *               1* { Host-IP-Address }
 *                  { Vendor-Id }
 *                  { Product-Name }
 *                  [ Origin-State-Id ]
 *                * [ Supported-Vendor-Id ]
 *                * [ Auth-Application-Id ]
 *                * [ Inband-Security-Id ]
 *                * [ Acct-Application-Id ]
 *                * [ Vendor-Specific-Application-Id ]
 *                  [ Firmware-Revision ]
 *                * [ AVP ]
 */
typedef struct diaBaseCerParam {
    diaRealmHost_t realmHost;
	osListPlus_t* hostIpList;
	uint32_t vendorId;
	osPointerLen_t* productName;
	osList_t optAvpList;	//contains optional orig-state-id, supported-vendor-id, auth-app-id, acct-app-id, vendor-specific-app-id, and firmware-revision.
    osList_t* pExtraOptAvpList;		//extra optional AVP that user wants to include
} diaBaseCerParam_t;



/*
 * rfc6733, section 5.3.2
 * <CEA> ::= < Diameter Header: 257 >
 *                  { Result-Code }
 *                  { Origin-Host }
 *                  { Origin-Realm }
 *               1* { Host-IP-Address }
 *                  { Vendor-Id }
 *                  { Product-Name }
 *                  [ Origin-State-Id ]
 *                  [ Error-Message ]
 *                  [ Failed-AVP ]
 *                * [ Supported-Vendor-Id ]
 *                * [ Auth-Application-Id ]
 *                * [ Inband-Security-Id ]
 *                * [ Acct-Application-Id ]
 *                * [ Vendor-Specific-Application-Id ]
 *                  [ Firmware-Revision ]
 *                * [ AVP ]
 *
 */
typedef struct diaBaseCeaParam {
    diaRealmHost_t realmHost;
	diaResultCode_t resultCode;
    osListPlus_t* hostIpList;
    uint32_t vendorId;
    osPointerLen_t* productName;
    osList_t optAvpList;    //contains optional orig-state-id, supported-vendor-id, auth-app-id, acct-app-id, vendor-specific-app-id, and firmware-revision.
	osList_t* pExtraOptAvpList; 	//extra optional AVP that user wants to include
} diaBaseCeaParam_t;


osMBuf_t* diaBuildCer(osListPlus_t* pHostIpList, uint32_t vendorId, osPointerLen_t* productName, osList_t* pSupportedVendorId, osList_t* pAuthAppId, osList_t* pAcctAppId, osList_t* pVendorSpecificAppId, uint32_t* firmwareRev, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo);

osMBuf_t* diaBuildCea(diaResultCode_e resultCode, osListPlus_t* pHostIpList, uint32_t vendorId, osPointerLen_t* productName, osList_t* pSupportedVendorId, osList_t* pAuthAppId, osList_t* pAcctAppId, osList_t* pVendorSpecificAppId, uint32_t* firmwareRev, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo);;

osStatus_e diaConnProcessCea(diaMsgDecoded_t* pDiaDecoded, struct diaConnBlock* pDcb);


#endif
