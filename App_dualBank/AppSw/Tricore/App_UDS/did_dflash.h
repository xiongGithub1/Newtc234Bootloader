/**********************************************************************************************************************
 * \file    did_dflash.h
 * \brief   DID DFlash Storage Manager - Provides persistent storage for UDS DID data
 * \note    All DID data stored in DFlash with UTF-8 encoding
 *          Sector 0 (0xAF000000): Bootloader Flags + F15A fingerprint
 *          Sector 1 (0xAF002000): Static DID data (F186~F197, F15B records)
 *********************************************************************************************************************/
#ifndef DID_DFLASH_H_
#define DID_DFLASH_H_

#include "Platform_Types.h"
#include "uds_cfg.h"

/*=============================================================================
 * DFlash DID Address Macros (direct pointer access, read-only)
 * Use these macros to read DID data directly from DFlash
 *===========================================================================*/

/* Static DID read pointers (Sector 1) */
#define DFLASH_PTR_F186     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F186_OFFSET))
#define DFLASH_PTR_F187     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F187_OFFSET))
#define DFLASH_PTR_F188     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F188_OFFSET))
#define DFLASH_PTR_F189     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F189_OFFSET))
#define DFLASH_PTR_F18A     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F18A_OFFSET))
#define DFLASH_PTR_F18B     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F18B_OFFSET))
#define DFLASH_PTR_F18C     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F18C_OFFSET))
#define DFLASH_PTR_F190     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F190_OFFSET))
#define DFLASH_PTR_F191     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F191_OFFSET))
#define DFLASH_PTR_F192     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F192_OFFSET))
#define DFLASH_PTR_F193     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F193_OFFSET))
#define DFLASH_PTR_F194     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F194_OFFSET))
#define DFLASH_PTR_F195     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F195_OFFSET))
#define DFLASH_PTR_F197     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F197_OFFSET))

/* F15A fingerprint write buffer (RAM) + F15B read records (DFlash) */
#define DFLASH_PTR_F15A     ((const uint8*)DFLASH_F15A_FINGERPRINT_ADDR)
#define DFLASH_PTR_F15B     ((const uint8*)(DFLASH_DID_BASE_ADDR + DFLASH_DID_F15B_OFFSET))

/*=============================================================================
 * Public API
 *===========================================================================*/

/**
 * @brief Initialize DID DFlash storage
 * @note  Called at system startup. Checks if DFlash DID area is initialized.
 *        If not (first boot), writes default values to DFlash.
 */
void DID_DFlash_Init(void);

/**
 * @brief Write DID data to DFlash (Sector 1)
 * @param offset  Offset within Sector 1 (use DFLASH_DID_Fxxx_OFFSET)
 * @param data    Pointer to data buffer (UTF-8 encoded)
 * @param length  Data length in bytes
 * @return TRUE if successful, FALSE otherwise
 * @note  This function handles erase+write of the entire Sector 1
 *        Call DID_DFlash_BackupSector1() before if you need to preserve other DID
 */
boolean DID_DFlash_Write(uint16 offset, const uint8* data, uint16 length);

/**
 * @brief Read DID data from DFlash
 * @param offset  Offset within Sector 1
 * @param buf     Output buffer
 * @param length  Number of bytes to read
 */
void DID_DFlash_Read(uint16 offset, uint8* buf, uint16 length);

/**
 * @brief Write F15A fingerprint to DFlash (Sector 0)
 * @param fingerprint  66-byte fingerprint data
 * @return TRUE if successful
 * @note  Also updates F15B read records in Sector 1
 */
boolean DID_DFlash_WriteF15A(const uint8* fingerprint);

/**
 * @brief Read F15B fingerprint records from DFlash
 * @param buf     Output buffer (must be large enough for all records)
 * @param maxLen  Maximum bytes to read
 * @return Actual bytes read
 */
uint16 DID_DFlash_ReadF15B(uint8* buf, uint16 maxLen);

/**
 * @brief Backup Sector 1 (static DID data) to RAM buffer
 * @note  Call before erasing/writing Sector 1
 */
void DID_DFlash_BackupSector1(void);

/**
 * @brief Restore Sector 1 from RAM backup
 * @note  Call after DID_DFlash_BackupSector1() to restore original data
 */
void DID_DFlash_RestoreSector1(void);

/**
 * @brief Check if DFlash DID area has been initialized
 * @return TRUE if initialized (valid magic found)
 */
boolean DID_DFlash_IsInitialized(void);

/**
 * @brief Erase and initialize DFlash DID area with default values
 * @return TRUE if successful
 */
boolean DID_DFlash_FactoryReset(void);

#endif /* DID_DFLASH_H_ */
