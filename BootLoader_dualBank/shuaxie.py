from ZXDoc import *
import time
import os
import struct
import zlib
import sys
# ============================================================================
# BootLoader20250714_UDS_tasking622 - ZXDoc 完整刷写脚本
# ============================================================================
# 适用场景: 基于 TC234 的 A/B 双区 Bootloader 刷写
# 刷写流程对照 ZxDoc 配置截图:
#   1. 10 01      -> 默认会话
#   2. 10 03      -> 扩展会话
#   3. 27 01/02   -> 安全访问 Level 1
#   4. 31 01 FF FD-> 检查编程条件
#   5. 85 02      -> 关闭 DTC
#   6. 10 02      -> 编程会话
#   7. 27 01/02   -> 安全访问 Level 2 (编程会话必须)
#   8. 2E F1 5A...-> 写指纹信息
#   9. 31 01 FF 00-> 逐个擦除目标 Bank sector
#  10. 34/36/37   -> 文件下载 (ZxDoc file_download API)
#  11. 31 01 DFFF -> 验证并激活 Bank
#  12. 11 01      -> ECU 复位
# ============================================================================

# ========== 用户配置区（请根据实际情况修改） ==========

# CAN 诊断 ID
PHY_ADDR = 0x74C           # ECU 物理请求地址 (RX)
TESTER_ADDR = 0x75C        # Tester 响应地址 (TX)
FUNC_ADDR = 0x7DF          # 功能地址（会话保持用）
CHANNEL = 1                # CAN 通道号

# Bank A HEX 文件路径（地址范围 0x80020000~0x800FFFFF）
HEX_FILE_A = r"E:\workFiles\IEBS\IEBSBootloader\App_dualBank\Debug\App_dualBank.hex"
# Bank B HEX 文件路径（地址范围 0x80100000~0x801FFFFF）
HEX_FILE_B = r"E:\workFiles\IEBS\IEBSBootloader\App_dualBank\debug_b\App_dualBank.hex"

# 当前使用的 HEX 文件（由脚本根据目标 Bank 动态选择）
HEX_FILE = HEX_FILE_A

# 安全访问 DLL 路径（用于 27 服务 Seed->Key 计算）
KEY_DLL = r"E:\visualStudioCode\ZcanProDll\Debug\ZcanProDll.dll"

# HEX 对齐后的输出文件路径（自动由脚本处理）
ALIGNED_HEX_FILE = r"E:\workFiles\IEBS\IEBSBootloader\tc234bootloader\App_dualBank_aligned.hex"

# ========== Bank 地址与大小（与 Bootloader LSL 保持一致） ==========
BANK_A_START_ADDR = 0x80020000
BANK_A_END_ADDR   = 0x80100000
BANK_B_START_ADDR = 0x80100000
BANK_B_END_ADDR   = 0x80200000
BANK_APP_A_SIZE   = 896 * 1024
BANK_APP_B_SIZE   = 1024 * 1024

# Bank 对应的 sector 列表（对应 IfxFlash_pFlashTableLog 索引）
BANK_SECTORS = {
    "A": list(range(8, 23)),    # S8 ~ S22
    "B": list(range(23, 27)),   # S23 ~ S26
}
alignment = 32

# 全局变量：记录手动擦除成功的 sector 列表，供 file_download 前置检查使用
_g_erased_sectors = []

TOTAL_CHECK_CMD = bytes.fromhex("31 01 DF FF")

FINGERPRINT_DATA = [
    # 2E F1 5A (SID + DID)
    0x2E, 0xF1, 0x5A,
    # 0~15: 诊断仪设备号 "TOOL1234567890AB"
    0x55, 0x4F, 0x4F, 0x4C, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x41, 0x42,
    # 16~25: 刷写前软件号 "SW_OLD_002"
    0x53, 0x57, 0x5F, 0x4F, 0x4C, 0x44, 0x5F, 0x30,
    0x30, 0x32,
    # 26~35: 刷写前版本号 "VER_OLD_02"
    0x56, 0x45, 0x52, 0x5F, 0x4F, 0x4C, 0x44, 0x5F,
    0x30, 0x32,
    # 36~45: 刷写日期 "2025052200"
    0x32, 0x30, 0x32, 0x35, 0x30, 0x35, 0x32, 0x32, 0x30, 0x30,
    # 46~55: 刷写后软件号 "SW_NEW_003"
    0x53, 0x57, 0x5F, 0x4E, 0x45, 0x57, 0x5F, 0x30,
    0x30, 0x33,
    # 56~65: 刷写后版本号 "VER_NEW_03"
    0x56, 0x45, 0x52, 0x5F, 0x4E, 0x45, 0x57, 0x5F,
    0x30, 0x33,
]


