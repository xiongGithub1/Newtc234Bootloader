/**********************************************************************************************************************
 * \file    can_nm.c
 * \brief   AUTOSAR CAN Network Management (CanNm) module implementation
 * \version V1.0.0
 * \date    2026-05-20
 *
 * Implements AUTOSAR CanNm state machine per AUTOSAR NM specification:
 * - Bus-Sleep Mode
 * - Prepare Bus-Sleep Mode
 * - Network Mode (Repeat Message / Normal Operation / Ready Sleep)
 *********************************************************************************************************************/
#include "can_nm.h"


/*============================================================================*/
/* Private Variables                                                          */
/*============================================================================*/
static CanNm_StateType gs_CanNmState = CANNM_STATE_BUS_SLEEP;
static CanNm_ModeType  gs_CanNmMode  = CANNM_MODE_BUS_SLEEP;

/* Timers (count down in ms) */
static volatile uint16 gs_CanNmTimeoutTimer      = 0;  /* NmTimeoutTimer */
static volatile uint16 gs_CanNmRepeatMsgTimer    = 0;  /* RepeatMessageTimer */
static volatile uint16 gs_CanNmWaitBusSleepTimer = 0;  /* WaitBusSleepTimer */
static volatile uint16 gs_CanNmMsgCycleTimer     = 0;  /* Message cycle timer */
static volatile uint8  gs_CanNmImmediateTxCnt    = 0;  /* Fast TX counter */

/* Flags */
static uint8 gs_CanNmNetworkRequested   = FALSE;
static uint8 gs_CanNmRepeatMsgRequested = FALSE;
static uint8 gs_CanNmFirstStart         = TRUE;

/* Transmit buffer */
static uint8 gs_CanNmTxBuf[CANNM_NM_PDU_LENGTH];

/* Callbacks */
static CanNm_SleepIndicationCbkType gs_pfSleepIndCbk = ((void *)0);
static CanNm_NetworkModeIndicationCbkType gs_pfNetworkModeCbk = ((void *)0);

/* Rx debug observation */
static volatile uint32 gs_CanNmExtRxCount = 0u;
static volatile uint32 gs_CanNmSelfFilteredCount = 0u;
static volatile uint16 gs_CanNmLastRxCanId = 0u;
static volatile uint8  gs_CanNmLastRxNodeId = 0u;
static volatile uint8  gs_CanNmLastRxCbv = 0u;

/*============================================================================*/
/* Local Function Prototypes                                                  */
/*============================================================================*/
static void CanNm_TransmitMessage(void);
static void CanNm_EnterState(CanNm_StateType newState);
static void CanNm_TxTimeoutException(void);

/*============================================================================*/
/* Internal Helper Functions                                                  */
/*============================================================================*/
static void CanNm_TransmitMessage(void)
{
    IfxMultican_Message msg;
    uint32 txBusyGuard = 10000u;
    uint32 dataLow  = ((uint32)gs_CanNmTxBuf[0]) |
                      (((uint32)gs_CanNmTxBuf[1]) << 8) |
                      (((uint32)gs_CanNmTxBuf[2]) << 16) |
                      (((uint32)gs_CanNmTxBuf[3]) << 24);
    uint32 dataHigh = ((uint32)gs_CanNmTxBuf[4]) |
                      (((uint32)gs_CanNmTxBuf[5]) << 8) |
                      (((uint32)gs_CanNmTxBuf[6]) << 16) |
                      (((uint32)gs_CanNmTxBuf[7]) << 24);

    IfxMultican_Message_init(&msg, (uint32)CANNM_NM_PDU_CAN_ID, dataLow, dataHigh, IfxMultican_DataLengthCode_8);
    while (IfxMultican_Can_MsgObj_sendMessage(&g_MulticanBasic.drivers.canNode0MsgTx2[0], &msg) == IfxMultican_Status_notSentBusy)
    {
        if (txBusyGuard == 0u)
        {
            CanNm_TxTimeoutException();
            return;
        }
        txBusyGuard--;
    }
}

/*============================================================================*/
/* Tx Timeout Exception Handler                                               */
/*============================================================================*/
static void CanNm_TxTimeoutException(void)
{
    /* AUTOSAR intent: on Tx timeout in Network Mode, re-enter Repeat Message
     * and restart RepeatMessageTimer. */
    if ((gs_CanNmState == CANNM_STATE_NORMAL_OPERATION) ||
        (gs_CanNmState == CANNM_STATE_READY_SLEEP) ||
        (gs_CanNmState == CANNM_STATE_REPEAT_MESSAGE))
    {
        CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
    }
}

