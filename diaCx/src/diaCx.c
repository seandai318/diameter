/********************************************************
 * Copyright (C) 2019, 2020, Sean Dai
 *
 * @file diaCx.c
 *********************************************************/

#include "osList.h"
#include "osDebug.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaCxAvp.h"



diaCxExpResult_e diaCx_getExpCode(diaMsgDecoded_t* pMsgDecoded)
{
    diaCxExpResult_e expCode = DIA_CX_EXP_RESULT_NONE;

    if(!pMsgDecoded)
    {
        logError("null pointer, pMsgDecoded.");
        goto EXIT;
    }

	diaAvp_t* pAvp = diaAvpListLookup(&pMsgDecoded->avpList, DIA_AVP_CODE_EXPERIMENTAL_RESULT);
    if(pAvp)
    {
        if(pAvp->avpData.dataType != DIA_AVP_DATA_TYPE_GROUPED)
        {
            logError("experimentalResult AVP is not a grouped data type.");
            goto EXIT;
        }

        diaAvpGroupedData_t* pAvpData = &pAvp->avpData.dataGrouped;
		diaAvp_t* pAvp1 = diaAvpListLookup(&pAvpData->dataList, DIA_AVP_CODE_EXPERIMENTAL_RESULT_CODE);
        if(pAvp1)
        {
            expCode = pAvp1->avpData.dataU32;
        }
    }

EXIT:
    return expCode;
}



diaDataType_e diaCxGetAvpInfo(diaAvpCxCode_e avpCode, uint8_t* pAvpFlag, diaAvpVendor_e* vendorId)
{
    diaDataType_e dataType = DIA_AVP_DATA_TYPE_UNKNOWN;
	uint8_t avpFlag = 0x00;
	if(vendorId)
	{
		*vendorId = DIA_AVP_VENDOR_3GPP;
	}

    switch(avpCode)
    {
    	case DIA_AVP_CODE_CX_ALLOWED_WAF_WWSF_ID:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;		//656, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_ASSOCIATED_ID:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;		//632, grouped, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_ASSOCIATED_REGISTRATION_ID:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;		//647, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_CALL_REF_INFO:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//720, grouped, no M, V, 3gpp29.329
			break;
    	case DIA_AVP_CODE_CX_CALLID_SIP_HEADER:
			avpFlag = 0xc0;
			dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//643, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_CHARGING_INFO:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//618, grouped, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_CONFIDENTIALITY_KEY:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING; 	//625, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_CONTACT:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//641, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_DEREG_REASON:
			avpFlag = 0xc0;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;		//615, grouped, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_FEATURE_LIST:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//630, unsigned32, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_FEATURE_LIST_ID:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//629, unsigned32, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_FROM_SIP_HEADER:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//643, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_IDENTITY_WITH_EMERGENCY_REG:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;		//651, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_INITIAL_CSEQ_SEQ_NUMBER:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//654, unsigned32, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_INTEGRITY_KEY:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//626, octetString, maybe M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_LIA_FLAGS:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//653, unsigned32, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_LOOSE_ROUTE_IND:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//638, enum, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_MANDATORY_CAPABILITY:
			avpFlag = 0xc0;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//604, unsigned32, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_MULTI_REG_INDICATION:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_ENUM;			//648, enum, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_OPTIONAL_CAPABILITY:
			avpFlag = 0xc0;
			dataType = DIA_AVP_DATA_TYPE_UINT32;		//605, unsigned32, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_ORIG_REQUEST:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//633, enum, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PATH:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//640, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PCSCF_SUBSCRIPTION_INFO:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//660, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PRI_CHG_COLLECTION_FUNC_NAME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_DIAM_IDEN;		//621, diaUri, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PRI_EVENT_CHG_FUNC_NAME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_DIAM_IDEN;     //619, diaUri, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PRIVILEDGED_SENDER_ID:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//652, enum, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_PUBLIC_ID:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;	//601, utf8String, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_REASON_CODE:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//616, enum, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_REASON_INFO:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;	//617, utf8String, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_RECORD_ROUTE:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//646, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_REGISTRATION_TIMEOUT:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_TIME;			//661, time, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_REQUESTED_DOMAIN:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//706, enum, M, V, 3gpp 29.329
			break;
    	case DIA_AVP_CODE_CX_RESTORATION_INFO:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//649, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_RTR_FLAGS:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UINT32;		//659, unsigned32, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SAR_FLAGS:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UINT32;		//655, unsigned32, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SCSCF_RESTORATION_INFO:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//639, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SEC_CHG_COLLECTION_FUNC_NAME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_DIAM_IDEN;		//622, diaUri, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SEC_EVENT_CHG_FUNC_NAME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_DIAM_IDEN;     //620, diaUri, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SESSION_PRIORITY:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//650, enum, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SERVER_ASSIGNMENT_TYPE:
			avpFlag = 0xc0;
			dataType = DIA_AVP_DATA_TYPE_ENUM;			//614, enum, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SERVER_CAPABILITY:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//603, grouped, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SERVING_NODE_IND:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_ENUM;			//714, enum, no M, V, 3gpp29.329
			break;
    	case DIA_AVP_CODE_CX_SERVER_NAME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;	//602, utf8String, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_AUTHENTICATE:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   // 609, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_AUTHORIZATION:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //610, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_AUTH_CONTEXT:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //611, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_AUTH_DATA_ITEM:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//612, grouped, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_AUTH_SCHEME:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //608, utf8String, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_DIGEST_AUTHENTICATE:
			avpFlag = 0x80;
			dataType = DIA_AVP_DATA_TYPE_GROUPED;       //635, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_ITEM_NUM:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UINT32;		//613, unsigned32, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SIP_NUM_AUTH_ITEMS:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_UINT32;        //607, unsigned32, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SUBSCRIPTION_INFO:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;       //642, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SUPPORTED_APPLICATIONS:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//631, grouped, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_SUPPORTED_FEATURE:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;		//628, grouped, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_TO_SIP_HEADER:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//643, octetString, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_UAR_FLAGS:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UINT32;		//637, unsigned32, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_USER_AUTH_TYPE:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//623, enum, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_USER_DATA_CX:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//606, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_USER_DATA_ALREADY_AVAILABLE:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_ENUM;			//624, enum, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_VISITED_NW_ID:
			avpFlag = 0xc0;
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;	//600, octetString, M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_WEBRTC_AUTH_FUNC_NAME:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;	//657, utf8string, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_WEBRTC_WEB_SERVER_FUNC_NAME:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //658, utf8string, no M, V, 3gpp29.229
			break;
    	case DIA_AVP_CODE_CX_WILDCARDED_PUB_ID:
			avpFlag = 0x80;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;	//634, utf8string, no M, V, 3gpp29.229
			break;
		default:
			break;
	}

	if(pAvpFlag)
	{
		*pAvpFlag = avpFlag;
	}

	return dataType;
}


