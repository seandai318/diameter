#ifndef _DIA_BASE_DWR_H
#define _DIA_BASE_DWR_H

#include "osList.h"
#include "osPL.h"
#include "osMBuf.h"

#include "diaAvp.h"
#include "diaBaseAvp.h"
#include "diaConnState.h"
#include "diaMsg.h"


/*
 * rfc6733, section 5.5.1
 *
 *        <DWR>  ::= < Diameter Header: 280, REQ >
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
typedef struct diaBaseDwrParam {
    diaRealmHost_t realmHost;
	osList_t optAvpList;	//contains optional orig-state-id.
    osList_t* pExtraOptAvpList;		//extra optional AVP that user wants to include
} diaBaseDwrParam_t;



/*
 * rfc6733, section 5.5.2
 *
 *        <DWA>  ::= < Diameter Header: 280 >
 *                   { Result-Code }
 *                   { Origin-Host }
 *                   { Origin-Realm }
 *                   [ Error-Message ]
 *                   [ Failed-AVP ]
 *                   [ Origin-State-Id ]
 *                 * [ AVP ]
 */
typedef struct diaBaseDwaParam {
    diaRealmHost_t realmHost;
	diaResultCode_t resultCode;
    osList_t optAvpList;    //contains optional orig-state-id, error-message, failed-avp.
	osList_t* pExtraOptAvpList; 	//extra optional AVP that user wants to include
} diaBaseDwaParam_t;


osMBuf_t* diaBuildDwr(osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo);
osMBuf_t* diaBuildDwa(diaResultCode_e resultCode, osVPointerLen_t* errorMsg, diaEncodeAvp_t* failedEvp, osList_t* pExtraOptList, diaCmdHdrInfo_t* pCmdHdrInfo);


#endif
