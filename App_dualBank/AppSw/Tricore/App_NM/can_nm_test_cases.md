# AUTOSAR CAN 网络管理 (CanNm) 测试用例

> 适用于 App_dualBank / TC234 / CanNm V1.0.0
> 测试工具: CANoe / ZCANPRO / 示波器

---

## 1. 测试环境准备

### 1.1 硬件环境
| 项目 | 要求 |
|------|------|
| ECU | App_dualBank 目标板 |
| CAN 分析仪 | CANoe 16.x / ZCANPRO |
| 示波器 | 可选，用于精确测量报文周期 |
| 电源 | 稳压电源 12V |

### 1.2 软件配置
| 参数 | 配置值 | 说明 |
|------|--------|------|
| NM CAN ID | `0x500` | 网络管理报文 ID |
| Node ID | `0x01` | 本节点标识 |
| NmTimeoutTime | 2000 ms | 等待其他节点 NM 超时 |
| RepeatMessageTime | 1600 ms | 重复消息状态持续时间 |
| NmMessageCycleTime | 20 ms | 正常 NM 报文周期 |
| WaitBusSleepTime | 4000 ms | 准备睡眠等待时间 |
| Immediate Cycle Time | 10 ms | 快速发送周期 |
| Immediate NMTx Count | 5 | 快速发送帧数 |

### 1.3 测试前置条件
1. ECU 上电，程序正常运行
2. CAN 总线连接正常，波特率 500kbps
3. CAN 分析仪能正常收发报文
4. 清除 DTC，复位 ECU

---

## 2. 功能测试用例

### TC-001: 上电初始化与默认状态检查

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 ECU 上电后 CanNm 正确初始化，处于 Bus-Sleep 状态 |
| **测试步骤** | 1. 给 ECU 上电<br>2. 等待 100 ms<br>3. 通过诊断读取 NM 状态（或断点查看 `gs_CanNmState`） |
| **期望结果** | 1. `CanNm_Init()` 被调用<br>2. `gs_CanNmState == CANNM_STATE_BUS_SLEEP`<br>3. `gs_CanNmMode == CANNM_MODE_BUS_SLEEP`<br>4. 总线上**无** `0x500` NM 报文发送 |
| **判定标准** | 示波器/CANoe Trace 中 0x500 报文计数 = 0 |
| **优先级** | P1 |

---

### TC-002: 本地网络请求 — Bus-Sleep -> Repeat Message 转换

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证调用 `CanNm_NetworkRequest()` 后状态正确转换 |
| **测试步骤** | 1. ECU 上电，确认处于 Bus-Sleep<br>2. 触发本地网络请求（如打开点火开关/应用层调用 `CanNm_NetworkRequest()`）<br>3. 观察 CAN 总线 200 ms 内报文 |
| **期望结果** | 1. 状态从 `BUS_SLEEP` 进入 `REPEAT_MESSAGE`<br>2. 首次 NM 报文在 `T_START_NM_TX + T_START_APPEND`（约 30ms）后发出<br>3. 随后连续发送 5 帧快速 NM（周期 10ms）<br>4. CBV 中 Active Wakeup Bit (bit 4) = 1 |
| **判定标准** | CANoe Trace 显示:<br>- `0x500` 报文 Data[0] = `0x01` (Node ID)<br>- Data[1] bit 4 = 1 (Active Wakeup)<br>- 前 5 帧间隔 ~= 10ms，之后间隔 ~= 20ms |
| **优先级** | P1 |

---

### TC-003: Repeat Message -> Normal Operation 状态转换

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 RepeatMessageTime (1600ms) 超时后进入 Normal Operation |
| **测试步骤** | 1. 完成 TC-002，确认 ECU 在 Repeat Message 状态<br>2. 保持本地网络请求有效<br>3. 等待 1600 ms<br>4. 观察状态与报文周期 |
| **期望结果** | 1. `gs_CanNmState` 从 `REPEAT_MESSAGE` 变为 `NORMAL_OPERATION`<br>2. NM 报文周期变为 20ms（标准周期）<br>3. CBV 中 Active Wakeup Bit 清零 |
| **判定标准** | CANoe 统计 0x500 报文周期 = 20ms +- 2ms |
| **优先级** | P1 |

---

