# 基于 REA 的 GTA 闭源与预编译二进制深度逆向及源码重构报告

> **工单关联**：`emrpg/gta6-oot-rea-reverse-engineering#1` (`[REA-GTA-Native]`)  
> **分析工具**：`/usr/local/bin/rea` (`rea-agents@6.9.0`)  
> **证据归档目录**：`/root/wsl-demo/rea-artifacts/gta/evidence/`  
> **源码重构目录**：`/root/wsl-demo/rea-reconstructions/gta-binaries/`

---

## 一、 逆向目标与 REA 取证清单

本次任务对本地及 GitHub Releases 获取的 **7 个** GTA 生态核心闭源/预编译二进制制品进行了系统化静态取证：

| 二进制制品 | 格式 / 架构 | 大小 | 来源 | 核心 REA 分析命令 |
| :--- | :--- | :--- | :--- | :--- |
| **`ScriptHookVDotNet.asi`** | PE32+ (AMD64 混合模式 C++/CLI) | 152,064 B | GitHub Release (`shvdn`) | `inspect-pe-imports`, `inspect-pe-load-config`, `inspect-managed-native-boundaries`, `audit-release-exposure` |
| **`ScriptHookVDotNet3.dll`** | PE32+ (.NET CLI v4.0.30319) | 1,055,744 B | GitHub Release (`shvdn`) | `inspect-managed-artifact`, `inspect-managed-members`, `inspect-managed-native-boundaries` |
| **`GTAIV.EFLC.FusionFix.asi`** | PE32 (x86 32-bit) | 14,526,680 B | GitHub Release (`FusionFix`) | `inspect-pe-imports`, `inspect-pe-resources`, `inspect-pe-load-config`, `inspect-pe-certificates`, `inspect-binary-hardening`, `audit-release-exposure` |
| **`XLivelessAddon.asi`** | PE32 (x86 32-bit) | 232,960 B | GitHub Release (`FusionFix`) | `inspect-pe-imports`, `inspect-native-exports`, `inspect-binary-hardening` |
| **`saat_sfx.exe`** | PE32 (x86 MinGW/GCC) | - | 本地 `San-Andreas-Audio-Toolkit` | `inspect-pe-imports`, `inspect-pe-resources`, `inspect-binary-hardening`, `audit-release-exposure` |
| **`saat_stream.exe`** | PE32 (x86 MinGW/GCC) | - | 本地 `San-Andreas-Audio-Toolkit` | `inspect-pe-imports`, `inspect-binary-hardening` |
| **`System.Xml.Mono.net.dll`** | PE32 (.NET CLI) | - | 本地 `fivem-handling-editor/dist` | `inspect-managed-artifact`, `inspect-managed-members`, `inspect-managed-native-boundaries` |

---

## 二、 关键二进制 REA 静态取证结果剖析

### 1. `ScriptHookVDotNet.asi` & `ScriptHookVDotNet3.dll`（混合模式 CLR 桥接架构）
- **文件指纹**：
  - `ScriptHookVDotNet.asi` SHA-256：`c5461f63b5844a19945e72667ff0dd629a232a341b97910d7106405a5329f50e`
  - `ScriptHookVDotNet3.dll` SHA-256：`f328a36445c1d3e282e70b2791efc0d516ae3009d0d1461bf076893805a01cd9`（MVID: `6cc7ddfb-691c-4a68-ad59-29fdced17d39`）
- **Native-Managed 边界（`rea inspect-managed-native-boundaries`）**：
  - REA 实证 `ScriptHookVDotNet.asi` 为混合模式 C++/CLI 模块（`mixed_mode_or_native_header: True`），包含 **14 个 P/Invoke 导入**与 **156 个原生方法实现**。
  - **完整的 `ScriptHookV.dll` C++ Mangled ABI 导入表**（由 `inspect-pe-imports` 与 `inspect-managed-native-boundaries` 联合还原）：
    - 脚本生命周期：`?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z`、`?scriptUnregister@@YAXPEAUHINSTANCE__@@@Z`、`?scriptWait@@YAXK@Z`
    - 键盘钩子：`?keyboardHandlerRegister@@YAXP6AXKGEHHHH@Z@Z`、`?keyboardHandlerUnregister@@YAXP6AXKGEHHHH@Z@Z`
    - RAGE Native 调用原语：`?nativeInit@@YAX_K@Z`、`?nativePush64@@YAX_K@Z`、`?nativeCall@@YAPEA_KXZ`
    - 纹理绘制与全局指针：`?createTexture@@YAHPEBD@Z`、`?drawTexture@@YAXHHHHMMMMMMMMMMMM@Z`、`?getGameVersion@@YA?AW4eGameVersion@@XZ`、`?getGlobalPtr@@YAPEA_KH@Z`
  - **纤程调度（Fiber Scheduling）与 CLR 宿主**：导入 `KERNEL32.dll` 的 `CreateFiber`、`SwitchToFiber`、`DeleteFiber` 实现脚本协程时间片切换，并通过 `mscoree.dll!_CorDllMain` 初始化 .NET CLR。
- **LoadConfig 与安全加固（`rea inspect-pe-load-config`）**：
  - 启用了 `IMAGE_GUARD_CF_INSTRUMENTED`（CFG）与 `guard_xfg_check_function_pointer`（`0x18000f160`）。
