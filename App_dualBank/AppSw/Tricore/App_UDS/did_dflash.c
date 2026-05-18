/**********************************************************************************************************************
 * \file    did_dflash.c
 * \brief   DID DFlash Storage Manager Implementation
 * \note    All text DID data stored with UTF-8 encoding
 *********************************************************************************************************************/

#include "did_dflash.h"
#include "Flash.h"
#include "tool_class.h"

/*=============================================================================
 * Private Constants
 *===========================================================================*/

/* Magic number to verify DFlash DID area is initialized */
#define DID_DFLASH_MAGIC        0x44494421u  /* "DID!" */
#define DID_DFLASH_MAGIC_ADDR   (DFLASH_DID_BASE_ADDR + 0x01F0u)

/* Sector 1 size (8KB for TC234 DFlash) */
#define DFLASH_SECTOR1_SIZE     8192u

/*=============================================================================
 * Private Variables
 *===========================================================================*/

/* RAM backup buffer for Sector 1 (used during erase/write) */
static uint8 s_sector1Backup[DFLASH_SECTOR1_SIZE];
static uint8 s_backupValid = FALSE;

/* Default DID values (UTF-8 encoded) */
static const uint8 s_default_F186[DID_SIZE_F186] = "BOOT_V2.00";
static const uint8 s_default_F187[DID_SIZE_F187] = "CK3565_100X9014";
static const uint8 s_default_F188[DID_SIZE_F188] = "SW_IEBS_001";
static const uint8 s_default_F189[DID_SIZE_F189] = "VER_1.0.0";
static const uint8 s_default_F18A[DID_SIZE_F18A] = "S01913G2";
static const uint8 s_default_F18B[DID_SIZE_F18B] = "20260518";
static const uint8 s_default_F18C[DID_SIZE_F18C] = "BATCH_0001";
static const uint8 s_default_F190[DID_SIZE_F190] = "W0L00043MB541326";
static const uint8 s_default_F191[DID_SIZE_F191] = "HW_IEBS_001";
static const uint8 s_default_F192[DID_SIZE_F192] = "SUP_HW_001";
static const uint8 s_default_F193[DID_SIZE_F193] = "E00";
static const uint8 s_default_F194[DID_SIZE_F194] = "SUP_SW_001";
static const uint8 s_default_F195[DID_SIZE_F195] = "B00";
static const uint8 s_default_F197[DID_SIZE_F197] = "ESC";

/*=============================================================================
 * Private Functions
 *===========================================================================*/

/**
 * @brief Copy default values to DFlash Sector 1
 * @note  Sector 1 must be erased before calling this function
 */
static boolean DID_DFlash_WriteDefaults(void)
{
    uint32 i;
    uint32 pageData[2];  /* DFlash page = 8 bytes = 2 x uint32 */
    uint32 addr;

    /* Write each DID default value */
    struct {
        uint16 offset;
        const uint8* data;
        uint16 size;
    } didList[] = {
        {DFLASH_DID_F186_OFFSET, s_default_F186, DID_SIZE_F186},
        {DFLASH_DID_F187_OFFSET, s_default_F187, DID_SIZE_F187},
        {DFLASH_DID_F188_OFFSET, s_default_F188, DID_SIZE_F188},
        {DFLASH_DID_F189_OFFSET, s_default_F189, DID_SIZE_F189},
        {DFLASH_DID_F18A_OFFSET, s_default_F18A, DID_SIZE_F18A},
        {DFLASH_DID_F18B_OFFSET, s_default_F18B, DID_SIZE_F18B},
        {DFLASH_DID_F18C_OFFSET, s_default_F18C, DID_SIZE_F18C},
        {DFLASH_DID_F190_OFFSET, s_default_F190, DID_SIZE_F190},
        {DFLASH_DID_F191_OFFSET, s_default_F191, DID_SIZE_F191},
        {DFLASH_DID_F192_OFFSET, s_default_F192, DID_SIZE_F192},
        {DFLASH_DID_F193_OFFSET, s_default_F193, DID_SIZE_F193},
        {DFLASH_DID_F194_OFFSET, s_default_F194, DID_SIZE_F194},
        {DFLASH_DID_F195_OFFSET, s_default_F195, DID_SIZE_F195},
        {DFLASH_DID_F197_OFFSET, s_default_F197, DID_SIZE_F197},
    };

    for (i = 0; i < sizeof(didList) / sizeof(didList[0]); i++)
    {
        addr = DFLASH_DID_BASE_ADDR + didList[i].offset;
        /* Pad with zeros to 8-byte boundary for DFlash page write */
        uint8 padded[16] = {0};
        tl_memcpy(padded, didList[i].data, didList[i].size);

        pageData[0] = ((uint32)padded[0]) | ((uint32)padded[1] << 8) |
                      ((uint32)padded[2] << 16) | ((uint32)padded[3] << 24);
        pageData[1] = ((uint32)padded[4]) | ((uint32)padded[5] << 8) |
                      ((uint32)padded[6] << 16) | ((uint32)padded[7] << 24);

        Flash_writeDFlash_port(addr, pageData, DFLASH_PAGE_LENGTH);
    }

    /* Write magic number at end of sector */
    addr = DID_DFLASH_MAGIC_ADDR;
    pageData[0] = DID_DFLASH_MAGIC;
    pageData[1] = 0;
    Flash_writeDFlash_port(addr, pageData, DFLASH_PAGE_LENGTH);

    return TRUE;
}

