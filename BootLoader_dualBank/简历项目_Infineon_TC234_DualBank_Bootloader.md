# Infineon TC234 A/B Dual-Bank Bootloader 开发

> **项目角色**：嵌入式软件工程师（独立负责 / 核心开发）  
> **项目周期**：2025.xx – 2026.xx  
> **平台**：Infineon AURIX TC234 (TriCore) + TASKING v6.2r2  
> **规范**：ISO 14229-1 (UDS) / ISO 15765-2 (CAN-TP) / OEM 刷写安全规范  

---

## 一、项目概述

面向汽车电子控制单元（ECU）的**双分区Bootloader固件升级系统**，实现A/B双Bank独立启动、无缝滚动升级与自动回滚。项目包含Bootloader主工程、APP应用工程、上位机刷写脚本及安全访问DLL，完整覆盖从启动仲裁到UDS诊断刷写的全链路。

### 系统组成
| 模块 | 技术栈 | 职责 |
|:-----|:-------|:-----|
| Bootloader | C / TriCore汇编 / iLLD | 启动仲裁、UDS诊断、Flash擦写、Bank切换 |
| APP Application | C / iLLD | 双Bank独立编译，支持OTA入口跳转 |
| 刷写脚本 | Python (ZXDoc API) | 完整UDS三阶段刷写时序自动化 |
| 安全算法库 | C++ (VS DLL) | UDS 0x27 Seed/Key安全访问算法 |

---

## 二、个人职责与核心工作

### 2.1 双Bank启动仲裁与安全管理
- 设计**DFlash双份冗余标志区**（Main + Shadow），实现掉电安全与自动修复
- 实现**启动失败回滚机制**：连续3次启动失败自动标记当前Bank无效，切换至备用Bank
- 实现**裸跳转（Bare-Metal Jump）**：禁用中断/Cache/ECC Trap，切断CSA上下文，使用`ji`指令安全移交控制权至APP

### 2.2 UDS诊断协议栈开发
- 完整实现15个UDS服务（0x10/0x11/0x14/0x19/0x22/0x23/0x27/0x28/0x2E/0x31/0x34/0x36/0x37/0x3E/0x85）
- **刷写安全机制**：
  - 运行Bank保护：禁止刷写当前激活Bank（NRC 0x22）
  - Bootloader区保护：禁止擦除Sector 0~7（NRC 0xFC）
  - 安全访问锁定：连续3次失败锁定10秒（NRC 0x36/0x37）
- **流式CRC校验**：下载过程中实时计算CRC32，传输完成后独立校验双重确认

### 2.3 Flash驱动与内存管理
- 解决**TC234单Bank PFlash的RWW（Read-While-Write）约束**：将擦除/写入例程拷贝至PSPR（RAM）执行
- 实现**32字节页对齐写入**：自动处理未对齐地址与尾部数据缓存
- 每次写入后**回读验证**，结合FSR错误掩码（SQER/PROER/PVER/EVER）进行硬件级错误检测

### 2.4 车企标准后编程流程
- 实现ISO 14229-1标准三阶段刷写：预编程 → 主编程 → 后编程
- 后编程阶段强制闭环：恢复通信（0x28）→ 恢复DTC（0x85）→ 清除DTC（0x14）→ 复位（0x11）
- 编程指纹写入（DID F15A）：记录刷写日期、诊断仪ID、软件版本，满足审计追溯要求

### 2.5 DTC故障诊断系统（ISO 15031-6）
- 实现4个标准DTC：U0100/U0121/P0601/B1000
- FDT（Fault Detection Counter）防抖机制：失败+2/通过-1，阈值±127
- 老化计数器（Aging Counter）：连续通过40次后ConfirmedDTC自动清除
- 快照数据（Freeze Frame）：故障确认时自动记录系统电压、环境温度、CAN状态等8组DID

### 2.6 AUTOSAR CAN网络管理（CanNm）
- 按AUTOSAR标准实现CanNm状态机：Bus-Sleep / Prepare Bus-Sleep / Repeat Message / Normal Operation / Ready Sleep
- NM报文周期20ms，CAN ID 0x500，支持网络请求/释放与2秒上电自动释放策略
- 通过UDS DID F520诊断接口实时读取NM状态

---

## 三、技术亮点与难点攻克

| 难点 | 解决方案 | 技术价值 |
|:-----|:---------|:---------|
| TC234单Bank PFlash擦写时不能取指 | 将Flash操作函数重定位至PSPR（0x70100000），编译优化-O2确保内联 | 满足RWW约束，避免Context Management Error Trap |
| AURIX PFlash擦除态为0x00（非常规0xFF） | 填充字节使用0x00，对齐工具`align_hex.py`预处理hex文件 | 避免回读验证失败 |
| 跳转APP后Bus Error Trap | 裸跳转前切断CSA链表（PCXI=0），禁用ECC Trap与Cache | 确保Bootloader上下文不污染APP运行环境 |
| 刷写过程中掉电导致Bank损坏 | DFlash双份冗余标志+Sequence号仲裁+CRC32全Bank校验 | 掉电后自动恢复有效标志，防止变砖 |
| CAN BusOff导致通信中断 | 中断中检测BOFF/EWRN标志，自动复位CCE/INIT恢复通信 | 提升刷写过程鲁棒性 |

---

## 四、关键技术指标

| 指标 | 数值 |
|:-----|:-----|
| 启动到跳转判定时间 | < 100ms |
| PFlash擦除单Sector | ~0.5~2s |
| PFlash写入32字节页 | ~50~100μs |
| UDS S3会话超时 | 5000ms |
| 安全访问失败锁定时间 | 10s |
| 最大启动尝试次数 | 3次（自动回滚） |
| DTC老化计数器阈值 | 40次 |
| NM报文周期 | 20ms |

---

## 五、技术栈关键词

**MCU/编译器**：Infineon AURIX TC234, TriCore, TASKING v6.2r2, iLLD  
**通信协议**：CAN 2.0B, UDS (ISO 14229-1), CAN-TP (ISO 15765-2)  
**存储/Flash**：PFlash/DFlash擦写, PSPR代码重定位, CRC32, ECC  
**安全**：Seed/Key算法, Security Access, 双份冗余, 掉电保护  
**工具链**：ZXDoc刷写工具, ZCAN Pro, Python自动化脚本, Visual Studio DLL开发  
**标准**：OEM刷写规范, ISO 15031-6 (DTC), AUTOSAR CanNm  

---

## 六、项目成果

- **零变砖风险**：双Bank+自动回滚机制，任一Bank损坏均可从另一Bank启动
- **通过车企审核**：UDS服务与后编程流程符合OEM标准，支持量产级刷写
- **完整工具链**：提供Python刷写脚本+C++安全DLL，实现一键自动化刷写
- **可维护架构**：模块化UDS服务实现，支持快速扩展新DID和新例程

---

> **简历使用建议**：
> - 应聘**嵌入式Bootloader/基础软件**岗位：重点突出2.1、2.2、2.3节
> - 应聘**汽车电子/诊断工程师**岗位：重点突出2.2、2.4、2.5节
> - 应聘**功能安全/网络安全**岗位：重点突出安全访问、运行Bank保护、双份冗余机制
