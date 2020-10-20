#ifndef _DIA_CX_AVP_H
#define _DIA_CX_AVP_H

#include "diaAvp.h"


typedef enum {
    DIA_AVP_CODE_CX_ALLOWED_WAF_WWSF_ID = 656,          	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_ASSOCIATED_ID = 632,              		//grouped, V, 3gpp29.229
    DIA_AVP_CODE_CX_ASSOCIATED_REGISTRATION_ID = 647,   	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_CALL_REF_INFO = 720,                  	//grouped, no M, V, 3gpp29.329
    DIA_AVP_CODE_CX_CALLID_SIP_HEADER = 643,          		//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_CHARGING_INFO = 618,              		//grouped, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_CONFIDENTIALITY_KEY = 625,            	//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_CONTACT = 641,                        	//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_DEREG_REASON = 615,                   	//grouped, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_FEATURE_LIST = 630,                   	//unsigned32, V, 3gpp29.229
    DIA_AVP_CODE_CX_FEATURE_LIST_ID = 629,                	//unsigned32, V, 3gpp29.229
    DIA_AVP_CODE_CX_FROM_SIP_HEADER = 644,                	//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_IDENTITY_WITH_EMERGENCY_REG = 651,    	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_INITIAL_CSEQ_SEQ_NUMBER = 654,        	//unsigned32, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_INTEGRITY_KEY = 626,              		//octetString, maybe M, V, 3gpp29.229
    DIA_AVP_CODE_CX_LIA_FLAGS = 653,                  		//unsigned32, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_LOOSE_ROUTE_IND = 638,                	//enum, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_MANDATORY_CAPABILITY = 604,           	//unsigned32, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_MULTI_REG_INDICATION = 648,           	//enum, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_OPTIONAL_CAPABILITY = 605,            	//unsigned32, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_ORIG_REQUEST = 633,                   	//enum, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PATH = 640,                         	//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PCSCF_SUBSCRIPTION_INFO = 660,        	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PRI_CHG_COLLECTION_FUNC_NAME = 621,   	//diaUri, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PRI_EVENT_CHG_FUNC_NAME = 619,        	//diaUri, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PRIVILEDGED_SENDER_ID = 652,      		//enum, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_PUBLIC_ID = 601,                  		//utf8String, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_REASON_CODE = 616,                		//enum, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_REASON_INFO = 617,                		//utf8String, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_RECORD_ROUTE = 646,                   	//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_REGISTRATION_TIMEOUT = 661,           	//time, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_REQUESTED_DOMAIN = 706,               	//enum, M, V, 3gpp 29.329
    DIA_AVP_CODE_CX_RESTORATION_INFO = 649,               	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_RTR_FLAGS = 659,                  		//unsigned32, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SAR_FLAGS = 655,                  		//unsigned32, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SCSCF_RESTORATION_INFO = 639,     		//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SEC_CHG_COLLECTION_FUNC_NAME = 622, 	//diaUri, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SEC_EVENT_CHG_FUNC_NAME = 620,    		//diaUri, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SESSION_PRIORITY = 650,               	//enum, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SERVER_ASSIGNMENT_TYPE = 614,     		//enum, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SERVER_CAPABILITY = 603,          		//grouped, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SERVING_NODE_IND = 714,               	//enum, no M, V, 3gpp29.329
    DIA_AVP_CODE_CX_SERVER_NAME = 602,                  	//utf8String, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_AUTHENTICATE = 609,               	//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_AUTHORIZATION = 610,          		//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_AUTH_CONTEXT = 611,               	//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_AUTH_DATA_ITEM = 612,             	//grouped, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_AUTH_SCHEME = 608,            		//utf8String, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_DIGEST_AUTHENTICATE = 635,        	//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_ITEM_NUM = 613,                   	//unsigned32, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SIP_NUM_AUTH_ITEMS = 607,         		//unsigned32, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SUBSCRIPTION_INFO = 642,          		//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SUPPORTED_APPLICATIONS = 631,     		//grouped, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_SUPPORTED_FEATURE=628,                	//grouped, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_TO_SIP_HEADER = 645,              		//octetString, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_UAR_FLAGS = 637,                  		//unsigned32, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_USER_AUTH_TYPE = 623,             		//enum, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_USER_DATA_CX = 606,                  	//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_USER_DATA_ALREADY_AVAILABLE = 624,    	//enum, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_VISITED_NW_ID = 600,                  	//octetString, M, V, 3gpp29.229
    DIA_AVP_CODE_CX_WEBRTC_AUTH_FUNC_NAME = 657,      		//utf8string, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_WEBRTC_WEB_SERVER_FUNC_NAME = 658,    	//utf8string, no M, V, 3gpp29.229
    DIA_AVP_CODE_CX_WILDCARDED_PUB_ID = 634,          		//utf8string, no M, V, 3gpp29.229
} diaAvpCxCode_e;