def uds_request(uds, sid, data, desc="UDS"):
    """
    发送 UDS 请求并自动检查响应状态。
    :return: ZUdsResponse 对象（成功时）；None（失败或超时）
    """
    req = ZUdsRequest(
        src_addr=PHY_ADDR,
        dst_addr=TESTER_ADDR,
        is_extend=False,
        suppress_response=False,
        sid=sid,
        data=data,
    )
    rsp = uds.request(req)
    if not rsp:
        app.log_e(f"[{desc}] ❌ No response (timeout)")
        return None
    if rsp.status != UDS_RSP_STATUS_OK:
        app.log_e(f"[{desc}] ❌ NRC 0x{rsp.status:02X}: {uds.get_error_message(rsp)}")
        return None
    app.log_i(f"[{desc}] ✅ OK, rsp: {rsp.data.hex()}")
    return rsp


def session_control(uds, session_type, desc):
    """10 服务：诊断会话控制"""
    return uds_request(uds, 0x10, [session_type], desc) is not None


def security_access(uds, level, desc):
    """
    27 服务：安全访问。
    Level 1: 扩展会话解锁；Level 2: 编程会话解锁（必须）。
    """
    sa_req = ZSecurityAccessReq(
        keyDllPath=KEY_DLL,
        srcAddr=PHY_ADDR,
        dstAddr=TESTER_ADDR,
        securityLevel=level,
        isExtend=False,
    )
    if uds.security_access(sa_req):
        app.log_i(f"[{desc}] ✅ Security Level {level} passed")
        return True
    else:
        app.log_e(f"[{desc}] ❌ Security Level {level} failed")
        return False


def write_fingerprint(uds):
    """2E F1 5A: write programming fingerprint."""
    if len(FINGERPRINT_DATA) != 69:
        app.log_e(f"[Fingerprint] ❌ Invalid fingerprint length: {len(FINGERPRINT_DATA)} bytes, expected 69")
        return False
    if FINGERPRINT_DATA[0:3] != [0x2E, 0xF1, 0x5A]:
        app.log_e("[Fingerprint] ❌ Invalid fingerprint header, expected 2E F1 5A")
        return False

    # uds_request() takes SID separately, so send DID + 66-byte fingerprint payload.
    return uds_request(uds, 0x2E, FINGERPRINT_DATA[1:], "Write Fingerprint F15A") is not None


def erase_target_bank(uds):
    """
    31 01 FF 00: 根据 TARGET_BANK 配置，逐个擦除目标 Bank 的所有 sector。
    由于 BootLoader 里每个 31 01 FF 00 只擦除单个 sector，
    因此需要循环发送（S8~S22 共 15 次，S23~S26 共 4 次）。
    """
    global _g_erased_sectors
    _g_erased_sectors.clear()

    sectors = BANK_SECTORS.get(TARGET_BANK, BANK_SECTORS[TARGET_BANK])
    app.log_i(f"[Erase] Start erasing Bank {TARGET_BANK}, sectors: {sectors}")

    for sec in sectors:
        high = (sec >> 8) & 0xFF
        low = sec & 0xFF
        rsp = uds_request(
            uds, 0x31,
            [0x01, 0xFF, 0x00, high, low],
            f"Erase S{sec}",
        )
        if not rsp:
            app.log_e(f"[Erase] Bank {TARGET_BANK} aborted at S{sec}")
            return False

        # 校验正响应: ZxDoc rsp.data 不包含 SID(0x71)，格式为:
        #   byte0: 0x01 (routineControlType)
        #   byte1: 0xFF (RID high)
        #   byte2: 0x00 (RID low)
        #   byte3: sector num
        #   byte4: result (0x01 = success)
        if len(rsp.data) >= 5 and rsp.data[0] == 0x01:
            result = rsp.data[4]
            if result == 0x01:
                _g_erased_sectors.append(sec)
                app.log_i(f"[Erase] S{sec} done, result=0x{result:02X}")
            else:
                app.log_e(f"[Erase] S{sec} returned error result=0x{result:02X}")
                return False
        else:
            app.log_w(f"[Erase] S{sec} unexpected rsp: {rsp.data.hex()}")

        # 延时 50ms，避免 CAN 总线报文过于密集
        time.sleep(0.05)

    app.log_i(f"[Erase] Bank {TARGET_BANK} all sectors erased successfully ({len(_g_erased_sectors)}/{len(sectors)})")
    return True





