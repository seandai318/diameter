#include "osDebug.h"
#include "osTimer.h"

#include "diaConnState.h"
#include "diaConnMgr.h"
#include "diaConfig.h"
#include "diaBaseCer.h"


static osStatus_e diaConnStateEnterState(diaConnState_e newConnState, diaConnMsgType_e msgType, diaConnBlock_t* pDcb);
static osStatus_e diaConnStateClientCosed_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateServerClosed_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateClientWaitCea_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateOpen_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateSuspect_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateDown_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateReopen_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnStateClosing_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId);
static osStatus_e diaConnProcessCer(diaMsgDecoded_t* pDiaDecoded, diaConnBlock_t* pDcb);



void diaConnState_init(diaConnBlock_t* pDcb, bool isServer, bool isEnabled)
{
	if(isServer)
	{
		pDcb->connState = DIA_CONN_STATE_SERVER_CLOSED;
	}
	else
	{
		pDcb->connState = DIA_CONN_STATE_CLIENT_CLOSED;
	}

	if(isEnabled)
	{
    	diaConnState_onMsg(DIA_CONN_MSG_TYPE_FORCE_CONN, pDcb, NULL, 0);
	}
}


osStatus_e diaConnState_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
	osStatus_e status = OS_STATUS_OK;

	if(!pDcb)
	{
		logError("null pointer, pDcb.");
		goto EXIT;
	}

	switch(pDcb->connState)
	{
		case DIA_CONN_STATE_CLIENT_CLOSED:
			diaConnStateClientCosed_onMsg(msgType, pDcb, pDiaDecoded, timerId);
			break;			
    	case DIA_CONN_STATE_CLIENT_WAIT_CEA:
			diaConnStateClientWaitCea_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_SERVER_CLOSED:
			diaConnStateServerClosed_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_OPEN:
			diaConnStateOpen_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_SUSPECT:
			diaConnStateSuspect_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_DOWN:
			diaConnStateDown_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_REOPEN:
			diaConnStateReopen_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
    	case DIA_CONN_STATE_CLOSING:
			diaConnStateClosing_onMsg(msgType, pDcb, pDiaDecoded, timerId);
            break;
		defult:
			logError("unexpected state(%d) for pDcb(%p).", pDcb->connState, pDcb);
			break;
	}

EXIT:
	return status;

}


