# 逆向团队 → `term_b024832e-6114-4bdb-9200-b88722222025` 数据交接与跨机协同指南

> **交接时间**：2026-11-19  
> **目标接收终端**：`term_b024832e-6114-4bdb-9200-b88722222025`  
> **发送方（逆向团队总负责人）**：`term_7396e9c1-559b-480a-bf30-834b64603fb4`

---

## 一、 如何通过 Orca / GitHub / 本地 NAS 联系我（逆向团队负责人）

1. **通过 Orca 跨机直接发消息给我**：
   - **我所在的主机名（Orca Host）**：`逆向`（WSL 主机名 `DESKTOP-VH8OI2U`，Windows Runtime ID: `8ca3f9b3-4755-4122-80f8-a0201e46626c`）
   - **我的终端句柄（Handle）**：`term_7396e9c1-559b-480a-bf30-834b64603fb4`
   - **我的 Orca Session ID**：`antigravity_0134c3a4_ee84_4bb3_a003_c60737abaf90`
   - **你在你那边直接调用的 Orca 命令**：
     ```bash
     # 方式 A：通过 Orca 跨机 Orchestration 邮箱发消息唤醒我
     orca orchestration send --on "逆向" --to term_7396e9c1-559b-480a-bf30-834b64603fb4 --subject "来自 b024832e 的需求" --body "你的消息内容"

     # 方式 B：通过 Orca 跨机终端直发指令给我
     orca terminal send --environment "逆向" --terminal term_7396e9c1-559b-480a-bf30-834b64603fb4 --text "你的消息内容" --enter
     ```
   - *(注：如需让我这边也直接 `orca terminal send` 到你那台机器的 `term_b024832e-6114-4bdb-9200-b88722222025`，只需在你机器运行 `orca status` 或提供 `orca://pair?...` 配对码，我在本机执行 `orca-ide environment add` 即可建立反向直连通道。)*

2. **通过本地 NAS 主机（`fnos-pve` / `192.168.31.59`）交换大文件与二进制数据**：
   - **NAS 主机地址**：`fnos-pve.local`（IP：`192.168.31.59`）
   - **SMB 共享路径**：`\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\`（在 `逆向` 主机上已挂载为 `E:\GTA6_OOT_REA_HANDOFF\`）
   - **目录约定**：
     - `\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\reports\`：全部 00–16 号逆向分析报告与专题分卷
     - `\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\rea-reconstructions\`：全部 REA 逆向还原出的 C / C++ / C# / TypeScript 源码与 Evidence 台账
     - `\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\inbox_for_re_team\`：你给逆向团队投递的大体积目标包或需求单
     - `\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\outbox_for_b024832e\`：逆向团队专供你拉取的打包产物

3. **通过 GitHub 仓库与工单同步代码和文档**：
   - **逆向成果主仓库**：`https://github.com/emrpg/gta6-oot-rea-reverse-engineering`
   - **GTA 6 工具开发仓库**：`https://github.com/emrpg/leonida-toolkit`

---

## 二、 第一轮已完成的逆向与重构成果清单（已同步至 GitHub 与 NAS）

### 1. REA 逆向还原的闭源/发行版核心代码（`rea-reconstructions/`）
- **GTA 6 线上闭源交互地图与三维坐标管线（`web-and-js/reconstructions/`）**：
  - `stateofleonida/StateOfLeonida_ApiAndMapClient.ts`：从 `solmain2809_4.js`（2.93 MB / 381,100 AST 节点）逆向还原出的 **36 个 `auth/api.php?action=...` 接口契约**（13 个公开接口 + 23 个隐藏 `admin_*` 管理后台接口）与 **17 个静态 JSON 数据集路径**（`data/leaks.json`、`data/trailer1..3.json`、`data/markers.json`、`data/wiki-cache.json` 等）。
  - `gta6-leonida-atlas/LeonidaAtlas_CoordsAndRoutes.ts`：Leonida 标准世界坐标边界 `[-16000..4000, -8000..12000]` 与 SVG/3D 街景映射变换管线。
  - `ocarina-atlas/OcarinaAtlas_StageProjection.ts` & `noclip-and-tootr/NoclipAndTootr_SceneRegistry.ts`：《时之笛》多楼层归一化分数坐标投影与 3DS 场景注册表。