def file_download(uds):
    """
    34/36/37 文件下载（使用 ZXDoc 内置 file_download API）。
    注意: 由于我们在调用本函数前已经手动完成了 Flash 擦除，
    如果 ZxDoc 的 file_download 内部也尝试自动擦除，可能会因命令格式
    不兼容而失败。建议:
      1. 先测试观察 file_download 是否自动发送了 31 擦除命令；
      2. 如有冲突，将 memEraseType 改为不自动擦除的模式
         （如 ZMemEraseType.WithoutErase，具体枚举名请参考 ZXDoc 文档）。
    """
    expected_sectors = set(BANK_SECTORS.get(TARGET_BANK, BANK_SECTORS["A"]))
    actual_sectors = set(_g_erased_sectors)
    missing = expected_sectors - actual_sectors
    if missing:
        app.log_e(
            f"[Download] ❌ Erase incomplete! Missing sectors: {sorted(missing)}. "
            f"Expected {len(expected_sectors)}, actually erased {len(actual_sectors)}. "
            f"Abort file download to prevent writing to un-erased flash."
        )
        return False
    else:
        app.log_i(
            f"[Download] ✅ Erase check passed: {len(actual_sectors)}/{len(expected_sectors)} sectors erased. "
            f"Proceeding to RequestDownload (0x34)."
        )
    if not os.path.exists(HEX_FILE):
        app.log_e(f"[Download] ❌ File not found: {HEX_FILE}")
        return False

    dl_req = ZFileDownloadReq(
        filePath=HEX_FILE,
        # memEraseType=ZMemEraseType.WithParam,  # 如需禁用自动擦除请修改此处
        srcAddr=PHY_ADDR,
        dstAddr=TESTER_ADDR,
        crcAlgorithm=ZCrcAlgorithm(
            type=ZCrcType.CRC32,
            polynomial=0x04C11DB7,
            initValue=0xFFFFFFFF,
            xorOutput=0xFFFFFFFF,
            reflectInput=True,
            reflectOutput=True,
        ),
        totalCheckCmd=TOTAL_CHECK_CMD,
    )

    if uds.file_download(dl_req):
        app.log_i("[Download] ✅ File download success")
        return True
    else:
        app.log_e("[Download] ❌ File download failed")
        return False


def parse_hex_file(filepath):
    """解析 Intel HEX 文件，返回 {abs_addr: byte_value} 和起始地址"""
    data = {}
    base_addr = 0
    start_addr = None

    with open(filepath, 'r') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if line[0] != ':':
                continue

            if len(line) < 11:
                continue

            try:
                byte_count = int(line[1:3], 16)
                addr = int(line[3:7], 16)
                rec_type = int(line[7:9], 16)
            except ValueError:
                continue

            if rec_type == 0x04:  # Extended Linear Address
                base_addr = int(line[9:13], 16) << 16
            elif rec_type == 0x02:  # Extended Segment Address
                base_addr = int(line[9:13], 16) << 4
            elif rec_type == 0x00:  # Data
                abs_addr = base_addr + addr
                for i in range(byte_count):
                    try:
                        data[abs_addr + i] = int(line[9 + i*2:11 + i*2], 16)
                    except (ValueError, IndexError):
                        pass
            elif rec_type == 0x05:  # Start Linear Address
                if len(line) >= 17:
                    start_addr = int(line[9:17], 16)
            elif rec_type == 0x01:  # End of File
                break

    return data, start_addr


def group_segments(data, max_gap=32):
    """将数据地址分组为连续段（空隙 <= max_gap 的合并）"""
    if not data:
        return []

    sorted_addrs = sorted(data.keys())
    segments = []
    seg_start = sorted_addrs[0]
    seg_end = sorted_addrs[0] + 1
    prev_addr = sorted_addrs[0]

    for addr in sorted_addrs[1:]:
        if addr <= prev_addr + max_gap:
            seg_end = max(seg_end, addr + 1)
        else:
            segments.append((seg_start, seg_end))
            seg_start = addr
            seg_end = addr + 1
        prev_addr = addr

    segments.append((seg_start, seg_end))
    return segments


