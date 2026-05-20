/**********************************************************************************************************************
 * \file    uds.c
 * \brief
 * \version V1.0.0
 * \date    2021??11??26??
 * \author  Administrator
 *********************************************************************************************************************/
#include "uds_app.h"
#include "did_dflash.h"


uint32 pageData1[128];



const  uint8 gs_aEraseMemoryRoutineControlId[4u] = { 0x31u, 0x01u, 0xFFu, 0x00u };


const  uint8 gs_aCheckSumRoutineControlId[4u] = { 0x31u, 0x01u, 0x02u, 0x02u };


const  uint8 gs_aCheckProgrammingDependencyId[4u] = { 0x31u, 0x01u, 0xFFu, 0x01u };





/* DID Data stored in DFlash - use direct pointers for read access
 * Sector 1: 0xAF002000 ~ 0xAF003FFF (Static DID data F186~F197)
 * All text data encoded in UTF-8
 */

tUDSCommCtrlMode g_CanMsgCommCtrlMode = UDS_CC_MODE_RX_TX;

uint32 p_rw_finger_data = 0;

/* g_rwDataTable: DID configuration for 0x22/0x2E services
 * DID 1-14 (F186~F197): Read-only from DFlash
 * DID 17 (F15B): Read from DFlash
 * DID 16 (F15A): Write to DFlash - handled separately in WriteDataByIdentifier0x2E
 */
tUDSRwDataTable g_rwDataTable[] =
{
	/* DID 1-14: Read-only DID from DFlash */
	{F186, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F186, DID_SIZE_F186, DID_SIZE_F186},
	{F187, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F187, DID_SIZE_F187, DID_SIZE_F187},
	{F188, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F188, DID_SIZE_F188, DID_SIZE_F188},
	{F189, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F189, DID_SIZE_F189, DID_SIZE_F189},
	{F18A, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F18A, DID_SIZE_F18A, DID_SIZE_F18A},
	{F18B, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F18B, DID_SIZE_F18B, DID_SIZE_F18B},
	{F18C, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F18C, DID_SIZE_F18C, DID_SIZE_F18C},
	{F190, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F190, DID_SIZE_F190, DID_SIZE_F190},
	{F191, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F191, DID_SIZE_F191, DID_SIZE_F191},
	{F192, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F192, DID_SIZE_F192, DID_SIZE_F192},
	{F193, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F193, DID_SIZE_F193, DID_SIZE_F193},
	{F194, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F194, DID_SIZE_F194, DID_SIZE_F194},
	{F195, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F195, DID_SIZE_F195, DID_SIZE_F195},
	{F197, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32) DFLASH_PTR_F197, DID_SIZE_F197, DID_SIZE_F197},
	/* DID 17: F15B - Read fingerprint from DFlash */
	{F15B, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_HEX, (uint32) DFLASH_PTR_F15B, FINGERPRINT_RECORD_SIZE, FINGERPRINT_RECORD_SIZE * FINGERPRINT_RECORD_MAX},
	//	{AFFF, UDS_RWDATA_RDONLY, UDS_RWDATA_DFLASH, UDS_RWDATA_ASCII, (uint32)DFLASH_PTR_F197, DID_SIZE_F197, DID_SIZE_F197},
};

#define IsWriteFingerprintRight(x) ((x == gs_aWriteFingerprintId)?TRUE:FALSE)



static const tUdsTimeInfo gs_stUdsAppCfg =
{
	1u,
	3u,
	10000u,
	5000u,
	2000u,    /* P2 Server time (ms): 2000ms */
	5000u   /* P2* Server time (ms): 5000ms */
};

static tUdsInfo gs_stUdsInfo =
{
	DEFALUT_SESSION,
	ERRO_REQUEST_ID,
	NONE_SECURITY,
	0u,
	0u,
	0u,
	0u
};

static const tUDSService gs_astUDSService[] =
{
	{
			0x10u,
			DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION ,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			NONE_SECURITY,
			DigSession0x10
	},
	{
			0x11u,
			DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			NONE_SECURITY,
			DoResetMCU0x11
	},
	{
			0x27u,
			DEFALUT_SESSION | PROGRAM_SESSION | EXTEND_SESSION,
			SUPPORT_PHYSICAL_ADDR,
			NONE_SECURITY,
			SecurityAccess0x27
	},
	{
			0x22u,
			PROGRAM_SESSION | EXTEND_SESSION,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			NONE_SECURITY,
			ReadDataByIdentifier0x22
	},
	{
			0x2Eu,
			PROGRAM_SESSION,
			SUPPORT_PHYSICAL_ADDR,
			SECURITY_LEVEL_1,
			WriteDataByIdentifier0x2E
	},
	{
			0x31u,
			PROGRAM_SESSION | EXTEND_SESSION,
			SUPPORT_PHYSICAL_ADDR,
			SECURITY_LEVEL_1,
			RoutineControl0x31
	},
	{
			0x34u,
			PROGRAM_SESSION ,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			SECURITY_LEVEL_2,
			RequestDownload0x34
	},

	{
			0x36u,
			PROGRAM_SESSION,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			SECURITY_LEVEL_2,
			TransferData0x36
	},
	{
			0x37u,
			PROGRAM_SESSION,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			SECURITY_LEVEL_2,
			RequestTransferExit0x37
	},

	/* Tester present service */
	{
			0x3Eu,
			PROGRAM_SESSION | EXTEND_SESSION,
			SUPPORT_PHYSICAL_ADDR | SUPPORT_FUNCTION_ADDR,
			NONE_SECURITY,
			TesterPresent0x3E
	},
};


uint8 IsCheckRoutineControlRight(tCheckRoutineCtlInfo i_eCheckRoutineCtlId,
	tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint8 Index = 0u;
	uint8 FindCnt = 0u;
	uint8* pDestRoutineCltId = NULL_PTR;

	ASSERT(NULL_PTR == m_pstPDUMsg);

	switch (i_eCheckRoutineCtlId)
	{
		case ERASE_MEMORY_ROUTINE_CONTROL:
			pDestRoutineCltId = (uint8*) &gs_aEraseMemoryRoutineControlId[0u];

			FindCnt = sizeof(gs_aEraseMemoryRoutineControlId);

			break;

		case CHECK_SUM_ROUTINE_CONTROL:
			pDestRoutineCltId = (uint8*) &gs_aCheckSumRoutineControlId[0u];

			FindCnt = sizeof(gs_aCheckSumRoutineControlId);

			break;

		case CHECK_DEPENDENCY_ROUTINE_CONTROL:
			pDestRoutineCltId = (uint8*) &gs_aCheckProgrammingDependencyId[0u];

			FindCnt = sizeof(gs_aCheckProgrammingDependencyId);

			break;

		default:

			return FALSE;


	}

	if ((NULL_PTR == pDestRoutineCltId) || (m_pstPDUMsg->xDataLen < FindCnt))
	{
		return FALSE;
	}

	while (Index < FindCnt)
	{
		if (m_pstPDUMsg->aDataBuf[Index] != pDestRoutineCltId[Index])
		{
			return FALSE;
		}

		Index++;
	}

	return TRUE;
}


uint8 IsEraseMemoryRoutineControl(tUdsAppMsgInfo* m_pstPDUMsg)
{


	return IsCheckRoutineControlRight(ERASE_MEMORY_ROUTINE_CONTROL, m_pstPDUMsg);
}


uint8 IsCheckSumRoutineControl(tUdsAppMsgInfo* m_pstPDUMsg)
{
	return IsCheckRoutineControlRight(CHECK_SUM_ROUTINE_CONTROL, m_pstPDUMsg);
}