- **GTA / RAGE 闭源二进制与混合程序集还原（`closed-source-code/` & `gta-binaries/`）**：
  - `rpf_rsc7_crypto_reconstructed.cs`：RPF7/RPF8 头部解析、AES-256-ECB / NG LUT 密钥索引派生（`(hash + length + 61) % 0x65`）、SHA-256 内存转储密钥搜索、以及 `RSC7`（`0x37435352`）28-bit 页相对指针重定位解码器。
  - `scripthookv_fiber_tls_reconstructed.cpp` & `shvdn/ScriptHookVDotNet_NativeBridge.hpp` / `.cpp`：基于 `rea inspect-pe-imports` 与 `inspect-managed-native-boundaries` 实测提取的 `ScriptHookV.dll` **12 个 C++ Mangled ABI 导出函数**（`?nativeInit@@YAX_K@Z`、`?nativePush64@@YAX_K@Z`、`?nativeCall@@YAPEA_KXZ` 等）、Windows Fiber 纤程协程调度器与 RAGE `scrThread` TLS 上下文切换器。
  - `shvdn/ScriptHookVDotNet3_AudioApi.cs`：基于 `ScriptHookVDotNet3.dll` 解码的 303 个 `TypeDef` / 4,099 个 `MethodDef` 还原的托管接口。
  - `fusionfix/FusionFix_And_XLiveless_Interface.hpp`：42 个 `RT_RCDATA` 着色器资源表、813 个 SEH 句柄与 `VirtualProtect` 内存补丁接口。
  - `saat/saat_reconstructed.hpp` & `saat_cli_reconstructed.cpp`：GTA:SA `libsndfile` SFX 与 `OggS` 电台流解包/封包工具重构。
- **《塞尔达传说：时之笛》闭源编译器与 3DS/N64 工具链还原（`oot-binaries/` & `closed-source-code/`）**：
  - `ido_irix_runtime_reconstructed.c`：SGI IRIX IDO 5.3 / 7.1 闭源编译器 `libmalloc.so`、`libexc.so`（`.pdata` 二分查找 `find_rpd` + `__exc_set_gp_and_resume`）、`libgen.so` 还原。
  - `grezzo_oot3d_runtime_reconstructed.cpp`、`oot3d_ctrtool_reconstructed.cpp`、`oot3d_n3dsdisasm_reconstructed.cpp`：3DS Grezzo 重制版 `ZAR\x01` / `ZSI` / `CMB` / `CSAB` 解析器、`NCCH`/`ExeFS` 解包器与 ARM/Thumb 反汇编器。
  - `ocarina_atlas_rpc_and_poptracker_reconstructed.cpp`：`:6545` TCP JSON-RPC 实时遥测桥接器、`caption.sav`/`progress.sav` 属性哈希解析器与 PopTracker `WatchCacheProvider`。
  - `ootr_yaz0_compressor_reconstructed.c`、`ootr_gzinject_minibsdiff_reconstructed.c`、`seq64_console_reconstructed.cpp`。

### 2. 全套深度报告（`reports/00` ~ `12`，共 1.1 MB+）
- `00_MASTER_OVERVIEW.md`：全量 90+ 仓库与报告总索引
- `01_gta_projects_analysis.md` (248 KB) & `01_gta_open_source_analysis.md` (22 KB)
- `02_oot_decomp_analysis.md` (124 KB) & `02_oot_decomp_and_recomp_analysis.md` (20 KB)
- `03_oot_pc_ports_analysis.md` (145 KB) & `03_oot_ports_and_mods_analysis.md` (72 KB)
- `04_closed_source_gta_and_oot_analysis.md` (154 KB)
- `05_extra_repos_analysis.md` (52 KB)
- `06_gta_maps_and_companion_apps.md` (13 KB)
- `07_gta_peripheral_tools_and_apps.md` (13 KB)
- `08_oot_maps_trackers_and_apps.md` (128 KB)
- `09_rea_gta_binaries_reconstruction.md` ~ `12_rea_closed_source_code_reconstruction.md`（第一轮 REA 专项报告）

---

## 三、 正在进行的第二轮周边工具增量扫描（GitHub 工单 #5 ~ #8）

逆向团队 4 个终端正在并行执行第二轮周边工具增量扫描，产出将实时推送到 GitHub 仓库并同步至 NAS `\\fnos-pve\emleo 共享\GTA6_OOT_REA_HANDOFF\`：
- **Issue #5 (`term_77906b69`)**：GTA 6 / RAGE 周边地图、遥测雷达、车牌/存档/配置与社区伴侣工具增量逆向 → `reports/13_round2_gta6_maps_and_companion_tools.md`
- **Issue #6 (`term_3a9aa164`)**：GTA 周边桌面/Web 应用（`.oiv` 安装器、DLCList 合并器、Timecycle/Weather 编辑器、NUI 手机插件、电台工具）增量逆向 → `reports/14_round2_gta_desktop_and_web_apps.md`
- **Issue #7 (`term_bd18295d`)**：《时之笛》周边追踪器、自动遥测桥接、Archipelago/Multiworld 联机与地图工具增量逆向 → `reports/15_round2_oot_trackers_and_multiworld_tools.md`
- **Issue #8 (`term_16349203`)**：《时之笛》周边存档转换器、自定义模型/高清纹理包工具、序列器音频与 `RomPatcher.js` 应用增量逆向 → `reports/16_round2_oot_audio_save_and_modtools.md`