diaDataType_e diaCxGetReusedAvpInfo(diaAvpCxCode_e avpCode, uint8_t* pAvpFlag, diaAvpVendor_e* vendorId)
{
    diaDataType_e dataType = DIA_AVP_DATA_TYPE_UNKNOWN;
    uint8_t avpFlag = 0x00;
    if(vendorId)
    {
        *vendorId = DIA_AVP_VENDOR_3GPP;
    }

    switch(avpCode)
    {
        case DIA_AVP_CODE_CX_DIGEST_ALGORITHM:
            avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //111, utf8String, M, no V, 3gpp29.229, rfc4590
            break;
        case DIA_AVP_CODE_CX_DIGEST_HA1:
            avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //121, utf8String, M, no V, 3gpp29.229, rfc4590
            break;
        case DIA_AVP_CODE_CX_DIGEST_QOP:
            avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //121, utf8String, M, no V, 3gpp29.229, rfc4590
            break;
        case DIA_AVP_CODE_CX_DIGEST_REALM:
            avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //104, utf8String, M, no V, 3gpp29.229, rfc4590
            break;
        case DIA_AVP_CODE_CX_DRMP:
            avpFlag = 0x00;
            dataType = DIA_AVP_DATA_TYPE_ENUM;          //301, enum, maybe M, maybe V,3gpp29.229, rfc7944
            break;
    	case DIA_AVP_CODE_CX_REUSE_FRAMED_IP_ADDRESS:
			avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //8, octetString, M, no V, rfc4005
			break;
    	case DIA_AVP_CODE_CX_REUSE_FRAMED_IPV6_PREFIX:
			avpFlag = 0x40;
            dataType = DIA_AVP_DATA_TYPE_UTF8_STRING;   //97, octetString, M, no V, rfc4005
			break;
        case DIA_AVP_CODE_CX_LOAD:
            avpFlag = 0x00;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;       //650, grouped, maybe M, maybe V,  rfc8583
            break;
        case DIA_AVP_CODE_CX_OC_OLR:
            avpFlag = 0x00;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;       //623, grouped, maybe M, maybe V, 3gpp29.229, rfc7683
            break;
        case DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES:
            avpFlag = 0x00;
            dataType = DIA_AVP_DATA_TYPE_GROUPED;       //621, grouped, maybe M, maybe V, 3gpp29.229, 3gpp29.329, rfc7683
            break;
        default:
            break;
    }

    if(pAvpFlag)
    {
        *pAvpFlag = avpFlag;
    }

    return dataType;
}
