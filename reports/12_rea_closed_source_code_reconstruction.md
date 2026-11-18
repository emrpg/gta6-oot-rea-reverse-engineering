# 闭源与发行版核心模块 REA 重构义务台账与源码级还原报告

> **工单编号**：`emrpg/gta6-oot-rea-reverse-engineering#4`  
> **分析工具**：`/usr/local/bin/rea` (`rea@6.9.0`)：`inspect-pe-imports`、`inspect-pe-resources`、`inspect-pe-load-config`、`inspect-managed-members`、`audit-release-exposure`、`build-reconstruction-obligation-ledger`  
> **证据存档**：`/root/wsl-demo/rea-reconstructions/closed-source-code/rea-evidence/`  
> **还原源码目录**：`/root/wsl-demo/rea-reconstructions/closed-source-code/`

---

## 1. 发行版二进制 REA 静态取证与重构台账总览

针对 `/root/wsl-demo/rea-reconstructions/release-binaries/ex/` 下解压的发行版包体，REA 实测结果如下：

| 目标二进制 | 格式 / 架构 | SHA-256 (前 12 位) | 导入 DLL (函数数) | LoadConfig / CFG | 资源项 | 托管类型数 |
| :--- | :--- | :--- | :---: | :--- | :---: | :---: |
| `OcarinaAtlas.exe` | PE32+ `x86_64` | `e5d7c72dc07e` | 0 (自举启动器) | 无 LoadConfig | 8 | — |
| `poptracker.exe` | PE32+ `x86_64` | `4d0be466cd8e` | 16 DLLs (591 函数) | 无 LoadConfig | 11 | — |
| `PopUpdater.exe` | PE32+ `x86_64` | `2c653aee0e9a` | 6 DLLs (43 函数) | 无 LoadConfig | 0 | — |
| `ScriptHookVDotNet.asi` | 混合 C++/CLI `x86_64` | `c5461f63b584` | 6 DLLs (37 函数) | `IMAGE_GUARD_CF_INSTRUMENTED` (`256`) | 1 | 121 |
| `ScriptHookVDotNet2.dll` | 混合 C++/CLI `x86_64` | `fe834372be56` | 4 DLLs (28 函数) | `IMAGE_GUARD_CF_INSTRUMENTED` (`256`) | 1 | 243 |
| `ScriptHookVDotNet3.dll` | 纯托管 IL | `f328a36445c1` | 0 (纯 IL) | 无 LoadConfig | 1 | 303 |

---

## 2. 四大核心闭源/发行版模块逆向分析与源码还原

### 2.1 RPF7/RPF8 密钥搜索与 RSC7 页重定位解密器 (`rpf_rsc7_crypto_reconstructed.cs`)
- **证据基础**：对照 `CodeWalker.Core/GameFiles/RpfFile.cs` 中 `RSC7` 魔数（`0x37435352`）与 `DecryptAES`/`DecryptNG` 边界，还原了：
  1. `Rpf7Header`（`0x52504637`）解析与三类加密模式枚举（`OPEN` / `AES` / `NG`）；
  2. `DecryptAesEcb()`（AES-256-ECB 块对齐解密）与 `ComputeNgKeyIndex()`（JOAAT 哈希 + 文件大小取模 `0x65` 的 NG LUT 索引派生）；
  3. `SearchKeyInDump()`（基于 SHA-256 固定时间比对的内存转储密钥搜索）；
  4. `Rsc7Header::ComputeSegmentSize()`（System/Graphics 虚拟页移位与分级计数位域解码）。

### 2.2 ScriptHookV 原生纤程调度与 TLS 上下文切换器 (`scripthookv_fiber_tls_reconstructed.cpp`)
- **REA 实测证据**：`ScriptHookVDotNet_asi__inspect-pe-imports.json` 直接观测到 `ScriptHookVDotNet.asi` 从 `ScriptHookV.dll` 导入的 5 个精确 C++ 修饰导出符号：
  - `?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z`
  - `?scriptUnregister@@YAXPEAUHINSTANCE__@@@Z`
  - `?scriptWait@@YAXK@Z`
  - `?keyboardHandlerRegister@@YAXP6AXKGEHHHH@Z@Z`
  - `?keyboardHandlerUnregister@@YAXP6AXKGEHHHH@Z@Z`
- **架构还原**：围绕上述 5 个原生边界还原了 `FiberScheduler`，实现脚本纤程注册/注销、毫秒级休眠调度（`wait_current`）以及在每帧 `tick()` 中保存/切换 RAGE 引擎 `scrThread` 与内存分配器 TLS 上下文的完整逻辑。

### 2.3 Grezzo OoT3D `ZAR/ZSI/CMB/CSAB` 解析与重编译运行时 (`grezzo_oot3d_runtime_reconstructed.cpp`)
- **证据基础**：结合 `noclip.website/src/OcarinaOfTime3D/{zar.ts, zsi.ts, cmb.ts, csab.ts}` 与 `ctrtool`/`n3dsdisasm` 逆向观测：
  - 还原了 `ZAR\x01` 归档头部（`0x20` 字节含文件数量、类型表、文件表、数据偏移表及 `queen`/`jenkins` 内部代号）与 `ZarArchive::parse()` 解析器；
  - 定义了 `ZsiCommand`、`CmbHeader`（`"cmb\0"` 版本 `6`）与 `CsabHeader`（`"csab"` 骨骼动画时长与节点数）二进制结构。

### 2.4 Ocarina-Atlas `:6545` RPC 桥接器与 PopTracker 自动追踪引擎 (`ocarina_atlas_rpc_and_poptracker_reconstructed.cpp`)
- **发行版实测证据**：从 `OcarinaAtlas-Windows-x64/app/standalone-server.mjs` 直接观测到：
  - `:6545` TCP JSON-RPC 客户端（换行分隔 JSON、最大 24 条队列、`timeoutMs = 800`、`setNoDelay(true)`）；
  - `caption.sav` / `progress.sav` 二进制属性哈希表格式（40 字节头部后接 `[u32 hash, u32 val/offset]` 对：黄金蜘蛛计数 `3700692099`、偏航角 `2899040791`、三维坐标 `2334548366` 与 `savedHashes` 集合 `2749067540`）。
- **协同还原**：结合 `PopTracker/src/luaconnector/luaconnector.h` 的 `IAutotrackProvider` 内存监视缓存接口，实现了 `parse_caption_sav()` 与 `WatchCacheProvider`。
