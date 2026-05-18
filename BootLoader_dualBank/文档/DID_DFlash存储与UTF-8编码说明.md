# DID DFlash 持久化存储设计说明

## 1. 概述

本文档描述 TC234 双 Bank Bootloader 项目中 DID (Data Identifier) 数据的 DFlash 持久化存储方案。

所有 DID 文本数据统一采用 **UTF-8 编码** 存储，确保诊断工具、上位机和不同系统之间的兼容性。

## 2. DFlash 存储布局

TC234 的 DFlash 按逻辑扇区 (Logical Sector) 划分，每个扇区大小为 **8KB (0x2000 bytes)**，页写入粒度为 **8 bytes**。

```
DFlash 地址空间：0xAF000000 ~ 0xAF00FFFF

Sector 0 (0xAF000000 ~ 0xAF001FFF): 8KB
  + 0x0000 ~ 0x007F: Bootloader Dual-Bank 主标志区 (BootFlagMain_t, CRC32)
  + 0x0080 ~ 0x00FF: Bootloader 影子备份标志区 (BootFlagShadow_t)
  + 0x0100 ~ 0x01FF: 预留
  + 0x0200 ~ 0x0241: F15A 最新指纹 (66 bytes, 写入时 2E 服务)
  + 0x0300 ~ 0x03FF: 预留
  + 其他: 预留

Sector 1 (0xAF002000 ~ 0xAF003FFF): 8KB
  + 0x0000 ~ 0x000F: F186 Bootloader参考号 (10 bytes, UTF-8)
  + 0x0010 ~ 0x001F: F187 OEM零部件号 (16 bytes, UTF-8)
  + 0x0020 ~ 0x002F: F188 OEM软件号 (16 bytes, UTF-8)
  + 0x0030 ~ 0x003F: F189 OEM软件版本号 (10 bytes, UTF-8)
  + 0x0040 ~ 0x004F: F18A 供应商代码 (10 bytes, UTF-8)
  + 0x0050 ~ 0x005F: F18B 供应商产品制造日期 (8 bytes, UTF-8)
  + 0x0060 ~ 0x006F: F18C 供应商生产流水号/批次号 (10 bytes, UTF-8)
  + 0x0070 ~ 0x008F: F190 整车VIN编号 (17 bytes, UTF-8)
  + 0x0090 ~ 0x009F: F191 OEM硬件号 (10 bytes, UTF-8)
  + 0x00A0 ~ 0x00AF: F192 供应商硬件号 (14 bytes, UTF-8)
  + 0x00B0 ~ 0x00BF: F193 硬件版本号 (8 bytes, UTF-8)
  + 0x00C0 ~ 0x00CF: F194 供应商软件号 (14 bytes, UTF-8)
  + 0x00D0 ~ 0x00DF: F195 软件版本号 (8 bytes, UTF-8)
  + 0x00E0 ~ 0x00EF: F197 控制器名称/系统名称 (8 bytes, UTF-8)
  + 0x00F0 ~ 0x00FF: 预留 (16 bytes)
  + 0x0100 ~ 0x01FE: F15B 指纹历史记录 (3 records x 67 bytes = 201 bytes)
  + 0x01F0 ~ 0x01F7: Magic Number (0x44494421 = "DID!")
  + 其他: 预留
```

## 3. 编码规范

- **所有 DID 文本数据统一采用 UTF-8 编码**
- 纯 ASCII 字符的 UTF-8 编码与 ASCII 完全相同（单字节）
- 中文、日文等非 ASCII 字符按 UTF-8 多字节编码
- 上位机读取 DFlash 后可直接按 UTF-8 解码显示

### UTF-8 编码优势
1. **兼容性**: ASCII 字符无需转换
2. **国际化**: 支持多语言字符
3. **通用性**: 诊断工具（如 CANoe、CANalyzer）原生支持
4. **无需 BOM**: 存储时省略 UTF-8 BOM 头，节省空间

## 4. F15A 指纹数据结构

F15A 写入数据（66 bytes），由 2E 服务接收：

