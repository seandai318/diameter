/********************************************************
 * Copyright (C) 2019, 2020, Sean Dai
 *
 * @file diaCxTest.c
 * ********************************************************/

#include "osMBuf.h"
#include "osPL.h"

#include "diaCxUar.h"
#include "diaAvp.h"
#include "diaCxMar.h"
#include "diaCxSar.h"
#include "diaCxAvp.h"
#include "diaConnState.h"


#define CX_USER_NAME	"310970200005171"
#define PUB_ID			"sip:+18638337086@ims.mnc970.mcc310.3gppnetwork.org"
#define VISITED_NW_ID	"ims.globalstar.com"



osMBuf_t* testDiaUar()
{
    diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(DIA_INTF_TYPE_CX);
    if(!pDcb)
    {
        logError("no active diameter connection exists for DIA_INTF_TYPE_CX.");
        return NULL;
    }
    osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
    osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};

	osVPointerLen_t userName = {{CX_USER_NAME, sizeof(CX_USER_NAME)-1}, false, false};
	osVPointerLen_t pubId = {{PUB_ID, sizeof(PUB_ID)-1}, false, false};
	osVPointerLen_t visitedNWId = {{VISITED_NW_ID, sizeof(VISITED_NW_ID)-1}, false, false};
	DiaCxUarAuthType_e authType = DIA_CX_AUTH_TYPE_REGISTRATION;

#if 0
	diaAvp_supportedFeature_t sf;
	sf.fl[0].vendorId = DIA_AVP_VENDOR_3GPP;
	sf.fl[0].featureListId = 1;
	sf.fl[0].featureList = 0x4;
	sf.flNum = 1;

	diaHdrSessInfo_t diaHdrSessInfo;
	return diaBuildUar(&userName, &pubId, &visitedNWId, authType, &sf, NULL, &diaHdrSessInfo);
#else
    diaHdrSessInfo_t diaHdrSessInfo;
    return diaBuildUar(&userName, &pubId, &visitedNWId, authType, &destHost, &destRealm, 0x4, NULL, &diaHdrSessInfo);
#endif
}



#define SERVER_NAME	 "sip:scscf01-mlplab.ims.globalstar.com:5060"
#define HSS_NAME	"hss01-mlplab.ims.globalstar.com"

osMBuf_t* testDiaMar()
{
    diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(DIA_INTF_TYPE_CX);
    if(!pDcb)
    {
        logError("no active diameter connection exists for DIA_INTF_TYPE_CX.");
        return NULL;
    }
    osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
    osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};

	osVPointerLen_t userName = {{CX_USER_NAME, sizeof(CX_USER_NAME)-1}, false, false};
    osVPointerLen_t pubId = {{PUB_ID, sizeof(PUB_ID)-1}, false, false};
	osVPointerLen_t serverName = {{SERVER_NAME, sizeof(SERVER_NAME)-1}, false, false};
	diaCxMarSipAuthDataItem_t authData={};

	authData.isReq = true;
	authData.sipAuthScheme = DIA_CX_AUTH_SCHEME_SIP_DIGEST;
	authData.authItem = 1;

    diaHdrSessInfo_t diaHdrSessInfo;
	return diaBuildMar(&userName, &pubId, &authData, &serverName, &destHost, &destRealm, NULL, NULL, &diaHdrSessInfo);
}



osMBuf_t* testDiaSar()
{
    diaConnBlock_t* pDcb = diaConnGetActiveDcbByIntf(DIA_INTF_TYPE_CX);
    if(!pDcb)
    {
        logError("no active diameter connection exists for DIA_INTF_TYPE_CX.");
        return NULL;
    }
    osVPointerLen_t destHost = {pDcb->peerHost.pl, false, false};
    osVPointerLen_t destRealm = {pDcb->peerRealm.pl, false, false};

    osVPointerLen_t userName = {{CX_USER_NAME, sizeof(CX_USER_NAME)-1}, false, false};
    osVPointerLen_t pubId = {{PUB_ID, sizeof(PUB_ID)-1}, false, false};
    osVPointerLen_t serverName = {{SERVER_NAME, sizeof(SERVER_NAME)-1}, false, false};

	diaCxSarInfo_t sarInfo;
	sarInfo.serverAssignmentType = DIA_3GPP_CX_REGISTRATION;
	sarInfo.userDataAvailable = DIA_3GPP_CX_USER_DATA_NOT_AVAILABLE;
	sarInfo.multiRegInd = DIA_3GPP_CX_MULTI_REG_IND_NO_EXIST;
	sarInfo.pRestorationInfo = NULL;

	diaAvp_supportedFeature_t sf;
    sf.fl[0].vendorId = DIA_AVP_VENDOR_3GPP;
    sf.fl[0].featureListId = 1;
    sf.fl[0].featureList = 1 << DIA_CX_FEATURE_LIST_ID_SIFC;
    sf.flNum = 1;

    diaHdrSessInfo_t diaHdrSessInfo;
    return diaBuildSar(&userName, &pubId, &serverName, &destHost, &destRealm, &sarInfo, &sf, NULL, &diaHdrSessInfo);
}
 
