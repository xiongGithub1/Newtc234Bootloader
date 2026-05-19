# 统一 HEX 地址重映射改造说明

## 1. 改造目标

解决 A/B 双区 Bootloader 刷写时需要维护两个不同地址的 HEX 文件（App_A.hex 和 App_B.hex）的问题。

改造后：
- **只使用一个统一 HEX 文件**（使用 Bank A 的地址范围 `0x80020000~0x800FFFFF`）
- Bootloader 在运行时根据目标 Bank，**动态将统一地址重映射到实际物理地址**
- 刷写脚本不再需要根据目标 Bank 选择不同的 HEX 文件

## 2. 核心设计思想

```
┌─────────────────────────────────────────────────────────────┐
│                     统一 HEX 文件                            │
│              地址范围: 0x80020000 ~ 0x800FFFFF              │
│                   (与 App A 链接地址相同)                     │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│              Bootloader 运行时地址重映射                      │
├─────────────────────────────────────────────────────────────┤
│  如果 targetWriteBank == BANK_A:                             │
│      实际地址 = 统一地址 (不变)                               │
│  如果 targetWriteBank == BANK_B:                             │
│      实际地址 = 统一地址 + (0x80100000 - 0x80020000)        │
│               = 统一地址 + 0x000E0000                       │
└─────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┴───────────────┐
              ▼                               ▼
┌─────────────────────────┐     ┌─────────────────────────┐
│    Bank A 物理地址       │     │    Bank B 物理地址       │
│  0x80020000~0x800FFFFF  │     │  0x80100000~0x801FFFFF  │
└─────────────────────────┘     └─────────────────────────┘
```

## 3. 代码改造清单

### 3.1 Boot_DualBank.h
**文件**: `AppSw/Tricore/App_bootloader/Boot_DualBank.h`

**新增内容**:
- 统一 HEX 地址范围定义：
  - `UNIFIED_HEX_BASE_ADDR` = `0x80020000u`
  - `UNIFIED_HEX_END_ADDR` = `0x80100000u`
  - `UNIFIED_HEX_SIZE` = `896 * 1024`
- Bank 大小定义：
  - `BANK_APP_A_SIZE` = `896 * 1024`
  - `BANK_APP_B_SIZE` = `1024 * 1024`
- 新增 API 声明：
  - `Boot_DualBank_RemapUnifiedAddr()` — 地址重映射
  - `Boot_DualBank_IsUnifiedHexAddr()` — 判断是否为统一 HEX 地址
  - `Boot_DualBank_RemapUnifiedSector()` — Sector 编号重映射

### 3.2 Boot_DualBank.c
**文件**: `AppSw/Tricore/App_bootloader/Boot_DualBank.c`

**新增内容**:
- `Boot_DualBank_RemapUnifiedAddr(uint32 unifiedAddr)`
  - 判断 cached/uncached 地址
  - 计算相对于 `UNIFIED_HEX_BASE_ADDR` 的偏移
  - 根据 `targetWriteBank` 映射到 Bank A 或 Bank B
  - 返回对应类型的地址（cached/uncached）

- `Boot_DualBank_IsUnifiedHexAddr(uint32 addr)`
  - 判断地址是否在统一 HEX 范围内
  - 自动处理 cached 和 uncached 地址

- `Boot_DualBank_RemapUnifiedSector(uint16 unifiedSector)`
  - 如果目标 Bank 是 Bank B，将 sector 编号从 Bank A 范围映射到 Bank B 范围
  - Bank B sector = unifiedSector + (23 - 8) = unifiedSector + 15
  - 例如：S8 → S23, S22 → S37 (但受 Bank B 大小限制)

### 3.3 uds_app.c (RequestDownload 0x34)
**文件**: `AppSw/Tricore/App_UDS/uds_app.c`

**修改内容**:
- `RequestDownload0x34()` 函数中的目标 Bank 判断逻辑：
  - 如果下载地址在统一 HEX 范围内 → 使用 `CheckProgrammingConditions()` 已设置的 `targetWriteBank`
  - 如果地址在 Bank B 范围 → 强制设置目标 Bank 为 B
  - 其他情况 → 默认 Bank A
- 在确定目标 Bank 后，调用 `Boot_DualBank_RemapUnifiedAddr()` 将地址重映射到实际物理地址

### 3.4 uds_app.c (EraseFlashSector)
**文件**: `AppSw/Tricore/App_UDS/uds_app.c`

**修改内容**:
- `EraseFlashSector()` 函数增加 unified HEX sector 重映射：
  - 如果传入的 sector 在 `BANK_A_SECTOR_START` ~ `BANK_A_SECTOR_END` 范围内
  - 调用 `Boot_DualBank_RemapUnifiedSector()` 映射到目标 Bank 的实际 sector
  - 使用映射后的 sector 编号进行擦除操作

### 3.5 fls_app.c (IsDownloadDataAddrValid)
**文件**: `AppSw/Tricore/App_UDS/Flah_app/fls_app.c`

