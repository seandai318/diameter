#ifndef _DIA_BASE_AVP_H
#define _DIA_BASE_AVP_H

#include "osTypes.h"
#include "osMBuf.h"




typedef enum {
    DIA_AVP_CODE_ACCESS_NW_INFO = 1263,                 //access-network-info
    DIA_AVP_CODE_ACCT_APP_ID = 259,                     //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_INTERIM_INTERVAL = 85,            //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_MULTI_SESSION_ID = 50,            //utf8string, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_REC_TYPE = 480,                   //accouting-record-number, enum, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_REALTIME_REQUIRED = 483,          //enum, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_REC_NUMBER = 485,                 //accouting-record-number, unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_SESSION_ID = 44,                  //octetString, M, no V, rfc6733
    DIA_AVP_CODE_ACCT_SUB_SESSION_ID = 287,             //unsigned64, M, no V, rfc6733
    DIA_AVP_CODE_AUTH_APP_ID = 258,                     //auth-application-id, unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_AUTH_GRACE_PERIOD = 276,               //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_AUTH_LIFETIME = 291,                   //authorization-lifetime, unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_AUTH_REQ_TYPE = 274,                   //enum, M, no V, rfc6733
    DIA_AVP_CODE_AUTH_SESSION_STATE = 277,              //enum, M, no V, rfc6733
    DIA_AVP_CODE_CLASS = 25,                            //octetString, M, no V, rfc6733
    DIA_AVP_CODE_DEST_HOST = 293,                       //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_DEST_REALM = 283,                      //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_DISCONNECT_CAUSE = 273,                //enum, M, no V, rfc6733
    DIA_AVP_CODE_ERROR_MESSAGE = 281,                   //utf8string, no M, no V, rfc6733
    DIA_AVP_CODE_ERROR_REPORTING_HOST = 294,            //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_EVENT_TIMESTAMP = 55,                  //time, M, no V, rfc6733
    DIA_AVP_CODE_EXPERIMENTAL_RESULT = 297,             //grouped, M, no V, rfc6733
    DIA_AVP_CODE_EXPERIMENTAL_RESULT_CODE = 298,        //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_FAILED_AVP = 279,                      //grouped, M, no V, rfc6733
    DIA_AVP_CODE_FIRMWARE_REV = 267,                    //unsigned32, no M, no V, rfc6733
    DIA_AVP_CODE_HOST_IP_ADDRESS = 257,                 //address, M, no V, rfc6733
    DIA_AVP_CODE_INBAND_SECURITY_ID = 299,              //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_MULTI_ROUND_TIMEOUT = 272,             //unsigned32, no M, no V, rfc6733
    DIA_AVP_CODE_ORIG_HOST = 264,                       //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_ORIG_REALM = 296,                      //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_ORIG_STATE_ID = 278,                   //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_PRODUCT_NAME = 269,                    //utf8string, no M, no V, rfc6733
    DIA_AVP_CODE_PROXY_HOST = 280,                      //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_PROXY_INFO = 284,                      //grouped, M, no V, rfc6733
    DIA_AVP_CODE_PROXY_STATE = 33,                      //octetString, M, no V, rfc6733
    DIA_AVP_CODE_REDIRECT_HOST = 292,                   //diamUri, M, no V, rfc6733
    DIA_AVP_CODE_ROUTE_RECORD = 282,                    //diamIdentity, M, no V, rfc6733
    DIA_AVP_CODE_REAUTH_REQUEST_TYPE = 285,             //enum, M, no V, rfc6733
    DIA_AVP_CODE_REDIRECT_HOST_USAGE = 261,             //enum, M, no V, rfc6733
    DIA_AVP_CODE_REDIRECT_MAX_CACHE_TIME = 262,         //unsigned32, no M, no V, rfc6733
    DIA_AVP_CODE_RESULT_CODE = 268,                     //unsigned32, no M, no V, rfc6733
    DIA_AVP_CODE_SESSION_BINDING = 270,                 //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_SESSION_ID = 263,                      //utf8String, no M, no V, rfc6733
    DIA_AVP_CODE_SESSION_SERVER_FAILOVER = 271,         //enum, M, no V, rfc6733
    DIA_AVP_CODE_SESSION_TIMEOUT = 27,                  //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_SUPPORTED_VENDOR_ID = 265,             //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_TERMINATION_CAUSE = 295,               //enum, M, no V, rfc6733
    DIA_AVP_CODE_VENDOR_ID = 266,                       //unsigned32, M, no V, rfc6733
    DIA_AVP_CODE_USER_NAME = 1,                         //utf8string, M, no V, rfc6733
    DIA_AVP_CODE_VENDOR_SPECIFIC_APP_ID = 260,          //grouped, M, no V, rfc6733
} diaAvpBaseCode_e;


typedef enum {
    DIA_RESULT_CODE_NONE = 0,
    DIA_RESULT_CODE_MULTI_ROUND_AUTH = 1001,
    DIA_RESULT_CODE_DIAMETER_SUCCESS = 2001,
    DIA_RESULT_CODE_DIAMETER_LIMITED_SUCCESS = 2002,
    DIA_RESULT_CODE_COMMAND_UNSUPPORTED = 3001,
    DIA_RESULT_CODE_UNABLE_TO_DELIVER = 3002,
    DIA_RESULT_CODE_REALM_NOT_SERVED = 3003,
    DIA_RESULT_CODE_TOO_BUSY = 3004,
    DIA_RESULT_CODE_LOOP_DETECTED = 3005,
    DIA_RESULT_CODE_REDIRECT_INDICATION = 3006,
    DIA_RESULT_CODE_APPLICATION_UNSUPPORTED = 3007,
    DIA_RESULT_CODE_INVALID_HDR_BITS = 3008,
    DIA_RESULT_CODE_INVALID_AVP_BITS = 3009,
    DIA_RESULT_CODE_UNKNOWN_PEER = 3010,    //A CER was received from an unknown peer
    DIA_RESULT_CODE_AUTHENTICATION_REJECTED = 4001,
    DIA_RESULT_CODE_OUT_OF_SPACE = 4002,
    DIA_RESULT_CODE_ELECTION_LOST = 4003,
    DIA_RESULT_CODE_AVP_UNSUPPORTED = 5001,
    DIA_RESULT_CODE_UNKNOWN_SESSION_ID = 5002,
    DIA_RESULT_CODE_AUTHORIZATION_REJECTED = 5003,
    DIA_RESULT_CODE_INVALID_AVP_VALUE = 5004,
    DIA_RESULT_CODE_MISSING_AVP = 5005,
    DIA_RESULT_CODE_RESOURCES_EXCEEDED = 5006,
    DIA_RESULT_CODE_CONTRADICTING_AVPS = 5007,
    DIA_RESULT_CODE_AVP_NOT_ALLOWED = 5008,
    DIA_RESULT_CODE_AVP_OCCURS_TOO_MANY_TIMES = 5009,
    DIA_RESULT_CODE_NO_COMMON_APPLICATION = 5010,
    DIA_RESULT_CODE_UNSUPPORTED_VERSION = 5011,
    DIA_RESULT_CODE_UNABLE_TO_COMPLY = 5012,
    DIA_RESULT_CODE_INVALID_BIT_IN_HEADER = 5013,
    DIA_RESULT_CODE_INVALID_AVP_LENGTH = 5014,
    DIA_RESULT_CODE_INVALID_MESSAGE_LENGTH = 5015,
    DIA_RESULT_CODE_INVALID_AVP_BIT_COMBO = 5016,
    DIA_RESULT_CODE_NO_COMMON_SECURITY = 5017,
} diaResultCode_e;


#endif