### TC-004: 正常网络运行 — Normal Operation 周期性发送

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 Normal Operation 状态下 NM 报文周期稳定性 |
| **测试步骤** | 1. 进入 Normal Operation 状态<br>2. 连续记录 50 帧 0x500 报文时间戳<br>3. 统计周期偏差 |
| **期望结果** | 1. 每帧 0x500 报文 Data[0] = `0x01`<br>2. 平均周期 = 20ms<br>3. 最大偏差 <= +-2ms |
| **判定标准** | CANoe 统计周期在 [18ms, 22ms] 范围内 |
| **优先级** | P1 |

---

### TC-005: 本地网络释放 — Normal Operation -> Ready Sleep 转换

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证调用 `CanNm_NetworkRelease()` 后正确停止发送 NM |
| **测试步骤** | 1. 确认 ECU 处于 Normal Operation<br>2. 应用层调用 `CanNm_NetworkRelease()`<br>3. 观察总线 100 ms |
| **期望结果** | 1. 状态从 `NORMAL_OPERATION` 变为 `READY_SLEEP`<br>2. ECU **停止**发送 `0x500` NM 报文<br>3. `gs_CanNmTimeoutTimer` 开始从 2000ms 递减 |
| **判定标准** | CANoe Trace 中 0x500 报文不再出现（除非收到远程 NM） |
| **优先级** | P1 |

---

### TC-006: Ready Sleep -> Prepare Bus-Sleep 转换（NmTimeout 超时）

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 NmTimeoutTimer (2000ms) 超时后进入 Prepare Bus-Sleep |
| **测试步骤** | 1. 完成 TC-005，处于 Ready Sleep<br>2. 总线上**不发送**任何其他节点的 NM 报文<br>3. 等待 2000 ms |
| **期望结果** | 1. `gs_CanNmState` 变为 `PREPARE_BUS_SLEEP`<br>2. ECU 仍不发送 NM 报文<br>3. `gs_CanNmWaitBusSleepTimer` 开始从 4000ms 递减 |
| **判定标准** | 断点或诊断读取状态 = `PREPARE_BUS_SLEEP` |
| **优先级** | P1 |

---

### TC-007: Prepare Bus-Sleep -> Bus-Sleep 转换（WaitBusSleep 超时）

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 WaitBusSleepTime (4000ms) 超时后进入 Bus-Sleep |
| **测试步骤** | 1. 完成 TC-006，处于 Prepare Bus-Sleep<br>2. 继续等待 4000 ms<br>3. 观察总线与状态 |
| **期望结果** | 1. `gs_CanNmState` 变为 `BUS_SLEEP`<br>2. 若注册了 Sleep Indication Callback，应被调用<br>3. 总线静默 |
| **判定标准** | 状态 = `BUS_SLEEP`，示波器无 0x500 报文 |
| **优先级** | P1 |

---

### TC-008: 远程唤醒 — Bus-Sleep -> Repeat Message（收到远程 NM）

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 ECU 在 Bus-Sleep 状态下收到远程 NM 报文后正确唤醒 |
| **测试步骤** | 1. 使 ECU 进入 Bus-Sleep（完成 TC-007）<br>2. CANoe 发送一帧 NM 报文: `0x500 [8] 02 00 00 00 00 00 00 00`<br>3. 观察 ECU 响应 |
| **期望结果** | 1. ECU 从 `BUS_SLEEP` 进入 `REPEAT_MESSAGE`<br>2. ECU 开始发送自身 NM 报文（Data[0]=0x01）<br>3. `gs_CanNmTimeoutTimer` 重置为 2000ms |
| **判定标准** | CANoe 在发送远程 NM 后 30ms 内收到 ECU 的 0x500 报文 |
| **优先级** | P1 |

---

### TC-009: 远程唤醒 — Prepare Bus-Sleep -> Repeat Message

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证在 Prepare Bus-Sleep 状态下收到远程 NM 也能唤醒 |
| **测试步骤** | 1. 使 ECU 进入 Prepare Bus-Sleep（完成 TC-006，但不等待 4000ms）<br>2. CANoe 发送 NM 报文: `0x500 [8] 02 00 00 00 00 00 00 00`<br>3. 观察 ECU 响应 |
| **期望结果** | 1. 状态变为 `REPEAT_MESSAGE`<br>2. ECU 发送 NM 报文<br>3. `WaitBusSleepTimer` 清零 |
| **判定标准** | 收到远程 NM 后 30ms 内 ECU 发送 0x500 |
| **优先级** | P2 |

---