**修改内容**:
- 扩展地址有效范围：
  - 原来：只检查 `FL_PFLASH_PF_ADDR_Start` ~ `FL_PFLASH_PF_ADDR_End`
  - 现在：接受 `BANK_A_START_ADDR` ~ `BANK_B_END_ADDR` 范围内的所有地址
  - 自动处理 cached/uncached 地址转换

### 3.6 shuaxie.py (刷写脚本)
**文件**: `shuaxie.py`

**修改内容**:
- 删除 `HEX_FILE_B` 变量（不再需要 Bank B 专用 HEX）
- 增加 `ALIGNED_HEX_FILE` 统一对齐输出路径
- `BANK_SECTORS["B"]` 修改为 `list(range(8, 23))`（与 Bank A 使用相同的 sector 编号）
- 简化 `Check Programming` 响应处理：
  - 无论目标 Bank 是 A 还是 B，都使用同一个 `HEX_FILE`
  - 对齐后输出到统一的 `ALIGNED_HEX_FILE`
- `align_hex_file()` 增加统一 HEX 地址范围校验：
  - 检查所有数据地址是否在 `UNIFIED_HEX_BASE_ADDR` ~ `UNIFIED_HEX_END_ADDR` 范围内
  - 如果地址越界，报错并中止刷写

## 4. 刷写流程变化

### 改造前
```
1. 31 01 FF FD → 获取目标 Bank
2. 如果目标 Bank = A → 加载 App_dualBank_A.hex
3. 如果目标 Bank = B → 加载 App_dualBank_B.hex
4. 分别对齐两个 HEX 文件
5. 发送 34/36/37 下载
```

### 改造后
```
1. 31 01 FF FD → 获取目标 Bank
2. 无论目标 Bank 是 A 还是 B → 加载同一个 App_dualBank.hex
3. 对齐为统一 HEX 文件 (App_dualBank_Unified.hex)
4. 发送 34/36/37 下载（Bootloader 自动重映射地址）
```

## 5. 统一 HEX 使用说明

### 5.1 编译 App
- 使用 **App A 的链接脚本** (`App_dualBank_a.lsl`) 编译
- 生成 `App_dualBank.hex`
- 此 HEX 的地址范围固定为 `0x80020000~0x800FFFFF`

### 5.2 Bootloader 内部映射
| 统一 HEX 地址 | 目标 Bank A 实际地址 | 目标 Bank B 实际地址 |
|--------------|---------------------|---------------------|
| 0x80020000   | 0x80020000          | 0x80100000          |
| 0x80090000   | 0x80090000          | 0x80170000          |
| 0x80098000   | 0x80098000          | 0x80178000          |
| 0x800FFFFF   | 0x800FFFFF          | 0x801FFFFF          |

### 5.3 Sector 映射
| 统一 HEX Sector | 目标 Bank A Sector | 目标 Bank B Sector |
|-----------------|--------------------|--------------------|
| S8              | S8                 | S23                |
| S9              | S9                 | S24                |
| ...             | ...                | ...                |
| S22             | S22                | S37                |

## 6. 注意事项

1. **App B 链接脚本保持不变**：
   - App B 的 `App_dualbank_b.lsl` 仍然使用 `0x80100000` 作为起始地址
   - 这是为了 Bootloader **跳转** 到 Bank B 时，向量表地址正确
   - 统一 HEX 只在**刷写阶段**使用 Bank A 的地址

2. **地址有效范围**：
   - 统一 HEX 只支持 `0x80020000~0x800FFFFF` 范围
   - 超出此范围的地址会被刷写脚本拒绝

3. **向后兼容**：
   - 如果刷写工具直接发送 Bank B 的地址 (`0x80100000`)，Bootloader 仍然支持
   - 这是为了兼容旧的刷写工具或测试场景

4. **CheckProgrammingConditions 服务**：
   - 在发送 34 请求下载之前，必须先调用 `31 01 FFFD`
   - 此服务设置 `targetWriteBank`，决定后续刷写的目标

## 7. 相关文件清单

| 文件路径 | 修改类型 |
|---------|---------|
| `AppSw/Tricore/App_bootloader/Boot_DualBank.h` | 新增统一 HEX 定义和 API 声明 |
| `AppSw/Tricore/App_bootloader/Boot_DualBank.c` | 新增地址/sector 重映射实现 |
| `AppSw/Tricore/App_UDS/uds_app.c` | 修改 0x34 请求下载和 sector 擦除逻辑 |
| `AppSw/Tricore/App_UDS/Flah_app/fls_app.c` | 扩展地址有效范围检查 |
| `shuaxie.py` | 使用统一 HEX，删除 Bank B 专用 HEX 路径 |

## 8. 验证方法

1. 编译 App A，生成 `App_dualBank.hex`
2. 运行刷写脚本，检查是否正确对齐到统一 HEX
3. 使用 CAN 工具发送 31 01 FFFD，确认返回的目标 Bank 正确
4. 发送 34 请求下载，确认 Bootloader 内部地址重映射正确
5. 完成刷写后，发送 31 01 DFFF 验证 CRC
6. 复位 ECU，确认 Bootloader 能正确跳转到新 Bank