uint8 IsCheckProgrammingDependency(tUdsAppMsgInfo* m_pstPDUMsg)
{
	return IsCheckRoutineControlRight(CHECK_DEPENDENCY_ROUTINE_CONTROL, m_pstPDUMsg);
}

uint16 GetUdsS3ServerTime(void)
{
	return (gs_stUdsInfo.xUdsS3ServerTime);
}

void SubUdsS3ServerTime(uint16 i_SubTime)
{
	gs_stUdsInfo.xUdsS3ServerTime -= i_SubTime;
}

uint16 GetUdsSecurityReqLockTime(void)
{
	return (gs_stUdsInfo.xSecurityReqLockTime);
}

void SubUdsSecurityReqLockTime(uint16 i_SubTime)
{
	gs_stUdsInfo.xSecurityReqLockTime -= i_SubTime;
}


uint8 IsS3ServerTimeout(void)
{
	uint8 TimeoutStatus = FALSE;

	if (0u == gs_stUdsInfo.xUdsS3ServerTime)
	{
		TimeoutStatus = TRUE;
	}
	else
	{
		TimeoutStatus = FALSE;
	}

	return TimeoutStatus;
}

uint8 IsCurDefaultSession(void)
{
	uint8 isCurDefaultSessionStatus = FALSE;

	if (DEFALUT_SESSION == gs_stUdsInfo.CurSessionMode)
	{
		isCurDefaultSessionStatus = TRUE;
	}
	else
	{
		isCurDefaultSessionStatus = FALSE;
	}

	return isCurDefaultSessionStatus;
}

uint8 IsCurSeesionCanRequest(uint8 i_SerSessionMode)
{
	uint8 status = FALSE;

	if ((i_SerSessionMode & gs_stUdsInfo.CurSessionMode)
		== gs_stUdsInfo.CurSessionMode)
	{
		status = TRUE;
	}
	else
	{
		status = FALSE;
	}

	return status;
}

uint8 IsCurSecurityLevelRequet(uint8 i_SerSecurityLevel)
{
	uint8 status = 0u;

	if ((gs_stUdsInfo.SecurityLevel & i_SerSecurityLevel) == i_SerSecurityLevel)
	{
		status = TRUE;
	}
	else
	{
		status = FALSE;
	}

	return status;
}

void SetCurrentSession(const uint8 i_SerSessionMode)
{
	gs_stUdsInfo.CurSessionMode = i_SerSessionMode;
}

void SetSecurityLevel(const uint8 i_SerSecurityLevel)
{
	gs_stUdsInfo.SecurityLevel = i_SerSecurityLevel;
}



void RestartS3Server(void)
{
	gs_stUdsInfo.xUdsS3ServerTime = UdsAppTimeToCount(gs_stUdsAppCfg.xS3Server);

}

uint16 GetUdsP2ServerTime(void)
{
	return (gs_stUdsInfo.xUdsP2ServerTime);
}

void SubUdsP2ServerTime(uint16 i_SubTime)
{
	gs_stUdsInfo.xUdsP2ServerTime -= i_SubTime;
}

uint16 GetUdsP2StarTime(void)
{
	return (gs_stUdsInfo.xUdsP2StarTime);
}

void SubUdsP2StarTime(uint16 i_SubTime)
{
	gs_stUdsInfo.xUdsP2StarTime -= i_SubTime;
}