### TC-010: NmTimeoutTimer 刷新 — 收到远程 NM 时重置超时

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 Normal Operation / Ready Sleep 状态下收到远程 NM 会刷新 NmTimeoutTimer |
| **测试步骤** | 1. ECU 处于 Normal Operation<br>2. 等待 1500 ms（不释放网络）<br>3. 在 1500ms 时发送一帧远程 NM: `0x500 [8] 02 00 00 00 00 00 00 00`<br>4. 继续等待 1500 ms |
| **期望结果** | 1. 收到远程 NM 时 `NmTimeoutTimer` 重置为 2000ms<br>2. ECU 不会进入 Prepare Bus-Sleep<br>3. ECU 继续周期性发送 NM 报文 |
| **判定标准** | 在步骤 4 结束后 ECU 仍发送 0x500，状态为 NORMAL_OPERATION |
| **优先级** | P1 |

---

### TC-011: Repeat Message Request (RMS) 处理

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 ECU 正确处理 CBV 中的 Repeat Message Request (bit 0) |
| **测试步骤** | 1. ECU 处于 Normal Operation<br>2. CANoe 发送带 RMS 的 NM: `0x500 [8] 02 01 00 00 00 00 00 00` (CBV=0x01)<br>3. 观察 ECU 状态与报文 |
| **期望结果** | 1. ECU 识别到 RMS，设置 `gs_CanNmRepeatMsgRequested = TRUE`<br>2. 若当前在 Normal Operation，不会强制回到 Repeat Message（AUTOSAR 可选行为，本实现不强制回退）<br>3. 若后续进入 Repeat Message，ECU 发出的 NM 中 CBV bit 0 = 1 |
| **判定标准** | 代码逻辑正确响应 RMS 标志 |
| **优先级** | P2 |

---

### TC-012: 忽略自身报文回环

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 ECU 不会将自身发出的 NM 报文识别为远程唤醒源 |
| **测试步骤** | 1. ECU 处于 Normal Operation<br>2. 在 CANoe 中设置只监听不发送<br>3. 记录 50 帧 ECU 发出的 0x500 报文<br>4. 观察 NmTimeoutTimer 是否被错误刷新 |
| **期望结果** | 1. ECU 发出的 NM（Data[0]=0x01）被 RxIndication 过滤（NodeId == CANNM_NODE_ID）<br>2. NmTimeoutTimer 正常递减，不被自身报文刷新<br>3. 2000ms 无其他节点 NM 时仍进入 Ready Sleep |
| **判定标准** | 移除总线上其他节点后，ECU 仍能在 2000ms 后进入 Ready Sleep |
| **优先级** | P1 |

---

### TC-013: NM PDU 内容正确性检查

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 NM 报文各字节内容符合 AUTOSAR 规范 |
| **测试步骤** | 1. ECU 处于 Normal Operation<br>2. 抓取一帧 ECU 发出的 0x500 报文 |
| **期望结果** | Byte 0 = `0x01` (Source Node Identifier)<br>Byte 1 = CBV (bit 4 Active Wakeup 按需置位)<br>Byte 2~7 = `0x00` (User Data 未使用) |
| **判定标准** | 逐字节比对上述期望值 |
| **优先级** | P1 |

---

### TC-014: 快速发送阶段计数正确性

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证首次进入 Repeat Message 时快速发送 5 帧 NM |
| **测试步骤** | 1. ECU 从 Bus-Sleep 被唤醒（TC-008）<br>2. 精确记录前 10 帧 0x500 报文的时间戳 |
| **期望结果** | 第 1 帧在 ~30ms 发出<br>第 2~6 帧间隔 ~= 10ms（Immediate Cycle）<br>第 7 帧起间隔 ~= 20ms（标准周期） |
| **判定标准** | 帧 2~6 的周期在 [8ms, 12ms]；帧 7~10 的周期在 [18ms, 22ms] |
| **优先级** | P2 |

---

### TC-015: 异常场景 — 总线 BUS-OFF 后恢复

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 NM 在总线 BUS-OFF 恢复后正常工作 |
| **测试步骤** | 1. ECU 处于 Normal Operation<br>2. 人为制造 BUS-OFF（如短接 CANH/CANL 500ms）<br>3. 恢复正常总线连接<br>4. 观察 ECU NM 行为 |
| **期望结果** | 1. BUS-OFF 期间 ECU 无法发送 NM<br>2. 恢复后 NM 报文周期恢复正常<br>3. 状态机不受影响，仍保持当前状态 |
| **判定标准** | 总线恢复后 100ms 内重新收到 0x500 报文 |
| **优先级** | P2 |

