#include <string.h>
#include "osMemory.h"

#include "diaMsg.h"
#include "diaAvp.h"
#include "diaCxAvp.h"
#include "diaShAvp.h"
#include "diaAvpEncode.h"


static diaDataType_e diaBaseGetAvpInfo(diaAvpBaseCode_e avpCode, uint8_t* avpFlag);


//if avpFlag==NULL, no avpFlag will be returned
diaDataType_e diaGetAvpInfo(diaCmdCode_e diaCmd, uint32_t avpCode, uint8_t* avpFlag, diaAvpVendor_e* vendorId)
{
	diaDataType_e dataType = DIA_AVP_DATA_TYPE_UNKNOWN;
	if(avpFlag)
	{
    	*avpFlag = 0x0;
	}

	if(vendorId)
	{
		*vendorId = DIA_AVP_VENDOR_INVALID;
	}

	switch(diaCmd)
	{
		case DIA_CMD_CODE_LIR:
		case DIA_CMD_CODE_MAR:
		case DIA_CMD_CODE_SAR:
		case DIA_CMD_CODE_UAR:
		case DIA_CMD_CODE_PPR:
		case DIA_CMD_CODE_RTR:
			dataType = diaCxGetAvpInfo(avpCode, avpFlag, vendorId);
			if(dataType == DIA_AVP_DATA_TYPE_UNKNOWN)
			{
				dataType = diaCxGetReusedAvpInfo(avpCode, avpFlag, vendorId);
			}
			break;
		case DIA_CMD_CODE_PNR:
		case DIA_CMD_CODE_PUR:
		case DIA_CMD_CODE_SNR:
		case DIA_CMD_CODE_UDR:
			//dataType = diaShGetAvpInfo(avpCode, avpFlag, vendorId);
			break;
		default:
			break;
	}
	

	if(dataType == DIA_AVP_DATA_TYPE_UNKNOWN)
	{
		dataType = diaBaseGetAvpInfo(avpCode, avpFlag);
	}

	return dataType;
}


diaDataType_e diaBaseGetAvpInfo(diaAvpBaseCode_e avpCode, uint8_t* avpFlag)
{
	diaDataType_e dataType = DIA_AVP_DATA_TYPE_UNKNOWN;
    if(avpFlag)
    {
        *avpFlag = 0x40;
    }

	switch(avpCode)
	{
    	case DIA_AVP_CODE_ACCESS_NW_INFO:				//=1263, octetString, no M, V
			if(avpFlag)
			{
				*avpFlag = 0x80;
			}
			dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
    		break;
		case DIA_AVP_CODE_ACCT_APP_ID:					//=259, unsigned32, M, no V, rfc6733
			dataType = DIA_AVP_DATA_TYPE_UINT32;
			break;
    	case DIA_AVP_CODE_ACCT_INTERIM_INTERVAL:		//= 85, unsigned32, M, no V, rfc6733
			dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ACCT_MULTI_SESSION_ID:		//= 50, utf8string, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ACCT_REC_TYPE:				//= 485, accouting-record-number, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ACCT_REALTIME_REQUIRED:		//= 483, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ACCT_REC_NUMBER:				//= 485, accouting-record-number, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ACCT_SESSION_ID:				//= 44, octetString, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ACCT_SUB_SESSION_ID:			// = 287, unsigned64, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT64;
            break;
    	case DIA_AVP_CODE_AUTH_APP_ID:					// = 258, auth-application-id, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_AUTH_GRACE_PERIOD:			// = 276, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_AUTH_LIFETIME:				// = 291, authorization-lifetime, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_AUTH_REQ_TYPE:				// = 274, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_AUTH_SESSION_STATE:			// = 277, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_CLASS:						// = 25, octetString, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_DEST_HOST:					// = 293, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_DEST_REALM:					// = 283, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_DISCONNECT_CAUSE:				// = 273, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ERROR_MESSAGE:				// = 281, utf8string, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ERROR_REPORTING_HOST:			// = 294, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_EVENT_TIMESTAMP:				// = 55, time, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_EXPERIMENTAL_RESULT:			// = 297, grouped, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_GROUPED;
            break;
    	case DIA_AVP_CODE_EXPERIMENTAL_RESULT_CODE:		// = 298, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_FAILED_AVP:					// = 279, grouped, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_GROUPED;
            break;
    	case DIA_AVP_CODE_FIRMWARE_REV:					// = 267, unsigned32, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_HOST_IP_ADDRESS:				// = 257, address, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_INBAND_SECURITY_ID:			// = 299, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_MULTI_ROUND_TIMEOUT:			// = 272, unsigned32, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_ORIG_HOST:					// = 264, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ORIG_REALM:					// = 296, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ORIG_STATE_ID:				// = 278, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_PRODUCT_NAME:					// = 269, utf8string, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_PROXY_HOST:					// = 280, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_PROXY_INFO:					// = 284, grouped, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_GROUPED;
            break;
    	case DIA_AVP_CODE_PROXY_STATE:					// = 33, octetString, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_REDIRECT_HOST:				// = 292, diamUri, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_ROUTE_RECORD:					// = 282, diamIdentity, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_REAUTH_REQUEST_TYPE:			// = 285, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_REDIRECT_HOST_USAGE:			// = 261, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_REDIRECT_MAX_CACHE_TIME:		// = 262, unsigned32, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_RESULT_CODE:					// = 268, unsigned32, no M, no V, rfc6733
            if(avpFlag)
            {
                *avpFlag = 0x00;
            }
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_SESSION_BINDING:				// = 270, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_SESSION_ID:					// = 263, utf8String, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_SESSION_SERVER_FAILOVER:		// = 271, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_SESSION_TIMEOUT:				// = 27, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_SUPPORTED_VENDOR_ID:			// = 265, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_TERMINATION_CAUSE:			// = 295, enum, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_VENDOR_ID:					// = 266, unsigned32, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_UINT32;
            break;
    	case DIA_AVP_CODE_USER_NAME:					// = 1, utf8string, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_OCTET_STRING;
            break;
    	case DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID:		// = 260, grouped, M, no V, rfc6733
            dataType = DIA_AVP_DATA_TYPE_GROUPED;
            break;
		default:
			logError("diameter AVP code(%d) is not supported.", avpCode);
			break;
	}

	return dataType;
}