uint8 IsP2ServerTimeout(void)
{
	if (0u == gs_stUdsInfo.xUdsP2ServerTime)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

uint8 IsP2StarTimeout(void)
{
	if (0u == gs_stUdsInfo.xUdsP2StarTime)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void RestartP2Server(void)
{
	gs_stUdsInfo.xUdsP2ServerTime = UdsAppTimeToCount(gs_stUdsAppCfg.xP2Server);
}

void RestartP2StarServer(void)
{
	gs_stUdsInfo.xUdsP2StarTime = UdsAppTimeToCount(gs_stUdsAppCfg.xP2Star);
}

void UDS_StartP2StarTimer(void)
{
	RestartP2StarServer();
}


void SaveRequestIdType(const uint32 i_SerRequestID)
{
	if (i_SerRequestID == TP_GetConfigRxMsgPHYID())
	{
		SetRequestIdType(SUPPORT_PHYSICAL_ADDR);
	}
	else if (i_SerRequestID == TP_GetConfigRxMsgFUNID())
	{
		SetRequestIdType(SUPPORT_FUNCTION_ADDR);
	}
	else
	{
		SetRequestIdType(ERRO_REQUEST_ID);
	}
}


void UDS_SystemTickCtl(void)
{
	if (GetUdsS3ServerTime())
	{
		SubUdsS3ServerTime(1u);
	}

	if (GetUdsSecurityReqLockTime())
	{
		SubUdsSecurityReqLockTime(1u);
	}

	if (GetUdsP2ServerTime())
	{
		SubUdsP2ServerTime(1u);
	}

	if (GetUdsP2StarTime())
	{
		SubUdsP2StarTime(1u);
	}

	/* P2 timeout: send 0x7F service response pending */
	if ((TRUE == IsP2ServerTimeout()) && (FALSE == IsP2StarTimeout()))
	{
		RestartP2Server();
		/* If a service is currently being processed (e.g., Flash erase which blocks
		 * UDS_MainFun), send NRC 0x78 automatically from the tick handler. */
		if (0u != gs_u8CurrentUdsServiceId)
		{
			tUdsAppMsgInfo stPendingMsg = { 0u, 0u, {0u}, NULL_PTR };
			stPendingMsg.xUdsId = TP_GetConfigTxMsgID();
			SetNegativeErroCode(gs_u8CurrentUdsServiceId, NRC_RESPONSE_PENDING, &stPendingMsg);
			(void) TP_WriteAFrameDataInTP(stPendingMsg.xUdsId, NULL_PTR, stPendingMsg.xDataLen, stPendingMsg.aDataBuf);
		}
	}

	/* S3 timeout: automatically return to default session and reset security level */
	if ((0u == GetUdsS3ServerTime()) && (TRUE != IsCurDefaultSession()))
	{
		SetCurrentSession(DEFALUT_SESSION);
		SetSecurityLevel(NONE_SECURITY);
		Flash_InitDowloadInfo();
		Flash_SetNextDownloadStep(FL_REQUEST_STEP);
		gs_DownloadCRC = 0xFFFFFFFFu;
		gs_bCrcActive = FALSE;
	}
}

uint8 IsCurRxIdCanRequest(uint8 i_SerRequestIdMode)
{
	uint8 status = 0u;

	if ((i_SerRequestIdMode & gs_stUdsInfo.RequsetIdMode)
		== gs_stUdsInfo.RequsetIdMode)
	{
		status = TRUE;
	}
	else
	{
		status = FALSE;
	}

	return status;
}

/* Check received Key against computed Key for Security Access
 * i_SecurityLevel: 1 for Level 1, 2 for Level 2
 */
static uint8 IsReceivedKeyRight(const uint8* i_pReceivedKey, const uint8* i_pTxSeed,
	const uint8 i_SecurityLevel)
{
	uint8 index = 0u;
	uint8 aComputedKey[SA_ALGORITHM_SEED_LEN] = { 0u };

	if (1u == i_SecurityLevel)
	{
		UDS_ALG_HAL_ComputeKey_Level1(i_pTxSeed, aComputedKey);
	}
	else if (2u == i_SecurityLevel)
	{
		UDS_ALG_HAL_ComputeKey_Level2(i_pTxSeed, aComputedKey);
	}
	else
	{
		return FALSE;
	}

	for (index = 0u; index < SA_ALGORITHM_SEED_LEN; index++)
	{
		if (i_pReceivedKey[index] != aComputedKey[index])
		{
			return FALSE;
		}
	}

	return TRUE;
}



void SetNegativeErroCode(const uint8 i_UDSServiceNum, const uint8 i_ErroCode,
	tUdsAppMsgInfo* m_pstPDUMsg)
{

	m_pstPDUMsg->aDataBuf[0u] = NEGTIVE_RESPONSE_ID;
	m_pstPDUMsg->aDataBuf[1u] = i_UDSServiceNum;
	m_pstPDUMsg->aDataBuf[2u] = i_ErroCode;
	m_pstPDUMsg->xDataLen = 3u;
}


tUDSService* GetUDSServiceInfo(uint8* m_pSupServItem)
{

	*m_pSupServItem = sizeof(gs_astUDSService) / sizeof(gs_astUDSService[0u]);
	return (tUDSService*) &gs_astUDSService[0u];
}

/* Tester present service  */
static void TesterPresent0x3E(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint8 RequestSubfunction = 0u;

	/* Check message length: must be SID + subfunction = 2 bytes */
	if (m_pstPDUMsg->xDataLen != 2u)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
		return;
	}

	RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

	/* Sub function */
	switch (RequestSubfunction)
	{
		case 0x00u:  /* Zero sub-function - send positive response */
			m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
			m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
			m_pstPDUMsg->xDataLen = 2u;
			RestartS3Server();
			break;

		case 0x80u:  /* Suppress positive response */
			m_pstPDUMsg->xDataLen = 0u;
			RestartS3Server();
			break;

		default:
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
			break;
	}
}
void UDS_MainFun(void)
{
	uint8 UDSSerIndex = 0u;
	uint8 UDSSerNum = 0u;
	tUdsAppMsgInfo stUdsAppMsg = { 0u, 0u, {0u}, NULL_PTR };

	uint8 isFindService = FALSE;
	uint8 SupSerItem = 0u;
	tUDSService* pstUDSService = nullptr;

#if defined (EN_AES_SA_ALGORITHM_SW) || defined (EN_ZLG_SA_ALGORITHM)
	UDS_ALG_HAL_AddSWTimerTickCnt();
#endif
	if (TRUE == TP_ReadAFrameDataFromTP(&stUdsAppMsg.xUdsId, &stUdsAppMsg.xDataLen, stUdsAppMsg.aDataBuf))
	{

		if (TRUE != IsCurDefaultSession())
		{


			RestartS3Server();
		}


		SaveRequestIdType(stUdsAppMsg.xUdsId);
	}
	else
	{
		return;
	}

	/* Start P2 server timer on receiving a new request */
	RestartP2Server();

	pstUDSService = GetUDSServiceInfo(&SupSerItem);


	UDSSerNum = stUdsAppMsg.aDataBuf[0u];
	while ((UDSSerIndex < SupSerItem) && (nullptr != pstUDSService))
	{
		if (UDSSerNum == pstUDSService[UDSSerIndex].SerNum)
		{
			isFindService = TRUE;

			if (TRUE != IsCurRxIdCanRequest(pstUDSService[UDSSerIndex].SupReqMode))
			{

				SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);

				break;
			}

			if (TRUE != IsCurSeesionCanRequest(pstUDSService[UDSSerIndex].SessionMode))
			{

				SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION, &stUdsAppMsg);
				break;
			}

			if (TRUE != IsCurSecurityLevelRequet(pstUDSService[UDSSerIndex].ReqLevel))
			{

				SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SECURITY_ACCESS_DENIED, &stUdsAppMsg);

				break;
			}

			stUdsAppMsg.pfUDSTxMsgServiceCallBack = nullptr;


			if (nullptr != pstUDSService[UDSSerIndex].pfSerNameFun)
			{
				gs_u8CurrentUdsServiceId = UDSSerNum;
				pstUDSService[UDSSerIndex].pfSerNameFun((tUDSService*) &pstUDSService[UDSSerIndex], &stUdsAppMsg);
			}
			else
			{

				SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_CONDITIONS_NOT_CORRECT, &stUdsAppMsg);
			}

			break;
		}
		UDSSerIndex++;
	}

	if (TRUE != isFindService)
	{

		SetNegativeErroCode(stUdsAppMsg.aDataBuf[0u], NRC_SERVICE_NOT_SUPPORTED, &stUdsAppMsg);
	}

	if (0u != stUdsAppMsg.xDataLen)
	{
		stUdsAppMsg.xUdsId = TP_GetConfigTxMsgID();
		(void) TP_WriteAFrameDataInTP(stUdsAppMsg.xUdsId, stUdsAppMsg.pfUDSTxMsgServiceCallBack, stUdsAppMsg.xDataLen, stUdsAppMsg.aDataBuf);
	}
	gs_u8CurrentUdsServiceId = 0u;
}

/* ??EEPROM?��??????
  *  ?????:??????????byteToRead
 *  	?????????0
 * */
uint8 readDataFromEEPROM(uint32 entry, uint8* pData, uint8 byteToRead)
{

	uint8 ret = tl_read_from_eeprom(entry, pData, byteToRead);
	if (ret == 0)
	{
		for (int i = 0; i < byteToRead; i++)
		{
			pData[i] = 0;
		}
	}
	return ret;
}
/* ??FLASH?��??????
  *  ?????:??????????byteToRead
 *  	?????????0
 * */
uint8 readDataFromFLASH(uint32 entry, uint8* pData, uint8 byteToRead)
{

	uint8 ret = tl_read_from_flash(entry, pData, byteToRead);
	if (ret == 0)
	{
		for (int i = 0; i < byteToRead; i++)
		{
			pData[i] = 0;
		}
	}
	return ret;
}
/* ??EEPROM??��??????
  *  ?????:��????????byteToWrite
 *  	��????????0
 * */
uint8 writeDataToEEPROM(uint32 entry, uint8* pData, uint8 byteToWrite)
{

	uint8 ret = tl_write_to_eeprom(entry, pData, byteToWrite);
	if (ret == 0)
	{
		for (int i = 0; i < byteToWrite; i++)
		{
			pData[i] = 0;
		}
	}
	return ret;
}
/* ??FLASH??��??????
  *  ?????:��????????byteToWrite
 *  	��????????0
 * */
uint8 writeDataToFLASH(uint32 entry, uint8* pData, uint8 byteToWrite)
{

	uint8 ret = tl_write_to_flash(entry, pData, byteToWrite);
	if (ret == 0)
	{
		for (int i = 0; i < byteToWrite; i++)
		{
			pData[i] = 0;
		}
	}
	return ret;
}


void SendMsgMainFun(void)
{
	uint8 aucMsgBuf[8u] = { 0xAA };
	tUdsId msgId = 0u;
	tUdsLen msgLength = 0u;
	tRxTxCanMsg txMsg;

	if (TRUE == TP_DriverReadDataFromTP(8u, &aucMsgBuf[0u], &msgId, &msgLength))
	{
		txMsg.usRxTxDataId = msgId;
		tl_memcpy(txMsg.aucDataBuf, aucMsgBuf, 8);
		DrvCanSendMessage(&txMsg);
		CANTP_DoTxMsgSuccessfulCallBack();




	}
}








/* Build 0x50 positive response with P2Server_max and P2*Server_max timing parameters.
 * Per ISO 14229-1:
 *   Bytes 3-4: P2Server_max in ms (1ms unit)
 *   Bytes 5-6: P2*Server_max in ms / 10 (10ms unit) */