---

### TC-016: 网络请求与释放交替压力测试

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证频繁请求/释放不会导致状态机异常 |
| **测试步骤** | 1. 循环执行以下操作 20 次:<br>&nbsp;&nbsp;a. `CanNm_NetworkRequest()`<br>&nbsp;&nbsp;b. 等待 500 ms<br>&nbsp;&nbsp;c. `CanNm_NetworkRelease()`<br>&nbsp;&nbsp;d. 等待 3000 ms |
| **期望结果** | 1. 每次请求后 ECU 发送 NM 报文<br>2. 每次释放后 NM 报文停止<br>3. 无状态机卡死、无异常复位 |
| **判定标准** | 20 次循环后 ECU 正常工作，无 Watchdog 复位 |
| **优先级** | P2 |

---

### TC-017: 通过诊断读取 NM 状态 — Bus-Sleep

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 DID F520 在 Bus-Sleep 状态下返回正确 |
| **测试步骤** | 1. 使 ECU 进入 Bus-Sleep（完成 TC-007）<br>2. 诊断仪发送: `22 F5 20`<br>3. 记录 ECU 响应 |
| **期望结果** | ECU 正响应: `62 F5 20 01 00 00 00`<br>Byte0=0x01 (BusSleep), Byte1=0x00 (BusSleep模式), Byte2=0x00 (无请求) |
| **判定标准** | 响应数据与期望值逐字节匹配 |
| **优先级** | P1 |

---

### TC-018: 通过诊断读取 NM 状态 — Repeat Message

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 DID F520 在 Repeat Message 状态下返回正确 |
| **测试步骤** | 1. 触发本地网络请求，ECU 进入 Repeat Message（TC-002）<br>2. 诊断仪发送: `22 F5 20`<br>3. 记录 ECU 响应 |
| **期望结果** | ECU 正响应: `62 F5 20 03 03 01 00`<br>Byte0=0x03 (RepeatMessage), Byte1=0x03 (Network模式), Byte2=0x01 (有请求)<br>CBV Active Wakeup Bit = 1 |
| **判定标准** | 响应数据与期望值逐字节匹配 |
| **优先级** | P1 |

---

### TC-019: 通过诊断读取 NM 状态 — Normal Operation

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 DID F520 在 Normal Operation 状态下返回正确 |
| **测试步骤** | 1. 等待 ECU 从 Repeat Message 进入 Normal Operation（TC-003）<br>2. 诊断仪发送: `22 F5 20`<br>3. 记录 ECU 响应 |
| **期望结果** | ECU 正响应: `62 F5 20 04 03 01 00`<br>Byte0=0x04 (NormalOperation), Byte1=0x03 (Network模式), Byte2=0x01 (有请求) |
| **判定标准** | 响应数据与期望值逐字节匹配 |
| **优先级** | P1 |

---

### TC-020: 通过诊断读取 NM 状态 — Ready Sleep

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 DID F520 在 Ready Sleep 状态下返回正确 |
| **测试步骤** | 1. 调用 `CanNm_NetworkRelease()` 使 ECU 进入 Ready Sleep（TC-005）<br>2. 诊断仪发送: `22 F5 20`<br>3. 记录 ECU 响应 |
| **期望结果** | ECU 正响应: `62 F5 20 05 03 00 00`<br>Byte0=0x05 (ReadySleep), Byte1=0x03 (Network模式), Byte2=0x00 (无请求) |
| **判定标准** | 响应数据与期望值逐字节匹配 |
| **优先级** | P1 |

---

### TC-021: 通过诊断读取 NM 状态 — Prepare Bus-Sleep

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 DID F520 在 Prepare Bus-Sleep 状态下返回正确 |
| **测试步骤** | 1. 使 ECU 进入 Prepare Bus-Sleep（TC-006）<br>2. 诊断仪发送: `22 F5 20`<br>3. 记录 ECU 响应 |
| **期望结果** | ECU 正响应: `62 F5 20 02 01 00 00`<br>Byte0=0x02 (PrepareBusSleep), Byte1=0x01 (PrepareBusSleep模式), Byte2=0x00 (无请求) |
| **判定标准** | 响应数据与期望值逐字节匹配 |
| **优先级** | P1 |