osStatus_e diaConnStateClientCosed_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
	DEBUG_BEGIN
	osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

	debug("msgType=%d", msgType);

	switch(msgType)
	{
		case DIA_CONN_MSG_TYPE_TRANSPORT_READY:
			if(pDcb->timerId_tpWaitConn)
			{
				pDcb->timerId_tpWaitConn = osStopTimer(pDcb->timerId_tpWaitConn);
			}
			if(pDcb->timerId_tpRetryConn)
			{
				pDcb->timerId_tpRetryConn = osStopTimer(pDcb->timerId_tpRetryConn);
			}

			//send out CER
			if(diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_CER, true, DIA_RESULT_CODE_NONE) != OS_STATUS_OK)
			{
				logError("fails to send CER.");
			}

			pDcb->timerId_twt = diaStartTimer(DIA_CONN_TIMER_TRANSMIT_WAIT_TIME, pDcb);
			diaConnStateEnterState(DIA_CONN_STATE_CLIENT_WAIT_CEA, msgType, pDcb);
			break;	
		case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
			if(pDcb->timerId_tpWaitConn)
			{
				osStopTimer(pDcb->timerId_tpWaitConn);
				pDcb->timerId_tpWaitConn = 0;
			}
			if(!pDcb->timerId_tpRetryConn)
			{
                pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_RETRY_CONN, pDcb);
            }
			break;
		case DIA_CONN_MSG_TYPE_FORCE_CONN:
            if(pDcb->timerId_tpRetryConn)
            {
                pDcb->timerId_tpRetryConn = osStopTimer(pDcb->timerId_tpRetryConn);
            }

			if(!pDcb->timerId_tpWaitConn)
			{
				if(diaConnMgr_startConn(pDcb) == OS_STATUS_OK)
            	{
                	pDcb->timerId_tpWaitConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);
            	}
				else
				{
                	pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_RETRY_CONN, pDcb);
				}
			}
			break;		
		case DIA_CONN_MSG_TYPE_STOP_CONN:
            if(pDcb->timerId_tpRetryConn)
            {
                pDcb->timerId_tpRetryConn = osStopTimer(pDcb->timerId_tpRetryConn);
            }

			if(pDcb->timerId_tpWaitConn)
			{
				diaConnMgr_stopConn(pDcb);
				pDcb->timerId_tpWaitConn = osStopTimer(pDcb->timerId_tpWaitConn);
			}
			break;	
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId = pDcb->timerId_tpWaitConn)
			{
				pDcb->timerId_tpWaitConn = 0;
				pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_RETRY_CONN, pDcb);
			}
			else if(timerId = pDcb->timerId_tpRetryConn)
			{
				transportStatus_e tpStatus = diaConnMgr_startConn(pDcb);
				switch(tpStatus)
				{
					case TRANSPORT_STATUS_TCP_CONN:
						pDcb->timerId_tpRetryConn = 0;
						pDcb->timerId_tpWaitConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);
						break;
                    case TRANSPORT_STATUS_TCP_FAIL:
                    case TRANSPORT_STATUS_FAIL:
						pDcb->timerId_tpWaitConn = 0;
						pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_RETRY_CONN, pDcb);
						break;
					default:
						//do nothing, notify function will trigger the next step.
						break;
				}
			}
            else
            {
                pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_RETRY_CONN, pDcb);
            }
			//DIA_CONN_TIMER_RETRY_TRANSPORT shall be longer than normal TCP conenction try time
			break;
		default:
			logError("dia req type (%d) is not processed.", msgType);
			break;
	}

EXIT:
	DEBUG_END
	return status;
}



osStatus_e diaConnStateServerClosed_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
        case DIA_CONN_MSG_TYPE_FORCE_CONN:
			//do nothing, as TCP listening is handled in the mas startup routine
			break;
		case DIA_CONN_MSG_TYPE_RCV_CER:
            status = diaConnProcessCer(pDiaDecoded, pDcb);
            if(status == OS_STATUS_OK)
            {
				diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_CER, false, DIA_RESULT_CODE_DIAMETER_SUCCESS); 

	            pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
    	        pDcb->isWaitDwa = false;

				diaConnStateEnterState(DIA_CONN_STATE_OPEN, DIA_CONN_MSG_TYPE_RCV_CER, pDcb);
			}
            break;
		//the open of conn is handled by diaConnMgr
        case DIA_CONN_MSG_TYPE_STOP_CONN:
            diaConnMgr_stopConn(pDcb);
            break;
        default:
            logError("dia req type (%d) is not processed.", msgType);
            break;
    }

EXIT:
    DEBUG_END
    return status;
}


osStatus_e diaConnStateClientWaitCea_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
		case DIA_CONN_MSG_TYPE_RCV_CEA:
			if(pDcb->timerId_twt)
			{
				pDcb->timerId_twt = osStopTimer(pDcb->timerId_twt);
			}
			status = diaConnProcessCea(pDiaDecoded, pDcb);
			if(status != OS_STATUS_OK)
			{
                //request tp to close TCP connection
                //start a new TCP connection
				diaConnMgr_stopConn(pDcb);
				pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);

	            diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_RCV_ERROR_CEA, pDcb);
				goto EXIT;
			}

            pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
            pDcb->isWaitDwa = false;

			diaConnStateEnterState(DIA_CONN_STATE_OPEN, DIA_CONN_MSG_TYPE_RCV_CEA, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId = pDcb->timerId_twt)
			{
				//request tp to close TCP connection
				//start a new TCP connection
                diaConnMgr_stopConn(pDcb);
                pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);

				diaConnStateEnterState(pDcb->isServer ? DIA_CONN_STATE_SERVER_CLOSED : DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_TIMEOUT, pDcb);
			}
			else
			{
				logError("timeout for timerId(0x%lx) is ignored.", timerId);
			}
			break;
        case DIA_CONN_MSG_TYPE_STOP_CONN:
			if(pDcb->timerId_twt)
			{
            	pDcb->timerId_twt = osStopTimer(pDcb->timerId_twt);
			}

			diaConnMgr_stopConn(pDcb);

			diaConnStateEnterState(DIA_CONN_STATE_OPEN, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
			break;
        case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
            if(pDcb->timerId_twt)
            {
				pDcb->timerId_twt = osStopTimer(pDcb->timerId_twt);
			}

            //request tp to close TCP connection
            //start a new TCP connection
            diaConnMgr_stopConn(pDcb);
            pDcb->timerId_tpRetryConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);

			diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_TRANSPORT_DOWN, pDcb);
			break;
		default:
			logInfo("msg type (%d) is ignored.", msgType);
			break;
	}

