/* based on 29.229 v16.1.0, 29.230 v16.2.0, 29.329 v15.2.0, 29.214 v16.2.0, 29.212 v16.3.0
 * supports Cx, Sh, Gx, Rx
 */

#ifndef _DIA_SH_AVP_H
#define _DIA_SH_AVP_H



typedef enum {
    DIA_AVP_CODE_SH_AS_NUMBER = 722,                      //grouped, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_CALL_REF_INFO = 720,                  //grouped, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_CALL_REF_NUMBER = 721,                //octetString, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_CURRENT_LOCATION = 707,               //enum, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_DATA_REFERENCE = 703,             //enum, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_DRMP = 301,                         //enum, maybe M, maybe V,3gpp29.329, rfc7944
    DIA_AVP_CODE_SH_DSAI_TAG = 711,                       //octString, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_EXPIRY_TIME = 709,                    //time, no M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_FEATURE_LIST = 630,                   //unsigned32, V, 3gpp29.329
    DIA_AVP_CODE_SH_FEATURE_LIST_ID = 629,                //unsigned32, V, 3gpp29.329
    DIA_AVP_CODE_SH_IDENTITY_SET = 708,                   //enum, no M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_LOAD = 650,                           //grouped, maybe M, maybe V,  3gpp 29.329, rfc8583
    DIA_AVP_CODE_SH_LOCAL_TIME_ZONE_IND = 718,            //enum, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_MSISDN = 701,                     //octetString, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_OC_SUPPORTED_FEATURES = 621,      //grouped, maybe M, maybe V, 3gpp29.329, rfc7683
    DIA_AVP_CODE_SH_OC_OLR = 623,                     //grouped, maybe M, mahybe V, 3gpp29.329, rfc7683
    DIA_AVP_CODE_SH_ONE_TIME_NOTIFICATION = 712,          //enum, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_PRE_PAGING_SUPPORTED = 717,           //enum, no M, V, 3gpp29.329
   	DIA_AVP_CODE_SH_PUBLIC_ID = 601,                  //utf8String, M, V, 3gpp29.329
    DIA_AVP_CODE_SH_REPOSITORY_DATA_ID = 715,             //grouped, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_REQUESTED_DOMAIN = 706,               //enum, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_REQUESTED_NODES = 713,                //unsigned32, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SEND_DATA_IND = 710,              //enum, no M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_SEQENCE_NUMBER = 716,                 //unsigned32, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SERVER_NAME = 602,                  //utf8String, M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SERVICE_IND = 704,                    //octetString, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_SERVING_NODE_IND = 714,               //enum, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SESSION_PRIORITY = 650,               //enum, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SUBS_REQ_TYPE = 705,              //enum, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_SUPPORTED_APPLICATIONS = 631,     //grouped, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_SUPPORTED_FEATURE=628,                //grouped, M, V, 3gpp29.329
    DIA_AVP_CODE_SH_UDR_FLAGS = 719,                  //unsigned32, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_USER_DATA = 702,                  //octetString, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_USER_ID = 700,                        //grouped, M, V, 3gpp 29.329
    DIA_AVP_CODE_SH_WILDCARDED_IMPU = 636,                //utf8string, no M, V, 3gpp29.329
    DIA_AVP_CODE_SH_WILDCARDED_PUB_ID = 634,          //utf8string, no M, V, 3gpp29.329
} diaAvpShCode_e;


#endif