/*=============================================================================
 * Public Functions
 *===========================================================================*/

void DID_DFlash_Init(void)
{
    if (!DID_DFlash_IsInitialized())
    {
        /* First boot: erase Sector 1 and write defaults */
        Flash_eraseDFlash_port(DFLASH_DID_BASE_ADDR);
        DID_DFlash_WriteDefaults();
    }
}

boolean DID_DFlash_IsInitialized(void)
{
    const uint32* pMagic = (const uint32*)DID_DFLASH_MAGIC_ADDR;
    return (*pMagic == DID_DFLASH_MAGIC);
}

boolean DID_DFlash_FactoryReset(void)
{
    Flash_eraseDFlash_port(DFLASH_DID_BASE_ADDR);
    return DID_DFlash_WriteDefaults();
}

void DID_DFlash_BackupSector1(void)
{
    const uint8* pSrc = (const uint8*)DFLASH_DID_BASE_ADDR;
    tl_memcpy(s_sector1Backup, pSrc, DFLASH_SECTOR1_SIZE);
    s_backupValid = TRUE;
}

void DID_DFlash_RestoreSector1(void)
{
    uint32 i;
    uint32 pageData[2];
    uint32 addr;

    if (!s_backupValid)
    {
        return;
    }

    Flash_eraseDFlash_port(DFLASH_DID_BASE_ADDR);

    /* Write backup data back page by page */
    for (i = 0; i < DFLASH_SECTOR1_SIZE; i += DFLASH_PAGE_LENGTH)
    {
        addr = DFLASH_DID_BASE_ADDR + i;
        pageData[0] = ((uint32)s_sector1Backup[i]) |
                      ((uint32)s_sector1Backup[i + 1] << 8) |
                      ((uint32)s_sector1Backup[i + 2] << 16) |
                      ((uint32)s_sector1Backup[i + 3] << 24);
        pageData[1] = ((uint32)s_sector1Backup[i + 4]) |
                      ((uint32)s_sector1Backup[i + 5] << 8) |
                      ((uint32)s_sector1Backup[i + 6] << 16) |
                      ((uint32)s_sector1Backup[i + 7] << 24);
        Flash_writeDFlash_port(addr, pageData, DFLASH_PAGE_LENGTH);
    }

    s_backupValid = FALSE;
}

void DID_DFlash_Read(uint16 offset, uint8* buf, uint16 length)
{
    const uint8* pSrc = (const uint8*)(DFLASH_DID_BASE_ADDR + offset);
    tl_memcpy(buf, pSrc, length);
}

boolean DID_DFlash_Write(uint16 offset, const uint8* data, uint16 length)
{
    /* Backup current sector content */
    DID_DFlash_BackupSector1();

    /* Modify backup buffer */
    tl_memcpy(&s_sector1Backup[offset], data, length);

    /* Erase and restore with modified data */
    DID_DFlash_RestoreSector1();

    return TRUE;
}

boolean DID_DFlash_WriteF15A(const uint8* fingerprint)
{
    uint8 buf[FINGERPRINT_RECORD_SIZE];
    uint32 pageData[2];
    uint32 addr;
    uint8 i;

    /* 1. Write F15A to Sector 0 (separate from Flags, at offset 0x200) */
    addr = DFLASH_F15A_FINGERPRINT_ADDR;
    tl_memset(buf, 0, FINGERPRINT_RECORD_SIZE);
    tl_memcpy(buf, fingerprint, FINGERPRINT_SIZE);

    for (i = 0; i < FINGERPRINT_RECORD_SIZE; i += DFLASH_PAGE_LENGTH)
    {
        pageData[0] = ((uint32)buf[i]) | ((uint32)buf[i + 1] << 8) |
                      ((uint32)buf[i + 2] << 16) | ((uint32)buf[i + 3] << 24);
        pageData[1] = ((uint32)buf[i + 4]) | ((uint32)buf[i + 5] << 8) |
                      ((uint32)buf[i + 6] << 16) | ((uint32)buf[i + 7] << 24);
        Flash_writeDFlash_port(addr + i, pageData, DFLASH_PAGE_LENGTH);
    }

    /* 2. Append to F15B records in Sector 1 */
    /* Read current F15B records */
    uint8 f15bBuf[FINGERPRINT_RECORD_MAX * FINGERPRINT_RECORD_SIZE];
    DID_DFlash_Read(DFLASH_DID_F15B_OFFSET, f15bBuf, sizeof(f15bBuf));

    /* Shift records (FIFO) and add new one at the beginning */
    tl_memcpy(&f15bBuf[FINGERPRINT_RECORD_SIZE], f15bBuf,
              (FINGERPRINT_RECORD_MAX - 1) * FINGERPRINT_RECORD_SIZE);
    tl_memcpy(f15bBuf, buf, FINGERPRINT_RECORD_SIZE);

    /* Write back to Sector 1 */
    DID_DFlash_Write(DFLASH_DID_F15B_OFFSET, f15bBuf, sizeof(f15bBuf));

    return TRUE;
}

uint16 DID_DFlash_ReadF15B(uint8* buf, uint16 maxLen)
{
    uint16 copyLen = FINGERPRINT_RECORD_MAX * FINGERPRINT_RECORD_SIZE;
    if (maxLen < copyLen)
    {
        copyLen = maxLen;
    }
    DID_DFlash_Read(DFLASH_DID_F15B_OFFSET, buf, copyLen);
    return copyLen;
}