def write_hex_record(f, byte_count, offset, rec_type, data_bytes):
    """写入一条 Intel HEX 记录"""
    record = f"{byte_count:02X}{offset:04X}{rec_type:02X}"
    if data_bytes:
        record += data_bytes.hex().upper()

    # 计算校验和：所有字节的和取低8位，再取补码
    checksum = byte_count + ((offset >> 8) & 0xFF) + (offset & 0xFF) + rec_type
    for b in data_bytes:
        checksum += b
    checksum = ((-checksum) & 0xFF)

    record += f"{checksum:02X}"
    f.write(f":{record}\n")


def align_hex_file(input_path, output_path, align=32, fill_byte=0x00):
    """
    对齐 Intel HEX 文件（统一 HEX 专用）。

    Args:
        input_path:  输入 HEX 文件路径
        output_path: 输出 HEX 文件路径
        align:       对齐边界（默认 32 字节，TC234 PFlash page size）
        fill_byte:   填充字节（默认 0x00，AURIX PFlash 擦除状态）
    """
    data, start_addr = parse_hex_file(input_path)

    if not data:
        print(f"[align_hex] Error: No data found in {input_path}")
        return False

    # Validate: ensure all data is within valid Bank address range
    for addr in data.keys():
        if not ((BANK_A_START_ADDR <= addr < BANK_A_END_ADDR) or
                (BANK_B_START_ADDR <= addr < BANK_B_END_ADDR)):
            print(f"[align_hex] Error: Address 0x{addr:08X} is outside valid Bank range")
            print(f"[align_hex]        Valid ranges: 0x{BANK_A_START_ADDR:08X}~0x{BANK_A_END_ADDR:08X} "
                  f"or 0x{BANK_B_START_ADDR:08X}~0x{BANK_B_END_ADDR:08X}")
            return False

    segments = group_segments(data, max_gap=align)

    with open(output_path, 'w') as f:
        current_base = -1

        for seg_start, seg_end in segments:
            aligned_start = seg_start & ~(align - 1)
            aligned_end = (seg_end + align - 1) & ~(align - 1)

            addr = aligned_start
            while addr < aligned_end:
                # 检查是否需要发送扩展线性地址记录
                base = addr >> 16
                if base != current_base:
                    current_base = base
                    write_hex_record(f, 2, 0x0000, 0x04,
                                     bytes([base >> 8, base & 0xFF]))

                offset = addr & 0xFFFF
                line_data = bytearray()
                for i in range(align):
                    line_data.append(data.get(addr + i, fill_byte))

                write_hex_record(f, align, offset, 0x00, bytes(line_data))
                addr += align

        # 起始地址记录（如果有）
        if start_addr is not None:
            write_hex_record(f, 4, 0x0000, 0x05,
                             bytes([(start_addr >> 24) & 0xFF,
                                    (start_addr >> 16) & 0xFF,
                                    (start_addr >> 8) & 0xFF,
                                    start_addr & 0xFF]))

        # 结束记录
        write_hex_record(f, 0, 0x0000, 0x01, b'')

    # 统计信息
    total_data = len(data)
    aligned_total = sum(
        ((seg_end + align - 1) & ~(align - 1)) - (seg_start & ~(align - 1))
        for seg_start, seg_end in segments
    )
    added = aligned_total - total_data

    print(f"[align_hex] Aligned HEX file saved to: {output_path}")
    print(f"[align_hex]   Original data bytes: {total_data}")
    print(f"[align_hex]   Aligned total bytes: {aligned_total}")
    print(f"[align_hex]   Added fill bytes:    {added} ({added/total_data*100:.1f}%)")
    print(f"[align_hex]   Alignment:           {align} bytes")
    print(f"[align_hex]   Fill byte:           0x{fill_byte:02X}")

    return True


