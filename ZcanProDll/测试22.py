from ZXDoc import *
import time
import ctypes

# ========================================================================
# UDS Security Access Level 1 Key Calculation (via DLL)
# DLL Path: E:\visualStudioCode\ZcanProDll\Debug\ZcanProDll.dll
# Export: ComputeKeyLevel1(seed[4], key[4])
# ========================================================================
KEY_DLL = r"E:\visualStudioCode\ZcanProDll\Debug\ZcanProDll.dll"
# CAN 诊断 ID
PHY_ADDR = 0x74C           # ECU 物理请求地址 (RX)
TESTER_ADDR = 0x75C        # Tester 响应地址 (TX)
FUNC_ADDR = 0x7DF          # 功能地址（会话保持用）
CHANNEL = 1                # CAN 通道号

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


# ========================================================================
# 2E F1 5A - Write Fingerprint (66 bytes payload)
# Layout: 0~15:  诊断仪设备号(TOOL1234567890AB)
#         16~25: 刷写前软件号(SW_OLD_001)
#         26~35: 刷写前软件版本号(VER_OLD_01)
#         36~45: 刷写日期(2025051900)
#         46~55: 刷写后软件号(SW_NEW_002)
#         56~65: 刷写后软件版本号(VER_NEW_02)
# ========================================================================
# Total UDS data = 2E F1 5A + 66 bytes = 69 bytes
FINGERPRINT_DATA = [
    # 2E F1 5A (SID + DID)
    0x2E, 0xF1, 0x5A,
    # 0~15: 诊断仪设备号 "TOOL1234567890AB"
    0x55, 0x4F, 0x4F, 0x4C, 0x31, 0x32, 0x33, 0x34,
    0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x41, 0x42,
    # 16~25: 刷写前软件号 "SW_OLD_001"
    0x53, 0x57, 0x5F, 0x4F, 0x4C, 0x44, 0x5F, 0x30,
    0x30, 0x31,
    # 26~35: 刷写前版本号 "VER_OLD_01"
    0x56, 0x45, 0x52, 0x5F, 0x4F, 0x4C, 0x44, 0x5F,
    0x30, 0x31,
    # 36~45: 刷写日期 "2025051900"
    0x32, 0x30, 0x32, 0x35, 0x30, 0x35, 0x31, 0x39,
    0x30, 0x30,
    # 46~55: 刷写后软件号 "SW_NEW_002"
    0x53, 0x57, 0x5F, 0x4E, 0x45, 0x57, 0x5F, 0x30,
    0x30, 0x32,
    # 56~65: 刷写后版本号 "VER_NEW_02"
    0x56, 0x45, 0x52, 0x5F, 0x4E, 0x45, 0x57, 0x5F,
    0x30, 0x32,
]


def build_isotp_multiframe(data):
    """Build ISO-TP FF + CF frames from data bytes"""
    total_len = len(data)
    frames = []

    # First Frame (FF)
    ff_data = [0x10 | ((total_len >> 8) & 0x0F), total_len & 0xFF]
    ff_data.extend(data[:6])
    frames.append(ff_data)

    # Consecutive Frames (CF)
    seq = 1
    idx = 6
    while idx < total_len:
        cf_data = [0x20 | seq]
        remain = total_len - idx
        cf_data.extend(data[idx:idx + 7])
        # Pad to 8 bytes if needed
        while len(cf_data) < 8:
            cf_data.append(0x00)
        frames.append(cf_data)
        seq = (seq + 1) & 0x0F
        idx += 7

    return frames


def send_can_frame(data_bytes, can_id=0x74C):
    """Send a single CAN frame via ZXDoc"""
    return channel.transmit(
        ZBusType.CAN,
        1,
        [ZCANFDData(
            can_id=can_id,
            is_canfd=0,
            is_brs=0,
            is_extend=0,
            is_remote=0,
            data=data_bytes,
        )],
    )


def transmit_demo():
    app.write_log(LOG_LVL_INFO, "========================================")
    app.write_log(LOG_LVL_INFO, "UDS Test: 2E F1 5A Write Fingerprint")
    app.write_log(LOG_LVL_INFO, "========================================")
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
    # ------------------------------------------------------------------
    # Step 1: Send Diagnostic Session Control (10 03) - Extend Session
    # ------------------------------------------------------------------
    rsp = uds_request(uds, 0x10, [0x03], "Sending 10 03 (extend Session)")
    if not rsp:
        return

    # ------------------------------------------------------------------
    # Step 2: Security Access - Request Seed (27 01)
    # ------------------------------------------------------------------
    app.write_log(LOG_LVL_INFO, "[Step 2] Sending 27 01 (Request Seed)")
    security_access(uds, 1, "SA L1")
    time.sleep(0.1)

    # ------------------------------------------------------------------
    # Step 3: Security Access - Send Key (27 02)
    # NOTE: In real scenario, parse 67 01 response to get Seed.
    # Here we use a pre-calculated example. If Seed changes, recalc Key.
    # Example: Seed = [0x12, 0x34, 0x56, 0x78] -> Key = [0x66, 0x0A, 0x7A, 0x6A]
    # ------------------------------------------------------------------
    # app.write_log(LOG_LVL_INFO, "[Step 1] Sending 10 02 (extend Session)")
    # send_can_frame([0x03, 0x22, 0xF1, 0x86, 0x00, 0x00, 0x00, 0x00])
    # time.sleep(0.5)
    rsp = uds_request(uds, 0x22, [0xF1, 0x86], "Sending 22 f1 86")
    if not rsp:
        return
    
    rsp = uds_request(uds, 0x22, [0xF1, 0x87], "Sending 22 f1 87")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x88], "Sending 22 f1 88")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x89], "Sending 22 f1 89")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x8A], "Sending 22 f1 8A")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x8B], "Sending 22 f1 8B")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x8C], "Sending 22 f1 8C")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x90], "Sending 22 f1 90")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x91], "Sending 22 f1 91")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x92], "Sending 22 f1 92")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x93], "Sending 22 f1 93")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x94], "Sending 22 f1 94")
    if not rsp:
        return

    rsp = uds_request(uds, 0x22, [0xF1, 0x95], "Sending 22 f1 95")
    if not rsp:
        return


    rsp = uds_request(uds, 0x22, [0xF1, 0x97], "Sending 22 f1 97")
    if not rsp:
        return
    


# ========================================================================
# ZXDoc Callbacks
# ========================================================================
def on_meas_stat_changed(stat):
    print(
        f'measurement { "started" if MEASUREMENT_STATUS_STARTED == stat else "stoped"}'
    )


def on_data(dataSet):
    for rawData in dataSet:
        direction = "Rx"
        if 1 == rawData.data.direction:
            direction = "Tx"
        app.write_log(
            LOG_LVL_DEBUG,
            f'[{rawData.channel}][{direction}]: {rawData.data.data.hex(" ").upper()}',
        )


def __zxdoc_main__():
   

    if not measurement.is_started():
        measurement.start()

  
    transmit_demo()


def __zxdoc_on_exit__():
    measurement.stop()