EXIT:
	return status;
}


osStatus_e diaConnStateOpen_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
		case DIA_CONN_MSG_TYPE_RCV_DWA:
        case DIA_CONN_MSG_TYPE_RCV_TRAFFIC_MSG:
			pDcb->isWaitDwa = false;
			pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);
			break;
		case DIA_CONN_MSG_TYPE_RCV_DWR:
            diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DWR, false, DIA_RESULT_CODE_DIAMETER_SUCCESS);

			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);
			}
			else
			{
            	pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
			}
            pDcb->isWaitDwa = false;
			break;	
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId != pDcb->timerId_watchdog)
			{
				logError("timerId(0x%lx) is ignored.", timerId);
				goto EXIT;
			}

			if(pDcb->isWaitDwa)
			{
				diaconnMgr_notifyFailover(pDcb);
				pDcb->isWaitDwa = false;
	            pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);

    	        diaConnStateEnterState(DIA_CONN_STATE_SUSPECT, DIA_CONN_MSG_TYPE_TIMEOUT, pDcb);
			}
			else
			{
				diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DWR, true, DIA_RESULT_CODE_NONE);
                pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
				pDcb->isWaitDwa = true;
			}
			break;
		case DIA_CONN_MSG_TYPE_STOP_CONN:
			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
			}
			pDcb->isWaitDwa = false;

			diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DPR, true, DIA_RESULT_CODE_NONE);
			pDcb->timerId_twt = diaStartTimer(DIA_CONN_TIMER_TRANSMIT_WAIT_TIME, pDcb);
			
			diaConnStateEnterState(DIA_CONN_STATE_CLOSING, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_RCV_DPR:
            if(pDcb->timerId_watchdog)
            {
                pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
            }
            pDcb->isWaitDwa = false;

            diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DPR, false, DIA_RESULT_CODE_DIAMETER_SUCCESS);
            pDcb->timerId_twt = diaStartTimer(DIA_CONN_TIMER_TRANSMIT_WAIT_TIME, pDcb);

            diaConnStateEnterState(DIA_CONN_STATE_CLOSING, DIA_CONN_MSG_TYPE_RCV_DPR, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
			}

			diaconnMgr_notifyFailover(pDcb);
            pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
			pDcb->isWaitDwa = false;
			diaConnStateEnterState(DIA_CONN_STATE_DOWN, DIA_CONN_MSG_TYPE_TRANSPORT_DOWN, pDcb);
			break;
		default:
            logInfo("msg type (%d) is ignored.", msgType);
            break;
    }


EXIT:
	return status;
}