void diaAvpEncode_setValue(diaEncodeAvp_t* pAvp, uint32_t avpCode, diaEncodeAvpData_u avpData, diaAvp_encodeGroupCallback_h avpEncodeFunc)
{
    if(!pAvp)
    {
        return;
    }

    pAvp->avpCode = avpCode;
    pAvp->avpData = avpData;
    pAvp->avpEncodeFunc = avpEncodeFunc;
}


bool diaListFindAvp(osListElement_t* pLE, void* arg)
{
	if(!pLE)
	{
		return false;
	}

	if(pLE->data && ((diaAvp_t*)pLE->data)->avpCode == *(uint32_t*)arg)
	{
		return true;
	}

	return false;
}



diaEncodeAvp_t* diaOptListFindAndRemoveAvp(osList_t* pOptList, uint32_t avpCode)
{
	return osList_deleteElement(pOptList, diaListFindAvp, &avpCode);
}


diaEncodeAvp_t* diaOptListGetNextAndRemoveAvp(osList_t* pOptList)
{
	diaEncodeAvp_t* pAvp = NULL;
	if(!pOptList)
	{
		return pAvp;
	}

	osListElement_t* pLE = pOptList->head;
	if(pLE)
	{
		pAvp = pLE->data;
		osList_unlinkElement(pLE);
		osfree(pLE);
	}

	return pAvp;
}


void* diaAvpListLookup(osList_t* pList, uint32_t avpCode)
{
	void* pData = NULL;

	if(!pList)
	{
		goto EXIT;
	}

	osListElement_t* pLE = osList_lookup(pList, true, diaListFindAvp, &avpCode);
	if(pLE)
	{
		pData = pLE->data;
	}

EXIT:
	return pData;
}


void diaWriteU16(char* pBuf, uint16_t v)
{
	memcpy(pBuf, (uint8_t *)&v, sizeof(v));
}


void diaWriteU32(char* pBuf, uint32_t v)
{
    memcpy(pBuf, (uint8_t *)&v, sizeof(v));
}


//this is a helper function to add avp data to the diaEncodeAvp_t avpList.  It first construct a avp, then append it.  the function does not allocate memory for each avp, instead, it uses avpMemory to store the constructed avp.  avpMemory is assumed to be an array, with array size of DIA_MAX_SAME_AVP_NUM
osStatus_e diaAvpAddList(osList_t* pAvpList, osList_t* pAvpData, uint32_t avpCode, diaAvpEncodeDataType_e dataType, diaEncodeAvp_t* avpMemory)
{
	osStatus_e status = OS_STATUS_OK;

	if(!pAvpList ||!avpMemory)
	{
		logError("null pointer, pAvpList=%p, avpMemory=%p.", pAvpList, avpMemory);
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
	}

	if(!pAvpData)
	{
		debug("avpData is NULL, do nothing.");
		goto EXIT;
	}

	osListElement_t* pLE = pAvpData->head;
	int i=0;
	while(pLE)
	{
		switch(dataType)
		{
		    case DIA_AVP_ENCODE_DATA_TYPE_INT32:
				diaAvpEncode_setValue(&avpMemory[i], avpCode, (diaEncodeAvpData_u)*(int32_t*)pLE->data, NULL);
				break;
    		case DIA_AVP_ENCODE_DATA_TYPE_INT64:
               	diaAvpEncode_setValue(&avpMemory[i], avpCode, (diaEncodeAvpData_u)*(int64_t*)pLE->data, NULL);
                break;
    		case DIA_AVP_ENCODE_DATA_TYPE_U32:
				diaAvpEncode_setValue(&avpMemory[i], avpCode, (diaEncodeAvpData_u)*(uint32_t*)pLE->data, NULL);
				break;
    		case DIA_AVP_ENCODE_DATA_TYPE_U64:
				diaAvpEncode_setValue(&avpMemory[i], avpCode, (diaEncodeAvpData_u)*(uint64_t*)pLE->data, NULL);
                break;
    		case DIA_AVP_ENCODE_DATA_TYPE_STR:
    		case DIA_AVP_ENCODE_DATA_TYPE_GROUP:
				diaAvpEncode_setValue(&avpMemory[i], avpCode, (diaEncodeAvpData_u)pLE->data, NULL);
				break;
			default:
				logError("dataType(%d) is not supported.", dataType);
				status = OS_ERROR_INVALID_VALUE;
				goto EXIT;
		}

		osList_append(pAvpList, &avpMemory[i]);
		pLE = pLE->next;
        if(++i >= DIA_MAX_SAME_AVP_NUM)
        {
        	logInfo("avpCode(%d) number(%d) exceeds DIA_MAX_SAME_AVP_NUM(%d), the exceeded avps are not appended to the pAvpList.", avpCode, i, DIA_MAX_SAME_AVP_NUM);
			status = OS_ERROR_INVALID_VALUE;
            break;
        }
	}

EXIT:
	return status;
}
