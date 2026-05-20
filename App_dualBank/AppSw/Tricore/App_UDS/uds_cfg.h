/*
 * @Author: qinXiong
 * @Date: 2026-04-29 09:02:27
 * @LastEditors: Qxiong&&2307975018@qq.com
 * @LastEditTime: 2026-05-20 18:11:14
 * @Description: 
 */
/**********************************************************************************************************************
 * \file    uds_cfg.h
 * \brief
 * \version V1.0.0
 * \date    2022锟斤拷2锟斤拷10锟斤拷
 * \author  Administrator
 *********************************************************************************************************************/
#ifndef UDSDIAGNOSTIC_UDS_CFG_H_
#define UDSDIAGNOSTIC_UDS_CFG_H_
#include "uds_common.h"

//锟斤拷锟斤拷锟斤拷锟斤拷模式只锟杰达拷一锟斤拷
#define DIAGNOSTIC_MODE_FOR_APP				//APP模式
//#define DIAGNOSTIC_MODE_FOR_BOOTLOADER	//BOOTLOADER模式

/*****************************************************************************/


/*****************************************************************************/
/* DTC Configuration - ISO 14229-1 / OEM Standard */
#define DTC_CODE_MAX_NUM                  (8u)
#define DTC_AVAILABILITY_STATUS_MASK      (0x7F)

#define DTC_DID_MAX_NUM                   (8u)        /* Max supported DTC entries */
#define SANP_EEPROM_BASE_ADDR             (0xA000u)   /* Snapshot EEPROM base address */
#define SANP_RECORD_MAX_NUM               (4u)        /* Snapshot records per DTC */
#define SANP_DATA_DID_NUM                 (8u)        /* DID entries per snapshot */
#define SANP_DATA_PER_SIZE                (SANP_DATA_DID_NUM * 4)

/* OEM Standard DTC Definitions (ISO 15031-6 / ISO 14229-1)
 * Encoding: [Category][Digit][Group][Fault]
 * U=Network, P=Powertrain, B=Body, C=Chassis
 */
typedef enum
{
	/* Network Communication DTCs */
	DTC_U0100 = 0x0000C100u,   /* Lost Communication with ECM/PCM (CAN BusOff) */
	DTC_U0121 = 0x0000C121u,   /* Lost Communication with ABS (CAN Ack Error) */

	/* Powertrain DTCs */
	DTC_P0601 = 0x00000601u,   /* Internal Control Module Memory Checksum Error */
	DTC_P0605 = 0x00000605u,   /* Internal Control Module Read Only Memory Error */

	/* Body DTCs */
	DTC_B1000 = 0x00009000u,   /* ECU Boot Failure Recorded */
	DTC_B1001 = 0x00009001u,   /* ECU Software Version Mismatch */

	/* Reserved / Placeholder */
	DTC_RESERVED_1 = 0x00000000u,
	DTC_RESERVED_2 = 0x00000000u
} dtc_did_name;

/* Snapshot DID Definitions (Freeze Frame data identifiers) */
typedef enum
{
	SNAP_DID_SYS_VOLTAGE   = 0xF442u,   /* System Voltage (mV) */
	SNAP_DID_AMB_TEMP      = 0xF446u,   /* Ambient Temperature (0.1C) */
	SNAP_DID_CAN_STATUS    = 0xF501u,   /* CAN Bus Status */
	SNAP_DID_RUN_TIME      = 0xF50Au,   /* ECU Run Time (s) */
	SNAP_DID_BANK_STATUS   = 0xF510u,   /* Active Bank Status */
	SNAP_DID_BOOT_CNT      = 0xF511u,   /* Boot Attempt Counter */
	SNAP_DID_SW_VERSION    = 0xF188u,   /* Software Version */
	SNAP_DID_HW_VERSION    = 0xF193u    /* Hardware Version */
} snap_did_name;

#define VIN_F190                          "W0L00043MB541326"
#define BSID_F180                         "1.2.3.4"

/* 22/2E 鏈嶅姟 DID 瀹氫箟 */
typedef enum
{
	/* 鎸囩汗淇℃伅 */
	F15A = 0xF15Au,	/* 璇婃柇浠埛鍐欐寚绾逛俊鎭啓鍏� (Write) */
	F15B = 0xF15Bu,	/* 璇婃柇浠埛鍐欐寚绾逛俊鎭鍙� (Read) */

	/* Bootloader / System Info */
	F14A = 0xF14Au,	/* Bootloader 鐗堟湰鍙� */
	F186 = 0xF186u,	/* Bootloader鍙傝�冨彿 */

	/* OEM 淇℃伅 */
	F187 = 0xF187u,	/* OEM闆堕儴浠跺彿 */
	F188 = 0xF188u,	/* OEM杞欢鍙� */
	F189 = 0xF189u,	/* OEM杞欢鐗堟湰鍙� */
	F18A = 0xF18Au,	/* 渚涘簲鍟嗕唬鐮� */
	F18B = 0xF18Bu,	/* 渚涘簲鍟嗕骇鍝佸埗閫犳棩鏈� */
	F18C = 0xF18Cu,	/* 渚涘簲鍟嗕骇鍝佺敓浜ф祦姘村彿/鎵规鍙� */

	/* Hardware / Software Info */
	F191 = 0xF191u,	/* OEM纭欢鍙� */
	F192 = 0xF192u,	/* 渚涘簲鍟嗙‖浠跺彿 */
	F193 = 0xF193u,	/* 渚涘簲鍟嗙‖浠剁増鏈彿 */
	F194 = 0xF194u,	/* 渚涘簲鍟嗚蒋浠跺彿 */
	F195 = 0xF195u,	/* 渚涘簲鍟嗚蒋浠剁増鏈彿 */
	F197 = 0xF197u,	/* 鎺у埗鍣ㄥ悕绉�/绯荤粺鍚嶇О */

	/* Vehicle Info */
	F190 = 0xF190u, /* 鏁磋溅VIN缂栧彿 */

	/* Network Management Status */
	F520 = 0xF520u, /* AUTOSAR CanNm 鐘舵€佷俊鎭� */
}rw_data_did;

/*=============================================================================
	* DFlash DID Storage Layout
	* Sector 0 (0xAF000000): Bootloader Flags + F15A fingerprint
	* Sector 1 (0xAF002000): Static DID data (F186~F197, F15B records)
	* All text data encoded in UTF-8
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