osStatus_e diaConnStateSuspect_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
        case DIA_CONN_MSG_TYPE_RCV_DWA:
        case DIA_CONN_MSG_TYPE_RCV_TRAFFIC_MSG:
            diaconnMgr_notifyFailback(pDcb);

            pDcb->isWaitDwa = false;
            pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);

			diaConnStateEnterState(DIA_CONN_STATE_OPEN, msgType, pDcb);
            break;

		case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);
			}
			else
			{
				pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
			}

			diaConnStateEnterState(DIA_CONN_STATE_DOWN, DIA_CONN_MSG_TYPE_TRANSPORT_DOWN, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_STOP_CONN:
			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
			}
			diaConnMgr_stopConn(pDcb);
			if(pDcb->isServer)
			{
				diaConnStateEnterState(DIA_CONN_STATE_SERVER_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
			}
			else
			{
				diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
			}
			break;
		case DIA_CONN_MSG_TYPE_TIMEOUT:	
			if(timerId = pDcb->timerId_watchdog)
			{
				diaConnMgr_stopConn(pDcb);
				pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
			
				diaConnStateEnterState(DIA_CONN_STATE_DOWN, DIA_CONN_MSG_TYPE_TRANSPORT_DOWN, pDcb);
			}
			else
			{
				logError("timeout(0x%lx) is not expected, ignore.", timerId);
			}
			break;
		default:
			logInfo("dia msg type(%d) is ignored.", msgType);
			break;
	}

EXIT:
	DEBUG_END
	return status;
}		


osStatus_e diaConnStateDown_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId != pDcb->timerId_watchdog && timerId != pDcb->timerId_tpWaitConn)
			{
				logError("received unexpected timeout(0x%lx), ignore.", timerId);
				break;
			}

			if(pDcb->isServer)
			{
				pDcb->timerId_watchdog = 0;
				pDcb->timerId_tpWaitConn = 0;
			}
			else
			{
				if(timerId == pDcb->timerId_tpWaitConn)
				{
					pDcb->timerId_tpWaitConn = 0;
					pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
				}
				else
				{
					diaConnMgr_startConn(pDcb);
					pDcb->timerId_watchdog = 0;
					pDcb->timerId_tpWaitConn = diaStartTimer(DIA_CONN_TIMER_WAIT_CONN, pDcb);
				}
			}				
			break;
		case DIA_CONN_MSG_TYPE_TRANSPORT_READY:
			if(pDcb->timerId_tpWaitConn)
			{
				pDcb->timerId_tpWaitConn = osStopTimer(pDcb->timerId_tpWaitConn);
			}

			if(pDcb->timerId_watchdog)
			{
				pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
			}

			diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DWR, true, DIA_RESULT_CODE_NONE);
			pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
			pDcb->numDwa = 0;
			pDcb->isWaitDwa = true;
			
			diaConnStateEnterState(DIA_CONN_STATE_REOPEN, msgType, pDcb);
			break;
        case DIA_CONN_MSG_TYPE_STOP_CONN:
            if(pDcb->timerId_tpWaitConn)
            {
                pDcb->timerId_tpWaitConn = osStopTimer(pDcb->timerId_tpWaitConn);
				diaConnMgr_stopConn(pDcb);
            }

            if(pDcb->timerId_watchdog)
            {
                pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
            }
            if(pDcb->isServer)
            {
                diaConnStateEnterState(DIA_CONN_STATE_SERVER_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
            }
            else
            {
                diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
            }
            break;
		default:
            logInfo("dia msg type(%d) is ignored.", msgType);
            break;
    }

EXIT:
    DEBUG_END
    return status;
}