typedef enum {
    DIA_AVP_CODE_CX_DIGEST_ALGORITHM = 111,                 //utf8String, M, no V, 3gpp29.229, rfc4590
    DIA_AVP_CODE_CX_DIGEST_HA1 = 121,                       //utf8String, M, no V, 3gpp29.229, rfc4590
	DIA_AVP_CODE_CX_DIGEST_QOP = 110,						//utf8String, M, no V, 3gpp29.229, rfc4590
    DIA_AVP_CODE_CX_DIGEST_REALM = 104,                     //utf8String, M, no V, 3gpp29.229, rfc4590
    DIA_AVP_CODE_CX_DRMP = 301,                             //enum, maybe M, maybe V,3gpp29.229, rfc7944
    DIA_AVP_CODE_CX_REUSE_FRAMED_IP_ADDRESS = 8,            //octetString, M, no V, rfc4005
    DIA_AVP_CODE_CX_REUSE_FRAMED_IPV6_PREFIX = 97,          //octetString, M, no V, rfc4005
	DIA_AVP_CODE_CX_LOAD = 650,                             //grouped, maybe M, maybe V,  rfc8583
    DIA_AVP_CODE_CX_OC_OLR = 623,                           //grouped, maybe M, maybe V, 3gpp29.229, rfc7683 
	DIA_AVP_CODE_CX_OC_SUPPORTED_FEATURES = 621,            //grouped, maybe M, maybe V, 3gpp29.229, 3gpp29.329, rfc7683
} diaAvpCxReusedCode_e;

typedef enum {
	DIA_CX_EXP_RESULT_NONE = 0,
    DIA_CX_EXP_RESULT_FIRST_REGISTRATION = 2001,
    DIA_CX_EXP_RESULT_SUBSEQUENT_REGISTRATION = 2002,
    DIA_CX_EXP_RESULT_UNREGISTERED_SERVICE = 2003,
    DIA_CX_EXP_RESULT_SUCCESS_SERVER_NAME_NOT_STORED = 2004,
    DIA_CX_EXP_RESULT_ERROR_USER_UNKNOWN = 5001,
    DIA_CX_EXP_RESULT_ERROR_IDENTITIES_DONT_MATCH = 5002,
    DIA_CX_EXP_RESULT_ERROR_IDENTITY_NOT_REGISTERED = 5003,
    DIA_CX_EXP_RESULT_ERROR_ROAMING_NOT_ALLOWED = 5004,
    DIA_CX_EXP_RESULT_ERROR_IDENTITY_ALREADY_REGISTERED = 5005,
    DIA_CX_EXP_RESULT_ERROR_AUTH_SCHEME_NOT_SUPPORTED = 5006,
    DIA_CX_EXP_RESULT_ERROR_IN_ASSIGNMENT_TYPE = 5007,
    DIA_CX_EXP_RESULT_ERROR_TOO_MUCH_DATA = 5008,
    DIA_CX_EXP_RESULT_ERROR_NOT_SUPPORTED_USER_DATA = 5009,
    DIA_CX_EXP_RESULT_ERROR_FEATURE_UNSUPPORTED = 5011,
    DIA_CX_EXP_RESULT_ERROR_SERVING_NODE_FEATURE_UNSUPPORTED = 5012,
} diaCxExpResult_e;


typedef struct diaCxServerCap {
	osListPlus_t manCap;
	osListPlus_t optCap;
	osListPlus_t serverName;
} diaCxServerCap_t;


typedef enum {
    DIA_AVP_CX_USER_AUTH_TYPE_REGISTRATION = 0,
} diaAvpCxUserAuthType_e;


typedef enum {
	DIA_CX_FEATURE_LIST_ID_SIFC	= 0,
	DIA_CX_FEATURE_LIST_ID_ALIAS_IND = 1,
	DIA_CX_FEATURE_LIST_ID_IMS_RESTORATION_IND = 2,
	DIA_CX_FEATURE_LIST_ID_PCSCF_RESTORATION_IND = 3,
} diaAvpCxFeatureListId_e;


diaDataType_e diaCxGetAvpInfo(diaAvpCxCode_e avpCode, uint8_t* avpFlag, diaAvpVendor_e* vendorId);
diaDataType_e diaCxGetReusedAvpInfo(diaAvpCxCode_e avpCode, uint8_t* pAvpFlag, diaAvpVendor_e* vendorId);


#endif