static void BuildSessionPositiveResponse(tUdsAppMsgInfo* m_pstPDUMsg, uint8 RequestSubfunction)
{
	m_pstPDUMsg->aDataBuf[0u] = 0x50u;
	m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;
	m_pstPDUMsg->aDataBuf[2u] = (uint8) (gs_stUdsAppCfg.xP2Server >> 8);
	m_pstPDUMsg->aDataBuf[3u] = (uint8) (gs_stUdsAppCfg.xP2Server);
	m_pstPDUMsg->aDataBuf[4u] = (uint8) ((gs_stUdsAppCfg.xP2Star / 10u) >> 8);
	m_pstPDUMsg->aDataBuf[5u] = (uint8) (gs_stUdsAppCfg.xP2Star / 10u);
	m_pstPDUMsg->xDataLen = 6u;
}

static void DigSession0x10(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint8 RequestSubfunction = 0u;
	RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];





	switch (RequestSubfunction)
	{
		case 0x01u:
			BuildSessionPositiveResponse(m_pstPDUMsg, RequestSubfunction);
			SetCurrentSession(DEFALUT_SESSION);
			SetSecurityLevel(NONE_SECURITY);
			break;
		case 0x81u:
			SetCurrentSession(DEFALUT_SESSION);
			SetSecurityLevel(NONE_SECURITY);
			if (0x81u == RequestSubfunction)
			{
				m_pstPDUMsg->xDataLen = 0u;
			}

			break;
		case 0x02u:
			if (gs_stUdsInfo.CurSessionMode == DEFALUT_SESSION)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION, m_pstPDUMsg);
				break;
			}
			if (TRUE != IsCurSecurityLevelRequet(SECURITY_LEVEL_1))
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
				break;
			}
			else
			{
				BuildSessionPositiveResponse(m_pstPDUMsg, RequestSubfunction);
				SetCurrentSession(PROGRAM_SESSION);
				/* OEM: mark programming session phase */
				g_bootPhase = BOOT_PHASE_PROG_SESSION;
				RestartS3Server();
			}
			break;
		case 0x82u:
			SetCurrentSession(PROGRAM_SESSION);
			RestartS3Server();
			if (0x82u == RequestSubfunction)
			{
				m_pstPDUMsg->xDataLen = 0u;
			}


			break;
		case 0x03u:
			BuildSessionPositiveResponse(m_pstPDUMsg, RequestSubfunction);
			SetCurrentSession(EXTEND_SESSION);
			RestartS3Server();
			break;
		case 0x83u:
			SetCurrentSession(EXTEND_SESSION);

			if (0x83u == RequestSubfunction)
			{
				m_pstPDUMsg->xDataLen = 0u;
			}

			RestartS3Server();

			break;

		default:
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
			break;
	}
}

/**
 * @brief UDS Tx callback: jump to active APP bank after positive response sent.
 * @note  Called when 0x31 02 jumpToApp response is successfully transmitted.
 */
static void DoJumpToActiveBank(uint8 status)
{
	if (TX_MSG_SUCCESSFUL == status)
	{
		Boot_DualBank_JumpToBank(Boot_DualBank_GetActiveBank());
	}
}

#ifdef DIAGNOSTIC_MODE_FOR_APP
/**
 * @brief UDS Tx callback: trigger soft reset to enter bootloader after positive response sent.
 * @note  Called when 0x10 02 session switch response is successfully transmitted.
 */
static void DoResetToBootloader(uint8 status)
{
	if (TX_MSG_SUCCESSFUL == status)
	{
		SW_Reset();
	}
}
#endif


/**
 * @brief UDS Tx callback: perform hard reset after positive response sent.
 */
static void DoHardReset(uint8 status)
{
	if (TX_MSG_SUCCESSFUL == status)
	{


		/* 确保所有 Flash 操作完成 */
		IfxFlash_waitUnbusy(FLASH_MODULE, IfxFlash_FlashType_P0);
		IfxFlash_waitUnbusy(FLASH_MODULE, IfxFlash_FlashType_D0);

		/* 禁用中断 */
		IfxCpu_disableInterrupts();

		/* 数据同步 */
		__dsync();

		/* 清除 Safety Endinit 以访问 SCU 复位寄存器 */
		uint16 pwd = IfxScuWdt_getSafetyWatchdogPassword();
		IfxScuWdt_clearSafetyEndinit(pwd);

		/* 触发系统软件复位 */
		SCU_SWRSTCON.U = 0x00000002;  /* SWRSTREQ = 1 */

		/* 如果以上没有成功，进入死循环等待看门狗复位 */
		while (1);
	}
}

/**
 * @brief UDS Tx callback: perform soft reset after positive response sent.
 */
static void DoSoftReset(uint8 status)
{
	if (TX_MSG_SUCCESSFUL == status)
	{
		SW_Reset();
	}
}

static void DoResetMCU0x11(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{

	uint8 RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];

	m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
	m_pstPDUMsg->aDataBuf[1u] = RequestSubfunction;



	switch (RequestSubfunction)
	{
		case RESET_NONE:
			break;
		case HARD_RESET:
			m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoHardReset;
			break;
		case SOFT_RESET:
			m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoSoftReset;
			break;
		default:
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
			break;
	}


}


static void SecurityAccess0x27(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{

	uint8 RequestSubfunction = m_pstPDUMsg->aDataBuf[1u];
	static uint8 s_aSeedBuf[SA_ALGORITHM_SEED_LEN] = { 0u };
	static uint8 s_securityAttemptCnt = 0u;
	static const uint8 MAX_SECURITY_ATTEMPTS = 3u;
	uint8 ret = FALSE;

	/* Check if security access is currently locked */
	if (GetUdsSecurityReqLockTime() > 0)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED, m_pstPDUMsg);
		return;
	}

	switch (RequestSubfunction)
	{
		case 1:
			m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;

			ret = UDS_ALG_HAL_GetRandom(SA_ALGORITHM_SEED_LEN, s_aSeedBuf);

			if (TRUE == ret)
			{
				fsl_memcpy(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, SA_ALGORITHM_SEED_LEN);
				m_pstPDUMsg->xDataLen = 2u + SA_ALGORITHM_SEED_LEN;
			}
			else
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
			}

			break;

		case 0x02u:

			if (TRUE == IsReceivedKeyRight(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, 1u))
			{
				m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
				m_pstPDUMsg->xDataLen = 2u;
				fsl_memset(s_aSeedBuf, 0x1u, sizeof(s_aSeedBuf));
				s_securityAttemptCnt = 0u;
				SetSecurityLevel(SECURITY_LEVEL_1);
			}
			else
			{
				s_securityAttemptCnt++;
				if (s_securityAttemptCnt >= MAX_SECURITY_ATTEMPTS)
				{
					gs_stUdsInfo.xSecurityReqLockTime = UdsAppTimeToCount(10000u);
					s_securityAttemptCnt = 0u;
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_EXCEEDED_NUMBER_OF_ATTEMPTS, m_pstPDUMsg);
				}
				else
				{
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
				}
			}

			break;
		case 0x03u:
			m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;

			ret = UDS_ALG_HAL_GetRandom(SA_ALGORITHM_SEED_LEN, s_aSeedBuf);

			if (TRUE == ret)
			{
				fsl_memcpy(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, SA_ALGORITHM_SEED_LEN);
				m_pstPDUMsg->xDataLen = 2u + SA_ALGORITHM_SEED_LEN;
			}
			else
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
			}

			break;

		case 0x04u:

			if (TRUE == IsReceivedKeyRight(&m_pstPDUMsg->aDataBuf[2u], s_aSeedBuf, 2u))
			{
				m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
				m_pstPDUMsg->xDataLen = 2u;
				fsl_memset(s_aSeedBuf, 0x1u, sizeof(s_aSeedBuf));
				s_securityAttemptCnt = 0u;
				SetSecurityLevel(SECURITY_LEVEL_2);
			}
			else
			{
				s_securityAttemptCnt++;
				if (s_securityAttemptCnt >= MAX_SECURITY_ATTEMPTS)
				{
					gs_stUdsInfo.xSecurityReqLockTime = UdsAppTimeToCount(10000u);
					s_securityAttemptCnt = 0u;
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_EXCEEDED_NUMBER_OF_ATTEMPTS, m_pstPDUMsg);
				}
				else
				{
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_KEY, m_pstPDUMsg);
				}
			}

			break;
		default:
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
			break;
	}
}