- **托管元数据统计（`rea inspect-managed-members`）**：
  - `ScriptHookVDotNet3.dll` 包含 **303 个 `TypeDef`**、**12,504 个 `Field`**、**4,099 个 `MethodDef`**、**12,295 个 `Constant`**，REA 完整解码了全部方法签名（如 `GTA.Audio` 的 `PlayMusic`、`PlaySoundAt`、`PlaySoundFrontend`、`SetAudioFlag` 等）。

### 2. `GTAIV.EFLC.FusionFix.asi` & `XLivelessAddon.asi`（RAGE IV 注入与资源打包）
- **文件指纹**：
  - `GTAIV.EFLC.FusionFix.asi` SHA-256：`14f6ac879bcc7df5fb18ce7b410bc436e16675b0dad640378dcb1e6dd3be98b3`
- **PE 结构、加固与数字签名**：
  - `inspect-binary-hardening` 与 `inspect-pe-load-config` 观测到：`dynamic_base`（ASLR, `DllCharacteristics & 0x40`）已启用，启用 `IMAGE_GUARD_CF_INSTRUMENTED`，包含 **813 个 SEH 异常处理程序**（`se_handler_count: 813`）及独立 TLS 目录（`.rdata` 起始 `0x102c14fc`）。
  - `inspect-pe-certificates` 观测到文件末尾（偏移 `14519288`，长度 `7392` 字节）嵌入了 Authenticode 签名表。
- **内嵌资源（`rea inspect-pe-resources`）**：
  - 共打包 **42 个 `RT_RCDATA`（类型 ID=10）二进制资源**（ID `101..`），对应预编译着色器包、字体与默认配置。
- **发行暴露面审计（`rea audit-release-exposure`）**：
  - 检出 `native.pdb_path: exposed` 告警，暴露了 GitHub Actions CI 构建路径：`D:\a\GTAIV.EFLC.FusionFix\GTAIV.EFLC.FusionFix\bin\GTAIV.EFLC.FusionFix.pdb`。
  - 对 `XLivelessAddon.asi` 提取字符串同样观测到构建 PDB 路径：`Z:\GitHub\xlivelessaddon\Release\plugins\XLivelessAddon.pdb`，并结合其 `KERNEL32`/`USER32`/`ADVAPI32` 导入还原了 `CustomSavePath`、`DoNotPauseOnMinimize` 与 `VirtualProtect` 内存补丁接口。

### 3. `saat_sfx.exe` & `saat_stream.exe`（GTA:SA 音频归档工具链）
- **编译器与运行时特征（`rea inspect-pe-imports`）**：
  - 两者均为 32 位 MinGW/GCC 编译的 PE32 可执行文件，仅依赖 `KERNEL32.dll`（`CreateFileA`、`CreateSemaphoreA`、`FlushFileBuffers` 等）与 `msvcrt.dll`（双导入表特征：`__getmainargs`、`_open`/`_read`/`_lseek`/`_mkdir`）。
- **静态字符串与逻辑边界还原**：
  - `saat_sfx.exe`（SAAT SFX Tool v1.10）静态链接了 **`libsndfile`**（`SF_INFO`、`SF_VIRTUAL_IO`、`psf_seek()`），用于将 GTA:SA SFX 归档与 `Banklkup` 索引在 RIFF/WAVE 之间双向转换；限制 `[Archive] lookup_index` 范围 `[0-8]`、`num_banks` 范围 `[1-400]`。
  - `saat_stream.exe` 解析包含 **`OggS`** 魔数的 Ogg Vorbis 音频流，支持 `-e`（导出）、`-r`（带电台元数据导出）与 `-i`（导入）三种模式；校验 `[Stream] lookup_index` 为 `[0-1]` 或 `[3-16]`，并强制解析 `Track_N.beat_total` 与以 **`33`** 作为结束标记的节拍控制序列。

### 4. `System.Xml.Mono.net.dll`（FiveM 托管 XML 运行时）
- `rea inspect-managed-members` 检出 **1,682 个 `TypeDef`**（覆盖 `MS.Internal.Xml.XPath.*` 等完整 XPath/XML 解析器）。
- `rea inspect-managed-native-boundaries` 检出 **3 个 P/Invoke 导入**（`kernel32.dll` 的 `GetCurrentPackageId`、`GetProcAddress`、`GetModuleHandle`）与 **107 个 runtime/native 实现方法**。

---

## 三、 源码重构产出清单（`/root/wsl-demo/rea-reconstructions/gta-binaries/`）

1. **`shvdn/ScriptHookVDotNet_NativeBridge.hpp` & `.cpp`**：完整还原 `ScriptHookV.dll` 的 12 个 C++ Mangled ABI 导入签名、Fiber 协程调度循环与 `_CorDllMain` 混合模式入口。
2. **`shvdn/ScriptHookVDotNet3_AudioApi.cs`**：依据 `shvdn3_members.json` 精确方法签名还原 `GTA.Audio` 类与 `GTA.AudioFlags` 枚举接口。
3. **`saat/saat_reconstructed.hpp` & `saat_cli_reconstructed.cpp`**：完整重构 `saat_sfx` 与 `saat_stream` 的 CLI 分发、INI 边界校验常量（`0..8`、`1..400`、`BEAT_END_TOKEN = 33`）与 `Banklkup`/Ogg 流处理骨架。
4. **`fusionfix/FusionFix_And_XLiveless_Interface.hpp`**：整理 `GTAIV.EFLC.FusionFix.asi` 的 `RT_RCDATA` 资源表、`LoadConfig` 安全特征及 `XLivelessAddon.asi` 的 `VirtualProtect` 补丁/配置接口。