osStatus_e diaConnStateReopen_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
        case DIA_CONN_MSG_TYPE_RCV_DWA:
			if(pDcb->numDwa++ < 2)
			{
				pDcb->isWaitDwa = false;
			}
			if(pDcb->numDwa == 2)
			{
				pDcb->isWaitDwa = false;
				diaConnStateEnterState(DIA_CONN_STATE_OPEN, DIA_CONN_MSG_TYPE_RCV_DWA, pDcb);
				//diaconnMgr_notifyFailback(pDcb);

				pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);
			}
			break;
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId != pDcb->isWaitDwa)
			{
				logError("received unexpected timeout(0x%lx).", timerId);
				break;
			}

			if(pDcb->isWaitDwa)
			{
				if(pDcb->numDwa >= 0)
				{
					pDcb->numDwa = -1;
					pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
				}
				else
				{
					diaConnMgr_stopConn(pDcb);
					pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);

					diaConnStateEnterState(DIA_CONN_STATE_DOWN, msgType, pDcb);
				}
			}
			else
			{
            	diaSendCommonMsg(pDcb->ifType, pDcb, DIA_CMD_CODE_DWR, true, DIA_RESULT_CODE_NONE);
            	pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
            	pDcb->isWaitDwa = true;
			}	
			break;
		case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
            if(pDcb->timerId_watchdog)
            {
                pDcb->timerId_watchdog = osRestartTimer(pDcb->timerId_watchdog);
            }
            else
            {
                pDcb->timerId_watchdog = diaStartTimer(DIA_CONN_TIMER_WATCHDOG, pDcb);
            }

            diaConnStateEnterState(DIA_CONN_STATE_DOWN, DIA_CONN_MSG_TYPE_TRANSPORT_DOWN, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_STOP_CONN:
            if(pDcb->timerId_watchdog)
            {
                pDcb->timerId_watchdog = osStopTimer(pDcb->timerId_watchdog);
            }
            diaConnMgr_stopConn(pDcb);
            if(pDcb->isServer)
            {
                diaConnStateEnterState(DIA_CONN_STATE_SERVER_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
            }
            else
            {
                diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_STOP_CONN, pDcb);
            }
			break;
        default:
            logInfo("dia msg type(%d) is ignored.", msgType);
            break;
    }

EXIT:
    DEBUG_END
    return status;
}

	
osStatus_e diaConnStateClosing_onMsg(diaConnMsgType_e msgType, diaConnBlock_t* pDcb, diaMsgDecoded_t* pDiaDecoded, uint64_t timerId)
{
    DEBUG_BEGIN
    osStatus_e status = OS_STATUS_OK;

    if(!pDcb)
    {
        logError("null pointer, pDcb.");
        status = OS_ERROR_NULL_POINTER;
        goto EXIT;
    }

    switch(msgType)
    {
		case DIA_CONN_MSG_TYPE_TIMEOUT:
			if(timerId != pDcb->timerId_twt)
			{
				logError("received unexpected timeout(0x%lx), ignore", timerId);
				break;
			}

			pDcb->timerId_twt = 0;
			diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_TIMEOUT, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_RCV_DPA:
		case DIA_CONN_MSG_TYPE_TRANSPORT_DOWN:
			if(pDcb->timerId_twt)
			{
				pDcb->timerId_twt = osStopTimer(pDcb->timerId_twt);
			}

            diaConnStateEnterState(DIA_CONN_STATE_CLIENT_CLOSED, DIA_CONN_MSG_TYPE_RCV_DPA, pDcb);
			break;
		case DIA_CONN_MSG_TYPE_STOP_CONN:
			logInfo("received DIA_CONN_MSG_TYPE_STOP_CONN, do nothing.");
			break;
        default:
            logInfo("dia msg type(%d) is ignored.", msgType);
            break;
    }

EXIT:
    DEBUG_END
    return status;
}


osStatus_e diaConnStateEnterState(diaConnState_e newConnState, diaConnMsgType_e msgType, diaConnBlock_t* pDcb)	
{
	osStatus_e status = OS_STATUS_OK;

	if(!pDcb)
	{
		logError("null pointer, pDcb");
		status = OS_ERROR_NULL_POINTER;
		goto EXIT;
	}

	logInfo("current State=%d, new State=%d", pDcb->connState, newConnState);
	if(pDcb->connState != newConnState && newConnState == DIA_CONN_STATE_OPEN)
	{
		diaconnMgr_notifyFailback(pDcb);
	}
    pDcb->connState = newConnState;

EXIT:
	return status;
}


static osStatus_e diaConnProcessCer(diaMsgDecoded_t* pDiaDecoded, diaConnBlock_t* pDcb)
{
	logInfo("to-do, for now empty.");
	return OS_STATUS_OK;
}