static void ReadDataByIdentifier0x22(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint16 did;
	uint8 not_find_did = 0;
	m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
	did = (m_pstPDUMsg->aDataBuf[1u] << 8) | m_pstPDUMsg->aDataBuf[2u];

	if (m_pstPDUMsg->xDataLen < 3)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
		return;
	}

	/* Special handling for F15B - Read fingerprint records from DFlash
	 * Returns all stored fingerprint records (up to 3)
	 */
	if (did == F15B)
	{
		uint16 totalLen = DID_DFlash_ReadF15B(&m_pstPDUMsg->aDataBuf[3], sizeof(m_pstPDUMsg->aDataBuf) - 3);
		m_pstPDUMsg->aDataBuf[1u] = 0xF1;
		m_pstPDUMsg->aDataBuf[2u] = 0x5B;
		m_pstPDUMsg->xDataLen = 3u + totalLen;
		return;
	}

	if (did == AFFF)
	{
		m_pstPDUMsg->aDataBuf[1u] = 0xAF;
		m_pstPDUMsg->aDataBuf[2u] = 0xFF;
		uint32 activeBank = Boot_DualBank_GetActiveBank();
		uint8 targetBankChar;
		/* Report the inactive bank as the target for next flashing */
		if (activeBank == BANK_B)
		{
			targetBankChar = 0x0A; /* Bank A is inactive, will be flashed next */
		}
		else
		{
			targetBankChar = 0x0B; /* Bank B is inactive, will be flashed next */
		}
		m_pstPDUMsg->aDataBuf[3u] = targetBankChar;
		m_pstPDUMsg->xDataLen = 4;
		return;
	}
	for (int i = 0; i < sizeof(g_rwDataTable) / sizeof(g_rwDataTable[0]); i++)
	{
		if (g_rwDataTable[i].did == did)
		{
			uint8 dataLen = g_rwDataTable[i].dlc;
			if (dataLen > g_rwDataTable[i].dlc_max)
			{
				dataLen = g_rwDataTable[i].dlc_max;
			}
			m_pstPDUMsg->aDataBuf[1u] = (did & 0xFF00) >> 8;
			m_pstPDUMsg->aDataBuf[2u] = did & 0xFF;
			for (uint8 j = 0; j < dataLen; j++)
			{
				m_pstPDUMsg->aDataBuf[3u + j] = *((uint8*) g_rwDataTable[i].p_entry + j);
			}
			m_pstPDUMsg->xDataLen = 3u + dataLen;
			return;
		}
		not_find_did++;
	}

	if (not_find_did)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
	}
	else
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
	}
}


static void WriteDataByIdentifier0x2E(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint16 did;
	uint8 not_find_did = 0;
	m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
	did = (m_pstPDUMsg->aDataBuf[1u] << 8) | m_pstPDUMsg->aDataBuf[2u];


	if (m_pstPDUMsg->xDataLen < 4)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
		return;
	}

	/* DID F15A - 诊断仪刷写指纹信息写入 (66 bytes)
	 * Layout: 0~15: 刷写诊断仪设备号
	 *         16~25: 刷写前软件号
	 *         26~35: 刷写前软件版本号
	 *         36~45: 刷写日期(年月日)
	 *         46~55: 刷写后软件号
	 *         56~65: 刷写后软件版本号
	 *
	 */
	if (did == F15A)
	{
		uint16 writeLen = m_pstPDUMsg->xDataLen - 3;
		if (writeLen > FINGERPRINT_SIZE)
		{
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
			return;
		}

		/* Write fingerprint to DFlash (updates F15A + F15B records) */
		if (DID_DFlash_WriteF15A(&m_pstPDUMsg->aDataBuf[3]) != TRUE)
		{
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_GENERAL_PROGRAMMING_FAILURE, m_pstPDUMsg);
			return;
		}

		m_pstPDUMsg->aDataBuf[1u] = 0xF1;
		m_pstPDUMsg->aDataBuf[2u] = 0x5A;
		m_pstPDUMsg->xDataLen = 3u;
		return;
	}

	for (int i = 0;i < sizeof(g_rwDataTable) / sizeof(g_rwDataTable[0]);i++)
	{
		if (g_rwDataTable[i].did == did)
		{
			if ((m_pstPDUMsg->xDataLen - 3) > g_rwDataTable[i].dlc_max)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
				return;
			}

			if (g_rwDataTable[i].rw_mode == UDS_RWDATA_RDONLY)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
				return;
			}

			if (g_rwDataTable[i].rw_store == UDS_RWDATA_DFLASH)
			{

				if (writeDataToFLASH(g_rwDataTable[i].p_entry, &m_pstPDUMsg->aDataBuf[3], m_pstPDUMsg->xDataLen - 3) == 0)
				{
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_GENERAL_PROGRAMMING_FAILURE, m_pstPDUMsg);
					return;
				}

				if (g_rwDataTable[i].rw_mode == UDS_RWDATA_RDWR_WRONCE)
				{
					g_rwDataTable[i].rw_mode = UDS_RWDATA_RDONLY;
				}

				g_rwDataTable[i].dlc = m_pstPDUMsg->xDataLen - 3;
			}
			else if (g_rwDataTable[i].rw_store == UDS_RWDATA_EEPROM)
			{

				if (writeDataToEEPROM(g_rwDataTable[i].p_entry, &m_pstPDUMsg->aDataBuf[3], m_pstPDUMsg->xDataLen - 3) == 0)
				{
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_GENERAL_PROGRAMMING_FAILURE, m_pstPDUMsg);
					return;
				}

				if (g_rwDataTable[i].rw_mode == UDS_RWDATA_RDWR_WRONCE)
				{
					g_rwDataTable[i].rw_mode = UDS_RWDATA_RDONLY;
				}

				g_rwDataTable[i].dlc = m_pstPDUMsg->xDataLen - 3;
			}
			else
			{
				for (int j = 0; j < m_pstPDUMsg->xDataLen - 3; j++)
				{
					*((uint8*) g_rwDataTable[i].p_entry + j) = m_pstPDUMsg->aDataBuf[3 + j];
				}

				g_rwDataTable[i].dlc = m_pstPDUMsg->xDataLen - 3;
			}

			m_pstPDUMsg->aDataBuf[1u] = (did & 0xFF00) >> 8;
			m_pstPDUMsg->aDataBuf[2u] = did & 0xFF;
			m_pstPDUMsg->xDataLen = 3u;

			return;
		}
		not_find_did++;
	}

	if (not_find_did)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
	}
	else
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
	}
}







/*
 * ?????app???????-250512
 * */
#define APP1_START  0xA0020000
#define APP1_END    0xA00FFFFF
#define APP2_START  0xa0100000
#define APP2_END    0xa01fffff




static tDowloadDataInfo gs_stDowloadDataInfo = { 0u, 0u };


static uint32 gs_RxBlockNum = 0u;