def do_flash_process():
    """完整刷写主流程"""
    uds = ZDoCanInterface(
        channelIndex=CHANNEL,
        cfg=ZUdsCANCfg(
            frameType=CAN_FRAME_TYPE_CAN,
            protocolVersion=CAN_TP_ISO15765_2_2016,
            fillByte=0xAA,
            isFillByte=True,
            p2Timeout=2000,       # P2 超时 2s（单帧响应）
            p2xTimeout=5000,      # P2* 超时 5s（多帧/长操作）
            isReplaceEcuSTmin=False,
            remoteSTmin=0,
            localSTmin=0,
            blockSize=0,          # 0 = 不限制 BlockSize
            fcTimeout=1000,
        ),
    )

    if uds.handle() < 0:
        app.log_e("[Init] ❌ Failed to create UDS interface")
        return

    # 启动会话保持（功能地址 0x7DF，周期 5000ms）
    uds.start_session_keep(FUNC_ADDR, 5000, True)

    try:
        # ------------------------------------------------------------
        # 1. 默认会话 10 01
        # ------------------------------------------------------------
        if not session_control(uds, 0x01, "Default Session"):
            return

        # ------------------------------------------------------------
        # 2. 扩展会话 10 03
        # ------------------------------------------------------------
        if not session_control(uds, 0x03, "Extended Session"):
            return

        # ------------------------------------------------------------
        # 3. 安全访问 Level 1（扩展会话下解锁）
        # ------------------------------------------------------------
        if not security_access(uds, 1, "SA L1"):
            return

        # ------------------------------------------------------------
        # 4. 检查编程条件 31 01 DF FD
        #    正响应: 71 01 FF FD <canFlash> <targetBank>
        #      canFlash  : 1=能刷写, 0=不能
        #      targetBank: 'A' 或 'B'（推荐刷写的 Bank）
        # ------------------------------------------------------------
        global TARGET_BANK
        global HEX_FILE

        # ------------------------------------------------------------
        # 4a. Check programming conditions (legacy RID 0xFFFD)
        # ------------------------------------------------------------
        rsp = uds_request(uds, 0x31, [0x01, 0xFF, 0xFD], "Check Programming")
        if not rsp:
            return
        if len(rsp.data) >= 4:
            can_flash = rsp.data[3]
            app.log_i(f"[Check] Bootloader reports: canFlash={can_flash}")
            if can_flash != 1:
                app.log_e(f"[Check] ❌ Programming conditions NOT OK (canFlash={can_flash}), abort flash!")
                return

        # ------------------------------------------------------------
        # 4b. Read target bank via DID AFFF
        #      Response: [AF, FF, targetBankChar]
        #      targetBankChar: 0x0A = Bank A, 0x0B = Bank B
        # ------------------------------------------------------------
        rsp = uds_request(uds, 0x22, [0xAF, 0xFF], "Read Target Bank (AFFF)")
        if not rsp:
            return
        if len(rsp.data) >= 3:
            target_bank_char = rsp.data[2]
            if target_bank_char == 0x0A:
                TARGET_BANK = 'A'
                HEX_FILE = HEX_FILE_A
            elif target_bank_char == 0x0B:
                TARGET_BANK = 'B'
                HEX_FILE = HEX_FILE_B
            else:
                app.log_w(f"[Check] ⚠️ Unexpected targetBank char=0x{rsp.data[2]:02X}, keep default={TARGET_BANK}")
                return
        else:
            app.log_e("[Check] ❌ Invalid response length for AFFF")
            return

        app.log_i(f"[Check] ✅ Target Bank = {TARGET_BANK}, HEX file = {HEX_FILE}")

        # ------------------------------------------------------------
        # 4c. Align HEX file for the selected bank
        # ------------------------------------------------------------
        input_file = HEX_FILE
        global alignment
        if not os.path.exists(input_file):
            app.log_e(f"[Check] ❌ Input file not found: {input_file}")
            sys.exit(1)

        success = align_hex_file(input_file, ALIGNED_HEX_FILE, align=alignment, fill_byte=0x00)
        if not success:
            app.log_e("[Check] ❌ Failed to align HEX file")
            sys.exit(1)

        HEX_FILE = ALIGNED_HEX_FILE
        # ------------------------------------------------------------
        # 5. 关闭通信 28 03 03 (Disable Rx/Tx)
        #    减少刷写过程中总线负载，防止应用报文干扰
        # ------------------------------------------------------------
        # if not uds_request(uds, 0x28, [0x03, 0x03], "Disable Communication"):
        #     return

        # ------------------------------------------------------------
        # 6. 关闭 DTC 85 02
        # ------------------------------------------------------------
        # if not uds_request(uds, 0x85, [0x02], "Disable DTC"):
        #     return

        # ------------------------------------------------------------
        # 7. 编程会话 10 02
        # ------------------------------------------------------------
        if not session_control(uds, 0x02, "Programming Session"):
            return

        # ------------------------------------------------------------
        # 7. 安全访问 Level 2（编程会话必须解锁 Level 2）
        #    BootLoader 中 31 01 FF 00 / 31 01 DFFF 都要求 SECURITY_LEVEL_2
        # ------------------------------------------------------------
        if not security_access(uds, 3, "SA L2"):
            return

        # ------------------------------------------------------------
        # 8. 预编程条件检查 31 01 02 03 (ISO 14229-1 标准 RID)
        #    正响应: 71 01 02 03 <canFlash> <targetBank>
        # ------------------------------------------------------------
        # rsp = uds_request(uds, 0x31, [0x01, 0x02, 0x03], "Check Preconditions")
        # if not rsp:
        #     return
        # if len(rsp.data) >= 5:
        #     can_flash = rsp.data[3]
        #     if can_flash != 1:
        #         app.log_e(f"[Precond] ❌ Preconditions NOT OK (canFlash={can_flash}), abort!")
        #         return
        #     app.log_i("[Precond] ✅ Preconditions OK")

        # ------------------------------------------------------------
        # 9. 写指纹信息 2E F1 5A + 66 bytes
        # ------------------------------------------------------------
        if not write_fingerprint(uds):
            return

        # ------------------------------------------------------------
        # 10. 擦除目标 Bank (31 01 FF 00)
        #    逐个 sector 擦除，每次只擦 1 个 sector，单次耗时 < 1s，
        #    不会触发 P2 超时。
        # ------------------------------------------------------------
        if not erase_target_bank(uds):
            app.log_e("[Flash] Erase failed, abort download!")
            return

        # ------------------------------------------------------------
        # 11. 文件下载 (34/36/37，不自动发送 totalCheckCmd)
        # ------------------------------------------------------------
        if not file_download(uds):
            return

        # ------------------------------------------------------------
        # 12. 检查编程依赖性 31 01 FF 01
        #     验证目标 Bank 是否已标记有效
        # ------------------------------------------------------------
        # rsp = uds_request(uds, 0x31, [0x01, 0xDF, 0xFF], "Check crc")
        # if not rsp:
        #     return
        # rsp = uds_request(uds, 0x31, [0x01, 0xFF, 0x01], "Check Dependencies")
        # if not rsp:
        #     app.log_w("[Post] ⚠️ CheckProgrammingDependencies no response, continue...")
        # elif len(rsp.data) >= 4 and rsp.data[3] == 0x01:
        #     app.log_i("[Post] ✅ Programming dependencies OK")
        # else:
        #     app.log_w("[Post] ⚠️ Programming dependencies may have issues, continue...")
        
                # ------------------------------------------------------------
        # 14. ECU 复位 11 01 (SoftReset)
        # ------------------------------------------------------------
        uds_request(uds, 0x11, [0x01], "ECU Reset")

        # ------------------------------------------------------------
        # 13. 后编程阶段 - 恢复系统正常工作状态
        # ------------------------------------------------------------
        time.sleep(1)
        app.log_i("[Post] ====== Post-Programming Phase ======")

        # 13.1 切换到扩展会话 10 03
        if not session_control(uds, 0x03, "Extended Session (Post)"):
            app.log_w("[Post] ⚠️ Failed to enter Extended Session, continue...")
        
        if not security_access(uds, 1, "SA L1"):
            return
        # 13.2 恢复通信 28 00 03 (Enable Rx/Tx)
        if not uds_request(uds, 0x28, [0x00, 0x03], "Enable Communication"):
            app.log_w("[Post] ⚠️ Failed to enable communication, continue...")

        # 13.3 恢复 DTC 85 01
        if not uds_request(uds, 0x85, [0x01], "Enable DTC"):
            app.log_w("[Post] ⚠️ Failed to enable DTC, continue...")

        # 13.4 清除所有 DTC 14 FF FF FF
        if not uds_request(uds, 0x14, [0xFF, 0xFF, 0xFF], "Clear All DTC"):
            app.log_w("[Post] ⚠️ Failed to clear DTC, continue...")

        # 13.5 回到默认会话 10 01
        if not session_control(uds, 0x01, "Default Session (Post)"):
            app.log_w("[Post] ⚠️ Failed to enter Default Session, continue...")


    finally:
        uds.stop_session_keep()


# ============================================================================
# ZXDoc 脚本入口
# ============================================================================
def __zxdoc_main__():
    


    if not measurement.is_started():
        measurement.start()
    
    do_flash_process()

    # 保持测量运行一段时间，便于观察最后几帧报文
    time.sleep(2)


def __zxdoc_on_exit__():
    measurement.stop()