| 字段 | 长度 | 说明 |
|------|------|------|
| deviceId | 16 bytes | 诊断仪设备编号 |
| beforeSw | 10 bytes | 刷写前 OEM 软件号 |
| beforeVer | 10 bytes | 刷写前软件版本号 |
| date | 10 bytes | 刷写日期 (YYYYMMDD) |
| afterSw | 10 bytes | 刷写后 OEM 软件号 |
| afterVer | 10 bytes | 刷写后软件版本号 |
| **总计** | **66 bytes** | |

写入时：
- F15A 写入到 **Sector 0 (0xAF000200)**
- 同时追加到 **F15B 历史记录 (Sector 1)**，形成 FIFO 队列（最多 3 条）

## 5. F15B 读取数据结构

F15B 读取返回 3 条历史记录（201 bytes），由 22 服务返回：

| 位置 | 内容 | 说明 |
|------|------|------|
| Record 0 (3~69) | 最新指纹 | 最近一次的 F15A 数据 |
| Record 1 (70~136) | 上一次指纹 | 倒数第二次 |
| Record 2 (137~203) | 上上次指纹 | 倒数第三次 |

- 记录顺序：**最新的在前**
- 不足 3 条时，空记录填充 0x00

## 6. DFlash 读写保护机制

### 6.1 Sector 1 写入策略（静态 DID 数据）
- 由于 DFlash 只能按扇区擦除（8KB），修改任何 DID 都需要擦除整个 Sector 1
- 采用 **"备份 -> 修改 -> 擦除 -> 恢复"** 策略：
  1. `DID_DFlash_BackupSector1()`: 将整个 Sector 1 读入 RAM 备份（8KB）
  2. 修改 RAM 备份中对应偏移的数据
  3. `DID_DFlash_RestoreSector1()`: 擦除扇区后按页（8 bytes）写回

### 6.2 Sector 0 标志区保护
- Bootloader 标志区（DualBankFlags）写入时会擦除整个 Sector 0
- 已实现 **F15A 指纹自动备份/恢复**：
  - 擦除前读取 0xAF000200 处的 F15A 数据
  - 写入标志后自动恢复 F15A 到原位置
  - Sector 1 数据不受影响（独立扇区）

## 7. 初始化流程

### 首次上电
1. `DID_DFlash_Init()` 检测 Sector 1 末尾的 Magic Number (0x44494421)
2. 若 Magic 无效：
   - 擦除 Sector 1
   - 写入所有 DID 默认值（UTF-8 编码）
   - 写入 Magic Number

### 正常启动
- Bootloader 和 App 均在启动时调用 `DID_DFlash_Init()`
- 若 Magic 已存在，仅做检查，不修改现有数据
- 支持多次调用（幂等操作）

## 8. 相关文件

| 文件 | 说明 |
|------|------|
| `BootLoader_dualBank/AppSw/Tricore/App_UDS/did_dflash.h` | Bootloader DFlash 管理模块头文件 |
| `BootLoader_dualBank/AppSw/Tricore/App_UDS/did_dflash.c` | Bootloader DFlash 管理模块实现 |
| `App_dualBank/AppSw/Tricore/App_UDS/did_dflash.h` | App DFlash 管理模块头文件（复制） |
| `App_dualBank/AppSw/Tricore/App_UDS/did_dflash.c` | App DFlash 管理模块实现（复制） |
| `BootLoader_dualBank/AppSw/Tricore/App_UDS/uds_cfg.h` | DID 定义与 DFlash 地址布局 |
| `App_dualBank/AppSw/Tricore/App_UDS/uds_cfg.h` | App DID 定义与 DFlash 地址布局 |
| `BootLoader_dualBank/AppSw/Tricore/App_UDS/uds_app.c` | Bootloader 0x22/0x2E 服务实现 |
| `App_dualBank/AppSw/Tricore/App_UDS/uds_app.c` | App 0x22 服务实现 |
| `BootLoader_dualBank/AppSw/Tricore/App_bootloader/Boot_DualBank.c` | 标志区写入（含 F15A 自动备份） |

## 9. 修改记录

| 日期 | 修改内容 |
|------|----------|
| 2026-05-18 | 将 DID 数据从 RAM 迁移到 DFlash，增加持久化 |
| 2026-05-18 | 增加 UTF-8 编码支持 |
| 2026-05-18 | F15A 写入时同时更新 F15B 历史记录（FIFO） |
| 2026-05-18 | Bootloader 标志区擦写时自动备份/恢复 F15A |