/* Streaming CRC32 accumulated during TransferData (0x36).
 * Final value = gs_DownloadCRC ^ 0xFFFFFFFFu after all data received.
 */
static uint32 gs_DownloadCRC = 0xFFFFFFFFu;
static uint8  gs_bCrcActive = FALSE;

/* Current UDS service ID being processed in UDS_MainFun.
 * Used by UDS_SystemTickCtl to send NRC 0x78 when P2 times out
 * while the main loop is blocked by long-running operations (e.g. Flash erase). */
static uint8 gs_u8CurrentUdsServiceId = 0u;


static void RequestDownload0x34(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{

	uint8 Index = 0u;
	uint8 Ret = TRUE;
	uint32  addrBytesLength, dataBytesLength;
	uint32 addrAndDataBytesLength;
	addrAndDataBytesLength = m_pstPDUMsg->aDataBuf[2u];
	addrBytesLength = addrAndDataBytesLength & 0x0f;
	dataBytesLength = (addrAndDataBytesLength & 0xf0) >> 4;


	if (m_pstPDUMsg->xDataLen < (1u + 2u + addrBytesLength + dataBytesLength))
	{
		Ret = FALSE;
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
	}

	if (TRUE == Ret)
	{
		gs_stDowloadDataInfo.StartAddr = 0u;
		for (Index = 0u; Index < addrBytesLength; Index++)
		{
			gs_stDowloadDataInfo.StartAddr <<= 8u;
			gs_stDowloadDataInfo.StartAddr |= m_pstPDUMsg->aDataBuf[Index + 3u];
		}
		gs_stDowloadDataInfo.StartAddr = (gs_stDowloadDataInfo.StartAddr & 0x00FFFFFF) | 0xA0000000;

		/* Determine target bank from the address in the HEX file */
		{
			uint32 cachedAddr = gs_stDowloadDataInfo.StartAddr - 0x20000000u;

			if ((cachedAddr >= BANK_B_START_ADDR) && (cachedAddr < BANK_B_END_ADDR))
			{
				Boot_DualBank_SetTargetWriteBank(BANK_B);
			}
			else if ((cachedAddr >= BANK_A_START_ADDR) && (cachedAddr < BANK_A_END_ADDR))
			{
				Boot_DualBank_SetTargetWriteBank(BANK_A);
			}
			else
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
				Ret = FALSE;
			}
		}

		if ((TRUE == Ret) && (Boot_DualBank_GetTargetWriteBank() == Boot_DualBank_GetActiveBank()))
		{
			/* Never allow flashing the active bank, even if it appears invalid.
			 * The dual-bank design always writes to the INACTIVE bank first,
			 * then switches. This prevents overwriting the firmware we are
			 * currently running from. */
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
			Ret = FALSE;
		}

		gs_stDowloadDataInfo.DataLen = 0u;
		for (Index = 0u; Index < dataBytesLength; Index++)
		{
			gs_stDowloadDataInfo.DataLen <<= 8u;
			gs_stDowloadDataInfo.DataLen |= m_pstPDUMsg->aDataBuf[Index + 3 + addrBytesLength];
		}
	}

	if (((TRUE != IsDownloadDataAddrValid(gs_stDowloadDataInfo.StartAddr)) ||
		(TRUE != IsDownloadDataLenValid(gs_stDowloadDataInfo.DataLen))) && (TRUE == Ret))
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
		Ret = FALSE;
	}

	if (TRUE == Ret)
	{
		/*set wait transfer data step(0x34 service)*/
		Flash_SetNextDownloadStep(FL_TRANSFER_STEP);

		/* Initialise streaming CRC at the start of a new download sequence */
		if (FALSE == gs_bCrcActive)
		{
			gs_DownloadCRC = 0xFFFFFFFFu;
			gs_bCrcActive = TRUE;
		}

		Flash_SaveDownloadDataInfo(gs_stDowloadDataInfo.StartAddr, gs_stDowloadDataInfo.DataLen);

		m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
		m_pstPDUMsg->aDataBuf[1u] = 0x10u;
		m_pstPDUMsg->aDataBuf[2u] = 0x80u;
		m_pstPDUMsg->xDataLen = 3u;

		gs_RxBlockNum = 1;
	}
	else
	{
		Flash_InitDowloadInfo();
		Flash_SetNextDownloadStep(FL_REQUEST_STEP);
		gs_DownloadCRC = 0xFFFFFFFFu;
		gs_bCrcActive = FALSE;
	}
}

static void TransferData0x36(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo* m_pstPDUMsg)
{

	uint8 Ret = TRUE;


	if ((FL_TRANSFER_STEP != Flash_GetCurDownloadStep()) && (TRUE == Ret))
	{
		Ret = FALSE;
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
	}

	/* Verify sequence number (SN) per UDS specification */
	{
		uint8 rxSN = m_pstPDUMsg->aDataBuf[1u];
		if (rxSN != gs_RxBlockNum)
		{
			Ret = FALSE;
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
			Flash_InitDowloadInfo();
			Flash_SetNextDownloadStep(FL_REQUEST_STEP);
			gs_RxBlockNum = 0u;
		}
		else
		{
			gs_RxBlockNum++;
			if (gs_RxBlockNum > 0xFFu)
			{
				gs_RxBlockNum = 0u;
			}
		}
	}

	uint8 actualDataLen = m_pstPDUMsg->xDataLen - 2;




	if (TRUE != Flash_ProgramRegion(gs_stDowloadDataInfo.StartAddr,
		&m_pstPDUMsg->aDataBuf[2],
		actualDataLen)
		&& (TRUE == Ret))
	{
		Ret = FALSE;

		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
	}
	else
	{
		/* Accumulate CRC over the received payload (matches tester-side calculation) */
		if (gs_bCrcActive != FALSE)
		{
			gs_DownloadCRC = Boot_CRC32_Update(gs_DownloadCRC,
				&m_pstPDUMsg->aDataBuf[2],
				actualDataLen);
		}

		gs_stDowloadDataInfo.StartAddr += actualDataLen;
		gs_stDowloadDataInfo.DataLen -= actualDataLen;
	}

	if ((0u == gs_stDowloadDataInfo.DataLen) && (TRUE == Ret))
	{

		gs_RxBlockNum = 0u;

		Flash_SetNextDownloadStep(FL_EXIT_TRANSFER_STEP);
	}

	if (TRUE == Ret)
	{


		m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
		m_pstPDUMsg->xDataLen = 4u;

	}
	else
	{
		Flash_InitDowloadInfo();

		Flash_SetNextDownloadStep(FL_REQUEST_STEP);
		gs_RxBlockNum = 0u;
		gs_DownloadCRC = 0xFFFFFFFFu;
		gs_bCrcActive = FALSE;
	}
}


static void RequestTransferExit0x37(struct UDSServiceInfo* i_pstUDSServiceInfo,
	tUdsAppMsgInfo* m_pstPDUMsg)
{

	uint8 Ret = TRUE;

	if (FL_EXIT_TRANSFER_STEP != Flash_GetCurDownloadStep())
	{
		Ret = FALSE;

		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
	}

	if (TRUE == Ret)
	{
		Flash_SetNextDownloadStep(FL_CHECKSUM_STEP);



		m_pstPDUMsg->aDataBuf[0u] = i_pstUDSServiceInfo->SerNum + 0x40u;
		m_pstPDUMsg->xDataLen = 1u;
	}
	else
	{
		Flash_InitDowloadInfo();
	}
}


