/*
 * @Author: qinXiong
 * @Date: 2026-04-29 09:02:27
 * @LastEditors: xiongGithub1&&qx20001119@163.com
 * @LastEditTime: 2026-05-19 00:43:01
 * @Description: 
 */
/**********************************************************************************************************************
 * \file    uds_cfg.h
 * \brief
 * \version V1.0.0
 * \date    2022??2??10??
 * \author  Administrator
 *********************************************************************************************************************/
#ifndef UDSDIAGNOSTIC_UDS_CFG_H_
#define UDSDIAGNOSTIC_UDS_CFG_H_
#include "uds_common.h"

//?????????????????
////#define DIAGNOSTIC_MODE_FOR_APP				//APP??
#define DIAGNOSTIC_MODE_FOR_BOOTLOADER	//BOOTLOADER??

/*****************************************************************************/


/*****************************************************************************/
/* DTC Configuration - ISO 14229-1 / OEM Standard */
#define DTC_CODE_MAX_NUM                  (8u)
#define DTC_AVAILABILITY_STATUS_MASK      (0x7F)

#define DTC_DID_MAX_NUM                   (8u)
#define SANP_EEPROM_BASE_ADDR             (0xA000u)
#define SANP_RECORD_MAX_NUM               (4u)
#define SANP_DATA_DID_NUM                 (8u)
#define SANP_DATA_PER_SIZE                (SANP_DATA_DID_NUM * 4)

typedef enum
{
	/* Network Communication DTCs */
	DTC_U0100 = 0x0000C100u,
	DTC_U0121 = 0x0000C121u,

	/* Powertrain DTCs */
	DTC_P0601 = 0x00000601u,
	DTC_P0605 = 0x00000605u,

	/* Body DTCs */
	DTC_B1000 = 0x00009000u,
	DTC_B1001 = 0x00009001u,

	DTC_RESERVED_1 = 0x00000000u,
	DTC_RESERVED_2 = 0x00000000u
} dtc_did_name;

typedef enum
{
	SNAP_DID_SYS_VOLTAGE   = 0xF442u,
	SNAP_DID_AMB_TEMP      = 0xF446u,
	SNAP_DID_CAN_STATUS    = 0xF501u,
	SNAP_DID_RUN_TIME      = 0xF50Au,
	SNAP_DID_BANK_STATUS   = 0xF510u,
	SNAP_DID_BOOT_CNT      = 0xF511u,
	SNAP_DID_SW_VERSION    = 0xF188u,
	SNAP_DID_HW_VERSION    = 0xF193u
} snap_did_name;

/* 22/2E ???? DID ???? */
typedef enum
{
	/* ?????? */
	F15A = 0xF15Au,	/* ??????д??????д?? (Write) */
	F15B = 0xF15Bu,	/* ??????д????????? (Read) */

	/* Bootloader / System Info */
	F14A = 0xF14Au,	/* Bootloader ?汾?? */
	F186 = 0xF186u,	/* Bootloader?ο??? */

	/* OEM ??? */
	F187 = 0xF187u,	/* OEM?????? */
	F188 = 0xF188u,	/* OEM?????? */
	F189 = 0xF189u,	/* OEM?????汾?? */
	F18A = 0xF18Au,	/* ???????? */
	F18B = 0xF18Bu,	/* ??????????????? */
	F18C = 0xF18Cu,	/* ????????????????/???κ? */

	/* Hardware / Software Info */
	F191 = 0xF191u,	/* OEM????? */
	F192 = 0xF192u,	/* ?????????? */
	F193 = 0xF193u,	/* ?????????汾?? */
	F194 = 0xF194u,	/* ??????????? */
	F195 = 0xF195u,	/* ??????????汾?? */
	F197 = 0xF197u,	/* ??????????/?????? */

	/* Vehicle Info */
	F190 = 0xF190u, /* ????VIN??? */
}rw_data_did;

#define VIN_F190                          "W0L00043MB541326"
#define BSID_F180                         "1.2.3.4"

/*=============================================================================
 * DFlash DID 存储区域定义
 * TC234 DFlash Sector 大小 = 8KB
 * Sector 0: 0xAF000000 ~ 0xAF001FFF (Bootloader Flags + F15A fingerprint)
 * Sector 1: 0xAF002000 ~ 0xAF003FFF (F186~F197 static DID data)
 *===========================================================================*/

/* Sector 0: Flags + Fingerprint */
#define DFLASH_FLAG_ADDR                0xAF000000u
#define DFLASH_F15A_FINGERPRINT_ADDR    0xAF000200u  /* 256 bytes offset from flag */
#define DFLASH_F15B_RECORD_ADDR         0xAF000300u  /* F15B read records */

/* Sector 1: Static DID data (F186~F197) */
#define DFLASH_DID_BASE_ADDR            0xAF002000u

/* DID offset within Sector 1 (each aligned to 16 bytes) */
#define DFLASH_DID_F186_OFFSET          0x0000u  /* Bootloader参考号 (16B) */
#define DFLASH_DID_F187_OFFSET          0x0010u  /* OEM零部件号 (16B) */
#define DFLASH_DID_F188_OFFSET          0x0020u  /* OEM软件号 (16B) */
#define DFLASH_DID_F189_OFFSET          0x0030u  /* OEM软件版本号 (16B) */
#define DFLASH_DID_F18A_OFFSET          0x0040u  /* 供应商代码 (16B) */
#define DFLASH_DID_F18B_OFFSET          0x0050u  /* 制造日期 (16B) */
#define DFLASH_DID_F18C_OFFSET          0x0060u  /* 批次号 (16B) */
#define DFLASH_DID_F190_OFFSET          0x0070u  /* VIN (32B) */
#define DFLASH_DID_F191_OFFSET          0x0090u  /* OEM硬件号 (16B) */
#define DFLASH_DID_F192_OFFSET          0x00A0u  /* 供应商硬件号 (16B) */
#define DFLASH_DID_F193_OFFSET          0x00B0u  /* 硬件版本号 (16B) */
#define DFLASH_DID_F194_OFFSET          0x00C0u  /* 供应商软件号 (16B) */
#define DFLASH_DID_F195_OFFSET          0x00D0u  /* 软件版本号 (16B) */
#define DFLASH_DID_F197_OFFSET          0x00E0u  /* 控制器名称 (16B) */
#define DFLASH_DID_RESERVED_OFFSET      0x00F0u  /* Reserved (16B) */

#define DFLASH_DID_F15B_OFFSET          0x0100u  /* F15B fingerprint records */

/* DID size definitions */
#define DID_SIZE_F186                   10u
#define DID_SIZE_F187                   16u
#define DID_SIZE_F188                   16u
#define DID_SIZE_F189                   10u
#define DID_SIZE_F18A                   10u
#define DID_SIZE_F18B                   8u
#define DID_SIZE_F18C                   10u
#define DID_SIZE_F190                   17u
#define DID_SIZE_F191                   15u
#define DID_SIZE_F192                   14u
#define DID_SIZE_F193                   8u
#define DID_SIZE_F194                   14u
#define DID_SIZE_F195                   8u
#define DID_SIZE_F197                   8u

/* F15A/F15B fingerprint */
#define FINGERPRINT_SIZE                66u
#define FINGERPRINT_RECORD_SIZE         67u
#define FINGERPRINT_RECORD_MAX          3u

#endif /* UDSDIAGNOSTIC_UDS_CFG_H_ */

