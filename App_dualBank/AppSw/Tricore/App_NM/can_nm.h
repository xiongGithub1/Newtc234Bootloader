/*
 * @Author: qinXiong
 * @Date: 2026-05-20 17:26:29
 * @LastEditors: Qxiong&&2307975018@qq.com
 * @LastEditTime: 2026-05-20 17:28:40
 * @Description: 
 */
/**********************************************************************************************************************
 * \file    can_nm.h
 * \brief   AUTOSAR CAN Network Management (CanNm) module header
 * \version V1.0.0
 * \date    2026-05-20
 *********************************************************************************************************************/
#ifndef CAN_NM_H_
#define CAN_NM_H_

#include "Platform_Types.h"
#include "uds_common.h"
#include "CANRxTxInterface.h"
#include "uds_common.h"
#include "string.h"
#include "MultiCAN.h"
/*============================================================================*/
/* NM Configuration                                                           */
/*============================================================================*/
#define CANNM_NODE_ID                       (0x01u)     /* This ECU node ID */
#define CANNM_NM_PDU_CAN_ID                 (0x500u)    /* NM message CAN ID */
#define CANNM_NM_PDU_LENGTH                 (8u)

/* AUTOSAR NM Timing Parameters (ms) */
#define CANNM_T_NM_TIMEOUT                  (2000u)     /* NmTimeoutTime */
#define CANNM_T_REPEAT_MESSAGE              (1600u)     /* RepeatMessageTime */
#define CANNM_T_NM_MESSAGE_CYCLE            (20u)       /* NmMessageCycleTime */
#define CANNM_T_WAIT_BUS_SLEEP              (4000u)     /* WaitBusSleepTime */
#define CANNM_T_START_NM_TX                 (20u)       /* Start-up first NM msg delay */
#define CANNM_T_START_APPEND                (10u)       /* Random append to first msg */
#define CANNM_T_NM_IMMEDIATE_CYCLE_TIME     (10u)       /* Immediate NM cycle time (fast) */
#define CANNM_T_NM_IMMEDIATE_NMTX           (5u)        /* Number of fast NM messages */

/* PDU CBV (Control Bit Vector) bit definitions */
#define CANNM_CBV_REPEAT_MSG_REQUEST        (0x01u)     /* Bit 0 */
#define CANNM_CBV_NM_COORD_SLEEP_BIT        (0x08u)     /* Bit 3 */
#define CANNM_CBV_ACTIVE_WAKEUP_BIT         (0x10u)     /* Bit 4 */
#define CANNM_CBV_PNI_BIT                   (0x40u)     /* Bit 6 */

/*============================================================================*/
/* NM State Definitions                                                       */
/*============================================================================*/
typedef enum
{
    CANNM_STATE_UNINIT = 0,
    CANNM_STATE_BUS_SLEEP,
    CANNM_STATE_PREPARE_BUS_SLEEP,
    CANNM_STATE_REPEAT_MESSAGE,
    CANNM_STATE_NORMAL_OPERATION,
    CANNM_STATE_READY_SLEEP
} CanNm_StateType;

/*============================================================================*/
/* NM Mode Definitions                                                        */
/*============================================================================*/
typedef enum
{
    CANNM_MODE_BUS_SLEEP = 0,
    CANNM_MODE_PREPARE_BUS_SLEEP,
    CANNM_MODE_SYNCHRONIZE,
    CANNM_MODE_NETWORK
} CanNm_ModeType;

/*============================================================================*/
/* NM Callback Functions                                                      */
/*============================================================================*/
typedef void (*CanNm_SleepIndicationCbkType)(void);
typedef void (*CanNm_NetworkModeIndicationCbkType)(void);

/*============================================================================*/
/* Function Prototypes                                                        */
/*============================================================================*/
void CanNm_Init(void);
void CanNm_DeInit(void);
void CanNm_MainFunction(void);
void CanNm_SystemTickCtl(void);  /* Call every 1ms */

void CanNm_RxIndication(uint16 rxCanId, const uint8 *rxData);
void CanNm_TxConfirmation(void);

void CanNm_NetworkRequest(void);
void CanNm_NetworkRelease(void);

uint8 CanNm_GetState(CanNm_StateType *nmStatePtr, CanNm_ModeType *nmModePtr);
uint8 CanNm_IsNetworkRequested(void);
uint8 CanNm_IsBusSleep(void);

void CanNm_RegisterSleepIndicationCbk(CanNm_SleepIndicationCbkType cbk);
void CanNm_RegisterNetworkModeCbk(CanNm_NetworkModeIndicationCbkType cbk);

#endif /* CAN_NM_H_ */