/*============================================================================*/
/* State Transition Handler                                                   */
/*============================================================================*/
static void CanNm_EnterState(CanNm_StateType newState)
{
    gs_CanNmState = newState;

    switch (newState)
    {
        case CANNM_STATE_BUS_SLEEP:
        {
            gs_CanNmMode = CANNM_MODE_BUS_SLEEP;
            gs_CanNmMsgCycleTimer = 0;
            gs_CanNmWaitBusSleepTimer = 0;
            gs_CanNmTimeoutTimer = 0;
            gs_CanNmRepeatMsgTimer = 0;
            gs_CanNmImmediateTxCnt = 0;
            break;
        }

        case CANNM_STATE_PREPARE_BUS_SLEEP:
        {
            gs_CanNmMode = CANNM_MODE_PREPARE_BUS_SLEEP;
            gs_CanNmWaitBusSleepTimer = CANNM_T_WAIT_BUS_SLEEP;
            gs_CanNmMsgCycleTimer = 0;
            gs_CanNmImmediateTxCnt = 0;
            break;
        }

        case CANNM_STATE_REPEAT_MESSAGE:
        {
            gs_CanNmMode = CANNM_MODE_NETWORK;
            gs_CanNmRepeatMsgTimer = CANNM_T_REPEAT_MESSAGE;
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;

            /* Set Active Wakeup Bit if this was a local request */
            if (gs_CanNmNetworkRequested == TRUE)
            {
                gs_CanNmTxBuf[1] |= CANNM_CBV_ACTIVE_WAKEUP_BIT;
            }
            else
            {
                gs_CanNmTxBuf[1] &= (uint8)(~CANNM_CBV_ACTIVE_WAKEUP_BIT);
            }

            /* Handle Repeat Message Request from remote */
            if (gs_CanNmRepeatMsgRequested == TRUE)
            {
                gs_CanNmTxBuf[1] |= CANNM_CBV_REPEAT_MSG_REQUEST;
                gs_CanNmRepeatMsgRequested = FALSE;
            }
            else
            {
                gs_CanNmTxBuf[1] &= (uint8)(~CANNM_CBV_REPEAT_MSG_REQUEST);
            }

            /* First transmission: delay by T_START_NM_TX + random offset */
            if (gs_CanNmFirstStart == TRUE)
            {
                gs_CanNmMsgCycleTimer = CANNM_T_START_NM_TX + CANNM_T_START_APPEND;
                gs_CanNmImmediateTxCnt = CANNM_T_NM_IMMEDIATE_NMTX;
                gs_CanNmFirstStart = FALSE;
            }
            else
            {
                gs_CanNmMsgCycleTimer = CANNM_T_NM_MESSAGE_CYCLE;
                CanNm_TransmitMessage(); /* Immediate first send on transition */
            }

            if (gs_pfNetworkModeCbk != ((void *)0))
            {
                gs_pfNetworkModeCbk();
            }
            break;
        }

        case CANNM_STATE_NORMAL_OPERATION:
        {
            gs_CanNmMode = CANNM_MODE_NETWORK;
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;
            gs_CanNmMsgCycleTimer = CANNM_T_NM_MESSAGE_CYCLE;
            gs_CanNmTxBuf[1] &= (uint8)(~CANNM_CBV_REPEAT_MSG_REQUEST);
            CanNm_TransmitMessage(); /* Send immediately on entering Normal Op */
            break;
        }

        case CANNM_STATE_READY_SLEEP:
        {
            gs_CanNmMode = CANNM_MODE_NETWORK;
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;
            gs_CanNmMsgCycleTimer = 0; /* Stop transmitting */
            gs_CanNmTxBuf[1] &= (uint8)(~CANNM_CBV_REPEAT_MSG_REQUEST);
            break;
        }

        default:
            break;
    }
}

/*============================================================================*/
/* External API                                                               */
/*============================================================================*/
void CanNm_Init(void)
{
    /* Initialize NM PDU */
    memset(gs_CanNmTxBuf, 0x00, CANNM_NM_PDU_LENGTH);
    gs_CanNmTxBuf[0] = CANNM_NODE_ID;  /* Source Node Identifier */
    gs_CanNmTxBuf[1] = 0x00;            /* CBV */

    gs_CanNmState = CANNM_STATE_BUS_SLEEP;
    gs_CanNmMode  = CANNM_MODE_BUS_SLEEP;

    gs_CanNmNetworkRequested   = FALSE;
    gs_CanNmRepeatMsgRequested = FALSE;
    gs_CanNmFirstStart         = TRUE;

    gs_CanNmTimeoutTimer      = 0;
    gs_CanNmRepeatMsgTimer    = 0;
    gs_CanNmWaitBusSleepTimer = 0;
    gs_CanNmMsgCycleTimer     = 0;
    gs_CanNmImmediateTxCnt    = 0;

    gs_CanNmExtRxCount = 0u;
    gs_CanNmSelfFilteredCount = 0u;
    gs_CanNmLastRxCanId = 0u;
    gs_CanNmLastRxNodeId = 0u;
    gs_CanNmLastRxCbv = 0u;
}

