# 第二轮增量扫描：GTA 系列周边桌面与 Web 应用架构及 REA 逆向报告

> **工单编号**：`emrpg/gta6-oot-rea-reverse-engineering#6`  
> **扫描范围**：排除第一轮已收录的 10 个工具，新增克隆 **11 个** GTA 周边开源桌面/Web/插件仓库至 `/root/wsl-demo/gta-peripherals/round2-desktop-web-apps/`，并结合 `/usr/local/bin/rea` 对托管 DLL 与 JS 构建应用执行静态取证与代码还原。

---

## 1. 第二轮增量项目清单（11 个全新项目）

| 分类 | 项目名称 | 技术栈 | 核心定位与架构特点 |
| :--- | :--- | :--- | :--- |
| **OpenIV 包格式与构建器** | `OpenIV-PackageFormat` | XML (`assembly.xml` v2.x) | OpenIV `.oiv` 自动安装包的官方格式规范，支持档案增删改与 XPath XML 注入 |
| | `MagicOGK-OIV-Builder` | C# WinForms | 可视化 `.oiv` 包创建、解包、编辑与重打包桌面应用 |
| **Gameconfig / DLCList** | `pnwparksfan/gameconfig` | XML / `.oiv` (`install.xml`) | 按 GTA V 版本号（2802–3889）组织的多版本 `gameconfig.xml` 内存池扩展包 |
| **Timecycle 视觉调校** | `gtav-timecycle-editor` | C++ / ScriptHookV | 游戏内实时 Timecycle 参数修改菜单 |
| | `fxReloader` | C++ / ASI (Legacy + Enhanced) | 支持 GTA V Legacy 与 Enhanced 的图形文件（`visualsettings.dat` / `w_*.xml` / `timecycle_mods_*.xml`）零重启热重载器 |
| **Ped/Vehicle 生成与菜单** | `MenyooSP` | C++ / ScriptHookV | 单机全能 Trainer（Spooner 场景生成、Ped 衣柜、载具生成与调校） |
| | `vMenu` | C# / MenuAPI / `Newtonsoft.Json` | FiveM 服务端权限控菜单，含完整玩家/载具/天气/衣柜子系统 |
| **FiveM NUI 手机插件** | `npwd-app-template` | TypeScript, React, Vite IIFE | NPWD 外部 App 官方模板，构建 `__npwd_ext_<id>` IIFE 包共享宿主 React/SDK 运行时 |
| **AWC / Ogg 电台与音频** | `AWCMaster` | Python, ElementTree | 将音频轨道数据构建为 CodeWalker 可识别的 `.awc.xml`、`.dat54.xml` 与 `.dat54.nametable` |
| | `native-audio-tool` | Node.js, `fluent-ffmpeg`, `ffprobe` | 全自动将 MP3/WAV/OGG 转码并生成 `simple`/`radio` 模式的 AWC + DAT54/DAT151 REL 资源 |
| | `aux-radio` | C# (.NET), SMTC, ScriptHookVDotNet | 通过 Windows 媒体会话（SMTC）将 Spotify/VLC 等外部播放器无缝映射为 `RADIO_02_POP` 游戏内电台 |

---

## 2. REA 静态取证结果 (`rea-reconstructions/round2-gta-apps/rea-evidence/`)

1. **托管程序集取证（`vMenu/dependencies/legacy/Newtonsoft.Json.dll`）**：
   - `rea inspect-managed-artifact` 实测确认程序集版本 `11.0.0.0`、目标框架 `.NETPortable,Version=v4.0,Profile=Profile328`、公钥 Token `30ad4fe6b2a6aeed`（SHA-256 前缀 `ceb17dfe5743`），`audit-release-exposure` 确认无伴随 `.pdb` 泄漏。
2. **JavaScript 应用图谱分析（`native-audio-tool` & `npwd-app-template`）**：
   - `rea analyze-javascript-application` 对两个 Node/Vite 工程完成了依赖文件清点与静态图谱摘要（确认两者均为纯 CLI/IIFE 库构建，无 Electron `BrowserWindow` 或 `contextBridge` 暴露面）。
3. **GTA V Enhanced 运行时逆向发现（`fxReloader/FINDINGS.md`）**：
   - 揭示 `GTA5_Enhanced.exe` 磁盘 `.text` 段采用类 Arxan 原位加密，必须通过 `pe-sieve64.exe /dmode 3 /imp 3` 从运行态内存转储后导入 Ghidra；并锁定了 `CTimeCycle::Update` 线程安全封送（Thread Marshaling）与 `CVisualSettings::LoadAll()` 重载原语。

---

## 3. 第二轮源码级还原产出 (`/root/wsl-demo/rea-reconstructions/round2-gta-apps/`)

1. **`oiv_assembly_and_dlclist_merger_reconstructed.cs`**：实现 `.oiv` ZIP 归档中 `assembly.xml` 的解析与 `dlclist.xml` `<Paths>` 节点的幂等去重注入。
2. **`fxreloader_timecycle_hotreload_reconstructed.cpp`**：实现跨线程原子位掩码（`RELOAD_VISUAL_SETTINGS` / `RELOAD_WEATHER_CYCLES` / `RELOAD_MODIFIERS`）与 `CTimeCycle::Update` 主模拟线程安全重载调度器。
3. **`awc_dat54_builder_reconstructed.cpp`**：实现 `.dat54.nametable` 与 `<AudioWaveContainer>` XML 元数据生成器。