// ����������ľ���ʵ��?
// ����ֵ: ��8λ = canFlash (1=��, 0=����), ��8λ = targetBank ('A' �� 'B')
uint16 CheckProgrammingConditions(void) {
	uint8 canFlash = 1;
	uint8 targetBankChar;
	uint32 targetWriteBank = Boot_DualBank_GetActiveBank();

	if (targetWriteBank == BANK_B)
	{
		targetBankChar = 0x0A;
		Boot_DualBank_SetTargetWriteBank(BANK_A);
	}
	else
	{
		targetBankChar = 0x0B;
		Boot_DualBank_SetTargetWriteBank(BANK_B);
	}

	return ((uint16) canFlash << 8) | (uint16) targetBankChar;
}

static void RoutineControl0x31(struct UDSServiceInfo* i_pstUDSServiceInfo, tUdsAppMsgInfo* m_pstPDUMsg)
{
	uint8 subFunc = m_pstPDUMsg->aDataBuf[1];
	uint16 routineIdentifier;
	static uint8 currentRoutine = 0;
	static uint16 routineResult = 0;
	uint8* p;

	if (m_pstPDUMsg->xDataLen < 2)
	{
		SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
		return;
	}

	switch (subFunc)
	{
		case 0x01:
			{
				//				if (Flash_ForceWriteRemaining() == 0)
				//					{
				//						SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
				//						break;
				//					}

				if (m_pstPDUMsg->xDataLen < 4)
				{
					SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
					break;
				}

				routineIdentifier = (m_pstPDUMsg->aDataBuf[2] << 8) | m_pstPDUMsg->aDataBuf[3];

				switch (routineIdentifier)
				{
					// #ifdef DIAGNOSTIC_MODE_FOR_APP
					case 0xFFFD:
						if (TRUE != IsCurSecurityLevelRequet(SECURITY_LEVEL_1))
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
							break;
						}
						if (m_pstPDUMsg->xDataLen < 4)
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
							break;
						}
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
						routineResult = CheckProgrammingConditions();
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
						m_pstPDUMsg->aDataBuf[0] = 0x71;
						m_pstPDUMsg->aDataBuf[1] = 0x01;
						m_pstPDUMsg->aDataBuf[2] = 0xFF;
						m_pstPDUMsg->aDataBuf[3] = 0xFD;
						m_pstPDUMsg->aDataBuf[4] = (uint8) (routineResult >> 8);   /* canFlash: 1=��, 0=���� */

						m_pstPDUMsg->aDataBuf[5] = (uint8) (routineResult & 0xFF); /* targetBank: 'A' �� 'B' */
						m_pstPDUMsg->xDataLen = 6;
						break;
						// #endif	
					// erase flash
										// erase flash
					case 0xFF00:
						if (TRUE != IsCurSecurityLevelRequet(SECURITY_LEVEL_2))
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
							break;
						}

						if (m_pstPDUMsg->xDataLen < 6)
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
							break;
						}

						/* Erase the requested sector only if it belongs to the currently
						 * selected target bank.  g_udsTargetBank is set during the
						 * preceding RequestDownload (0x34) service based on the download
						 * start address.  This prevents accidental erasure of:
						 *   - Bootloader sectors (S0 ~ S7)
						 *   - The inactive bank (e.g. erasing Bank A while updating Bank B)
						 */
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
						routineResult = EraseFlashSector(m_pstPDUMsg->aDataBuf[4], m_pstPDUMsg->aDataBuf[5]);
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());

						if (routineResult == 0xFE)
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_OUT_OF_RANGE, m_pstPDUMsg);
							break;
						}
						if (routineResult == 0xFC)
						{
							/* Bootloader / reserved area protection */
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
							break;
						}
						if (routineResult == 0xFD)
						{
							/* Sector does not belong to the target bank selected by 0x34 */
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_REQUEST_SEQUENCE_ERROR, m_pstPDUMsg);
							break;
						}
						if (routineResult == 0x00)
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_GENERAL_PROGRAMMING_FAILURE, m_pstPDUMsg);
							break;
						}

						/* Positive response: 71 01 FF 00 <sector> <result> */
						m_pstPDUMsg->aDataBuf[0] = 0x71;
						m_pstPDUMsg->aDataBuf[1] = 0x01;
						m_pstPDUMsg->aDataBuf[4] = m_pstPDUMsg->aDataBuf[4] << 8 | m_pstPDUMsg->aDataBuf[5];
						m_pstPDUMsg->aDataBuf[5] = (uint8) routineResult;
						m_pstPDUMsg->xDataLen = 6;
						break;

					case 0x0203:
						/* CheckProgrammingPreconditions (ISO 14229-1 standard RID)
						 * Returns: canFlash(1=ok, 0=denied) + targetBank('A' or 'B')
						 */
						if (m_pstPDUMsg->xDataLen < 4)
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
							break;
						}
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
						routineResult = CheckProgrammingConditions();
						IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
						m_pstPDUMsg->aDataBuf[0] = 0x71;
						m_pstPDUMsg->aDataBuf[1] = 0x01;
						m_pstPDUMsg->aDataBuf[2] = 0x02;
						m_pstPDUMsg->aDataBuf[3] = 0x03;
						m_pstPDUMsg->aDataBuf[4] = (uint8) (routineResult >> 8);
						m_pstPDUMsg->aDataBuf[5] = (uint8) (routineResult & 0xFF);
						m_pstPDUMsg->xDataLen = 6;
						break;

					case 0xFF01:
						{
							/* CheckProgrammingDependencies
							 * Verifies that the target bank has valid application code.
							 */
							uint32 targetBank = Boot_DualBank_GetTargetWriteBank();
							BankStatus_t status;

							if (m_pstPDUMsg->xDataLen < 4)
							{
								SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
								break;
							}
							IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
							status = Boot_DualBank_VerifyBank(targetBank);
							IfxScuWdt_serviceCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());

							m_pstPDUMsg->aDataBuf[0] = 0x71;
							m_pstPDUMsg->aDataBuf[1] = 0x01;
							m_pstPDUMsg->aDataBuf[2] = 0xFF;
							m_pstPDUMsg->aDataBuf[3] = 0x01;
							m_pstPDUMsg->aDataBuf[4] = (status == BANK_STATUS_VALID) ? 0x01 : 0x00;
							m_pstPDUMsg->xDataLen = 5;
						}
						break;


					case 0xDFFF:
						{
							if (TRUE != IsCurSecurityLevelRequet(SECURITY_LEVEL_2))
							{
								SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
								break;
							}

							/* Expect: SID(1) + subFunc(1) + RID(2) + CRC32(4) = 8 bytes */
							if (m_pstPDUMsg->xDataLen < 8)
							{
								SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
								break;
							}

							{
								uint32 expectedCRC;
								uint32 actualCRC;
								uint32 targetBank = Boot_DualBank_GetTargetWriteBank();

								/* Parse expected CRC32 from tester (big-endian) */
								expectedCRC = ((uint32) m_pstPDUMsg->aDataBuf[4] << 24) |
									((uint32) m_pstPDUMsg->aDataBuf[5] << 16) |
									((uint32) m_pstPDUMsg->aDataBuf[6] << 8) |
									((uint32) m_pstPDUMsg->aDataBuf[7]);


								/* Use streaming CRC accumulated during 0x36 TransferData.
								 * This avoids re-reading Flash (and potential Data Cache staleness)
								 * and matches the tester-side calculation over the raw data stream. */
								if (gs_bCrcActive != FALSE)
								{
									actualCRC = gs_DownloadCRC ^ 0xFFFFFFFFu;
								}
								else
								{
									actualCRC = 0u;
								}

								if (actualCRC != expectedCRC)
								{
									/* CRC mismatch: do not mark valid */
									routineResult = 0x00;
									m_pstPDUMsg->aDataBuf[0] = 0x71;
									m_pstPDUMsg->aDataBuf[1] = 0x01;
									m_pstPDUMsg->aDataBuf[2] = 0xDF;
									m_pstPDUMsg->aDataBuf[3] = 0xFF;
									m_pstPDUMsg->aDataBuf[4] = (uint8) routineResult;
									m_pstPDUMsg->xDataLen = 5;
									gs_DownloadCRC = 0xFFFFFFFFu;
									gs_bCrcActive = FALSE;
									break;
								}

								/* CRC OK: mark valid and activate */
								g_bootPhase = BOOT_PHASE_PROG_VERIFY;
								Boot_DualBank_MarkBankValid(targetBank, 0x00010000u, Flash_GetReceivedDataLength());
								Boot_DualBank_SetActiveBank(targetBank);
								routineResult = 0x01;
								gs_DownloadCRC = 0xFFFFFFFFu;
								gs_bCrcActive = FALSE;
							}

							m_pstPDUMsg->aDataBuf[0] = 0x71;
							m_pstPDUMsg->aDataBuf[1] = 0x01;
							m_pstPDUMsg->aDataBuf[2] = 0xDF;
							m_pstPDUMsg->aDataBuf[3] = 0xFF;
							m_pstPDUMsg->aDataBuf[4] = (uint8) routineResult;
							m_pstPDUMsg->xDataLen = 5;
							break;
						}

					default:
						SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
						break;
				}
				break;
			}

		case 0x02:
			currentRoutine = m_pstPDUMsg->aDataBuf[2];
			if (currentRoutine == 0)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
				break;
			}
			if (m_pstPDUMsg->xDataLen < 3)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT, m_pstPDUMsg);
				break;
			}

			switch (currentRoutine)
			{

				case jumpToApp:
					{
						g_bootPhase = BOOT_PHASE_JUMP_DECISION;
						if (TRUE != IsCurSecurityLevelRequet(SECURITY_LEVEL_2))
						{
							SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SECURITY_ACCESS_DENIED, m_pstPDUMsg);
							break;
						}
						m_pstPDUMsg->aDataBuf[0] = 0x71;
						m_pstPDUMsg->aDataBuf[1] = 0x02;
						m_pstPDUMsg->aDataBuf[2] = jumpToApp;
						m_pstPDUMsg->xDataLen = 3;


						m_pstPDUMsg->pfUDSTxMsgServiceCallBack = &DoJumpToActiveBank;
						break;
					}