void CanNm_DeInit(void)
{
    gs_CanNmState = CANNM_STATE_UNINIT;
}

void CanNm_NetworkRequest(void)
{
    gs_CanNmNetworkRequested = TRUE;

    if ((gs_CanNmState == CANNM_STATE_BUS_SLEEP) ||
        (gs_CanNmState == CANNM_STATE_PREPARE_BUS_SLEEP))
    {
        CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
    }
    else if (gs_CanNmState == CANNM_STATE_READY_SLEEP)
    {
        CanNm_EnterState(CANNM_STATE_NORMAL_OPERATION);
    }
    /* In REPEAT_MESSAGE or NORMAL_OPERATION: no state change needed */
}

void CanNm_NetworkRelease(void)
{
    gs_CanNmNetworkRequested = FALSE;

    if (gs_CanNmState == CANNM_STATE_NORMAL_OPERATION)
    {
        CanNm_EnterState(CANNM_STATE_READY_SLEEP);
    }
    /* In other states: no state change needed */
}

void CanNm_RxIndication(uint16 rxCanId, const uint8 *rxData)
{
    uint8 rxNodeId;
    uint8 rxCbv;
    uint8 repeatMsgBitSet;

    if (rxData == ((void *)0))
    {
        return;
    }

    if (CANNM_IS_NM_CAN_ID(rxCanId) == FALSE)
    {
        return;
    }

    rxNodeId = rxData[0];
    rxCbv    = rxData[1];
    repeatMsgBitSet = ((rxCbv & CANNM_CBV_REPEAT_MSG_REQUEST) != 0u) ? TRUE : FALSE;

    gs_CanNmLastRxCanId = rxCanId;
    gs_CanNmLastRxNodeId = rxNodeId;
    gs_CanNmLastRxCbv = rxCbv;

    /* Ignore our own messages (loopback) */
    if (rxNodeId == CANNM_NODE_ID)
    {
        gs_CanNmSelfFilteredCount++;
        return;
    }

    gs_CanNmExtRxCount++;

    /* Handle Repeat Message Request */
    if (repeatMsgBitSet == TRUE)
    {
        gs_CanNmRepeatMsgRequested = TRUE;
    }

    /* State dependent handling */
    switch (gs_CanNmState)
    {
        case CANNM_STATE_BUS_SLEEP:
        {
            /* Remote wakeup: enter network mode */
            CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
            break;
        }

        case CANNM_STATE_PREPARE_BUS_SLEEP:
        {
            /* Remote wakeup: enter network mode */
            CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
            break;
        }

        case CANNM_STATE_REPEAT_MESSAGE:
        {
            /* Restart NmTimeoutTimer */
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;

            /* AUTOSAR: Repeat Message Bit while in Repeat Message restarts
             * RepeatMessageTimer. */
            if (repeatMsgBitSet == TRUE)
            {
                gs_CanNmRepeatMsgTimer = CANNM_T_REPEAT_MESSAGE;
            }
            break;
        }

        case CANNM_STATE_NORMAL_OPERATION:
        {
            /* Restart NmTimeoutTimer */
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;

            /* AUTOSAR: Repeat Message Bit in Normal Operation shall trigger
             * transition to Repeat Message State. */
            if (repeatMsgBitSet == TRUE)
            {
                CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
            }
            break;
        }

        case CANNM_STATE_READY_SLEEP:
        {
            /* AUTOSAR: in Network Mode (including Ready Sleep), RxIndication
             * shall restart NmTimeoutTimer. */
            gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;

            /* AUTOSAR: Repeat Message Bit in Ready Sleep triggers transition
             * to Repeat Message State. */
            if (repeatMsgBitSet == TRUE)
            {
                CanNm_EnterState(CANNM_STATE_REPEAT_MESSAGE);
            }
            break;
        }

        default:
            break;
    }
}

void CanNm_TxConfirmation(void)
{
    /* Transmission confirmed - can be used for error handling if needed */
}

void CanNm_SystemTickCtl(void)
{
    if (gs_CanNmState == CANNM_STATE_UNINIT)
    {
        return;
    }

    if (gs_CanNmTimeoutTimer > 0)
    {
        gs_CanNmTimeoutTimer--;
    }

    if (gs_CanNmRepeatMsgTimer > 0)
    {
        gs_CanNmRepeatMsgTimer--;
    }

    if (gs_CanNmWaitBusSleepTimer > 0)
    {
        gs_CanNmWaitBusSleepTimer--;
    }

    if (gs_CanNmMsgCycleTimer > 0)
    {
        gs_CanNmMsgCycleTimer--;
    }
}