---

### TC-022: 诊断读取 NM 状态 — 跨状态一致性验证

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 F520 返回值与 CAN 总线实际 NM 行为一致 |
| **测试步骤** | 1. 同时使用 CANoe 监控 0x500 报文和诊断读取 F520<br>2. 按以下序列操作并记录:<br>&nbsp;&nbsp;a. 上电 -> 读 F520<br>&nbsp;&nbsp;b. 网络请求 -> 读 F520<br>&nbsp;&nbsp;c. 等待进入 Normal Op -> 读 F520<br>&nbsp;&nbsp;d. 网络释放 -> 读 F520<br>&nbsp;&nbsp;e. 等待进入 Prepare Bus-Sleep -> 读 F520<br>&nbsp;&nbsp;f. 等待进入 Bus-Sleep -> 读 F520 |
| **期望结果** | 每个阶段的 F520 返回值与该阶段理论状态一致，且与 CAN Trace 中 0x500 报文的有无对应 |
| **判定标准** | 全序列无状态不一致现象 |
| **优先级** | P1 |

---

### TC-023: ECU 上电 2s 后自动释放 NM

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 ECU 上电后 NM 保持约 2s，随后自动释放，允许进入 Bus-Sleep |
| **测试步骤** | 1. ECU 断电后重新上电<br>2. 用 CANoe 记录 0x500 报文，持续时间 > 10s<br>3. 观察报文发送行为 |
| **期望结果** | 1. 上电后 0~2s: ECU 周期性发送 NM 报文（0x500）<br>2. 约 2s 后: ECU 调用 `CanNm_NetworkRelease()`，停止发送 NM<br>3. 停止发送后 2s: NmTimeout 超时，进入 Ready Sleep<br>4. 再 4s 后: WaitBusSleep 超时，进入 Bus-Sleep |
| **判定标准** | CANoe Trace 显示:<br>- 0~2s 有 0x500 报文<br>- 2s 后无 0x500 报文<br>- 约 8s 后确认进入 Bus-Sleep（可用 F520 验证） |
| **优先级** | P1 |

---

## 3. 性能测试用例

### TC-Perf-001: 状态切换时延

| 项目 | 内容 |
|------|------|
| **测试目的** | 测量状态切换的实际时延 |
| **测试步骤** | 1. 从 Bus-Sleep -> NetworkRequest，测量到首帧 NM 时间<br>2. 从 Normal Op -> NetworkRelease，测量到最后 1 帧 NM 时间 |
| **期望结果** | 首帧时延 <= 30ms（T_START_NM_TX + T_START_APPEND）<br>最后 1 帧到停止发送时延 <= 25ms |
| **判定标准** | 示波器时间测量 |
| **优先级** | P2 |

### TC-Perf-002: 定时器精度

| 项目 | 内容 |
|------|------|
| **测试目的** | 验证 1ms 定时器精度对 NM 周期的影响 |
| **测试步骤** | 1. ECU 进入 Normal Operation<br>2. 连续记录 100 帧 0x500 时间戳<br>3. 计算周期标准差 |
| **期望结果** | 平均周期 = 20ms<br>标准差 sigma <= 1ms |
| **判定标准** | CANoe 统计分析 |
| **优先级** | P2 |

---

## 4. 测试记录模板

| 用例编号 | 测试日期 | 测试人 | 结果 | 备注 |
|----------|----------|--------|------|------|
| TC-001 | | | Pass / Fail | |
| TC-002 | | | Pass / Fail | |
| TC-003 | | | Pass / Fail | |
| TC-004 | | | Pass / Fail | |
| TC-005 | | | Pass / Fail | |
| TC-006 | | | Pass / Fail | |
| TC-007 | | | Pass / Fail | |
| TC-008 | | | Pass / Fail | |
| TC-009 | | | Pass / Fail | |
| TC-010 | | | Pass / Fail | |
| TC-011 | | | Pass / Fail | |
| TC-012 | | | Pass / Fail | |
| TC-013 | | | Pass / Fail | |
| TC-014 | | | Pass / Fail | |
| TC-015 | | | Pass / Fail | |
| TC-016 | | | Pass / Fail | |
| TC-017 | | | Pass / Fail | |
| TC-018 | | | Pass / Fail | |
| TC-019 | | | Pass / Fail | |
| TC-020 | | | Pass / Fail | |
| TC-021 | | | Pass / Fail | |
| TC-022 | | | Pass / Fail | |
| TC-023 | | | Pass / Fail | |