#if 0  
					* (uint16*) RAM_BOOT_MODE_Addr = RAM_BOOT_MODE_NORMAL;
					p = (uint8*) RAM_BOOT_MODE_Addr;
					m_pstPDUMsg->aDataBuf[0] = 0x71;
					m_pstPDUMsg->aDataBuf[1] = 0x02;
					m_pstPDUMsg->aDataBuf[2] = jumpToApp;
					m_pstPDUMsg->aDataBuf[3] = p[1];
					m_pstPDUMsg->aDataBuf[4] = p[2];
					m_pstPDUMsg->xDataLen = 5;

					break;
#endif
				case jumpToBL:
					*(uint16*) RAM_BOOT_MODE_Addr = RAM_BOOT_MODE_APP;
					p = (uint8*) RAM_BOOT_MODE_Addr;
					m_pstPDUMsg->aDataBuf[0] = 0x71;
					m_pstPDUMsg->aDataBuf[1] = 0x02;
					m_pstPDUMsg->aDataBuf[2] = jumpToBL;
					m_pstPDUMsg->aDataBuf[3] = p[1];
					m_pstPDUMsg->aDataBuf[4] = p[2];
					m_pstPDUMsg->xDataLen = 5;
					break;
			}
			break;

		case 0x03:
			if (currentRoutine == 0)
			{
				SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_CONDITIONS_NOT_CORRECT, m_pstPDUMsg);
				break;
			}

			m_pstPDUMsg->aDataBuf[0] = 0x71;
			m_pstPDUMsg->aDataBuf[1] = 0x03;
			m_pstPDUMsg->aDataBuf[2] = (routineResult >> 24) & 0xFF;
			m_pstPDUMsg->aDataBuf[3] = (routineResult >> 16) & 0xFF;
			m_pstPDUMsg->aDataBuf[4] = (routineResult >> 8) & 0xFF;
			m_pstPDUMsg->aDataBuf[5] = routineResult & 0xFF;
			m_pstPDUMsg->xDataLen = 6;
			break;

		default:
			SetNegativeErroCode(i_pstUDSServiceInfo->SerNum, NRC_SUBFUNCTION_NOT_SUPPORTED, m_pstPDUMsg);
			break;
	}
}



/**
 * @brief Check if a logical PFlash sector belongs to the current target bank.
 * @param sector Logical sector index (0 ~ IFXFLASH_PFLASH_NUM_LOG_SECTORS-1)
 * @return TRUE if the sector is within the target bank's allowed range
 */
static boolean IsSectorInTargetBank(uint16 sector)
{
	if (Boot_DualBank_GetTargetWriteBank() == BANK_B)
	{
		return (sector >= BANK_B_SECTOR_START) && (sector <= BANK_B_SECTOR_END);
	}
	else /* BANK_A */
	{
		return (sector >= BANK_A_SECTOR_START) && (sector <= BANK_A_SECTOR_END);
	}
}

/**
 * @brief Erase a single PFlash sector with target-bank and bootloader protection.
 * @param blockHigh High byte of sector number
 * @param blockLow  Low byte of sector number
 * @return 0x01 : success
 *         0x00 : flash erase error
 *         0xFE : sector number out of range
 *         0xFC : sector is in bootloader/reserved area (forbidden)
 *         0xFD : sector is not in the current target bank
 */
static uint16 EraseFlashSector(uint8 blockHigh, uint8 blockLow)
{
	uint16 blockNum = (blockHigh << 8) | blockLow;
	uint16 actualBlockNum = blockNum;

	/* 1. Check valid sector range */
	if (actualBlockNum >= IFXFLASH_PFLASH_NUM_LOG_SECTORS)
	{
		return 0xFE;
	}

	/* 2. Protect bootloader / reserved sectors (S0 ~ S7) */
	if (actualBlockNum <= BOOTLOADER_SECTOR_MAX)
	{
		return 0xFC;
	}

	/* 3. Auto-detect target bank from sector number if it doesn't match current target.
	 *    This resolves the issue where DFlash flags retain the previous session's
	 *    targetWriteBank (e.g. BANK_A), causing Bank B erase (S23~S26) to fail
	 *    with 0xFD (sector not in target bank).
	 *    shuaxie.py sends physical sector numbers: Bank A = 8~22, Bank B = 23~26. */
	if (!IsSectorInTargetBank(actualBlockNum))
	{
		if ((actualBlockNum >= BANK_B_SECTOR_START) && (actualBlockNum <= BANK_B_SECTOR_END))
		{
			Boot_DualBank_SetTargetWriteBank(BANK_B);
		}
		else if ((actualBlockNum >= BANK_A_SECTOR_START) && (actualBlockNum <= BANK_A_SECTOR_END))
		{
			Boot_DualBank_SetTargetWriteBank(BANK_A);
		}
		else
		{
			return 0xFD; /* Sector not in any valid bank range */
		}
	}

	/* 4. Execute erase on the actual (remapped) sector */
	if ((uint8) Flash_erasePFlash_port(IfxFlash_pFlashTableLog[actualBlockNum].start) != 0)
	{
		return 0x00;
	}

	return 0x01;
}