void CanNm_MainFunction(void)
{
    if (gs_CanNmState == CANNM_STATE_UNINIT)
    {
        return;
    }

    switch (gs_CanNmState)
    {
        /*====================================================================*/
        case CANNM_STATE_REPEAT_MESSAGE:
        {
            /* Check if message cycle timer expired -> transmit NM message */
            if (gs_CanNmMsgCycleTimer == 0)
            {
                CanNm_TransmitMessage();

                if (gs_CanNmImmediateTxCnt > 0)
                {
                    gs_CanNmImmediateTxCnt--;
                    gs_CanNmMsgCycleTimer = CANNM_T_NM_IMMEDIATE_CYCLE_TIME;
                }
                else
                {
                    gs_CanNmMsgCycleTimer = CANNM_T_NM_MESSAGE_CYCLE;
                }
            }

            /* Check if Repeat Message timer expired */
            if (gs_CanNmRepeatMsgTimer == 0)
            {
                if (gs_CanNmNetworkRequested == TRUE)
                {
                    CanNm_EnterState(CANNM_STATE_NORMAL_OPERATION);
                }
                else
                {
                    CanNm_EnterState(CANNM_STATE_READY_SLEEP);
                }
            }
            break;
        }

        /*====================================================================*/
        case CANNM_STATE_NORMAL_OPERATION:
        {
            /* Check if message cycle timer expired -> transmit NM message */
            if (gs_CanNmMsgCycleTimer == 0)
            {
                gs_CanNmMsgCycleTimer = CANNM_T_NM_MESSAGE_CYCLE;
                CanNm_TransmitMessage();
            }

            /* AUTOSAR CanNm: in Normal Operation, NmTimeout expiry shall not
             * force transition to Prepare Bus-Sleep. Keep network mode active
             * and restart NmTimeoutTimer. */
            if (gs_CanNmTimeoutTimer == 0)
            {
                gs_CanNmTimeoutTimer = CANNM_T_NM_TIMEOUT;
            }
            break;
        }

        /*====================================================================*/
        case CANNM_STATE_READY_SLEEP:
        {
            /* Do NOT transmit NM messages in Ready Sleep */

            /* Check NmTimeoutTimer */
            if (gs_CanNmTimeoutTimer == 0)
            {
                CanNm_EnterState(CANNM_STATE_PREPARE_BUS_SLEEP);
            }
            break;
        }

        /*====================================================================*/
        case CANNM_STATE_PREPARE_BUS_SLEEP:
        {
            /* Check Wait Bus Sleep timer */
            if (gs_CanNmWaitBusSleepTimer == 0)
            {
                CanNm_EnterState(CANNM_STATE_BUS_SLEEP);

                if (gs_pfSleepIndCbk != ((void *)0))
                {
                    gs_pfSleepIndCbk();
                }
            }
            break;
        }

        /*====================================================================*/
        case CANNM_STATE_BUS_SLEEP:
        {
            /* In Bus-Sleep, wait for NetworkRequest or RxIndication */
            break;
        }

        default:
            break;
    }
}

uint8 CanNm_GetState(CanNm_StateType *nmStatePtr, CanNm_ModeType *nmModePtr)
{
    if ((nmStatePtr == ((void *)0)) || (nmModePtr == ((void *)0)))
    {
        return 1; /* E_NOT_OK */
    }

    *nmStatePtr = gs_CanNmState;
    *nmModePtr  = gs_CanNmMode;
    return 0; /* E_OK */
}

uint8 CanNm_IsNetworkRequested(void)
{
    return gs_CanNmNetworkRequested;
}

uint8 CanNm_IsBusSleep(void)
{
    return (gs_CanNmState == CANNM_STATE_BUS_SLEEP) ? TRUE : FALSE;
}

void CanNm_RegisterSleepIndicationCbk(CanNm_SleepIndicationCbkType cbk)
{
    gs_pfSleepIndCbk = cbk;
}

void CanNm_RegisterNetworkModeCbk(CanNm_NetworkModeIndicationCbkType cbk)
{
    gs_pfNetworkModeCbk = cbk;
}

void CanNm_GetRxDebugInfo(uint32 *extRxCount,
                          uint32 *selfFilteredCount,
                          uint16 *lastCanId,
                          uint8  *lastNodeId,
                          uint8  *lastCbv)
{
    if (extRxCount != ((void *)0))
    {
        *extRxCount = gs_CanNmExtRxCount;
    }
    if (selfFilteredCount != ((void *)0))
    {
        *selfFilteredCount = gs_CanNmSelfFilteredCount;
    }
    if (lastCanId != ((void *)0))
    {
        *lastCanId = gs_CanNmLastRxCanId;
    }
    if (lastNodeId != ((void *)0))
    {
        *lastNodeId = gs_CanNmLastRxNodeId;
    }
    if (lastCbv != ((void *)0))
    {
        *lastCbv = gs_CanNmLastRxCbv;
    }
}