---

## 5. 诊断接口说明（新增 DID F520）

### 5.1 诊断请求格式
```
服务: 0x22 ReadDataByIdentifier
DID:  0xF520 (AUTOSAR CanNm 状态信息)
请求帧: 22 F5 20
```

### 5.2 正响应格式
```
响应帧: 62 F5 20 [Byte0] [Byte1] [Byte2] [Byte3]
```

### 5.3 响应数据定义

| 字节 | 字段 | 值 | 说明 |
|------|------|-----|------|
| Byte 0 | NM State | `0x00` | `CANNM_STATE_UNINIT` |
| | | `0x01` | `CANNM_STATE_BUS_SLEEP` |
| | | `0x02` | `CANNM_STATE_PREPARE_BUS_SLEEP` |
| | | `0x03` | `CANNM_STATE_REPEAT_MESSAGE` |
| | | `0x04` | `CANNM_STATE_NORMAL_OPERATION` |
| | | `0x05` | `CANNM_STATE_READY_SLEEP` |
| Byte 1 | NM Mode | `0x00` | `CANNM_MODE_BUS_SLEEP` |
| | | `0x01` | `CANNM_MODE_PREPARE_BUS_SLEEP` |
| | | `0x02` | `CANNM_MODE_SYNCHRONIZE` |
| | | `0x03` | `CANNM_MODE_NETWORK` |
| Byte 2 | Network Requested | `0x00` | 无网络请求 |
| | | `0x01` | 有网络请求 |
| Byte 3 | Reserved | `0x00` | 保留 |

### 5.4 典型场景响应示例

| 场景 | 诊断请求 | ECU 正响应 | 说明 |
|------|----------|------------|------|
| 上电未请求网络 | `22 F5 20` | `62 F5 20 01 00 00 00` | Bus-Sleep, 无请求 |
| Repeat Message | `22 F5 20` | `62 F5 20 03 03 01 00` | RepeatMessage, 有请求 |
| Normal Operation | `22 F5 20` | `62 F5 20 04 03 01 00` | NormalOp, 有请求 |
| Ready Sleep | `22 F5 20` | `62 F5 20 05 03 00 00` | ReadySleep, 无请求 |
| Prepare Bus-Sleep | `22 F5 20` | `62 F5 20 02 01 00 00` | PrepareBusSleep, 无请求 |

### 5.5 实现文件清单

| 文件 | 修改内容 |
|------|----------|
| `AppSw/Tricore/App_UDS/uds_cfg.h` | 新增 DID 枚举 `F520 = 0xF520` |
| `AppSw/Tricore/App_UDS/uds_app.c` | `ReadDataByIdentifier0x22()` 中增加 F520 分支，调用 `CanNm_GetState()` 与 `CanNm_IsNetworkRequested()` |

---

## 6. 常见问题排查

| 现象 | 可能原因 | 排查方法 |
|------|----------|----------|
| ECU 上电不发送 NM | 未调用 `CanNm_NetworkRequest()` | 检查 `AppBL_init()` 是否调用 |
| NM 周期偏差大 | `CanNm_SystemTickCtl()` 未 1ms 调用 | 检查 `AppUds_main()` 调用周期 |
| 收到远程 NM 不唤醒 | CAN ID 不匹配 / 中断未注册 | 检查 `isrCAN0_RX` 是否包含 0x500 |
| 状态机卡在 Repeat Message | `RepeatMessageTimer` 未递减 | 检查 `CanNm_SystemTickCtl()` |
| 进入 Bus-Sleep 后无法唤醒 | `CanNm_NetworkRequest()` 未触发 | 检查应用层唤醒源 |
| 诊断读取 F520 返回 NRC 31 | DID 未在 `uds_cfg.h` 中定义 | 检查 `F520` 是否在 `rw_data_did` 枚举中 |
| 诊断读取 F520 返回 NRC 13 | 请求长度错误 | 确保请求为 `22 F5 20`（3字节） |
| F520 返回值与实际状态不符 | `CanNm_GetState()` 未被调用 | 检查 `uds_app.c` 中 F520 分支是否正确实现 |
| 上电后 NM 一直发送不停止 | `CanNm_NetworkRelease()` 未触发 | 检查 `Cpu0_Main.c` 中 2s 定时释放逻辑 |
