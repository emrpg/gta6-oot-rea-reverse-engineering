# GTA 6 / GTA 系列与《塞尔达传说：时之笛》开源及闭源项目全量研究总览（Master Index）

> **生成时间**：2026-11-18  
> **本地源码库规模**：共克隆 **90+ 个** 核心引擎、逆向反编译、静态重编译、PC 移植版、地图测绘、追踪器与周边工具仓库（分布于 `/root/wsl-demo/` 与 `/root/orca-workspaces/`）  
> **分析报告总规模**：共 **10 份主报告 + 21 份专题分卷**，总计超过 **870 KB** 源码级中文深度分析。

---

## 一、 核心结论速览

1. **闭源引擎的四层可及性差异（L1 容器格式 / L2 内存类布局 / L3 运行时 API / L4 源码与构建系统）**：
   - **《塞尔达传说：时之笛》（N64 1998 / 3DS 2011）**：L1–L4 **四层全暴露**。本地 [`zeldaret/oot`](file:///root/wsl-demo/oot-decomp-projects/oot)（`src/` 452,643 行 C 代码，`NON_MATCHING` 仅余 2 处）已将任天堂 EAD 原版引擎的 513 个结构体、8 个常驻 Overlay + 453 个动态 Actor/Effect Overlay 完整还原；3DS Grezzo 闭源重制版（OoT3D）也在 [`TriAevum`](file:///root/wsl-demo/oot-port-projects/TriAevum)（AOT 静态重编译）与 [`noclip.website`](file:///root/wsl-demo/oot-peripherals/noclip.website)（WebGL `ZAR`/`ZSI`/`CMB`/`CSAB` 解析器）中被深度拆解。
   - **GTA V / IV / SA（RAGE 与 RenderWare 时代）**：GTA III/VC/SA 达到 L1–L4 暴露（[`re3`](file:///root/wsl-demo/extra-repos/gta/re3) 23.1 万行、[`gta-reversed`](file:///root/wsl-demo/gta-projects/gta-reversed) 42.7 万行、[`openrw`](file:///root/wsl-demo/gta-projects/openrw) 5 万行）；GTA V 达到 **L1–L3 三层暴露**（[`CodeWalker`](file:///root/wsl-demo/gta-projects/CodeWalker) 41 类 RPF7/RSC7 解析器、[`fivem`](file:///root/wsl-demo/gta-projects/fivem) `rage::fiDevice` 虚表与 `netSyncTree` OneSync 状态树、[`scripthookvdotnet`](file:///root/wsl-demo/gta-projects/scripthookvdotnet) 6,460 个 64-bit Native 哈希）。
   - **GTA 6（GTA VI · RAGE 新世代）**：由于主机版（PS5/Xbox Series X|S）包体加密且无 PC 版，L1–L4 二进制层**零暴露**（预载 PlayGo 元数据显示 212 GB 包体分 32 个数据块，可执行主程序仅约 187 MB，含两套 12 GiB 独立光追 BVH 数据集 `bvh4`/`bvh8` 与 60 万+ 实时重定向骨骼动画）。当前开源生态集中在 **L0 外部观测与摄影测量层**（[`gtamaplib`](file:///root/wsl-demo/gta-projects/gtamaplib)、[`gtamaplib-vc`](file:///root/wsl-demo/gta-peripherals/maps-and-apps/gtamaplib-vc)、[`gta6-leonida-atlas`](file:///root/wsl-demo/gta-projects/gta6-leonida-atlas)、[`gta6map.github.io`](file:///root/wsl-demo/gta-peripherals/maps-and-apps/gta6map.github.io)）。

---

## 二、 主报告导航索引（`/root/wsl-demo/reports/`）

| 编号 | 报告文件 | 体积 | 负责模块与核心内容 |
| :--- | :--- | :--- | :--- |
| **01A** | [01_gta_projects_analysis.md](file:///root/wsl-demo/reports/01_gta_projects_analysis.md) | **248 KB** | **GTA 6 闭源架构 & GTA 系列核心开源项目全量深度长卷**（含 `_parts/00`–`90` 共 15 个分卷：OpenRW、re3/reVC、ScriptHookVDotNet、CodeWalker、MTA:SA、CitizenFX/FiveM、librw、GTA IV FusionFix、Cfx Natives & txAdmin、GTA 6 闭源架构与 Take-Two 法务边界） |
| **01B** | [01_gta_open_source_analysis.md](file:///root/wsl-demo/reports/01_gta_open_source_analysis.md) | **22 KB** | **GTA 10 大本地开源仓库架构精要报告**（`gta6-leonida-atlas`、`gtamaplib`、`openrw`、`gta-reversed`、`fivem`、`mtasa-blue`、`CodeWalker`、`Sollumz`、`GTAIV.EFLC.FusionFix`、`scripthookvdotnet`） |
| **02A** | [02_oot_decomp_analysis.md](file:///root/wsl-demo/reports/02_oot_decomp_analysis.md) | **124 KB** | **《塞尔达传说：时之笛》逆向反编译与静态重编译超详尽源码报告**（`zeldaret/oot`、`oot3d`、`oot-gc`、`oot-vc`、`N64Recomp`、`N64ModernRuntime`、`Zelda64Recomp`、`ZAPD`、`fast64`、`OoT-Randomizer`、`HackerOoT` 等 18 个仓库） |
| **02B** | [02_oot_decomp_and_recomp_analysis.md](file:///root/wsl-demo/reports/02_oot_decomp_and_recomp_analysis.md) | **20 KB** | **《时之笛》Actor 引擎、Overlay 重定位、MIPS-to-C 静态重编译与 RT64 架构精要** |
| **03A** | [03_oot_pc_ports_analysis.md](file:///root/wsl-demo/reports/03_oot_pc_ports_analysis.md) | **122 KB** | **《时之笛》PC 原生移植、重编译与掌机 AOT 引擎深度报告**（含 `oot3_00`–`05` 共 6 个分卷：`Shipwright`、`2ship2harkinian`、`libultraship`、`Torch`、`ZAPDTR`、`Zelda64Recomp`、`rt64`、`TriAevum`、`Ocarina-ReTuned`、`oot3d_practice_menu`） |
| **03B** | [03_oot_ports_and_mods_analysis.md](file:///root/wsl-demo/reports/03_oot_ports_and_mods_analysis.md) | **72 KB** | **《时之笛》Fast3D 现代图形渲染后端、OTR/O2R 资产格式与 C/C++ 桥接专项报告** |
| **04** | [04_closed_source_gta_and_oot_analysis.md](file:///root/wsl-demo/reports/04_closed_source_gta_and_oot_analysis.md) | **154 KB** | **闭源项目深度调研与对比报告：《时之笛》vs《GTA 6 / GTA 系列》**（四层可及性模型、任天堂 EAD 原版引擎、iQue 神游机、GC 大师任务、Grezzo 3DS 重制版、Switch NSO 模拟器、CryZENx UE5 重制版 vs Rockstar RAGE 演进、GTA 6 PlayGo 分块、OpenIV `.black`、RageMP/alt:V/FiveM 闭环，以及推翻二手误传的 12 条实证勘误） |
| **05** | [05_extra_repos_analysis.md](file:///root/wsl-demo/reports/05_extra_repos_analysis.md) | **52 KB** | **GTA 与《时之笛》16 个补充开源工具链源码剖析**（`re3`、`open.mp`、`gta-toolkit`、`SampSharp`、`WidescreenFixesPack`、`YimMenuV2`、`m2c`、`splat`、`decomp-permuter`、`decomp-toolkit`、`GLideN64`、`mupen64plus-core`、`mm`、`OoT3D_Randomizer`、`Ocarina-Atlas`、`wwrando`） |
| **06** | [06_gta_maps_and_companion_apps.md](file:///root/wsl-demo/reports/06_gta_maps_and_companion_apps.md) | **13 KB** | **GTA 6 及 GTA 系列地图、制图测绘与伴侣 App 专项报告**（`gtamaplib`、`gtamaplib-vc` 稀疏束调整与零依赖 WebGL 3D 查看器、`gta6map.github.io` 1m/px 金字塔切图、`gta6-leonida-atlas`、`gta-vi-beyond`、`live_map`/`webmap`/`TrackM`；闭源 DuPz0r Mapping Project、`vimap`、`GTAWeb.eu`、`MapGenie`；官方 `iFruit` 停服剖析与 `RDR2 Companion App` 局域网实时第二屏幕遥测协议） |
| **07** | [07_gta_peripheral_tools_and_apps.md](file:///root/wsl-demo/reports/07_gta_peripheral_tools_and_apps.md) | **13 KB** | **GTA 系列桌面与 Web 周边应用源码及对比报告**（`txAdmin` WebPipe 架构、`project-error/npwd` 游戏内 React 微前端智能手机、`gta5view` Snapmatic 解析、`gtasa-savegame-editor`、`GTAVHandlingEditor` & `fivem-handling-editor`、`GTA-V-Enhanced-Mod-Manager`、`modloader`、`SAAT` + `RadioSanAndreas` Icecast 电台流媒体、`GXT-Extractor`；对比闭源 `XB36Hazard Save Editor`、`Lenny's Mod Loader`、`Bilago Mod Manager`） |
| **08** | [08_oot_maps_trackers_and_apps.md](file:///root/wsl-demo/reports/08_oot_maps_trackers_and_apps.md) | **11 KB** | **《塞尔达传说：时之笛》周边地图、追踪器与特色应用源码报告**（`noclip.website` WebGL 3DS `ZAR/ZSI/CMB/CSAB` 3D 场景渲染器、`Ocarina-Atlas` `:6545` 实时内存遥测地图、`OoT-Interactive-Map`、`OoT_MapExt`、`tootr` 图可达性引擎、`Track-OOT`、`hyrule-tracker`、`poptracker`、`EmoTrackerPacks`、`OoTMMR_tracker_pack`、`oot-remake-rando`、`oot3d_practice_menu`、`ocarina`/`OcarinaPlayer`/`ocarina-songbook`/`OoTROcarinaMapper`、以及 Flipper Zero 硬件陶笛外设 `flipparina`；对比闭源 `MapGenie`、`Zelda Dungeon`、`EmoTracker`） |

---

## 三、 本地克隆仓库目录结构总表

- [`/root/wsl-demo/gta-projects/`](file:///root/wsl-demo/gta-projects)（10 个核心 GTA 仓库）
- [`/root/orca-workspaces/gta-repos/`](file:///root/orca-workspaces/gta-repos)（9 个 GTA 仓库，含 `citizenfx-natives`、`txAdmin`、`librw`）
- [`/root/wsl-demo/gta-peripherals/maps-and-apps/`](file:///root/wsl-demo/gta-peripherals/maps-and-apps)（6 个 GTA 6 / FiveM 地图与伴侣前端仓库）
- [`/root/wsl-demo/gta-peripherals/tools-and-apps/`](file:///root/wsl-demo/gta-peripherals/tools-and-apps)（10 个 GTA 手机系统、存档编辑、Handling、电台与 Mod 管理仓库）
- [`/root/wsl-demo/extra-repos/gta/`](file:///root/wsl-demo/extra-repos/gta)（6 个 GTA 补充仓库，含 `re3`、`open.mp`、`gta-toolkit`）
- [`/root/wsl-demo/oot-decomp-projects/`](file:///root/wsl-demo/oot-decomp-projects)（18 个《时之笛》反编译、各平台版本与静态重编译仓库）
- [`/root/wsl-demo/oot-port-projects/`](file:///root/wsl-demo/oot-port-projects)（15 个《时之笛》PC 原生移植、渲染引擎与 3DS AOT 重编译仓库）
- [`/root/wsl-demo/oot-peripherals/`](file:///root/wsl-demo/oot-peripherals)（19 个《时之笛》3D/2D 地图、Randomizer 追踪器、陶笛合成器与 Flipper Zero 硬件应用仓库）
- [`/root/wsl-demo/extra-repos/n64-oot/`](file:///root/wsl-demo/extra-repos/n64-oot)（10 个 N64 / 《时之笛》反编译工具链与模拟器基准仓库）
