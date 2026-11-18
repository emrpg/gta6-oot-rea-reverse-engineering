# 《塞尔达传说：时之笛》原生 PC 移植引擎与衍生开源项目技术分析报告

> 报告日期：2026-11-18
> 分析对象根目录：`/root/wsl-demo/oot-port-projects/`（下称 `$P`；工作区内另有 `oot-decomp-projects/`、`oot-ports-repos/`、`extra-repos/n64-oot/` 三处镜像/补充仓库，凡引用会写明绝对路径）
> 分析方法：本地浅克隆仓库源码实读（结构体/枚举/函数级）+ CMake/CI/配置文件核对 + 跨仓库同名文件比对 + GitHub 只读检索补全
> 配套报告：`02_oot_decomp_analysis.md`（反编译与静态重编译生态），本篇聚焦**可运行的 PC/掌机移植引擎、图形后端、资产容器格式与增强特性**

**目录**：[0 范围·清单·核心结论](#0-范围方法与核心结论) · [1 生态谱系](#1-生态谱系与仓库版图) · [2 Shipwright/SoH](#2-ship-of-harkinianshipwright-移植主干) · [3 libultraship](#3-libultraship运行时基座解剖) · [4 资产工具链](#4-资产工具链与容器格式族) · [5 重编译路线](#5-重编译路线zelda64recomp--n64modernruntime--rt64) · [6 掌机 AOT 路线](#6-掌机与二进制-aot-衍生路线) · [7 衍生生态](#7-衍生生态与增强叠加层) · [8 横向对比与选型](#8-横向对比与工程评价) · [附录 A 源文件索引](#附录-a关键源文件索引) · [附录 B 证据说明](#附录-b证据标记说明与复核建议)

---

## 0. 范围、方法与核心结论

### 0.1 本次任务的实际动作

任务给定的 9 个仓库（Shipwright、2ship2harkinian、libultraship、Torch、ZAPDTR、Ocarina-ReTuned、oot3d_practice_menu、TriAevum、retro）在 `$P` 下**均已存在**，但存在两类缺口，本次已补齐：

| 动作 | 对象 | 结果 |
|---|---|---|
| 修复损坏克隆 | `$P/retro` | 原目录 HEAD 指向 `refs/heads/.invalid`、objects 仅 12 KB（工作树为空）。已删除重克隆，现为完整 Flutter/Dart 工程 |
| 新增下载 `--depth 1` | `Zelda64Recomp`、`N64ModernRuntime`、`N64Recomp`、`rt64`、`n64-fast3d-engine`、`OTRExporter`、`neoretro` | 7 个仓库，共约 44 MB。它们是"**原生 PC 移植引擎**"这一命题的核心证据：Zelda64Recomp 是另一条不依赖反编译源码的官方级移植路线，rt64 是其 Fast3D→现代 GPU 后端，OTRExporter/neoretro 是 OTR/O2R 资产链的缺失环节 |

需要事先声明的重要限制：**Shipwright / 2ship2harkinian / TriAevum 的 git 子模块均未初始化**（`git submodule status` 输出以 `-` 开头，目录为空），因此 SoH 侧的 libultraship 代码只能靠"调用点"证据；作为替代，本报告用独立克隆 `$P/libultraship`（`main` @ `759b134`）分析引擎本体，并在 §3.7 明确它与 SoH 所 pin 的 `port-maintenance`（`da6daf1a`）之间的架构差异。

### 0.2 仓库清单与版本基线

| 目录（`$P` 下） | 上游 | HEAD | 规模（源码行/文件） | 角色 |
|---|---|---|---|---|
| `Shipwright` | HarbourMasters/Shipwright | `de1ab1b6`（develop） | 720,798 / 3,166 | **OoT decomp 移植主干**（Ship of Harkinian，版本 9.3.1 "Dewey"） |
| `2ship2harkinian` | HarbourMasters/2ship2harkinian | `e5e9f33`（master） | 801,943 / 3,457 | MM 移植（Ship of Harkinian for MM，5.1.0） |
| `libultraship` | Kenix3/libultraship | `759b134`（main） | 71,969 / 419 | 运行时基座（引擎层），epoch 2 架构 |
| `Torch` | HarbourMasters/Torch | `22d3218` | 177,319 / 665 | 新一代资产抽取/打包器（YAML factory） |
| `ZAPDTR` | HarbourMasters/ZAPDTR | `be1c68a` | 44,804 / 208 | ZAPD 的 HarbourMasters fork（ROM→XML/C/.bin） |
| `OTRExporter` | HarbourMasters/OTRExporter | `c5465ba`（develop） | 6,017 / 59 | ZAPD 的 OTR/O2R 导出**插件** |
| `retro` | HarbourMasters/retro | `43c8e91` | 7,187 / 71 | OTR/O2R 打包 GUI（Flutter/Dart），已宣布停维护 |
| `neoretro` | HarbourMasters/neoretro | `24fa283` | 13,118 / 130（C/C++） | retro 的 C++/CMake 后继者 |
| `Zelda64Recomp` | Zelda64Recomp/Zelda64Recomp | `b65c482` | 45,929 / 154 | **产品级静态重编译**（当前主目标 MM，OoT 计划中） |
| `N64Recomp` | N64Recomp/N64Recomp | `598ce20` | 14,355 / 24 | MIPS→C 重编译器（含 RSPRecomp、LiveRecomp） |
| `N64ModernRuntime` | N64Recomp/N64ModernRuntime | `e4342ae` | 57,186 / 72 | ultramodern + librecomp 运行时 |
| `rt64` | rt64/rt64 | `d9817c6` | 81,519 / 269 | **Fast3D→D3D12/Vulkan/Metal 高精度渲染器** |
| `n64-fast3d-engine` | Emill/n64-fast3d-engine | `881eb68`（2021-10） | 31,006 / 26 | LUS Fast3D 后端的直系祖先（GL/D3D11/D3D12） |
| `TriAevum` | coccofresco/TriAevum | `672d13f` | 882,749 / 3,115 | **OoT3D/MM3D 二进制 AOT→PC**（含 vendored LUS） |
| `Ocarina-ReTuned` | OTPR26/Ocarina-ReTuned | `fecb9f9` | 29,651 / 389 | TriAevum 的下游发行分叉（单屏 ReTuned/ReFramed） |
| `oot3d_practice_menu` | gamestabled/oot3d_practice_menu | `e8bdabe` | 15,789 / 68 | 3DS 实体机/Luma IPS mod（练习菜单） |

许可情况（读 `LICENSE*`/`COPYING` 首部）：`libultraship`、`rt64`、`ZAPDTR`、`Torch` 为 MIT（ZAPDTR 版权行署 "Zelda Reverse Engineering Team"）；`N64ModernRuntime`/`Zelda64Recomp` 用 `COPYING`；`Shipwright`、`oot3d_practice_menu`、`Ocarina-ReTuned` 顶层**未见**独立 LICENSE 文件（SoH 的许可声明散落于源文件头与 README，属"未找到独立许可证文件"状态）。

### 0.3 本报告的八条核心判断

1. **"OoT 原生 PC 移植"在开源世界并非单一技术，而是三个互斥家族**：① **decomp 源码路线**（把 zeldaret/oot 的 C 源码直接编成宿主机原生码，资产离线转成容器包）——代表 Shipwright/SoH、2ship2harkinian；② **静态重编译路线**（把 MIPS 机器码逐指令翻成 C，游戏码一行不改）——代表 Zelda64Recomp + N64ModernRuntime + rt64；③ **二进制 AOT 路线**（把 3DS/Switch2 版标题整程序 ARM→C++ 翻译）——代表 TriAevum、Hookshot、ProjectReLink、oot-pc-recomp。三者的**增强特性可实施范围**差别极大（§8.4）。
2. **SoH 的渲染核心是"既无 RSP 也无 RDP"的 GBI 直译器**：`$P/libultraship/src/fast/interpreter.cpp`（5,546 行）用 CPU 重实现 RSP 的输出口（`GfxSpVertex`/`LoadedVertex`）与 RDP 的寄存器面（`RDP` 结构 + `rdpHandlers`），像素端交给**按 combiner key 特化生成的现代 shader**；它**不建模 TMEM**，`G_RDP*SYNC` 一律 stub（`interpreter.cpp:4686-4718`）。因此它的精度是"状态级近似"，而非像素级 RDP 模拟。
3. **rt64 走的是相反取舍**：保留 `RDP_TMEM_WORDS 512`/8 tile 的显式建模（`rt64/src/hle/rt64_rdp.h:21-30`），把 RSP 变换搬上 **compute shader**（`src/shaders/RSPProcessCS.hlsl`，双帧权重 `prevFrameWeight/curFrameWeight`），用 **ubershader + spec-constant** 把 combiner 当数据传（`src/shaders/RenderParams.hlsli:96-139`），并在片段里做**真 2's-complement 回卷**（`src/shared/rt64_color_combiner.h:210,558-567`）。代价是复杂度与显存，收益是与硬件一致的色彩环绕与逐周期求值。
4. **`.otr` 与 `.o2r` 都不是自研容器**：当前 LUS HEAD 里 `.otr` = **MPQ**（StormLib，默认编译关闭 `INCLUDE_MPQ_SUPPORT`），`.o2r` = **ZIP**（libzip/miniz）。真正跨格式稳定的持久结构只有**每资源 64 字节头**（`OTR_HEADER_SIZE=64`，`include/ship/resource/archive/Archive.h:17`；写出侧 `OTRExporter/OTRExporter/Exporter.cpp:4-21`，magic 槽填 `0xDEADBEEFDEADBEEF`）。索引是运行时用 CRC64(path) 建的 `unordered_map`，**不存在**旧文献常说的"排序索引表 + 二分查找"。O2R 相对 OTR 的真实增益是：可写（`OtrArchive.cpp:108-111` 明确拒绝 `WriteFile`）、无 32 位尺寸上限、免可选依赖、并发读句柄池。
5. **SoH 的"高帧率"其实是两条独立机制**：渲染侧在一个 20 Hz 逻辑 tick 内**重复提交同一条显示列表 N 次**并逐次替换矩阵（`Shipwright/soh/soh/OTRGlobals.cpp:1826-1873` 的 `Graph_ProcessGfxCommands`/`RunCommands` + `FrameInterpolation_Interpolate`），逻辑帧率并未改变；玩法侧的"60 FPS 手感"则是**逐个 bug 打补丁**（`FastDrops`、`FixFlexDrops`、`FixHoldingGoronPot`、`ClimbSpeed`、`FixDaruniaDanceSpeed` 等，经 491 个 `VB_*` vanilla-behavior 开关落地）。重编译路线的做法结构上更干净：`framerateDivisor` 上报给渲染器（`Zelda64Recomp/patches/ui_patches.c:99-100` 的 `gEXSetRefreshRate`），由 rt64 用帧权重插值。
6. **资产管线已经分叉**：Shipwright 已把 ZAPDTR/OTRExporter 换成 **Torch**（`.gitmodules` 只留 `libultraship`+`torch`；14 套 `soh/assets/yml/<版本>/` + `config.yml` 按 SHA1 选择目录），而 2ship2harkinian 仍走 `ZAPDTR + OTRExporter + extract_assets.py`。且 SoH 明确承认迁移未完成：`Shipwright/CMakeLists.txt:330-335` 的 `ExtractAssetHeaders` 目标是**故意失败**的（"torch does not emit OoT asset headers yet"）。
7. **重编译路线的"桥接"不是 C↔C++，而是 MIPS ABI↔宿主 C**：唯一 ABI 是 `void f(uint8_t* rdram, recomp_context* ctx)`（`N64Recomp/include/recomp.h:443`），字节序用地址异或融进 `MEM_H/B` 宏（`:95-108`），分支延迟槽靠**复制**而非影子寄存器，libultra 靠三张名单交接给运行时（`N64Recomp/src/symbol_lists.cpp`：`reimplemented_funcs` 116 条 @`:3`、`ignored_funcs` 384 条 @`:143`、`renamed_funcs` 83 条 @`:564`）。补丁以 MIPS 源码编译成 ELF，再用**链接顺序**覆盖原函数（`Zelda64Recomp/CMakeLists.txt:342-352`）+ 运行期写 `movabs rax; jmp rax` 做 mod detour（`N64ModernRuntime/librecomp/src/mods.cpp:558-575`）。
8. **TriAevum 的实质是把 3DS 标题 AOT 成 PC 原生程序，而不是"在 3DS 上跑 PC 移植"**（任务描述里的常见误解）。它源自 Shipwright fork（`TriAevum/THIRD_PARTY_NOTICES.md:14-20` 自述），但渲染路径已彻底换掉：LUS 的 Fast3D 解释器对 3DS 标题不生效，实际执行**原生 PICA 语料 → NRI/Vulkan**（`runtime/three_ds_recomp/src/fast/renderer3ds/` 35 个文件；`docs/OOT3D_NRI_PICA_AOT_RENDERER_PARITY.md:5-6` "PICA state remains the authority"），纹理走 ETC1 自行解码（`pica_texture_decode.cpp:50-65`），`.gitmodules` 里的 libultraship 声明已陈旧（该路径实际是被摊平 vendoring 的 1,281 个文件）。

---

## 1. 生态谱系与仓库版图

### 1.1 依赖/派生关系（只列有硬证据的边）

```
N64 OoT ROM (o3d)
├─ zeldaret/oot（反编译，见报告 02）
│   ├─ HarbourMasters/Shipwright  ── 子模块 Kenix3/libultraship@port-maintenance（da6daf1a）
│   │     │                         └─ 子模块 HarbourMasters/Torch@main（2ab12fe9）
│   │     ├─ 产物 oot.o2r / oot-mq.o2r / soh.o2r（Torch，14 版本目录）
│   │     └─ 随机化：soh/soh/Enhancements/randomizer/3drando/ ← OoT-Randomizer 的 C++ 派生
│   │            （LICENSE.md:1 "derivative code from OoT-Randomizer"）
│   ├─ HarbourMasters/2ship2harkinian（MM）── 子模块 libultraship + ZAPDTR + OTRExporter
│   └─ HackerOoT（mod 基座，非 PC 移植）
│
├─ N64Recomp（MIPS→C 重编译器，lib/{rabbitizer,sljit,ELFIO,fmt,tomlplusplus}）
│   └─ N64ModernRuntime（ultramodern + librecomp）
│         └─ Zelda64Recomp（产品集成，lib/{rt64, RmlUi, lunasvg, mm-decomp, N64ModernRuntime}）
│               └─ rt64（contrib/plume 提供 D3D12/Vulkan/Metal 后端，本地未检出）
│
├─ Emill/n64-fast3d-engine（2021）──► libultraship 的 src/fast/（文件一一对应，见 §3.5）
│
└─ 3DS/Switch2 标题
    ├─ HarbourMasters/Shipwright ──(fork)──► coccofresco/TriAevum ──(fork)──► OTPR26/Ocarina-ReTuned
    │        runtime/three_ds_recomp = vendored kenix3/libultraship@port-maintenance（1281 文件，无 gitlink）
    ├─ gamestabled/oot3d_practice_menu（Luma IPS，独立于以上全部）
    └─ Hookshot / ProjectReLink / oot-pc-recomp（Switch2 重制版 ARM64→C + NVN→Vulkan）

资产工具链：ZAPD(zeldaret) ──fork──► ZAPDTR(HM) ──插件──► OTRExporter ──取代中──► Torch
            retro(Flutter GUI) ──停维护──► neoretro(C++/CMake)
            fast64（Blender 正向创作）与 ZAPD/ZAPDTR（ROM 反向导出）互补
```

### 1.2 谱系证据表

| 边 | 证据 |
|---|---|
| Shipwright → libultraship@port-maintenance | `Shipwright/.gitmodules`（`branch = port-maintenance`），`git submodule status` → `-da6daf1a...`（`-` 即未检出） |
| Shipwright → Torch（替换 ZAPD 链） | `Shipwright/.gitmodules` 只有 `libultraship`+`torch`；`Shipwright/docs/BUILDING.md:83-85` "The Torch migration removed the ZAPD-based header generator" |
| 2ship2harkinian → ZAPDTR + OTRExporter | `2ship2harkinian/.gitmodules:1-9`；`CMakeLists.txt:207` `add_subdirectory(ZAPDTR/ZAPD)`；`CMakeLists.txt:255-282` 调 `extract_assets.py` |
| OTRExporter 是 ZAPDTR 的插件而非替代 | `OTRExporter/OTRExporter/TextureExporter.cpp:2` `#include "../ZAPD/ZFile.h"`；`Main.cpp:437` `Globals::AddExporter("OTR", …)`；`extract_assets.py:12-36` 用 `-se OTR` 调 ZAPD |
| Zelda64Recomp → rt64 / N64ModernRuntime | `.gitmodules`（`lib/rt64`、`lib/N64ModernRuntime`）；集成点 `Zelda64Recomp/src/main/rt64_render_context.cpp:321-325` |
| N64Recomp → sljit（live JIT） | `N64Recomp/lib/sljit`；`N64Recomp/LiveRecomp/live_generator.cpp:12` `#include "sljitLir.h"` |
| TriAevum ⇐ Shipwright | `TriAevum/THIRD_PARTY_NOTICES.md:14-20` 明文"originated as a fork of HarbourMasters/Shipwright" |
| TriAevum → vendored libultraship | `.gitmodules` 声明 `runtime/three_ds_recomp → kenix3/libultraship@port-maintenance`，但 `git ls-files` 计 1,281 文件、无 `.git`（已摊平）；`THIRD_PARTY_NOTICES.md:41-47` |
| Ocarina-ReTuned ⇐ TriAevum | `resources/source-pins.json` 将 `renderer_reference` 钉到 `coccofresco/TriAevum@17218b0c`；`README.md:26` "TriAevum provides the runtime and translation foundation" |
| SoH ⇐ OoT-Randomizer | `Shipwright/soh/soh/Enhancements/randomizer/3drando/LICENSE.md:1` |
| rt64 → plume（RHI） | `rt64/.gitmodules` `src/contrib/plume`；`rt64/src/common/rt64_plume.h:17-27`；`CMakeLists.txt:421` 链 `plume` |

### 1.3 按角色归类（16 个目录 → 6 类）

| 类别 | 仓库 | 一句话 |
|---|---|---|
| 移植主干/引擎 | Shipwright、2ship2harkinian、Zelda64Recomp、TriAevum、Ocarina-ReTuned | 可交付的"原生 PC 版时之笛/假面/3D 版" |
| 运行时基座 | libultraship、N64ModernRuntime | libultra 的现代重实现 / MIPS 桥接层 |
| 图形后端 | rt64、n64-fast3d-engine、（TriAevum 的 renderer3ds） | Fast3D/PICA → D3D11/D3D12/Vulkan/Metal |
| 重编译工具 | N64Recomp | MIPS/RSP → C，另有 sljit JIT 后端 |
| 资产工具链 | Torch、ZAPDTR、OTRExporter、retro、neoretro | ROM ↔ XML/YAML ↔ `.otr`/`.o2r` |
| mod/衍生 | oot3d_practice_menu、（OoT-Randomizer、fast64、SharpOcarina、Ocarina-Atlas 在别处） | 不改引擎的旁挂能力 |

---

## 2. Ship of Harkinian（`Shipwright`）移植主干

> 取证对象：`$P/Shipwright`，`project(Ship VERSION 9.3.1)`（`CMakeLists.txt:9`），构建代号 `Dewey <NATO>`（`:41`），分支 `develop` @ `de1ab1b6`（2026-11-17）。规模：`.c` 816 / `.cpp` 487 / `.h` 1,847 / `.hpp` 14。
> 两项需要纠偏的常见认知：① 该仓库 HEAD **不含 MM/2ship 代码**（只有 `MMBunnyHood` 这类 OoT 开关与一句 "3DS rando" 注释），MM 在 `2ship2harkinian` 与 `TriAevum`；② 资产管线已 Torch 化，`soh/assets/xml` 已改名 `soh/assets/yml`。

### 2.1 仓库结构与内部细分

| 顶层 | 作用 | 证据 |
|---|---|---|
| `CMake/` | 14 个 CVar 前缀宏与依赖发现模块 | `CMake/soh-cvars.cmake:1-15`（`gSettings`/`gEnhancements`/`gCheats`/`gCosmetics`/`gRandoSettings`/`gOpenWindows`/`gTrackers`/`gDeveloperTools`/`gGeneral`/`gRemote`/`gGameplayStats`/`gTimeDisplay`/`gRandoEnhancements`/`gAudioEditor`），随后 `add_compile_definitions` 全局注入 |
| `soh/` | 游戏本体 + 移植层 | 见下表 |
| `roms/` | 用户放置原始 ROM | `CMakeLists.txt:311` `SOH_ROM_PATH` 默认 `${CMAKE_SOURCE_DIR}/roms` |
| `scripts/` | Switch/WiiU/Linux AppImage 构建脚本 | `scripts/switch/build.sh:3`（target `soh_nro`） |
| `torch/`、`libultraship/` | 子模块（**未检出，空目录**） | `.gitmodules` |
| `docs/` | BUILDING(353 行)/MODDING(207 行)/CUSTOM_MUSIC/FORMATTING/GAME_CONTROLLER_DB/CREDITS/VERSIONING(5 行)/supportedHashes.json(19 条) | — |

`soh/` 内部：

| 子目录 | 内容 | 关键事实 |
|---|---|---|
| `soh/src/code/` | 反编译游戏代码 153 文件（85 个 `z_*.c`） | `soh/CMakeLists.txt:186` 全量 GLOB |
| `soh/src/overlays/` | `actors/` 428 个 `ovl_*` 目录、`effects/` 36、`gamestates/` 4 | **已全部静态链进可执行文件**，不再是运行时 `.ovl`（`soh/CMakeLists.txt:206-212`） |
| `soh/src/libultra/` | 仅保留 `gu` 与 `libc/sprintf.c` | 其余被 EXCLUDE，交给 LUS（`soh/CMakeLists.txt:191-215`） |
| `soh/soh/` | Ship 专属 C++：`OTRGlobals`、`ActorDB`、`SaveManager`、`frame_interpolation`、`SohGui/`、`Enhancements/`、`randomizer/`、`Network/`、`Extractor/` | — |
| `soh/assets/` | `yml/` 20,353 个 torch 声明（14 版本目录）、`custom/` 1,052、`objects/` 382、`scenes/` 511、`textures/` 154、`tools/` | `custom/` 打进 `soh.o2r` |
| `soh/include/` | `z64*.h` 类型头 + `tables/actor_table.h` 等 X-macro 表 | — |

### 2.2 从 `zeldaret/oot` 到 SoH 的五类改造手法

与 `/root/wsl-demo/oot-decomp-projects/oot`（`14759ab`）逐文件比对，427 个同名 actor overlay 目录。改动可归纳为：

**(a) include 折叠与替换。** oot 的细粒度 include 全被折成 `global.h`/`vt.h` + LUS/soh 头。注意 **SoH 不用 `<ultra64.h>`**（全仓命中 0 次），而是 `<libultraship/libultra.h>` + `<libultraship/bridge/*.h>`，`soh/src` 内共 **590 处** `#include <libultraship…>`：

```c
/* soh/src/code/z_bgcheck.c:1-7 */
#include "global.h"
#include "vt.h"
#include "soh/OTRGlobals.h"
#include "soh/ResourceManagerHelpers.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
```

**(b) 平台宏/调试宏清除。** `#pragma increment_block_number`、`PLATFORM_N64`、`DEBUG_FEATURES` 删除；`z_bgcheck.c` 中 oot 新增的 `BGCHECK_RAYCAST_*` 常量被回退成 SoH 的 `COLPOLY_IGNORE_*` 命名——说明 SoH 与上游 decomp 的**符号演进是单向滞后**的（跟随但不完全同步）。

**(c) 类型与常量还原。** `z_view.c:33-34` 把 `G_MAXZ / 2` 写回字面量 `0x01FF`；`:26` 把 `SYSTEM_ARENA_MALLOC(sizeof(View), "../z_view.c", 285)` 简化为 `SYSTEM_ARENA_MALLOC_DEBUG(sizeof(View))`；`main.c:14-16` `u32 gSystemHeapSize` → `size_t`。

**(d) overlay 注册结构改写。**

```c
/* soh/src/overlays/actors/ovl_Arms_Hook/z_arms_hook.c:17 */
const ActorInit Arms_Hook_InitVars = {
    ACTOR_ARMS_HOOK, ACTORCAT_ITEMACTION, FLAGS, OBJECT_LINK_BOY, sizeof(ArmsHook),
    (ActorFunc)ArmsHook_Init, ..., (ActorFunc)ArmsHook_Draw, NULL,
};
```
oot 侧对应的是 `ActorProfile Arms_Hook_Profile = {…}`（无 cast、无尾部 NULL）。

**(e) 注入点密度。** `soh/src` 内 **1,242 处 `GameInteractor_*` 调用、1,571 处 `CVarGet*`**。例如 `z_play.c:1138` `if (GameInteractor_Should(VB_FLASH_SCREEN_FOR_FINISHING_BLOW, true))`、`z_player.c:2618` `VB_ALLOW_QUICK_PUTAWAY`。

> 未在两个仓库中找到 `soh/src/code/z_matrixpack.c`、`soh/src/graphic_list/`、`GetItemId` —— 任务描述中的这些示例名不成立。

### 2.3 入口与生命周期

真实入口不在 `src/main/cpp`，而在游戏侧 C：

```c
/* soh/src/code/main.c:46-73（节选） */
#ifdef _WIN32
int SDL_main(int argc, char* argv[]) { AllocConsole(); ... }
#else
int main(int argc, char* argv[]) {
#endif
    GameConsole_Init();  InitOTR(argc, argv);
    CrashHandlerRegisterCallback(CrashHandler_PrintSohData);
    BootCommands_Init(); Heaps_Alloc(); Main(0);
```

`InitOTR`（`soh/soh/OTRGlobals.cpp:1550-1607`）依次构造 `OTRGlobals`（它本身就 `: Ship::Context`，`:279` `CreateUninitializedInstance`）、`RunExtract`、`GameInteractor::Instance`、`SaveManager`、`ActorDB`、TTS（Win=SAPI / macOS=Darwin / Linux=espeak-ng）、CrowdControl/Sail/Anchor，并注册配置文件拖拽（`:1597`）。

帧循环链路：`main.c:139-141` `osCreateThread(... Graph_ThreadEntry)` → `soh/src/code/graph.c:509-513` `while (WindowIsRunning()) { RunFrame(); }` → `graph.c:437-500` `RunFrame()`（`Overlay_LoadGameState` → `GameState_Init` → 内层循环 `Graph_StartFrame`/`PadMgr_ThreadEntry`/`Graph_Update`/`Graph_ProcessGfxCommands(workBuffer)`，末尾 `ProcessSaveStateRequests()`）→ `game.c:259-357` `GameState_Update()`（`:269` `GameInteractor_ExecuteOnGameStateMainStart()` → `gameState->main()` → `:357` `GameInteractor_ExecuteOnGameFrameUpdate()`）→ `z_play.c:1650 Play_Main` → `:1682 Play_Update(play)`（逻辑帧）→ `:1687-1689 FrameInterpolation_StartRecord(); Play_Draw(play); FrameInterpolation_StopRecord();`。

`GameLaunch`/`RunDebug` 在 SoH 中**不存在**（那是 Zelda64Recomp 的命名）。`SaveContext_Init` 在 `soh/src/code/z_common_data.c:6-19`；随机器判定 `soh/include/z64save.h:414` `#define IS_RANDO (gSaveContext.ship.quest.id == QUEST_RANDOMIZER)`。

### 2.4 C/C++ 桥接层（decomp 路线的核心）

SoH 的跨语言边界靠四件套，**不是** `RegisterShipOverlay`/`SohInterface`（这两个在当前 tree 中 0 命中，是历史 API）：

| 机制 | 位置 | 要点 |
|---|---|---|
| `GameInteractor` hook 框架 | `soh/soh/Enhancements/game-interactor/{GameInteractor,GameInteractionEffect,GameInteractor_Hooks,GameInteractor_HookTable,GameInteractor_RawAction,GameInteractor_State}` | 头文件 `GameInteractor_Hooks.h:5-8` 用 `#ifdef __cplusplus extern "C" {` 暴露约 120 个 `GameInteractor_Execute*/Should*`；实现 `GameInteractor_Hooks.cpp:5` 逐条转发到 `GameInteractor::Instance->ExecuteHooks<OnZTitleInit>(...)`；`GameInteractor_HookTable.h` 用 X-macro `DEFINE_HOOK(OnPlayerUpdate, ())` 声明约 140 个 hook；`GameInteractor.h:515` 是 `DEFINE_HOOK` 宏本体；每帧开头 `GameInteractor_Hooks.cpp:21-23` 清理排队 hook |
| Vanilla-Behavior 开关 | `soh/soh/Enhancements/game-interactor/vanilla-behavior/GIVanillaBehavior.h` | 491 个 `VB_*` 枚举，C 侧一行 `GameInteractor_Should(VB_X, 默认值)` 即可把"原版行为"改成可配置 |
| 符号集中声明 | `soh/soh/GbiWrap.cpp:8-35` | 一个集中 `extern "C"` 块声明游戏侧符号（`InitOTR`、`Graph_ProcessGfxCommands`、`ResourceMgr_LoadGfxByName`、`GetPerfCounter`…），C++ 侧由此进入 C 世界 |
| 条件初始化表 | `soh/soh/ShipInit.hpp:11-60,61` | `ShipInit::InitAll` + `RegisterShipInitFunc(fn, {"cvar-or-path"})`，按 CVar 路径增量重注册（改了哪个开关就只重跑相关初始化） |
| Actor 静态表 | `soh/soh/ActorDB.cpp:7-27,500,598` | `actor_table.h` X-macro 生成 `constexpr AddPair initialActorTable[]`；C 入口 `extern "C" ActorDBEntry* ActorDB_Retrieve(int id)` |
| 游戏态表 | `soh/src/code/z_game_dlftbls.c:9-14` | `GAMESTATE_OVERLAY(...)` 的 `romStart/romEnd` 全填 0 —— 静态链接的物证 |

`IS_C` 宏在本仓**未找到定义**（`grep "define IS_C"` 0 命中）。**Lua 绑定同样未找到证据**：LUS 侧有 `ENABLE_SCRIPTING`/`ScriptGetFunction`（`libultraship/include/libultraship/bridge/scriptingbridge.h:29`），但 SoH 源码无 `sol::`；`Shipwright/docs/MODDING.md:31-36` 教的 "mod" 实际是 **fork 源码改 C++ + `GameInteractor`/`REGISTER_VB_SHOULD` hook**。运行时替换只剩**资产**一层：Alt Assets 开关 `OTRGlobals.cpp:1885-1892`（Tab → `SetAltAssetsEnabled` + `gfx_texture_cache_clear` + `SkeletonPatcher::UpdateSkeletons`）。

### 2.5 资产构建管线（Torch 版）

| CMake target | 位置 | 产物/说明 |
|---|---|---|
| `soh-torch` | `CMakeLists.txt:290-295` | = `soh/assets/tools/torch-cli/main.cpp` + `soh/soh/Extractor/TorchExtract.cpp` |
| `soh-o2r-packer` | `:297-301` | 打包 `soh.o2r` |
| `ExtractAssets` | `:312-325` | `soh-torch --src soh/assets/yml --dest build/soh --version 9.3.1 ${SOH_ROM_PATH}` → `oot.o2r`/`oot-mq.o2r` |
| `ExtractAssetHeaders` | `:330-335` | **故意失败**："torch does not emit OoT asset headers yet"（约 24k 处 `#define d<sym>` 仍依赖签入头文件） |
| `GenerateSohOtr` | `:341-357` | `soh.o2r`（`assets/custom/**` + LUS shader） |
| `SOH_TOOLS_ONLY` | `:364-368` | CI 只产 `soh.o2r` |

ROM→版本的解析链：`soh/assets/yml/config.yml:5-19` 用 **SHA1 → 版本目录** 映射（`0227d7c0…` → `pal_gc`，并声明 `gbi: F3DEX2_OoT`、`primary_virtual_segment: 0x80`、`output.binary: oot.o2r`）；14 个版本目录为 `ntsc_1-0/1-1/1-2`、`pal_1-0/1-1`、`pal_gc`、`pal_gc_dbg`、`ntsc_u_gc`、`ntsc_j_gc`、`ntsc_j_gc_collection`、`pal_mq`、`pal_mq_dbg`、`ntsc_u_mq`、`ntsc_j_mq`。运行时抽取复用同一库：

```cpp
/* soh/soh/Extractor/TorchExtract.cpp:34-52（节选） */
auto companion = std::make_unique<Companion>(std::move(rom), ArchiveType::O2R, false, srcDir, destDir);
Companion::Instance = companion.get();
companion->SetVersion(portVersion);
companion->Init(ExportType::Binary);
```

校验：`soh/soh/Extractor/Extract.cpp:88-109` 21 条 CRC32C 白名单（`0xFD9913B1` N64 PAL1.0、`0x460C938C` NTSC US1.0、GC/MQ/DBG 变体）；`:355` 取头部版本 CRC `BSWAP32(((uint32_t*)data)[4])`；`:375` 用 `FastCrc32C.c`；ROM 尺寸需 32/54/64 MB（`:128`）；`docs/supportedHashes.json` 19 条 SHA1。版本兼容策略见 `docs/VERSIONING.md:3`（major 变更要求新 `oot.o2r`）+ `soh/soh/config/ConfigUpdaters.cpp:60-110`。

### 2.6 增强特性清单（以真实开关名为准）

SoH 的 CVar 已从裸 `gXxx` 迁移到分组前缀（`CMake/soh-cvars.cmake` + `soh/soh/config/ConfigUpdaters.cpp:60-77` 的旧名映射）。核实过的关键项：

| 功能域 | 真实开关 | 位置 |
|---|---|---|
| 插帧目标帧率 | `gSettings.InterpolationFPS`（旧 `gInterpolationFPS`，默认 20）、`gSettings.MatchRefreshRate` | `OTRGlobals.cpp:1051-1059` |
| 垂直同步 | `CVAR_VSYNC_ENABLED` | `OTRGlobals.cpp:1055` |
| 内部分辨率倍率 | `gSettings.InternalResolution` → `Window::SetResolutionMultiplier` | `SohMenuSettings.cpp:341` → `libultraship/src/fast/interpreter.cpp:5448` |
| MSAA / 纹理过滤 / Z-fighting | `gSettings.MSAAValue`(1..8) / `gSettings.TextureFilter`（Three-Point 默认） / `gSettings.ZFightingMode` | `SohMenuSettings.cpp:366,427,47` |
| 低分辨率模式 | `gSettings.LowResMode`（1=N64 320×240，2=240p 宽屏，3=480p 宽屏） | `libultraship/src/fast/Fast3dGui.cpp:372,416` |
| 高级分辨率编辑器 | `gAdvancedResolution.*`（PixelPerfectMode/IntegerScale.Factor/VerticalResolutionToggle） | `soh/soh/SohGui/ResolutionEditor.cpp:12,105,188,200` |
| HD 材质包 | `gSettings.AltAssets`（热键 Tab） | `OTRGlobals.cpp:1784,1885-1890` |
| 宽屏/裁剪 | `OTRGetAspectRatio()`、`DisableBlackBars`、`WidescreenActorCulling`、`DisableDrawDistance` | `OTRGlobals.cpp:2223-2232`、`ShipUtils.cpp:42-60` |
| 相机 | `gSettings.FreeLook.*`、`MoveInFirstPerson` | `soh/soh/Enhancements/camera/FreeLook.cpp:14-40` |
| 帧率依赖修复 | `FastDrops`、`NewDrops`、`FixFlexDrops`、`BushDropFix`、`FixHangingLedgeSwingRate`、`FixDaruniaDanceSpeed`、`FixCameraDrift/Swing`、`ClimbSpeed`、`CrawlSpeed`、`FasterRupeeAccumulator` | `soh/soh/Enhancements/Fixes/*.cpp`、`randomizer/draw.cpp:1180` |
| 文本/过场 | `TextSpeed`、`SlowTextSpeed`、`TimeSavers.SkipCutscene.*`、`SkipText` | `SohMenuEnhancements.cpp` |
| 输入延迟 | `gSettings.SimulatedInputLag`、`ExtraLatencyThreshold` | `ConfigUpdaters.cpp:60-66` |
| 音频 | 自定义 Seq64 音乐、`AudioCollection`、`EnemyBGMDisable` | `soh/soh/Enhancements/audio/`、`docs/CUSTOM_MUSIC.md` |
| 其他 | 存档状态 `savestates.cpp`、自动存档、CrowdControl/Sail/Anchor 多人联动、怪物血条 | `soh/soh/Enhancements/…` |

**未找到证据的项**（网上常见说法，本仓不成立）：`gExtMap`、RDRAM 泄漏修复专用开关、`No Input Delay`（只有 `SimulatedInputLag`）、体感 `Motion Controls`/GamepadMotionHelpers、`Eyeboss`。

### 2.7 平台与依赖

CI（`.github/workflows/generate-builds.yml`）四条主线：`generate-soh-otr`（ubuntu-22.04，`-DSOH_TOOLS_ONLY=ON`）、`build-macos`（macos-15 + MacPorts，`CMAKE_OSX_ARCHITECTURES="x86_64;arm64"`）、`build-linux`（x86_64 与 aarch64 矩阵）、`windows-latest`；另有 distro 冒烟、clang-format、lint-json、whitespace、pr-artifacts。**注意：CI 不产出 3DS/Switch/WiiU 包**，尽管代码里有 `__SWITCH__`/`__WIIU__` 分支（`soh/CMakeLists.txt:170,514,667,677`；`OTRGlobals.cpp:818-822` `Ship::Switch::Init`/`Ship::WiiU::Init`；`scripts/{switch,wiiu}/build.sh`）。Windows 支持 x64/Win32/ARM64（`soh/CMakeLists.txt:23-42`）。

依赖：Windows vcpkg 一次装 `zlib bzip2 libzip libpng sdl2 sdl2-net glew glfw3 nlohmann-json tinyxml2 spdlog libogg libvorbis opus opusfile`（`CMakeLists.txt:106`）；Linux `linux-build-deps/apt.txt` 另含 **espeak-ng**（TTS）；`dr_libs` 经 FetchContent（`soh/CMakeLists.txt:293-297`，自定义音乐解码）。ImGui/RmlUi/GamepadMotionHelpers/oniguruma 属 LUS 子模块，本仓不可直接取证。运行时文件：`shipofharkinian.json` + `oot.o2r`/`oot-mq.o2r`/`soh.o2r`；Linux 下 `SHIP_HOME` 可重定向（`OTRGlobals.cpp:1516-1545`）。**未发现自动更新检查代码**（`api.github.com` 0 命中）。

### 2.8 2ship2harkinian 的结构性差异（对照表）

| 项 | 2ship2harkinian（`$P/2ship2harkinian`，5.1.0） | Shipwright |
|---|---|---|
| 目录 | `mm/` + 移植层 `mm/2s2h/`（`CMakeLists.txt:231`、`project(2s2h VERSION 5.1.0)` `:8`） | `soh/` + `soh/soh/` |
| 子模块 | `libultraship`(无 branch) + `ZAPDTR` + `OTRExporter`（`.gitmodules:1-9`；pin `7f9b86a5`/`ee3397a3`/`32e088e2`） | `libultraship@port-maintenance` + `torch` |
| 资产管线 | CMake → `OTRExporter/extract_assets.py -z $<TARGET_FILE:ZAPD> --xml-root ../mm/assets/xml` → `mm.o2r` + `2ship.o2r`（`:255-282`） | Torch（§2.5） |
| 运行时抽取 | `mm/2s2h/Extractor/Extract.cpp:601-624` 进程内构造 argv 调 `zapd_report`（`ed … -se OTR --otrfile mm.o2r --portVer`） | `TorchExtract.cpp` 调 `SohTorch::Extract` |
| 区域/版本 | **仅 2 套**：`mm/assets/xml/{GC_US,N64_US}`；`docs/supportedHashes.json` 只列 NTSC-U 1.0 与 NTSC-U GC | 14 套（含日版 GC） |
| MM 独有资源类型 | `mm/2s2h/resource/type/2shResourceType.h:22-27`：`OTXM`(TextMM)、`OTAN`(TexAnim)、`OKFA`/`OKFS`（关键帧动画/骨架） | `SohResourceType.h` 无这 4 类 |
| MM 独有玩法系统 | `mm/2s2h/Enhancements/{Cycle,Masks,Songs,Timesavers}`；`Cycle/MoonCrashSave.cpp:11-27` 用 `BeforeMoonCrash` hook；`Cycle/EndOfCycle.cpp:11-24` 8 个 `gEnhancements.Cycle.DoNotReset*` | 无 Cycle（三日循环是 MM 特有） |
| 音频 | 保留 MM 原生 sequencer：`mm/src/audio/code_8019AF00.c:60` `SeqPlayerIOPort`、`:79` `Audio_PlaySequenceWithSeqPlayerIO`、`:134-156` 空间化 BGM 状态 `sSpatialSeqPlayerIndex` | `soh/soh/Enhancements/audio/OotrsArchive.{h,cpp}` + Seq64 自定义音乐 |
| 帧率 | 同一函数名仍沿用 SoH 的 `OTRGlobals::GetInterpolationFPS()`（`mm/2s2h/BenPort.cpp:788-797`，默认 20） | — |

**2SoH 未合并回 Shipwright**：Shipwright 无 MM 相关 `.gitmodules` 条目与 `mm/` 目录，README 全文只讲 OoT；两仓 PR 序列互不引用（2SoH 已到 #2007、Shipwright #7536）；管线已分叉（Shipwright 删除 ZAPD 头生成器，2SoH 仍 `add_subdirectory(ZAPDTR/ZAPD)`）。`Torch/src/factories/` 下**未见 MM factory 目录**，是 Torch 化尚未覆盖 MM 的直接信号。

---

## 3. libultraship（运行时基座）解剖

> 取证对象：`$P/libultraship`（`Kenix3/libultraship`，`main` @ `759b134`，71,969 行 / 419 文件）。`/root/wsl-demo/oot-ports-repos/libultraship` 是同 SHA 的镜像副本。**这是 epoch 2 架构**：`src/graphic/Fast3D/` 已重构为 `src/fast/`，本地全盘 `find -type d -name Fast3D` 无任何命中（§3.7 讨论版本关系）。

### 3.1 三层命名空间与目录

```
include/{ship,fast,libultraship}   src/{ship,fast,libultraship}   cmake/ docs/ keys/ tests/ tools/
```

| 命名空间 | 职责 | 规模（src+include） |
|---|---|---|
| `Ship::` | 平台无关运行时：资源、窗口、音频、输入、配置、线程、签名、事件、脚本 | `ship/controller` 91 文件 7,714 行；`ship/window` 34 文件 7,140 行；`ship/resource` 36 文件 4,155 行；`ship/core` 20 文件 3,845 行；`ship/utils` 3,099；`ship/config` 1,132；`ship/audio` 1,839；`ship/scripting` 871；`ship/debug` 806；`ship/security` 201；`ship/port` **仅 37 行** |
| `Fast::` | N64 F3D 解释器 + 渲染后端 + `Fast3dWindow` | 55 文件 18,992 行，最大 `src/fast/interpreter.cpp` **5,546 行** |
| `LUS::`/libultra | libultra 头文件 ABI + 极薄实现 | 35 文件 7,519 行，但 **`src/libultraship/libultra/` 只有 8 个 cpp / 372 行**；`include/libultraship/libultra/gbi.h` 独占 4,311 行 |

组件生命周期是 ECS 化的：`include/ship/core/{Context,Component,ComponentList,Part,PartList,Action,ActionList,EventActionList,Tickable,TickableComponent}.h`；`Ship::Window : public Ship::Component`（`include/ship/window/Window.h:62`），`Fast::Fast3dWindow : public Ship::Window`（`include/fast/Fast3dWindow.h:45,48`）。游戏与引擎的每帧契约只有四个调用（`docs/FAST-RENDERER.md:19-29`）：`gfx_start_frame(); gfx_run(cmds); gfx_end_frame();`。

`docs/PORTING.md:9-11` 对自身的定位很坦白：LUS **不是重编译工具**，而是 N64 SDK 的 header-compatible 包装，"**compile-time WINE**"。

### 3.2 容器格式：`.otr` = MPQ，`.o2r` = ZIP

这是本报告最需要纠偏的一节。HEAD 版本的实现只有三类归档 + 一个 SoH 自造的第四类：

| 归档 | 实现 | 容器 | 压缩 | 可写 |
|---|---|---|---|---|
| `OtrArchive` | `src/ship/resource/archive/OtrArchive.cpp:71` `SFileOpenArchive(..., MPQ_OPEN_READ_ONLY, &mHandle)` | **MPQ v2**（StormLib） | `MPQ_FILE_COMPRESS` + `MPQ_COMPRESSION_ZLIB`（逐文件） | 否：`:108-111` `SPDLOG_INFO("otr does not support WriteFile, please use an o2r instead"); return false;` |
| `O2rArchive` | `src/ship/resource/archive/O2rArchive.cpp:83-96` `zip_name_locate`→`zip_stat_index`→`zip_fopen_index`→`zip_fread` | **ZIP**（libzip） | deflate（默认） | 是：`:143-194` `zip_source_buffer` + `ZIP_FL_OVERWRITE` + 重开 + 失效句柄池 + 重建索引 |
| `FolderArchive` | `src/ship/resource/archive/FolderArchive.cpp:22-41` | 目录 | 无 | 是（`WriteFile` = `WriteAllBytes`） |
| `.ootrs` | **在 SoH 侧**：`Shipwright/soh/soh/Enhancements/audio/OotrsArchive.h:29` `class OotrsArchive final : virtual public Ship::Archive` | 内部包一个 `shared_ptr<Ship::O2rArchive>` + `unordered_map<string, OotrsSong>` | 随 ZIP | 音乐 mod 专用 |

关键事实与反直觉点：

* **没有 OTR 头**。`include/ship/resource/archive/Archive.h:17` 只有 `#define OTR_HEADER_SIZE ((size_t)64)`，注释（`:52-61`）说明它是"mounted archive (OTR, O2R, or folder)"。归档层不存 magic/version。MPQ 支持是**可选编译**：`CMakeLists.txt:77 option(INCLUDE_MPQ_SUPPORT ... OFF)`；SoH 为了兼容老 mod 显式打开（`Shipwright/CMakeLists.txt:188` + `:375`）。
* **索引是哈希表不是排序表**：`Archive.cpp:182-185` `(*mHashes)[CRC64(filePath.c_str())] = filePath;`（`include/ship/resource/archive/Archive.h:215` 声明为 `unordered_map<uint64_t,std::string>`）。`src/ship/utils/StrHash64.cpp:176-183` 的 `CRC64(const char*)` 用 MSB-first 直查表且**无最终取反**，而同文件 `:163-174` 的 `crc64(buf,len)` 返回 `~crc` —— 两个入口取反语义不一致，是移植时易踩的坑（`tests/crc64_tests.cpp` 覆盖）。
* **真正的持久结构是 64 字节资源头**：读侧 `src/ship/resource/ResourceLoader.cpp:314-338` 依次取 `ByteOrder(0x00)`、`IsCustom(0x01)`、2 字节保留、`Type(0x04, fourcc)`、`ResourceVersion(0x08)`、`Id(0x0C, u64)`；写侧权威定义在 `OTRExporter/OTRExporter/Exporter.cpp:4-21`（补齐到 0x40，`0x10` 存资源版本表、`0x14` 存 ROM CRC、`0x1C` 存 ROM 枚举，`0x0C` 填 `0xDEADBEEFDEADBEEF`）。`:108-142` 的 `ReadResourceInitDataLegacy` 用"首字符是否 `<`"判 XML，否则要求 `size >= 64` 并把 `BufferOffset` 设为 64。
* **归档级元数据是文本文件**：`Archive.cpp:38-103` 先读 `version`（1 字节 endianness + u32 游戏版本），再读 `manifest.json`（`Archive.h:30-49` `ArchiveManifest`：`name/icon/author/version/website/description/license/code_version/game_version/main/binaries/dependencies/checksum/signature/public_key`）。
* **完整性 = BLAKE2b-512 + Ed25519**：`Archive.cpp:187-300` 按路径排序后对 (normalizedPath, buffer) 做 `crypto_blake2b_init(&ctx, 64)`，与 `mManifest.Checksum` 比对，再用 keystore 公钥 `crypto_ed25519_check`（monocypher；签名脚本 `tools/sign.py`）。
* **压缩只有 deflate**：lz4/gzip/整包压缩/O2R 分块/64 位归档索引均**未找到证据**。MPQ 侧尺寸上限是 32 位（`OtrArchive.cpp:36` `DWORD`；Torch `SWrapper.cpp:68-70` 甚至 `if (size >> 32) throw "too large"`），ZIP 侧是 `zip_stat.size`（`size_t`）—— 这正是 O2R 取代 OTR 的硬动机之一。
* **O2R 的并发读靠句柄池**：`O2rArchive.h:74-81` `zip_t* GetZipHandle()/ReleaseZipHandle(); std::mutex mPoolMutex; std::vector<zip_t*> mZipArchivePool;`（`cpp:19-36`，`ZIP_RDONLY`）。OTR 只有一个 `HANDLE mHandle`。

### 3.3 资源类型与注册派发

LUS 本体只保留 8 个通用类型，游戏类型全部下沉：

| 位置 | 类型 |
|---|---|
| `include/ship/resource/ResourceType.h:15-21` | `None=0`、`Blob=OBLB`、`Json=JSON`、`Shader=SHAD` |
| `include/fast/resource/ResourceType.h:5-13` | `DisplayList=ODLT`、`Light=0x46669697`、`Matrix=OMTX`、`Texture=OTEX`、`Vertex=OVTX` |
| `Shipwright/soh/soh/resource/type/SohResourceType.h:4-21` | 16 项：`OARR OANM OPAM OROM OCOL OSKL OSLB OPTH OCUT OTXT OAUD OSMP OSFT OSEQ OBGI ORCM` |

派发键是**三元组** `{format, type, version}`：`include/ship/resource/ResourceLoader.h:19-39` `ResourceFactoryKey` + `:86 RegisterResourceFactory(factory, format, typeName, type, version)`；实现 `src/ship/resource/ResourceLoader.cpp:54-74` 做冲突检测并维护 `mResourceTypes[name]`，`:76-85 DecodeASCII` 把 fourcc 反解回字符串用于日志，`:285-292` 真正 `factory->ReadResource(fileToLoad, initData)`。SoH 注册 31 条工厂（`soh/soh/OTRGlobals.cpp:890-961`）。`.meta` 旁路（`:104-215`）允许无头资源用 JSON 声明 `path/format/type/version/isCustom`；`:217-238 SetBufferOffset` 的注释解释这是为了将来彻底去掉 64 字节头（引 upstream issue #1160）。可执行测试：`tests/archive_resource_meta_resolution_tests.cpp:23-58`。

> **旧文献中的 fourcc `MTEx/MTAB/VSAB/DOAB/SCSS/ITBL/GIOV/ovl/arc/arignore/arlist` 在整个工作区零命中**（唯一 grep 结果是 rt64 contrib 里的 `GIOVANNI_DICANIO` 与 N64Recomp 的 `SHT_SYMTAB`）。现代对应物是 `Omtx/Otex/Ovtx` + `SOH_Array(OARR)` + `Text` + `.meta` 字符串类型名。

### 3.4 加载、缓存与覆盖优先级

* **不存在 `AsyncLoader` 类**（grep 无命中）。三件套是 `ResourceManager` + `ResourceLoader` + `ThreadPool`（`src/ship/thread/ThreadPool.cpp:5-6` 包 `BS::thread_pool`）。线程数 `src/ship/core/Context.cpp:129` `max(1, hardware_concurrency - reservedThreadCount - 1)`，SoH 传 3（`OTRGlobals.cpp:303`，`:871` 注释"reserve 3 SoH specific threads (Game, Audio, Save)"）。
* **缓存**：`ResourceManager.h:440-442` `unordered_map<ResourceIdentifier, variant<ResourceLoadError, shared_ptr<IResource>>> mResourceCache`，**全局单 `std::mutex`**（无分片/读写锁）；`:213-240` 双检并把 miss 也写回缓存以免重复打盘。异步接口 `:264-291 LoadResourceAsync`（命中缓存直接 fulfill，否则带 `BS::priority_t` 提交），批量 `:397-407`，同步等待走 `BS::pr::highest`（`:296/:415`）。
* **热替换**：`:419-431 DirtyResources(mask|filter)` → 资源支持则 `resource->Dirty()` 否则 unload；`:364-371` 对 dirty 返回 nullptr 触发重载。SoH 的 C 封装 `soh/soh/ResourceManagerHelpers.cpp:213-221`。另有 `CacheExternalResource`（`:211-217,429-437`）不落盘直接把内存资源塞进缓存，是 HD 包/mod 注入的主路径。`WriteResource`（`:220-235`）只对 O2R/Folder 有效。
* **覆盖优先级只看挂载顺序**：`ArchiveManager.h:36-50` 注释写明 "the archive added most recently (highest index) takes precedence, allowing mod archives to override base-game assets"；`src/ship/resource/archive/ArchiveManager.cpp:299-306 AddArchive` 赋 `SetPriority(mArchives.size()-1)` 并逐文件 `AddFileToVfs`。`:163-172` 有个特例：`foo.meta` 只登记基路径、不写 `mFileToArchive`。`:226-257` 扫 `.otr/.zip/.mpq/.o2r`，**未识别扩展名回落到 O2R**（`:276-278`）。SoH 的实际挂载顺序：`soh.o2r` → `oot-mq.o2r`/`oot.o2r`（按 ROM，15 个 `OOT_*` 枚举白名单 `OTRGlobals.cpp:850-854`）→ `mods/` 内启用 mod（用户可拖排序，`mod_menu.cpp:155-177`，且**故意不收集 `.zip`**以免误吃普通压缩包，`:194-210`）→ `.ootrs` 音乐 mod。
* **`patch_` 文件名特殊语义：未找到证据**——补丁效果纯粹由"后挂载覆盖"实现。

一个贯穿全系统的机制值得单独指出：字符串 `"__OTR__"` 是唯一的持久"指针"编码。`ResourceManager.cpp:545-548` 定义签名、`:143-146/:268-271` 剥离；解释器侧 `interpreter.cpp:5365-5390 gfx_check_image_signature` 先做"奇地址 / `<0x10000` / `>0x0000FFFFFFFFFFFF`"三重否决再比对签名；`:4017-4058` 的 `gfx_set_timg_handler_rdp` 据此 `ResolveResourceCached`。游戏代码里因此能看到：

```c
/* Shipwright/soh/src/code/gfxprint.c:8-11 */
#define drGfxPrintFontData     "__OTR__textures/font/sGfxPrintFontData"
#define drGfxPrintAltFontData  "__OTR__alt/textures/font/sGfxPrintFontData"
```

`alt/` 前缀即 HD 材质包的命名空间（`ResourceManager.cpp:136-206` 处理，SoH 侧开关见 §2.6）。

### 3.5 Fast3D 渲染子系统（现代图形后端）

**入口与数据流**（SoH → LUS → GPU 的完整链）：

```
soh/src/code/graph.c:487  Graph_ProcessGfxCommands(workBuffer)
soh/soh/OTRGlobals.cpp:1873 RunCommands(...); :1829 wnd->DrawAndRunGraphicsCommands(Commands, mtx_replacements)
src/fast/Fast3dWindow.cpp:232  Interpreter::StartFrame / Run / EndFrame
src/fast/interpreter.cpp:5176-5213  g_exec_stack.start(cmds); while(...) gfx_step();
src/fast/interpreter.cpp:4894-4917  gfx_step(): opcode=(w0>>24)，三张 constexpr 表依次查
    → otrHandlers(:4720) / rdpHandlers(:4685) / ucode_handlers[f3dex2|f3dex|f3d|s2dex](:4761,:4782,:4805,:4829)
src/fast/interpreter.cpp:1801 GfxSpTri1 → 攒批 mBufVbo
src/fast/interpreter.cpp:138 Flush() → mRapi->DrawTriangles(mBufVbo, len, numTris)
```

指令字**没有中间 IR**：`include/fast/lus_gbi.h:1175` 的 `union F3DGfx { F3DGwords words; long long force_structure_alignment; }` 就是内存布局本身。LUS 的扩展微码 opcode 在 `include/fast/lus_gbi.h:39-71`（`OTR_G_SETTIMG_OTR_HASH 0x20 … OTR_G_POP_SHADER 0x44`，共 28 个），加上 RDP 侧 `RDP_G_LOADBLOCK_WIDE / VTX_WIDE / TRI1_WIDE`；微码族枚举 `include/fast/ucodehandlers.h:3-11`（f3db/f3d/f3dex/f3dexb/f3dex2/s2dex）。

> **命名纠偏**：`Fast3D::ProcessDisplayList`、`GbiRenderer`、`TwoDRenderer`、`ThreeDRenderer`、`PipelineShader`、`Fast::Sched`、`Fast::Mesh` 在 LUS 与 Shipwright 中**全部不存在**。真实符号是 `Fast::Interpreter::Run`（`include/fast/interpreter.h:397`）+ `gfx_step` + `GfxExecStack{cmd_stack, gfx_path, disp_stack, openDisp/closeDisp/branch/call/ret}`（`interpreter.h:15-40`）。Mesh 不是资源类型，而是 scene command（`soh/soh/resource/importer/scenecommand/SetMesh.h:9-101`）。

**RDP 语义的覆盖度：状态级近似，非像素级模拟。** `rdpHandlers` 表（`interpreter.cpp:4685-4718`）真解析 `G_TEXRECT/TEXRECTFLIP/FILLRECT/SETSCISSOR/SETCOMBINE/SETTILE/SETTILESIZE/LOADTLUT/LOADBLOCK/LOADTILE/SETTIMG/SETCIMG/SETZIMG/SETPRIMCOLOR/SETENVCOLOR/SETBLENDCOLOR/SETFOGCOLOR/SETFILLCOLOR/SETKEYGB/SETKEYR/SETCONVERT/SETPRIMDEPTH/SETOTHERMODE`；`G_TRI2`(:3915)、`G_QUAD`(:3924 拆两个 Tri1)、TexRect 连 COPY 模式的 `dsdx >>= 2` 与"边缘多一像素"修正都实现了（`:2931`）。但是：**无 TMEM 建模**（`RDP.texture_tile[8].tmem` 只是索引记账）、**四条 `G_RDP*SYNC` 全部 stub**、无 fill/blend 逐像素模拟、alpha dither 用噪声近似。`BLTCrc` 这个符号在整个工作区不存在（最接近的是 s2dex 的 `G_BG_COPY/G_BG_1CYC/G_OBJ_RECTANGLE` 与 `G_BGLT_LOADBLOCK/LOADTILE`）。

**后端矩阵**：`include/fast/Fast3dWindow.h:29` 的枚举只有三个值 —— `FAST3D_DXGI_DX11=1 / FAST3D_SDL_OPENGL=2 / FAST3D_SDL_METAL=3`；`src/fast/Fast3dWindow.cpp:162 InitWindowManager()` 按 `#ifdef ENABLE_DX11 / ENABLE_OPENGL / __APPLE__` 实例化 `GfxWindowBackend*` + `GfxRenderingAPI*`（抽象接口 `include/fast/backends/gfx_rendering_api.h:29`，约 45 个纯虚）。**LUS 没有 Vulkan，也没有 D3D12**（全库唯一命中是 `gfx_dxgi.h:128` 的一行注释）。对比：竞品 `n64-fast3d-engine` 反而有 `gfx_direct3d12.cpp`（完整 root signature/descriptor heap），rt64 通过 plume 有 D3D12/Vulkan/Metal 三后端。

**Shader 是"模板 + 运行时展开编译"**：模板文本以 OTR 资源分发（`f3d.o2r`），用 prism 模板引擎展开后即时编译——GLSL `src/fast/shaders/opengl/default.shader.glsl` → `glCompileShader`（`backends/gfx_opengl.cpp:385-450`）；HLSL `src/fast/shaders/directx/default.shader.hlsl` → `D3DCompile`（动态加载 `D3DCompiler_47.dll`，`gfx_direct3d11.cpp:207-212`）；MSL `src/fast/shaders/metal/default.shader.metal` → `newLibrary`（`gfx_metal.cpp:235`）。**无 SPIR-V、无离线编译**。游戏可换整套 shader 模板（`gfx_get_shader(cc_features.shader_id)`）。

**Combiner：per-key 特化，把 combiner 当"程序"而非"数据"。** `interpreter.cpp:197 GenerateCC()` 从 `combine_mode` 逐 28-bit 周期解出 `rgbA/B/C/D`，做输入去重与归一化（`if (rgbA==rgbB || rgbC==G_CCMUX_0)` 全部折成 0，以提高 shader 复用率），1-cycle 时把 TEXEL1 重映射到 TEXEL0；`CCFeatures`（`interpreter.h:110`）的 `do_single/do_multiply/do_mix` 是**代数强度削减**标志（`:5439 gfx_cc_get_features`）。后端各自持有一份表达式生成器：`gfx_opengl.cpp:194 append_formula` / `gfx_direct3d11.cpp:1333` / `gfx_metal_shader.cpp:120`，按 `do_single → D`、`do_multiply → A*C`、`do_mix → mix(B,A,C)`、否则完整 `(A-B)*C+D` 展开。`LookupOrCreateColorCombiner`（`:427`）**miss 时先 `Flush()` 再建**（`:440`），即 combiner 切换是硬 draw-break。

一个值得记录的**后端不一致**：GLSL 的回绕宏是 clamp，HLSL/MSL 才是真 mod 回绕：

```
src/fast/shaders/opengl/default.shader.glsl:133   #define WRAP(x, low, high) clamp((x), (low), (high))
src/fast/shaders/directx/default.shader.hlsl:188  #define WRAP(x, low, high) MOD((x)-(low), (high)-(low)) + (low)
src/fast/shaders/metal/default.shader.metal:137   #define WRAP(x, low, high) mod((x)-(low), (high)-(low)) + (low)
```
即 OpenGL 路径在 `A-B` 溢出处的语义与 D3D11/Metal 不同（rt64 三后端统一做真回绕，§5.8）。此外全流程浮点、无 5-bit 量化建模；dither 用 `fract(sin(dot(...))*143758.5453)` 噪声（`default.shader.glsl:136`）；`G_CCMUX_LOD_FRACTION` 处有明确的针对性 hack 注释（`interpreter.cpp:2242` "Hack that works for Bowser - Peach painting"）。**没有 accurate/fast 双模式开关**（`grep -i accurate` 只命中数学函数）。

**per-prim 状态如何进 GPU**：组合器的 PRIM/ENV/SHADE/PRIM_LOD_FRAC/KEY_CENTER 等输入**不是 uniform，而是逐顶点写进 VBO**（`interpreter.cpp:2205-2265` 双层循环按 `shader_input_mapping` 把颜色追加进 `mBufVbo`）。只有 noise（per-frame）、texture w/h/filtering（per-draw）、prim_depth（per-prim-depth）走 uniform block：D3D11 三级常量缓冲 `PerFrameCB(b0)/PerDrawCB(b1)/PerPrimDepthCB(b2)`（`include/fast/backends/gfx_direct3d_common.h:23,32,44`，绑定 `gfx_direct3d11.cpp:829`）；Metal 用 `constant FrameUniforms [[buffer(0)]]` / `DrawUniforms [[buffer(1)]]`（`default.shader.metal:165-166`）。

**顶点与矩阵**：CPU 全量移植。RSP 结构里就是硬件语义的 11 层 modelview 栈（`interpreter.h:240 modelview_matrix_stack[11][4][4]` + `MP_matrix`/`P_matrix`）；`GfxSpMatrix`（`:1507`）手工解 Q16.16；`GfxSpVertex`（`:1602`，约 190 行）完成变换→宽高比修正→s/t 缩放→**逐顶点 CPU 光照**（`:1658` 明确注释溯源 GLideN64 `gSP.cpp#L636`）→ `G_TEXTURE_GEN` env-bump（`:1712`，自承公式不确定）→ 平凡裁剪 `clip_rej`（`:1758`，近裁剪面 16 被注释掉）→ fog。顶点缓存**没有 GPU 端复用**：`mBufVbo = new float[256*(32*3)]`（`:113,118`），满 256 三角形自动 flush（`:2298`），GL 侧 `glBufferData(GL_STREAM_DRAW)+glDrawArrays`（无索引、无去重、无持久化），D3D11 侧 `Map(WRITE_DISCARD)+memcpy`（`gfx_direct3d11.cpp:789-822`）。

**分辨率/视口**：`CalcAndSetViewport`（`:2361`，U10.2 → 像素，含 Y 翻转）；`AdjustVIewportOrScissor`（`:2314`，`RATIO_X/RATIO_Y` 缩放 + floor/ceil 取外沿避免右/下边缘少 2px）；`AdjXForAspectRatio`（`:1577`）`x * (4/3) / (w/h)` 是宽屏的核心一行；离屏 framebuffer 通过自定义 GBI `G_SETFB/G_RESETFB/G_COPYFB/G_READFB/G_IMAGERECT/G_SETTIMG_FB`（`:4722-4746`）暴露给 SoH 的 `soh/src/framebuffer_effects.c`（HUD 效果、暂停菜单 Link 预览）。Three-point filtering 即硬件 3-tap 双线性（`default.shader.glsl:147 filter3point()`）。

**节流**：`src/fast/backends/gfx_sdl.cpp:712 SyncFramerateWithTime()`，`FRAME_INTERVAL_US_DENOMINATOR = mTargetFps`（`:303`），Windows 用 waitable timer + `YieldProcessor()` 忙等收尾；`Interpreter::EndFrame()`（`interpreter.cpp:5241`）顺序为 `Rapi->EndFrame → Wapi->SwapBuffersBegin → Rapi->FinishRender → Wapi->SwapBuffersEnd`；DXGI 侧 `mVsyncEnabled` 直接当 `Present()` 的 SyncInterval 并支持 `DXGI_PRESENT_ALLOW_TEARING`（`gfx_dxgi.cpp:921,944-948`）。丢帧由 `Fast3dWindow.cpp:232 if (!IsFrameReady()) return false;` + `Interpreter::mDroppedFrame` 实现（SDL 后端恒 true，`:704`）。

### 3.6 非图形子系统（引擎的另一半）

| 子系统 | 事实 | 证据 |
|---|---|---|
| libultra 兼容层 | **厚头薄实现**：`src/libultraship/libultra/` 8 文件 372 行 vs `include/…/libultra/` 25 头 7,126 行。`osCreateThread`/`osCreateHeap` **LUS 完全没有**，`docs/PORTING.md:55` 要求游戏侧 stub；`osSetTimer(){return 0;}`（`os.cpp:82-84`）、AI `// TODO: Implement`（`:90-98`）、`os_vi.cpp:5-17` 用 `SDL_AddTimer(16ms)` 灌 VI 消息；`os_pi.cpp:17-28` 越界钳制 + 补零模拟"ROM 无边界"；EEPROM 固定 4Kbit、`fopen("default.sav")` 512 B（`os_eeprom.cpp:5-38`）；`gu*` 仅声明，实现在游戏侧（`2ship2harkinian/mm/2s2h/gu_pc.c`） | 同左 |
| bridge API 面貌 | 10 头 / 12 cpp / 1,558 行：`resource gfx window consolevariable controller audio events scripting crashhandler gfxdebugger`。C 边界用 `include/ship/Api.h:17-37` 的 `API_EXPORT`（`extern "C"` + Win `dllexport/dllimport`），配合 `src/CMakeLists.txt:8-9 ENABLE_EXPORTS`；类型只走 POD（`Color_RGBA8` 传值、`bool` 降级 `uint8_t`、返回 `const char*` 注释警告"do not store long-term"）；C++ 组件以文件级 `static shared_ptr` 注入，**空指针时静默返回默认值**以保护 C 侧 | `bridge/consolevariablebridge.h:27` `API_EXPORT int32_t CVarGetInteger(const char*, int32_t);`、`audiobridge.h:36` `AudioPlayerPlayFrame(const uint8_t*, size_t)`、`windowbridge.h:26` `WindowGetAspectRatio()` |
| 音频 | LUS **没有序列播放器**（`SeqPlayer`/`SoundFont`/`AudioBank`/`.ootrs` 0 命中），只做 PCM 设备生命周期 + **Dolby Pro Logic II 矩阵解码**（`SoundMatrixDecoder.cpp` 284 行）。`AudioSettings{44100, 1024, DesiredBuffered=2480, Stereo}`（`include/ship/audio/AudioPlayer.h:14-20`）；后端 `AudioBackend{WASAPI,SDL,COREAUDIO,NUL}`，失败回落 NUL；无 mixer、无 3D 空间化、**无音频线程**；延迟控制只有缓冲水位（`AudioPlayer.h:50-55`）。序列/SoundFont 全在游戏侧（`2ship2harkinian/mm/src/audio/lib/seqplayer.c`、`SoH/soh/soh/Enhancements/audio/`） | 同左 |
| 输入 | **引擎第一大模块**：`ship/controller` 91 文件 7,714 行。`ControlDeck → ControlPort → Controller → {Button,Stick,Gyro,Rumble,LED} → *Mapping` 多态 + factories；设备类型 Keyboard/Mouse/SDLGamepad（SDL3，`os.cpp:21-33` 载入 `gamecontrollerpoolb.txt`）。体感存在但极简：`SDLGyroMapping.cpp:59-63` 直接读 `SDL_SENSOR_GYRO` 减手动标定的 neutral，只用前两轴；**GamepadMotionHelpers 在 LUS 与 SoH 双侧均无证据**（无传感器融合/无加速度计）。震动 `SDLRumbleMapping.cpp:20-31`。`SimulatedInputLag` 是 **6 帧环形缓冲的延迟模拟**（`src/libultraship/controller/.../Controller.cpp:43-74`，cvar `gSimulatedInputLag`，`cmake/cvars.cmake:16`），不是测量工具。重映射直接写 CVar 树（`SDLButtonToButtonMapping.cpp:41-44`，前缀 `gControllers.ButtonMappings.<id>`）。ImGui 抢占用引用计数式 `ControllerBlockGameInput(uint16_t)`（`ControlDeck.cpp:84-109`）。编辑 UI `InputEditorWindow.cpp` 1,368 行 |
| GUI | 纯 ImGui（`v1.91.9b-docking` + 自有补丁，`cmake/dependencies/common.cmake:9-16`）。`ship/window/gui` 28 文件 6,265 行：`Gui.cpp` 438（ctx/后端/纹理表/docking/多视口/手柄导航）、`ConsoleWindow` 756（内置 `set/get/bind/bindtoggle/unbind/clear/help`）、`EventDebuggerWindow` 142（每事件调用者 + `path:line` + 次数）、`StatsWindow` 仅 38 行、`GameOverlay` 277（`TextDraw`/`TextDrawNotification`，只提供 C++ 方法**没有宏**）、LUS 侧 `GfxDebuggerWindow` 769（GBI 反汇编）。字体：`Fonts.h` 2,304 行内嵌 base85 压缩 FontAwesome4。**RmlUi 在本仓未找到证据**（那是 Zelda64Recomp 的 UI 栈） |
| 配置 | **没有 CDI**（grep 无实体），纯 CVar + JSON。`ConsoleVariableManager` 已改名 `Ship::ConsoleVariable : public Component("ConsoleVariables")`（`include/ship/config/ConsoleVariable.h:58`）；值是 type-tagged union（`:16-42`，String 手写 `free()`），查找表用 transparent hash 支持 `string_view` 免分配（`:206-218`）；CVar **寄生在同一份 Config JSON 的 `CVars.` 子对象**（`ConsoleVariable.cpp:258`），颜色拆成 `.R/.G/.B/.A` 四个整型键；文件名由调用方传入（SoH 传 `shipofharkinian.json`、2SoH 传 `2ship2harkinian.json`）；版本迁移抽象在 LUS（`ConfigVersionUpdater`，`include/ship/config/Config.h:23-43`）、具体 updater 在游戏侧；另有两条硬编码迁移：旧 `cvars.cfg`（`LoadLegacy()` 无条件调用）与音频后端 `"pulse"→"sdl"`。**CMake 注入的 24 个 `CVAR_*` 宏**（`cmake/cvars.cmake`）是前缀机制；LUS **完全不解析命令行**（`argc/argv` 在 `src/`+`include/` 0 命中） |
| 用户数据/崩溃 | `Ship::SaveManager` 不在 LUS（在游戏侧）；目录解析 `Context.cpp:333,347,353-360`，`SHIP_HOME` **只在 Apple/Linux 生效**，Android 用 `SDL_GetAndroidExternalStoragePath`、iOS 用 `$HOME/Documents`（判自定义宏 `__IOS__`）；`CrashHandler.cpp` 477 行（Win：`SetUnhandledExceptionFilter` + DbgHelp，条件 dump x86/ARM64 寄存器；POSIX：`sigaction` 收 `SIGILL/ABRT/FPE/SEGV`，`backtrace_symbols`，Linux/macOS 共用同一实现）+ 扩展点 `CrashHandlerRegisterCallback`；日志 `Logger.cpp` 96 行：rotating file sink 10 MB×10 恒开，Release 走 `async_logger` + `warn` + `overflow_policy::block`，Debug 同步 + `trace`，pattern 含 `[%@]`（源文件行号） |
| 脚本/mod 运行时 | 顶层 `ENABLE_SCRIPTING`（默认 **OFF**）+ `ENABLE_TCC_COMPILER`（默认 ON）。`ScriptLoader.cpp` 405 行用 TinyCC 在内存里编译 mod C 源码：`tcc_define_symbol(s,"__DLL__","1")`（`:174`，让 `HM_API` 在 JIT 下走对分支）→ `TCC_OUTPUT_MEMORY` → `tcc_compile_string` → `loader.InitInMemory(s)`（`:264-266` 注释与 HEAD 提交标题一致："不落盘、不 dlopen，避免杀软误报"）；入口约定 `ModInit/ModExit`；C 侧唯一出口 `ScriptGetFunction(module, function)` 返回裸函数指针。`ENABLE_TCC_COMPILER=OFF` 时改走 `LoadLibraryA/dlopen` 加载 `binaries/<platform>`。**信任链**：`SafeLevel{DISABLE_SCRIPTS, ONLY_TRUSTED_SCRIPTS, WARN_UNTRUSTED_SCRIPTS(默认), ALLOW_ALL_SCRIPTS}`（`ScriptLoader.h:22-26`）+ `Keystore.cpp` 94 行把 `keys/script/*.pem`（6 个：HM64 + Kenix1-5）经 `tools/generate_keys_header.cpp` 编成的 `DefaultKeys.h` 以 `KeyOrigin::System` 灌入；未知公钥走 `GetUntrustedArchiveHandler()` 用户批准流程。与 §3.4 的"后挂载覆盖"合起来即"改贴图丢文件、改逻辑写 C 塞进 o2r"的免重编 mod 能力。`docs/SCRIPTING.md:16,44-58,117,162-174,218` 是格式规范（manifest 字段、`code_version` 强制匹配、`build.gen`/`dist/mod.c`/`binaries/`） |
| 平台与测试 | `docs/PORTING.md:19-26` 官方矩阵：Windows→GL/D3D11、macOS→GL/Metal、Linux→GL、iOS→GL/Metal、Android→GLES、OpenBSD→GL。CI 6 job（macos-14、iOS 14.0、linux、android NDK r29 arm64-v8a `c++_static`、windows x64、windows arm64）。源码宏命中：`_WIN32` 26 / `__APPLE__` 15 / `__linux__` 13 / `__ANDROID__` 8 / `__IOS__` 6 / `__CYGWIN__` 5 / `__OpenBSD__` 2；**Switch/WiiU/3DS/PS/Xbox 在 LUS 零命中**（SoH 里的那些分支属于游戏侧）。构建选项：`NON_PORTABLE`、`INCLUDE_PROFILING`、`COMPONENT_THREAD_SAFE`、`DISABLE_DLL_LOADER`、`INCLUDE_MPQ_SUPPORT`、`GBI_UCODE`(默认 F3DEX_GBI_2)、`USE_OPENGLES`、`GFX_DEBUG_DISASSEMBLER`、`LUS_VERSION_{EPOCH,MAJOR,MINOR,PATCH}`。测试 `tests/` 29 个 cpp / 7,521 行（GTest v1.16.0，`LUS_BUILD_TESTS` 默认 OFF 且会 FORCE 打开 `COMPONENT_THREAD_SAFE`）；覆盖 ECS/资源/归档/二进制 IO/CRC64/矩阵解码，**空白**在 CVar 持久化、GUI、音频后端、CrashHandler、libultra 兼容层；CI 只在 Linux+Windows 跑 ctest |

十大文件（行数）：`src/fast/interpreter.cpp` 5,546 → `include/libultraship/libultra/gbi.h` 4,311 → `include/ship/window/gui/Fonts.h` 2,304 → `src/fast/backends/gfx_direct3d11.cpp` 1,472 → `include/fast/lus_gbi.h` 1,380 → `src/libultraship/window/gui/InputEditorWindow.cpp` 1,368 → `src/fast/backends/gfx_metal.cpp` 1,302 → `src/fast/resource/factory/DisplayListFactory.cpp` 1,198 → `src/fast/backends/gfx_dxgi.cpp` 1,144 → `src/fast/backends/gfx_opengl.cpp` 1,119。

### 3.7 LUS 的版本关系（一条容易被忽略的风险）

`Shipwright` pin 的是 `kenix3/libultraship` 的 **`port-maintenance` 分支 @ `da6daf1a`**，而本地独立克隆是 `main` @ `759b134`；由于子模块未检出、本地又没有 `port-maintenance` 分支，**两者的具体代码差异无法比对**。但可以从 SoH HEAD 的 include 与调用点确定它已经在用 epoch-2 的 API 面貌：

```cpp
/* Shipwright/soh/soh/OTRGlobals.cpp:30,63 与 :20 */
#include <ship/resource/archive/O2rArchive.h>
#include <fast/interpreter.h>
#include <ship/resource/File.h>
/* :2241,2257,2462 等 5 处 assert(false && "Lost reference to Fast::Interpreter"); */
```

LUS 自己给出了迁移文档：`libultraship/docs/migration/EPOCH-2.md:7-19` "migration path from the `port-maintenance` line to libultraship epoch 2"，列出六大影响面（Context 创建与组件生命周期、bridge cache/singleton holder、资源 identifier/init-data、事件系统、Window/GUI/input 面、构建/版本）。**实践含义**：阅读 SoH 相关二手资料时，凡是出现 `src/graphic/Fast3D/`、`gfx_run(cmds)`、`Fast3D::` 命名空间的描述，都属于 epoch 1 布局；本报告所有 §3.5/§3.6 的路径都是 epoch 2（`src/fast/`、`Ship::`），本地不存在任何名为 `Fast3D` 的目录。

---

## 4. 资产工具链与容器格式族

> 取证对象：`$P/ZAPDTR`、`$P/OTRExporter`、`$P/Torch`、`$P/retro`、`$P/neoretro`，以及镜像 `$P/../oot-ports-repos/ZAPD`（zeldaret 上游）、`oot-ports-repos/fast64`。
> 快照声明：所有仓库都是浅克隆（1 commit），**提交日期只代表 HEAD 快照日期，不代表真实历史**；§4.6 的时间线因此在方法上属于"由代码依赖关系推断 + 快照日期佐证"。

### 4.1 ZAPDTR：ROM → XML + C + `.bin`

`ZAPDTR/README.md:1` 自称 "ZAPD: Zelda Asset Processor for Decomp"。注意源码不在 `ZAPDTR/src/` 而在 **`ZAPDTR/ZAPD/`**。

* **类型注册是自注册工厂，不是 switch**：`ZResource.h:246` 定义 `REGISTER_ZFILENODE(nodeName, zResClass)`，构造静态对象时调 `ZFile::RegisterNode`（`ZFile.cpp:1086-1089`，map 在 `:1082`）；解析按 XML 节点名查表：

```cpp
/* ZAPD/ZFile.cpp:280-290（节选） */
std::string nodeName = std::string(child->Name());
if (nodeMap.find(nodeName) != nodeMap.end()) {
    ZResource* nRes = nodeMap[nodeName](this);
    if (mode == ZFileMode::Extract || mode == ZFileMode::ExternalFile ||
        mode == ZFileMode::ExtractDirectory) {
        if (!isCompilable) nRes->ExtractWithXML(child, rawDataIndex);
```

  真实注册点：`ZBlob.cpp:10`、`ZDisplayList.cpp:137`、`ZVtx.cpp:7`、`ZTexture.cpp:19`、`ZMtx.cpp:7`、`ZBackground.cpp:11`、`ZAnimation.cpp:12-15`（Animation/PlayerAnimation/CurveAnimation/LegacyAnimation）、`ZPlayerAnimationData.cpp:8`、`ZCollision.cpp:12`、`ZRoom/ZRoom.cpp:50-52`（Room/Scene/AltHeader）、`ZSkeleton.cpp:10-11`、`ZLimb.cpp:11`、`ZCutscene.cpp:11`、`ZText.cpp:10`、`ZTextMM.cpp:10`、`ZAudio.cpp:10`、`ZWaterbox.cpp:7`、`ZSurfaceType.cpp:7`、`ZArray.cpp:10`；枚举 `ZResource.h:25-68`。**`MTex` / `MaterialList` 作为独立节点在 HEAD 不存在**（只有 room 命令 `ZRoom/Commands/SetAnimatedMaterialList.cpp`）——`MTex` 属早期版本。
* **fork 独有能力：直接在内存里切 ROM**。`ZRom.cpp` 内置 OoT/MM 全部版本的 DMA 表偏移与 CRC 表：

```c
/* ZAPD/ZRom.cpp:1-40（节选） */
#define OOT_OFF_NTSC_10 0x7430
#define OOT_OFF_NTSC_12 0x7960
#define OOT_OFF_PAL_GC  0x7170
#define MM_OFF_US_10    0x1A500
```
* **`XmlWriter` / `BoundsChecker` 在 HEAD 无同名文件**（全库零命中）；职能由 `OutputFormatter.h:6`（C 代码格式化）与 `ZFile::HandleUnaccountedData()` 承担，且后者在 OTR 模式下直接短路：`ZFile.cpp:1332-1340 if (Globals::Instance->otrMode) return;`
* **模式与开关是真实存在的那几个**：`ZFileMode` 在 `ZFile.h:12-24`（含 fork 独有的 `ExtractDirectory` 与 `Custom = 1000`），`Main.cpp:469-490 ParseFileMode` 把 `btex/bren/bsf/bblb/e/ed` 映射过去；`Globals.h:51 bool otrMode = true`、`:53 onlyGenCustomOtr`、`:54 xmlExtractModes`；CLI `-se/--set-exporter`（`Main.cpp:432-433`）、`--norom`→`onlyGenCustomOtr`（`:585-588`）、`-brt`→`buildRawTexture`（**唯一**的 raw 纹理开关）、`-fl` 文件列表、`-oxml asf,asp,asq`。**`--otr`/`SoHExport`/`RawDump`/`n64pngs` 这些网传参数名不存在**。
* **插件机制**：`ExporterSet.h:11-28`（11 个钩子）+ `ZResource.h:262 REGISTER_EXPORTER`；`Main.cpp:568-571 ImportExporters()`；`ZFile.cpp:389-440` 依次触发 `beginFileFunc`/`exporter->Save`/`resSaveFunc`/`endFileFunc`，`:428-434` 把带自定义导出器的资源写成 `<File>.bin`；样例 `ExporterTest/Main.cpp:59-79`。
* **与上游 ZAPD 的差异**（`oot-ports-repos/ZAPD`）：ZAPDTR 独有 `ZRom`、`ZText/ZTextMM/ZAudio/ZAudioDecode`、`FileWorker`（多线程）、`Overlays`、`yaz0`、内联 `Utils/`；上游把 Utils 独立成 `ZAPDUtils/`，`ZFileMode` 没有 `ExtractDirectory`，flag 表有 `--cs-float/--base-address/--start-offset/--end-offset` 但没有 `-fl/-brt/--norom/-oxml`。两版都有 `REGISTER_ZFILENODE` 与 `ExporterSet.h`——**注册/插件骨架已回流上游，"从 ROM 直抽 + OTR bin 输出"仍是 fork 独有**。

### 4.2 OTRExporter：ZAPD 的导出插件（不是替代品）

硬证据：`OTRExporter/OTRExporter/TextureExporter.cpp:2` `#include "../ZAPD/ZFile.h"`；`Main.cpp:437` `Globals::AddExporter("OTR", …)`；驱动脚本 `extract_assets.py:12-36` 展示的实际命令行：

```
ZAPD ed -i <xml> -b <rom> -fl assets/extractor/filelists -gsf 0 -se OTR \
       --customAssetsPath ... --otrfile oot.o2r --portVer ...
```

自定义资产走另一条模式：`ZAPD botr -se OTR --norom ...`（`extract_assets.py:47-53`；`botr` 由 `OTRExporter/Main.cpp:56-75` 注册为 `ZFileMode::Custom+1`）。MM 侧用 `#ifdef GAME_MM` 条件注册额外类型（`Main.cpp:432-436`）。64 字节头的写出见 §3.2。

### 4.3 Torch：YAML 驱动的通用资产工厂（最新一代）

`Torch/README.md:1`："[T]orch is [O]ur [R]esource [C]onversion [H]elper / A generic asset processor for games"，用法 `./torch otr baserom.z64`、`./torch code baserom.z64`。

* **`TemplateFactory.{h,cpp}` 不是生产逻辑，而是工厂模板**（类名一律 `Type*`），演示三件事：`parse()` 解 segment、三种 exporter（Header/Code/Binary）、以及 OTR 模式下头部只写占位符：

```cpp
/* Torch/TemplateFactory.cpp:13-20 */
if (Companion::Instance->IsOTRMode()) {
    write << "static const char " << symbol << "[] = \"__OTR__" << (*replacement) << "\";\n\n";
    return std::nullopt;
}
write << "extern Type " << symbol << ";\n";
```
  真正的框架在 `src/factories/BaseFactory.h`：`BaseExporter::Export` 纯虚 + `static WriteHeader`（`:99`）、`BaseFactory::parse/parse_modding`（`:104-105`）、`ExportType {Header,Code,Binary,Modding,XML}`（`:46-52`）、`REGISTER(type,c)` 宏（`:30`）。
* **多游戏化**：`src/factories/{oot,sm64,mk64,sf64,pm64,bk64,fzerox,mario_artist,naudio}` —— 这也解释了为什么 OoT 移植的资产工具正在向 Torch 收敛，而 **`src/factories/` 下没有 mm 目录**（MM 的 4 个独有类型 `OTXM/OTAN/OKFA/OKFS` 尚无落点），迁移未覆盖 MM。
* **同时写两种容器**：`src/Companion.h:51-56 enum class ArchiveType {None, OTR, O2R}`；OTR 走 `src/archive/SWrapper.cpp:17-30`（`SFileCreateArchive(..., 16*1024, &hMpq)`）+ `:69-80`（>32 位抛异常），O2R 走 `src/archive/ZWrapper.cpp:33-46`（`mz_zip_writer_init_file(path + ".tmp")` + `MZ_BEST_COMPRESSION`）+ `:78-101`（finalize 后 `rename`）。CLI：`src/main.cpp:139-156` `o2r` 子命令、`:229-236 pack <archive-type> otr|o2r`、`:269-293` 三种 modding import 模式。
* **根条目**：`portVersion`/`version`（`Companion.cpp:1905-1912`，`ParseVersionString` `:2560-2578`）与 `torch.hash.yml`（`:1930-1933`）。
* **文档不一致（应记录为已知问题）**：`Torch/docs/otr_format/mainpage.md:9-12` 声称"`.otr` 或 `.o2r` 都是标准 ZIP 归档"，与 `SWrapper` 的 MPQ 实现相矛盾。

### 4.4 retro / neoretro：面向 mod 作者的打包 GUI

`$P/retro` 修复后是一个 **Flutter/Dart 桌面工程**：`pubspec.yaml:1-2` "retro / An OTR / O2R generation tool."；`lib/otr/resource.dart:17` `int magicID = 0xDEADBEEFDEADBEEF`，`_writeHeader()`（`:26-41`）写出的正是与 OTRExporter 同构的 64 B 头（native endian @0x00、type @0x04、gameVersion @0x08、magicID @0x0C、resourceVersion @0x10、isCustom @0x14，其余补零到 0x40），`lib/otr/resource_type.dart:1-27` 复刻 magic 表，`lib/arc/arc.dart:24-40` 按扩展名分派（`.otr` → `MPQArchive.create(path, MPQ_CREATE_SIGNATURE | MPQ_CREATE_ARCHIVE_V2, 65535)`，否则 `ZipFileEncoder`；`.o2r` 解码走 `zdec.decodeBytes`）。README 顶部宣布**已停维护**：`retro/README.md:5-11` "# Retro has moved to NeoRetro … It has been retired in favor of **NeoRetro** … This repository is kept for reference only."

后继者 `$P/neoretro`（本次新增克隆）是 C++/CMake 桌面工程：130 个 C/C++ 文件、13,118 行（cpp 11,136 + h 1,982），目录 `src/{app,features,games,image,l10n,models,otr,services,ui,utils}`、`tests/`、`tools/`、`docs/texture-packs.md`。要点：

* **UI 栈是 raylib + ImGui(rlimgui) + clay**：`CMakeLists.txt:190-246` 依次 FetchContent `raylib`、`ImGui`、`rlimgui`、`clay`，另拉 `spdlog`、`lodepng`、`stb`、`picosha2`。
* **它把 Torch 当库用**：`CMakeLists.txt:252-265` `FetchContent` 固定 `GIT_TAG 72960ca16f…`（本地浅克隆 HEAD 是 `22d32182…`，**版本不一致，本节对 Torch 的引用以本地快照为准**），并 `foreach(TORCH_GAME SM64 MK64 SF64 PM64 FZERO BK64 MARIO_ARTIST NAUDIO OOT) set(BUILD_${TORCH_GAME} OFF)` —— 这份列表等于官方承认的 Torch 游戏矩阵；同时用 `cmake/patches/torch-swrapper-close.patch` 给 Torch 的 `SWrapper::Close` 打补丁（修正"关闭失败被反向判断"并把 `hMpq` 置空 + 打印 `GetLastError()`）。
* **格式侧直接复用 Torch 枚举**：`src/otr/Types.h:5-6` `using ResourceType = Torch::ResourceType`，同文件 `TextureType` 11 项（含 `JPEG32bpp = 10`）、`kTextureFlagLoadAsRaw = 1 << 0`；`src/otr/Archive.h:13-40` 用 `ArchiveReader`/`ArchiveWriter` 抽象同时覆盖 MPQ 与 ZIP（`bool IsZip()`）。
* **功能面按游戏分屏**：`src/features/games/GameScreens.h:5-30` 有 `SohGameScreen / GhostshipGameScreen / PaperboatGameScreen / Ship2GameScreen`，`src/app/Routes.cpp:26-37` 注册对应路由（SoH 主屏 + 地图字体转换、Ghostship 文本三类 dialogs/texts/strings、Paperboat 图像与文本），另有 `src/features/inspect_otr/InspectOtrScreen.*` 归档浏览器与 `src/features/create`、`src/features/debug`。
* README 仍自述 "An OTR generation tool … Currently only Ship of Harkinian uses this format"，并描述了 `appcast.json` 自更新流程（SHA-256 校验后替换自身）。

结论：`retro` **既不是 retro-patch，也不是 SoH 的分支**，而是 OTR/O2R 手工打包器 —— 它和 `2ship2harkinian/README.md:67` 的推荐语一致。

### 4.5 容器与格式族对照（本报告 §3.2 的镜像视图）

| 维度 | `.otr`(MPQ) | `.o2r`(ZIP) | Folder | `.ootrs` |
|---|---|---|---|---|
| 解析侧 | `OtrArchive.cpp:22-62,71` | `O2rArchive.cpp:41-96` | `FolderArchive.cpp:22-41` | SoH `OotrsArchive.h:29`（内含 `O2rArchive`） |
| 写出侧 | OTRExporter `ExporterArchiveOTR.cpp:63-66,100`（`MPQ_COMPRESSION_ZLIB`）；Torch `SWrapper.cpp` | OTRExporter `ExporterArchiveO2R.cpp:51-77`；Torch `ZWrapper.cpp` | n/a | Torch `--norom` 系 |
| 编译开关 | `INCLUDE_MPQ_SUPPORT`（LUS 默认 OFF / SoH 强制 ON） | 无（libzip 必需） | 无 | 游戏侧 |
| 可写 | 否（显式拒绝） | 是 | 是 | — |
| 尺寸上限 | 32 位 `DWORD` | `size_t` | 文件 | ZIP |
| 索引来源 | `(listfile)` | ZIP 中央目录 | `Directory::ListFiles` | ZIP + 歌曲表 |
| 签名/manifest | 共用（`Archive.cpp:187-300`，BLAKE2b-512 + Ed25519 + keystore） | 同左 | 同左 | 未验证 |

产物侧现状：SoH 已完全产出 `oot.o2r / oot-mq.o2r / soh.o2r`（`OTRGlobals.cpp:281-282,418-419,616,841-848`；`CMakeLists.txt:309-349`；CI `generate-builds.yml:35-43`），mod 收集时 `.o2r` 总收、`.otr` 仅当 MPQ 支持开启时收、`.zip` 故意不收、`.ootrs` 单列（`Enhancements/mod_menu.cpp:194-210`）。

### 4.6 工具链取代关系（含推断标注）

1. **ZAPD（zeldaret）**：decomp 侧原始工具，XML→C，需要预切 baserom。
2. **ZAPDTR（HarbourMasters fork）**：加 ROM 内切分（`ZRom`）、`ExtractDirectory`/`-fl`、`otrMode`、`FileWorker` 多线程、MM 类型。依据：文件集合与 flag 集合的差集。
3. **OTRExporter**：ZAPDTR 的**插件**（§4.2 的 `#include` 与 `-se OTR` 是硬证据），把 ZAPD 输出直接变 `.bin`→`.o2r`。2SoH 至今仍是这条链。
4. **retro → neoretro**：面向 mod 作者的手工打包 GUI，README 明说 retired。
5. **Torch**：把 ZAPDTR+OTRExporter 的职责合并进 YAML factory（同一套 magic/64 B 头、同时写 OTR/O2R）并扩到 SM64/MK64/SF64/BK64。取代关系的硬证据在 Shipwright 侧：`.gitmodules` 移除 ZAPDTR/OTRExporter 而挂 `torch`；`CMakeLists.txt:327-335` 承认 Torch 尚缺 OoT header 导出；`docs/BUILDING.md:83` 直言迁移删掉了 ZAPD-based header generator。
6. **未完成迁移**：2SoH 仍嵌 ZAPDTR + OTRExporter；Torch 无 mm factory 目录。

> 本节的"取代/收敛"叙事基于代码依赖关系与文档措辞，属**强推断**；浅克隆无法用提交时间序列独立验证。

### 4.7 fast64 与 ZAPD 的正反向互补

`oot-ports-repos/fast64/README.md:5-9`：导出 F3D 显示列表 + SM64/OoT **decomp 工程资产**，支持自定义 combiner/geom mode。导出面 `fast64_internal/z64/exporter/{actor,collision,cutscene,scene,skeleton,animation}`（`z64/README.md:23-91` 逐项说明场景/actor 放置/碰撞体含水陆空与相机/灯光/出口索引/骨骼 SkinLimbs/过场）。微码枚举 `f3d/f3d_enums.py:380-393`：F3D、F3DEX/LX、**F3DEX2/LX2/ZEX（注释明确 "used in OoT and MM"）**、F3DEX2.Rej、F3DEX3 及多种顶点打包方案。与 ZAPD/TR 的关系：**fast64 做 Blender→C/dlist 的正向创作，ZAPD/TR 做 ROM→XML/C 的反向导出**（此句为基于两者文档面的推断）。值得注意的是 LUS 的 dlist 资源就是词流（`src/fast/resource/factory/DisplayListFactory.cpp:170-223`：int8 ucode → 8 字节对齐 → 逐条 w0/w1 → 扩展 op 多读 64 bit），因此 fast64 产出的 dlist 与移植引擎读取的 dlist **同构**，这是"用 Blender 重做素材再塞回 o2r"可行的根本原因。

---

## 5. 重编译路线：Zelda64Recomp + N64ModernRuntime + rt64

> 取证对象：`$P/N64Recomp`、`$P/N64ModernRuntime`、`$P/rt64`、`$P/Zelda64Recomp`。
> **重大快照限制**：`Zelda64Recomp` 的 7 个 submodule（`lib/N64ModernRuntime@df7e820`、`lib/rt64@23cab60`、`lib/RmlUi@7a06f27`、`lib/mm-decomp@5607ee`、`lib/lunasvg`、`lib/freetype-windows-binaries`、`Zelda64RecompSyms@a325f8f`）与 `rt64` 的 15 个 `src/contrib/*` submodule（含 **plume@d890ac8**、dxc、re-spirv、spirv-cross、imgui…）**全部未初始化**（`git submodule status` 前缀 `-`）。因此：① 运行时/渲染器只能用**本目录独立克隆的 HEAD**读，与 Zelda64Recomp 钉住的版本存在偏差；② rt64 的 D3D12/Vulkan/Metal 后端实现（在 Plume 内）**本次无法阅读**，§5.4 中相关论断只能由 `CMakeLists.txt` 与 README 佐证；③ `Zelda64RecompSyms/*.syms.toml` 缺失，符号表实际内容不可验证（只能读 `N64Recomp/src/symbol_lists.cpp` 的内嵌表）。

### 5.1 路线谱系与职责切分

```
                    N64Recomp/N64Recomp（离线静态重编译器，emit C++）
                          │  + rabbitizer（指令解码）+ sljit（LiveRecomp 用）
        ┌─────────────────┼───────────────────────────┐
        │                 │                           │
  N64ModernRuntime   Zelda64RecompSyms           RSPRecomp（ucode 重编译）
  ultramodern +      （符号 toml，缺失）
  librecomp
        │                 │
        └──── rt64（渲染器，Plume 提供 RHI）────┘
                          │
                   Zelda64: Recompiled（前端/UI/patches/mod）
```

* **org 名称即证据**：`N64ModernRuntime/.gitmodules` 写 `submodule.N64Recomp.url=https://github.com/N64Recomp/N64Recomp`（本次早前按 `HarbourMasters/N64Recomp` 拉取失败是 404，不是网络问题）。Zelda64Recomp README 则用旧署名 `Mr-Wiseguy/N64Recomp`、`rt64/rt64`（`Zelda64Recomp/README.md:2-3`）。
* **职责切分照 README 原文**（`N64ModernRuntime/README.md:5-27`）：`ultramodern` = libultra 核心重实现（Threads / Controllers / Audio / Message Queues / Timers / RSP Task Handling / VI timing），平台相关 I/O 由项目提供回调；`librecomp` = "bridge the gap between code generated by N64Recomp and ultramodern"，补 Overlay handling、PI DMA（ROM reads）、EEPROM/SRAM/Flashram 存档；推荐渲染器 "RT64"。
* **规模**：`ultramodern` 2,480 行、`librecomp` 10,991 行、`N64Recomp`（src+include）8,511 行、`rt64`（cpp/h/hpp/slang）70,536 行、`Zelda64Recomp/patches` 20,524 行。
* 该路线**不需要反编译工程**：它直接吃 ROM 解出的 ELF + 符号表，产出 C++ 再编译，因此游戏逻辑保持 100% 原始 MIPS 语义（`N64Recomp/README.md:18` 明确 "The recompiler has mostly been tested on binaries built with old MIPS compilers (e.g. mips gcc 2.7.2 and IDO) as well as modern clang targeting mips"）。

### 5.2 N64Recomp：从 MIPS 到 C++ 的字面翻译

README 的自述即是最好的规格说明（`N64Recomp/README.md:16-27`）：逐指令字面翻译（`addiu $r4,$r4,0x20` → `ctx->r4 = ADD32(ctx->r4, 0X20);`）、`jal`→函数调用、可识别的尾调用 `j/b` 也转成调用、**分支延迟槽靠复制指令处理**、`jr`+跳表能识别时转成 switch-case、可重定位 overlay 里带 reloc 的 `lui/addiu/load/store` 额外包一层宏：`ctx->r24 = S32(RELOC_HI16(1754, 0X630) << 16);`。

**（1）内存模型：无 byteswap + 按字节 lane 的 XOR 校正。** `include/recomp.h` 全文 **0 次** `byteswap`，只有 5 个读宏 + 双字写宏：

```c
/* N64Recomp/include/recomp.h:95-118（节选） */
#define MEM_W(offset, reg) \
    (*(int32_t*)(rdram + ((((reg) + (offset))) - 0xFFFFFFFF80000000)))
#define MEM_H(offset, reg) \
    (*(int16_t*)(rdram + ((((reg) + (offset)) ^ 2) - 0xFFFFFFFF80000000)))
#define MEM_B(offset, reg) \
    (*(int8_t*)(rdram + ((((reg) + (offset)) ^ 3) - 0xFFFFFFFF80000000)))
#define SD(val, offset, reg) { /* 高低字分别写入 +4 / +0 */ }
```

  要点：① 基址用 `reg - 0xFFFFFFFF80000000` 把 kseg0 虚拟地址折回 rdram 偏移（符号扩展的 64 位指针）；② MIPS 是大端，子字访问靠 `^2 / ^3` 把字节 lane 翻到小端主机上等价位置，`MEM_HU/MEM_BU` 同理（`:104-107`）；③ 双字读靠 `load_doubleword()` 手工拼 hi/lo（`:113-121`）。
  **kseg1（0xA0…）不在这套宏的照顾范围内**——这正是 Zelda64Recomp 要在指令级把它改成 kseg0 的原因（`Zelda64Recomp/us.rev1.toml:26-31` 注释："This saves the recompiler from needing to handle checking for kseg1 addresses on every memory operation"）。
  另一处佐证：`N64ModernRuntime/ultramodern/include/ultramodern/ultra64.h:102` 注释 "These two are swapped to reflect rdram byteswapping" —— rdram 里存的是**按 32 位字字节序翻转后的镜像**（ROM 解压阶段处理，见 `Zelda64Recomp/src/game/rom_decompression.cpp`）。

**（2）指令覆盖：数据驱动的 4 张表，合计 157 个 `InstrId`。** HEAD 已经不是"每种指令一个 emitter"的老结构，而是把语义压成表 + 通用生成器：

| 表 | 定义处 | 条目数（本次统计） |
|---|---|---|
| `unary_ops` | `src/operations.cpp:4` | 35 |
| `binary_ops` | `src/operations.cpp` | 92（含 load：`BinaryOpType::LD/LW/LWU/LH/LHU/LB/LBU/LDL/LDR/LWL/LWR`，`include/recompiler/operations.h:104-116`） |
| `conditional_branch_ops` | `src/operations.cpp` | 20 |
| `store_ops` | `src/operations.cpp` | 10（`StoreOpType{SD,SDL,SDR,SW,SWL,SWR,SH,SB,SDC1,SWC1}`，`operations.h:12-23`） |
| 合计 | — | **157** |

  指令解码外包给 **rabbitizer**（`lib/rabbitizer`，`operations.h:8-9` `using InstrId = rabbitizer::InstrId::UniqueId; using Cop0Reg = rabbitizer::Registers::Cpu::Cop0;`）。`Operand` 枚举里能看到 FR 位/NaN 处理的痕迹：`FdU32L/FsU32H/...` 注释 "Raw low 32-bit values of FPRs with handling for mips3 float mode behavior"（`operations.h:130-141`），`UnaryOp::check_fr` / `check_nan`（`:171-178`）配合 generator 的 `emit_check_fr` / `emit_check_nan`（`generator.h:51-52`）。
  **系统调用面**：`emit_syscall` → `recomp_syscall_handler(rdram, ctx, 0x{:08X})`、`emit_do_break` → `do_break(vram)`、`emit_pause_self` → `pause_self(rdram)`、`emit_trigger_event` → `recomp_trigger_event(rdram, ctx, base_event_index + N)`（`src/cgenerator.cpp:544-560`）。
  **未覆盖项（已核对，零命中）**：全仓 `src/ include/` 内 **搜不到 `bc0`、`fcr31`、`Fcr31`**；CP0 只暴露 Status 一个寄存器（`include/recomp.h:437-438` `cop0_status_write/read(recomp_context*, gpr)`）。也就是说：协处理器分支与 FPU 控制/状态寄存器不参与重编译，中断/异常/TLB 一概不建模——这是"只跑游戏二进制正常路径"的设计取舍。

**（3）双 Generator 后端。** `include/recompiler/generator.h:28-66` 定义 30 个纯虚方法的 `Generator` 接口，实现只有两个：`CGenerator final : Generator`（同文件 `:70`，输出 C++ 源码；公开 API `recompile_function_custom(Generator&, ...)` 在 `context.h:595`）与 `LiveGenerator final : Generator`（`include/recompiler/live_recompiler.h:90`，基于 sljit，API `recompile_function_live` 在 `:146`，`friend class LiveGenerator` 在 `:67`）。**即：静态路线默认产出 C++ 交给宿主编译器优化；只有 mod 的运行时补丁才走 JIT。** 延迟槽逻辑在两后端共用的驱动里：

```cpp
/* N64Recomp/src/recompilation.cpp:215-232（节选） */
auto process_delay_slot = [&](bool use_indent) {
    if (instr_index < instructions.size() - 1) {
        ...
        if (!process_instruction(generator, context, func, func_index, stats, ...,
                                  instr_index + 1, instructions, output_file, use_indent, false, ...))
            return false;
    }
    return true;
};
```

 `print_return_with_delay_slot` / `print_goto_with_delay_slot` / `print_func_call_by_register` 都先复制延迟槽再生成 `emit_return/goto`（`:238-266`），再 `print_link_branch()` 跳到 `after_{link_branch_index}`；`bl`/`bcl` 这类 likely 分支用 `in_likely_delay_slot` 状态机在跳过时插入 goto（`:837-875`）。跳表在 `cgenerator.cpp:469` 生成 `switch (var >> 2) {`，`JumpTable{addend_reg, lw_vram, addu_vram, addu_gp_vram, jr_vram, got_offset, entries}` 由分析阶段填（`context.h:39-56`）。

**（4）配套工具与产物形态。** `N64Recomp/` 顶层目录：`RSPRecomp/`（把 RSP ucode 也静态重编译，配置如 `$P/Zelda64Recomp/aspMain.us.rev1.toml`、`njpgdspMain.us.rev1.toml`）、`LiveRecomp/`（`live_generator.cpp` + `live_recompiler_test.cpp`）、`OfflineModRecomp/`（`main.cpp`，配合 `.offline.nrm`，见 §5.5）、`RecompModTool/`、`RecompModMerger/`；`lib/`  vendored `ELFIO fmt rabbitizer sljit tomlplusplus`。`src/symbol_lists.cpp`（668 行）是**三张 `std::unordered_set<std::string>`**：`reimplemented_funcs`（`:3-141`，116 条，运行时已重实现、重编译器不再出码，如 `__ull_to_f`）、`ignored_funcs`（`:143-563`，384 条，OS 初始化/网络/调试一类直接丢弃，如 `__createSpeedParam`、`__osInitialize_common`、`send`、`kdebugserver`）、`renamed_funcs`（`:564-668`，83 条，改名让位给运行时实现，如 `sincosf/sinf/cosf`）。这正是"游戏码一行不改也能链接到现代 libultra 替身"的机制，与 SoH 的 `VB_*` + `GameInteractor` 挂钩是两种完全不同的桥接哲学。`src/{analysis,elf,mdebug,mod_symbols,config,recompilation,cgenerator,main}.cpp` 构成 ELF(STABS/mdebug)→分析→重编译→输出的主流水。

### 5.3 N64ModernRuntime：libultra 的运行时替身

`ultramodern/include/ultramodern/`：`ultra64.h`（类型定义）、`threads.hpp`、`input.hpp`、`rsp.hpp`、`ultramodern.hpp`、`config.hpp`、`events.hpp`、`extensions.h`、`renderer_context.hpp`、`error_handling.hpp`。
`librecomp/include/librecomp/`：`addresses.hpp config.hpp mods.hpp patcher.hpp overlays.hpp rsp.hpp rsp_vu.hpp rsp_vu_impl.hpp files.hpp game.hpp sections.h helpers.hpp euc-jp.hpp pak.cpp?`；`librecomp/src/` 29 个文件覆盖 `ai/cont/dp/eep/flash/pi/sp/vi/rsp/heap/print/files/mods/mod_hooks/mod_events/mod_manifest/mod_config_api/overlays/pak/recomp/ultra_stubs/ultra_translation/math_routines/extensions/euc-jp/config(_option)`。

* **函数签名 ABI 就是路线的"桥"**：`Zelda64Recomp/src/main/main.cpp:347-348` `extern "C" void recomp_entrypoint(uint8_t* rdram, recomp_context* ctx); gpr get_entrypoint_address();`，`librecomp/include/librecomp/game.hpp:21-36` 的 `GameEntry` 要求提供 `entrypoint_address` + `void (*entrypoint)(uint8_t* rdram, recomp_context* context)`，并附带 `rom_hash / internal_name / game_id / mod_game_id / save_type / is_enabled / decompression_routine / has_compressed_code`。
* **VI 事件与 dlist 事件是 mod 的挂钩点**：`ultramodern/events.hpp:7-20` 只暴露两个回调（`vi_callback_t* vi_callback` 注释 "Called in each VI"、`gfx_init_callback_t* gfx_init_callback`）+ `set_callbacks()`；更细的 dlist 生命周期在 `ultramodern/extensions.h:18-38`：

```c
typedef enum {
    OS_EX_DISPLAYLIST_EVENT_SUBMITTED,  // 提交给渲染器
    OS_EX_DISPLAYLIST_EVENT_PARSED,     // 已被渲染器完整解析（含顶点/矩阵/纹理引用）
    OS_EX_DISPLAYLIST_EVENT_COMPLETED   // 已在 RAM 中出图（不含高分辨率输出）
} DisplaylistEventType;
void osExQueueDisplaylistEvent(PTR(OSMesgQueue) mq, OSMesg mesg, PTR(void) displaylist, u32 event_type);
```
  注释直言这是"不需要给游戏 scheduler 打补丁就能观测 dlist 事件"的通道（`extensions.h:33-36`），C++ 侧对应 `ultramodern::extensions::on_displaylist_{submitted,parsed,completed}`（`:41-45`）。
* **节流/倍速**：`ultramodern.hpp:104 uint32_t get_speed_multiplier()`、`:115 uint32_t get_display_refresh_rate()` —— 与 SoH 的"逻辑帧率不变、渲染帧率提高"同构，但这里的实现位于 VI 计时层（`librecomp/src/vi.cpp`）。
* **堆**：`librecomp/src/heap.cpp` + `thirdparty/o1heap`（N64Recomp org 自己 fork 的确定性分配器），`thirdparty/` 另有 `concurrentqueue json miniz o1heap sse2neon xxHash`。

### 5.4 rt64：带 TMEM 模型的 HLE 渲染器

`README.md:2` "RT64 is an N64 graphics renderer for playing games with enhancements in emulators and native ports."；`README.md:5-11` 声明 **本仓库尚不提供模拟器插件与光追（RT）**，公开目的是"给 native port 提供可用实现"；`:16-27` 特性清单（D3D12/Vulkan/Metal、ubershader 防卡顿、"High level of accuracy and no tolerance for game-specific workarounds"、输入延迟削减、高分辨率渲染后降采样、任意宽屏、插值到 60 FPS 以上、extended command set、DDS 纹理包且兼容 Rice 文件名）。

* **目录**：`src/{hle,render,rhi,gbi,shaders,common,preset,gui,imgui,apple,shared,tools,contrib}`；公开头文件只有 `include/rt64_extended_gbi.h`。
* **RDRAM 里的 TMEM 是被建模的**：`src/hle/rt64_rdp.h:21` `#define RDP_TMEM_WORDS 512`，`TMEM[RDP_TMEM_WORDS]`（`:72`）、`LoadOperation lastLoadOpByTMEM[RDP_TMEM_WORDS]`（`:105`）；`RGBA32 ? RDP_TMEM_WORDS>>1 : RDP_TMEM_WORDS`（`rt64_state.cpp:351`）与 palette 上半区偏移 `rt64_rdp_tmem.cpp:156`。这是它与 LUS Fast3D 最本质的差别：**LUS 把 TMEM 抹平成 tile 描述符，rt64 保留 512×8B 的存储语义并据此推导 tile/CI4 调色板/填充顺序**。
* **CPU 侧解释器 + GPU 侧 RSP**：`src/hle/rt64_interpreter.h:26-29`（`loadUCodeGBI/processRDPLists/processDisplayLists`）、`rt64_rsp.cpp`；计算着色器 `src/shaders/RSPProcessCS.hlsl`、`RSPModifyCS.hlsl`、`RSPWorldCS.hlsl`、`RSPSmoothNormalCS.hlsl` 统一 `#define GROUP_SIZE 64`（各文件第 5/7/12 行附近），`TextureDecodeCS.hlsl:7` 为 8。RSP 阶段与 `prevFrameWeight/curFrameWeight` 绑定：

```hlsl
/* src/shaders/RSPProcessCS.hlsl:17-18, 66, 92 */
float prevFrameWeight; float curFrameWeight;
float4 tfPos = mul(mul(viewProjMat, worldMat), float4(pos - vel * (1.0f - gConstants.curFrameWeight), 1.0f));
tc -= tcVel * (1.0f - gConstants.curFrameWeight);
```
  同样的 `curFrameWeight` 出现在 `rsp/vertex/tile/transform/projection/look_at` 六个 processor 头里（如 `src/render/rt64_transform_processor.h:22`、`rt64_look_at_processor.h:22`）——**插值是以"每类矩阵/UV 各自带权重"的粒度做的，而不是整帧一刀切**。
* **跨帧调用身份映射（插值的前提）**：`src/hle/rt64_game_frame.h:45-88` 的 `GameFrameMap` 用 `WorkloadMap{viewProjections, transforms, tiles, lookAt}` + `prevTransformsMapped/prevTilesMapped/prevLookAtMapped` 记录上一帧的对应关系，`ModifiedBuffers{positionVelocity, texcoordVelocity}`（`:103-106`）标记需要写速度的缓冲；`GameIndices::FramebufferPair/Projection/Call`（`:17-36`）是"第几个 call 的第几个 workload"的定位三元组。
* **ubershader + SPIR-V 常量特化 + 8 管线**：`src/render/rt64_raster_shader.h:81` `std::unique_ptr<RenderPipeline> pipelines[8];`，`:99 getPipeline(bool zCmp, bool zUpd, bool cvgAdd)`（3 个 bool → 8 组合）；另有 3 条 dither-noise 管线 `postBlendDitherNoise{Add,Sub,SubNegative}Pipeline`（`:82-84`）。真正的"组合器特化"发生在管线创建时把 5 个 spec constant 折进 SPIR-V：

```cpp
/* src/render/rt64_raster_shader.cpp:115-134（节选） */
thread_local std::vector<respv::SpecConstant> specConstants;
...  specConstants.push_back(respv::SpecConstant(i, { 0 }));   // 循环建 5 个
specConstants[0].values[0] = desc.otherMode.L;
specConstants[1].values[0] = desc.otherMode.H;
specConstants[2].values[0] = desc.colorCombiner.L;
specConstants[3].values[0] = desc.colorCombiner.H;
specConstants[4].values[0] = desc.flags.value;
bool vsRun = respv::Optimizer::run(*VS, specConstants.data(), ..., optimizedVS);
```
  `respv` = submodule `src/contrib/re-spirv`（**未初始化，源码本次不可读**）。与 LUS 的对照：LUS 用 prism + 模板着色器按 combiner key 生成变体、运行期靠 spec constant 只做轻量特化；rt64 则是"单一 ubershader + 离线把 otherMode/CC 常量折叠成专用变体"，`wait/signal` 式管线创建线程（`:96 threadCreatePipelines(uint32_t threadIndex)`、`:85-86 firstPipelineMutex/Condition`）负责避免首次卡顿 —— 与 README "Uses ubershaders to guarantee no stutters due to pipeline compilation" 一致。
* **framebuffer 相关计算着色器族**（`src/shaders/`，54 个文件）：`FbWriteColorCS / FbWriteDepthCS / FbReadAnyChangesCS / FbReadAnyFullCS / FbReinterpretCS / FbChangesClearCS / FbChangesDrawColorPS / FbChangesDrawDepthPS`（工作组统一 `FB_COMMON_WORKGROUP_SIZE`，`FbCommon.hlsli`），配合 `rt64_framebuffer_{manager,changes,pair,storage}.cpp`、`rt64_framebuffer_renderer.cpp`、`rt64_vi_renderer.cpp`、`rt64_upscaler.cpp` + `BicubicScalingCS.hlsl` + `BoxFilterCS.hlsl`。**"framebuffer 再解释（reinterpret）+ 变化区回写"是它能同时保住原分辨率 effects 与高分辨率输出的机制**（`FbReinterpretCS.hlsl:75`）。
* **扩展 GBI**：`include/rt64_extended_gbi.h:24-26` `#define RT64_EXTENDED_OPCODE 0x64`（与 Zelda64Recomp 的 `patches/patches.h:34` `#include "rt64_extended_gbi.h"` 呼应，即 **patches 在 MIPS 侧直接发扩展命令**）；统计：**88 个 `G_EX_*` 命令 id**、**61 个 `gEX*` 宏**。节选与其用途：

| 宏（行号） | 作用 |
|---|---|
| `gEXGetVersion:205` / `gEXEnable:210` / `gEXDisable:215` / `gEXNoOp:230` / `gEXPrint:235` | 握手与调试 |
| `gEXBranchList:220` / `gEXDisplayList:225` / `gEXForceBranch:365` | 调用图/身份 |
| `gEXTextureRectangle:240`、`gEXSetRectAlign:270`、`gEXSetViewportAlign:279`、`gEXSetScissorAlign:288`、`gEXViewport:252`、`gEXSetScissor:261` | 2D 对齐修正（宽屏必需的 origin 概念） |
| `gEXSetRefreshRate:300` | ROM 侧声明刷新率，供插值权重使用 |
| `gEXMatrixGroup{,Simple,Decomposed,NoInterpolate,…SkipRot,…SkipPosRot}:318-346`、`gEXPopMatrixGroup{,N}:335-346`、`gEXEditGroupByAddress:377` | 刚体/变换分组（pos/rot/scale/skew/persp/vert/tile/order 逐分量 `G_EX_COMPONENT_*`） |
| `gEXVertexZTest:306` / `gEXEndVertexZTest:312`、`gEXVertex:385`、`gEXSetProjMatrixFloat:393` | 顶点级测试与矩阵 |
| `gEXForceUpscale2D:347` / `gEXForceTrueBilerp:353` / `gEXForceScaleLOD:359`、`gEXSetRenderToRAM:371` | 逐 dlist 画质开关 |
* **用户配置枚举**（`src/common/rt64_user_configuration.h:12-140`，全部 `nlohmann` 序列化）：`GraphicsAPI{D3D12,Vulkan,Metal,Automatic}`、`Resolution{Original,WindowIntegerScale,Manual}`、`DisplayBuffering{Double,Triple}`、`Antialiasing{None,MSAA2X,MSAA4X,MSAA8X}`、`Filtering{Nearest,Linear,AntiAliasedPixelScaling}`、`AspectRatio{Original,Expand,Manual}`（+`extAspectRatio`）、`Upscale2D{Original,ScaledOnly,All}`、`RefreshRate{Original,Display,Manual}`、`InternalColorFormat{Standard,High,Automatic}`、`HardwareResolve{Disabled,Enabled,Automatic}`、`PresentWait{Disabled,Enabled,Automatic}`、`threePointFiltering`、`downsampleMultiplier`、`idleWorkActive`、`developerMode`。
* **增强配置**（`src/common/rt64_enhancement_configuration.h:10-50`）：`Framebuffer::reinterpretFixULS`、`Presentation::Mode{Console,SkipBuffering,PresentEarly}` + `removeBlackBorders`、`Rect::fixRectLR`、`F3DEX::forceBranch`、`S2DEX::{fixBilerpMismatch, framebufferFastPath}`、`TextureLOD::scale`。**注意网传的 `targetRate/frameLimit` 字段名在 HEAD 里并不存在**（零命中），语义由 `RefreshRate/refreshRateTarget` + `PresentWait` + `Presentation::Mode` 承担。
* **线程模型**：`src/hle/rt64_present_queue.h:20-43`（`PresentQueue` 持 `std::thread* presentThread` + `cursorCondition`/`presentIdCondition`）与 `rt64_workload_queue.h/cpp`、`rt64_buffer_uploader.cpp`，即 **workload（RSP/RDP 计算）与 present 分离**；`src/rhi/rt64_render_hooks.*` 是宿主注入 GPU 对象的口子。后端实现在缺失的 Plume submodule 中（`CMakeLists.txt:461` 引用 `src/contrib/plume/contrib/metal-cpp`，`:474` 链接 `D3D12/DXGI`，`:78/:305` `PLUME_SDL_VULKAN_ENABLED`，`:301` `IMGUI_IMPL_VULKAN_NO_PROTOTYPES`）——**这三处是"D3D12/Vulkan/Metal 三后端"结论在本快照里能拿到的最强证据**。

### 5.5 Zelda64: Recompiled：重编译路线的功能前端

`Zelda64Recomp/README.md:2`："uses N64: Recompiled to **statically recompile** Majora's Mask (and soon Ocarina of Time) into a native port with many new features, enhancements, and extensive mod support… uses RT64 as the rendering engine"。**当前快照仅 MM**：`src/main/main.cpp:351-362` 的 `supported_games` 只有一项 `{rom_hash=0xEF18B4A9E2386169, internal_name="ZELDA MAJORA'S MASK", game_id=u8"mm.n64.us.1.0", mod_game_id="mm", save_type=SaveType::Flashram, is_enabled=false, decompression_routine=zelda64::decompress_mm, has_compressed_code=true, ...}`，UI 全部按下标 `supported_games[0]` 使用（`src/ui/ui_launcher.cpp:21,60,82`）；`us.rev1.toml:1` 标题即 "Majora's Mask NTSC 1.0 Recompilation"。**OoT 在本仓库尚无可重编译的符号/配置**（`Zelda64RecompSyms` 缺失，无法核对未来 oot 条目）。

**（1）patches：用 clang 交叉编译 MIPS，再重编译回 C++。** `patches/Makefile:5-16`：

```make
CFLAGS   := -target mips -mips2 -mabi=32 -O2 -G0 -mno-abicalls -mno-odd-spreg -mno-check-zero-division \
            -fomit-frame-pointer -ffast-math ...
CPPFLAGS := -nostdinc -D_LANGUAGE_C -DMIPS -I dummy_headers -I ../lib/mm-decomp/include ... -I../lib/rt64/include
LDFLAGS  := -nostdlib -T patches.ld -T syms.ld -Map patches.map --unresolved-symbols=ignore-all --emit-relocs
```
 `patches/patches.ld:1-20` 定义 `RAMBASE = 0x80801000`、`PATCH_RAM_END = 0x81000000`、MEMORY `extram/rom` 各 64M，并把段顺序排成 `.ctors .dtors .text .recomp_patch .recomp_export .recomp_event .rodata .data .bss` + `ASSERT(. <= PATCH_RAM_END, "Maxed out recomp extra ram")`。段属性宏在 `patches/patches.h:4-13`：

```c
#define RECOMP_EXPORT __attribute__((section(".recomp_export")))
#define RECOMP_PATCH  __attribute__((section(".recomp_patch")))
#define RECOMP_FORCE_PATCH __attribute__((section(".recomp_force_patch")))
#define RECOMP_DECLARE_EVENT(func) \
    __attribute__((noinline, weak, used, section(".recomp_event"))) void func {}
```
 同文件 `:16-32` 是"改名避让"表：`osCreateMesgQueue → osCreateMesgQueue_recomp`、`osGetTime → osGetTime_recomp`、`sinf → __sinf_recomp`、`bzero → bzero_recomp`、`gRandFloat → sRandFloat` 等——补丁里调用的运行时实现与被重编译的原函数用后缀区分。
 **`patches/` 共 61 个文件、20,524 行**，`RECOMP_PATCH` 出现 **189** 次。按行数最大的文件：`camera_patches.c` 2,173 行、`input.c` 1,553 行、`ocarina_effect_patches.c` 1,487 行、`specific_actor_transform_tagging.c` 1,486 行、`actor_transform_tagging.c` 1,445 行、`ui_patches.c` 1,231 行、`options.c` 1,103 行（`.c` 文件合计 19,609 行）。按主题可分四类：① transform tagging（`{actor,billboard,camera,effect,item,particle,sky,song,sword_trail,terrain,ui,specific_actor}_transform_tagging.c` + `transform_ids.h`/`extended_actors.h`，为 rt64 的矩阵分组分配 id）；② 玩法补丁（`autosaving.c`、`quicksave_patches.c`、`save_patches.c`、`room_patches.c`、`gamestate_patches.c`、`skip_sos.c`、`memory_patches.c`、`misc/required/fixes.c`）；③ 输入（`input.c`、`input_latency.c`、`cheats.c`、`options.c`、`print.c`）；④ 资源替换（`*.rgba32.bin/png`、`rewind.ia16.*`、`autosave.*` 图标）。
 **事件（mods 可挂钩的导出点）共 24 个**，全部用 `RECOMP_DECLARE_EVENT` 声明：`actor_transform_tagging.c:47-50` = `recomp_should_actor_init / after_actor_init / should_actor_update / after_actor_update`；`autosaving.c:73-74` = `recomp_on_autosave / after_autosave`，`:551-552` = `recomp_on_load_save / after_load_save`，`:706-707` = `recomp_on_moon_crash / after_moon_crash`，`:778-780` = `recomp_on_owl_update / on_owl_save / after_owl_save`；`input.c:9-11` = `recomp_before_first_person_aiming_update_event / after_first_person_aiming_update_event / recomp_set_extra_item_slot_statuses`；`play_patches.c:31-33` = `recomp_on_play_main / on_play_update / after_play_update`，`:153-154` = `recomp_on_play_init / after_play_init`；`required_patches.c:12` = `recomp_on_init`；`save_patches.c:64` = `recomp_after_init_save`。`should_*` 形式带 `bool* should` 出参，是"可否决原行为"的钩子。
 指令级补丁在 `us.rev1.toml`：`[patches] stubs=[...]`（`:14-19`，把直接摸 RCP 寄存器的 `RcpUtils_PrintRegisterStatus/Reset` 与访问 kseg1 的 `CIC6105_Init` 桩掉）、`ignored=["D_80186028"]`（`:22-24`），`[[patches.instruction]]` 共 6 处，其中 **4 处生效、2 处被注释**（`:30-64`）：`AudioHeap_WritebackDCache@0x8018B510` `lui $at,0x2000 → lui $at,0x0000`；`AudioMgr_HandleRetrace@0x80172DBC` 与 `Graph_TaskSet00@0x80174218` 各把 `jal osSetTimer → nop`（关 RCP 超时检测）；`func_80106644@0x80106684` `slti $at,$t7,0x4 → 0x3`（防暂停时小地图崩溃）。被注释的两条是 `Sched_HandleAudioCancel/GfxCancel` 的 `li $t7,1`（"假装 RSP 已停"），注释说明"关掉 RCP timeout 后可能不再需要"。
 `patches.toml`（重编译补丁的配置）关键项：`single_file_output = true`、`use_absolute_symbols = true`、`output_binary_path = "patches/patches.bin"`（注释说明"由重编译器写 binary 而非 objcopy，才能修补 MIPS32 reloc"）、`allow_exports = true`（"让 mod 能使用 patch 导出的函数与事件"）、`strict_patch_mode = true`（校验被 patch 的符号存在，且非 patch 函数不得是符号）、`unpaired_lo16_warnings = false`（clang 会产生大量未配对 LO16）。

**（2）mod 体系：`.nrm` = ZIP + manifest + 代码补丁。** `N64ModernRuntime/librecomp/src/mods.cpp:253` `constexpr std::string_view default_mod_extension = "nrm"`，`:919` 注释"注册默认 mod 容器类型（.nrm），传空 vector 表示允许任意 content type"；内容类型是**可扩展注册**而非固定枚举（`include/librecomp/mods.hpp:303 struct ModContentTypeId`、`:377 register_content_type`、`:378 register_container_type(extension, content_types, requires_manifest)`），运行时默认注册 `code_content_type`（`mods.cpp:900`）与 `rom_patch_content_type`（`:910`）；**游戏侧扩展示例**：`Zelda64Recomp/src/main/main.cpp:722-732` 注册 texture pack 内容类型并把扩展名 **`rtz`** 绑成容器（`register_mod_container_type("rtz", {texture_pack_content_type_id}, false)` —— 第三参 false 即"不要求 manifest"）。
 实际样本（`Zelda64Recomp/mods/mm_recomp_dpad_builtin.nrm`，136,857 B，本次直接解压核对）条目为 `mod.json`、`mod_syms.bin`、`mod_binary.bin`、`thumb.dds`；`mod.json` 内含 `authors/description/config_schema.options[{id:"draw_dpad", type:"Enum", options:["Off","On"], default:"On"}]` 等，证明"mod 自带配置面板 schema"（对应 `mod_config_api.cpp`）。`mods/BUILTIN_MODS.md:3-7` 说明内置 mod 可被 appdata `mods/` 目录里同名 `.nrm` 覆盖。
 **代码 mod 的执行路径**：`mods.cpp:2378-2379` —— 文件名以 `.offline.nrm` 结尾则走离线重编译（对应 `N64Recomp/OfflineModRecomp/`），否则用 sljit 现场重编译（`LiveGenerator`）。函数替换的实现是**打指令 detour**：

```cpp
/* librecomp/src/mods.cpp:558-590（节选） */
void patch_func(recomp_func_t* target_func, recomp::mods::GenericFunction replacement_func) {
    uint64_t old_flags; unprotect(target_func_u8, &old_flags);
#if defined(IS_X86_64)
    static const uint8_t movabs_rax[] = {0x48, 0xB8};
    static const uint8_t jmp_rax[]     = {0xFF, 0xE0};
    ... write_bytes(movabs_rax); write_bytes(&native_func); write_bytes(jmp_rax);
#elif defined(IS_ARM64)
    static const uint8_t ldr_x2_8__br_x2[] = {0x42,0x00,0x00,0x58, 0x40,0x00,0x1F,0xD6};
#endif
    protect(target_func_u8, old_flags);
}
```
 `unpatch_func` 用保存的 `PatchData::replaced_bytes` 还原（`:593-598`）；`unprotect/protect` 说明宿主做了页可写性切换（W^X 友好）。
* **UI 是 RmlUi（不是 ImGui）**：`src/ui/` 40 个文件 + `src/ui/elements`、`src/ui/core`、`src/ui/util`，含 `ui_rml_hacks.cpp/hpp`（作者自述的 hack 位）、`ui_launcher.cpp`（ROM 选择/校验界面）、`ui_mod_installer.cpp`、`ui_mod_details_panel.cpp`、`ui_config.cpp`/`ui_config_sub_menu.cpp`、`ui_color_hack.cpp`、`ui_api{,_events,_images}.cpp`；submodule `lib/RmlUi@7a06f27` 未初始化，故样式文件（SCSS/RCSS）与布局本次未逐一核对——**"RmlUi + 外部样式"这一说法的硬证据是 `lib/RmlUi` submodule 声明与 `ui_rml_hacks.*` 文件名，样式内容未找到证据**。
* **存档/即时存档/回放**：`patches/quicksave_patches.c`、`patches/save_patches.c`、`patches/autosaving.c`、`src/game/quicksaving.cpp`、`src/game/config.cpp`；保存介质在 `librecomp/src/{eep,flash,dp?}.cpp` 三件（EEPROM / Flashram / SRAM），`GameEntry::save_type` 为 MM 选 `Flashram`；`patches/rewind.ia16.bin/png` 是回滚功能的图标资源。
* **配置以 JSON 呈现给玩家**：根目录 `config_example.cheats.json`、`config_example.cheats.en_us.json`（cheat 配置样例，与 `patches/cheats.c` 配对），运行时配置读写在 `librecomp/src/config.cpp`/`config_option.cpp`；控制器映射表 `recompcontrollerdb.txt`（SDL gamepad 数据库风格）。
* **其他**：`flatpak/`（Linux 打包）、`docs/deck_gyro_1.jpg`（Steam Deck 陀螺仪说明配图，对应 `lib/GamepadMotionHelpers`）、`shadercache/`+`shaders/`（rt64 着色器缓存与注入）、`overlays.us.rev1.txt`/`rev1_overlays.txt`（可重定位 overlay 表，喂给 `recomp::overlays::register_overlays`，见 `src/main/register_overlays.cpp:7-19`，其数据来自 `RecompiledFuncs/recomp_overlays.inl`）。
* **No-AI 立场**（与 §6 的 TriAevum 形成鲜明对照）：`README.md:11-12` "**The Zelda 64: Recompiled team has not, does not, and will not use generative AI tools in the development of this project for any purpose.** As per the CONTRIBUTING.md file, contributions that use any GenAI tooling in any capacity are blanket banned."（`README.md:11-12`）

---

## 6. 掌机与二进制 AOT 衍生路线

> 取证对象：`$P/TriAevum`（HEAD `672d13f`）、`$P/Ocarina-ReTuned`（`fecb9f9`，2026-11-13）、`$P/oot3d_practice_menu`（`e8bdabe`）。
> 本节专门纠正一个流传很广的误解：**这三个仓库都不是"在 3DS 上跑的 PC 移植"，也不是"音乐重制"**。TriAevum/Ocarina-ReTuned 是**把 3DS 二进制搬到 PC**；`oot3d_practice_menu` 是**留在 3DS 实体机/Luma 环境的 IPS mod**。

### 6.1 TriAevum：3DS 标题的 whole-AOT + 原生 PICA→Vulkan 渲染器

**定位**（`TriAevum/README.md:7-12, 86-90`）：一个引擎承载两个作品 —— **Echoes of Time**（*Ocarina of Time 3D* → PC，v0.8.0-beta.2）与 **Moonfall Curse**（*Majora's Mask 3D* → PC，v0.5.0-alpha.1，仅接受 EUR `CTR-P-AJRP`），"They share a modern **NRI/Vulkan** renderer, with lighting, shadows and customizable graphics"；作者自述 "Echoes of Time is an **AOT recomp in the same general spirit as the Xbox 360 recomp projects**"。输入是**玩家自己的解密 dump**（EUR/USA，原版或 Rev 1；日/韩/台版本尚不支持），仓库不含任何游戏资产。

**AOT 形态**（`docs/OOT3D_LINUX_WHOLE_AOT_PORT.md:5-9`）：

> "The Linux product compiles the same **12,419 generated whole-AOT functions** as the Windows product into native x86-64 ELF objects. The product audit permits **three explicit host boundaries and zero residual A32 entries**. The decoded A32 interpreter and the source-native reconstruction are not linked into the gameplay closure; source-native remains documentation only."

即：**产物是生成的 C++ 函数集合**，ARM32(A32) 解释器与"源码级重建"两条备选路径**不参与发布构建**（只作对照文档）。翻译流水线可见证据：`tools/ctr_title/{ghidra_inventory.py, ghidra_scripts/, match_functions.py, normalize_inventory.py, generate_whole_aot_cpp.py, data_pointer_entries.py, init_array_entries.py}` —— **Ghidra 出库存 → 归一/配对 → 生成 whole-AOT C++**；另一份历史包袱在 `tools/oot3d/native_a32_runtime/upstream/`（`src/oot3d_pack/arm_lift.py`、`arm_ir.py` 用 Capstone 把 ARM 提升为 IR；`recomp/a32_core_alu.cpp`、`a32_core_memory.cpp` 是被保留的 A32 核），以及 `tools/oot3d/third_party/azahar_pica/video_core/pica`（**Azahar 3DS 模拟器的 PICA 代码作为参考语料**）。

**引擎与 vendoring 形态**：`runtime/three_ds_recomp/README.md:1-16` 自称 "Reusable platform and rendering runtime for native Nintendo 3DS recompilation projects…native PICA submission and texture decoding…OpenGL and NRI/Vulkan rendering…renderer extension contracts for scene-aware advanced effects"，主目标 `three_ds_recomp_runtime` / 别名 `ThreeDsRecomp::Runtime`；`:13-16` 明确 "**The `Ship::` and Fast3D surfaces are retained compatibility adapters.** New code must depend on the neutral renderer and host-service contracts"；`:21-24` "**This runtime is derived from libultraship.** Its history and MIT copyright are preserved"。`src/` 因此同时有 `fast/`（LUS epoch-2 命名的遗产）、`ship/`（`Context.cpp`、`audio/controller/config/events/resource`）、`legacy_compat/`。

**渲染器**（这是本节的技术重心）：`runtime/three_ds_recomp/src/fast/renderer3ds/` **35 个文件 / 9,182 行**，命名直接把 PICA 概念搬进 Vulkan：

| 文件 | 含义 |
|---|---|
| `pica_tev_program.cpp`、`pica_native_fragment.spvpack`、`pica_native_fragment_binaries.cpp`、`pica_gl_shader_dialect.cpp` | TEV（PICA 的组合器流水线）→ 着色器程序；上游带**预编译 SPIR-V pack**（Ocarina-ReTuned 把它删掉改成本地反编译，见 §6.2） |
| `nri_pica_pipeline_bridge.cpp`、`nri_pica_upload_arena.cpp`、`pica_nri_shader_contract.cpp`、`pica_nri_upload.cpp` | 与 **NRI**（NVIDIA 的 GPU 抽象层）之间的管线/上传桥 |
| `pica_surface_lighting{,_pass}.cpp`、`pica_surface_emission.cpp`、`pica_pbr_shading.cpp`、`pica_pbr_seams.cpp`、`pica_pbr_seam_shader.cpp`、`pica_gi_geometry.cpp` | 在 PICA 状态之上叠加 **PBR/GI/自发光** 增强层 |
| `virtual_shadow_map.cpp`、`point_shadow_map.cpp`、`room_shadow_shell.cpp`、`pica_shadow_geometry.cpp` | 逐房间阴影壳 + 虚拟/点光源阴影图 |
| `pica_semantic_scene_view.cpp`、`pica_scene_capabilities.cpp`、`pica_geometry_provider_authorization.cpp`、`pica_extension_schedule.cpp`、`pica_composition_schedule.cpp`、`pica_preparation_queue.cpp` | 场景语义视图与扩展调度（"scene-aware"契约的实现面） |
| `vulkan_pipeline_cache_store.cpp`、`vulkan_breadcrumbs.cpp`、`vulkan_device_fault.cpp`、`vulkan_memory_budget.cpp`、`pipeline_cache_flusher.cpp` | 管线缓存/崩溃面包屑/显存预算（README 提到草地块"within a VRAM budget"） |

  后端层 `src/fast/backends/` 共 32 个文件，覆盖 **D3D11(`gfx_direct3d11.cpp`)、Metal(`gfx_metal*.cpp`)、OpenGL(`gfx_opengl.cpp` + `gfx_opengl_pica.cpp`)、Vulkan(`gfx_vulkan*.cpp`，含 `gfx_vulkan_pica_{draw,pipelines,state,textures,texture_copy,display,attachments}.cpp`)** —— 即**四套 API**，比 LUS 的三套多一个 Vulkan（含 Android surface、scanout probe、validation、GPU profiler、diagnostics 等独立文件）。另有 `src/fast/dlss/dlss_module.cpp`（DLSS 接入，`dlss.cmake`）与 `src/fast/immerse/`（HaptX 类触觉）。纹理侧 ETC1 解码在 `src/fast/extension/pica_texture_decode.cpp`（`include/fast/renderer3ds/pica_native_state.h` 同域）。
  语料→管线的一致性方法见 `docs/OOT3D_NRI_PICA_AOT_RENDERER_PARITY.md:19-33`：`oot3d_native_pica_trace_inventory` 把 trace 收敛成清单 → `oot3d_native_pica_aot_compiler` 合并清单并校验 → `oot3d_native_pica_pipeline_manifest` **离线**合并清单；配套 `tools/oot3d/native_pica_frontend/profiles/*.json`（如 `oot3d_pica_boot_title_kokiri_effective.json`）与默认着色器包 `oot3d_pica_default.o3ps`。文档通篇以 `pica-*` 命名的 `visual-parity` 实验目录（`:102,112,122,132`）说明其验证方式是**逐特性做像素/结构对齐**，而不是"看起来差不多"。
  文档规模本身是特征：`TriAevum/docs/` **239 个 md**，包括 `BAKED_SURFACE_LIGHTING.md`、`AZAHAR_AUDIO_PROVENANCE.md`（音频移植来源声明）、`NATIVE_FRAGMENT_ENVIRONMENT_PROFILES.md`、`NATIVE_TEMPORAL_HISTORY.md`、`OOT3D_CHARACTER_FORMAT_INTERPRETATION.md`、`TRIAEVUM_TITLE_PGO.md`（PGO）、`TRIAEVUM_UPDATE_SIGNING.md`、`TRIAEVUM_WINDOWS_SYSROOT.md`、`TRIAEVUM_WINDROID_FORK_EVALUATION.md`、`ZELDA3DRECOMP_READONLY_AUDIT.md`、`OOT3D_CONSUMER_TO_DECOMP_HANDOFF.md` 等；`ports/{android,macos,switch_gl_probe,switch_vulkan_probe}` 说明在评估 Switch 侧探针。

**增强特性（README 原文要点，`:216-236`）**：卡通着色与描边（可关）；AO/阴影/水面反射/AA，"the expensive ones at half resolution so 4K no longer requires an upscaler"；各向异性过滤（Enhanced 预设 4x）；**真宽屏 16:9 / 21:9 / 原始 5:3 + 可调 FOV**；**"60/90 FPS visual interpolation of the original 30 Hz game state. Game logic and gameplay speed remain unchanged; this is not 60/90 Hz physics. Input is sampled late, so Link jumps when you press the button"**（`:223-226`）；Azahar 兼容自定义纹理与纹理导出；单屏重制的 HUD/暂停菜单/笛子/希卡石视域；自由相机与拍照模式；命名配置档、音频与即时存档；体感震动与跟随手柄的按键字形；程序化草地。已知未修问题在 `:380-394`（issue #105、#96 "objects can flicker at interpolated 60 FPS with native lighting"）。

**合规与 AI 立场**（§7/§8 会拿这三份文件与 Zelda64Recomp 对照）：`LICENSE_SCOPE.md:1-22` 承认"mixed-provenance work"——原创贡献 `GPL-3.0-or-later`，与 GPL-2.0-or-later 的 Azahar 派生代码组合后按 "or later" 以 GPL-3.0-or-later  convey，MIT/BSD 部件保留原许可，`LICENSES/` 逐部件列文本，`THIRD_PARTY_NOTICES.md` 汇总；对第三方 mod 明确"仅在你开启时从 GameBanana 自行下载，TriAevum 不再分发"（`README.md:80-83,127`）。`SOURCE_OFFER.md:1-18` 是一份书面**对应源码要约**：每个二进制发布必须带 `source/` 与 `release-manifest.json`（含 commit 与依赖身份），"Under the precompiled release model, `source/titles/` also contains the exact generated C++"，且"ROM inputs, extracted assets and local user data remain excluded"、"translated source 的收录不构成对原作权利的再许可"。AI 立场在 `README.md:93-97`：**"Development of this project is entirely AI-assisted, under human direction."** 并补一句"That describes the TriAevum-specific work, not the mature donor projects and libraries it builds on: Azahar, Ship of Harkinian, NRI and others. Aside from those foundations, you could fairly call this concentrated AI slop."

### 6.2 Ocarina-ReTuned：以 TriAevum 为基线、把准备工作搬到用户机器上

`Ocarina-ReTuned/README.md:3`："A native, single-screen Recomp of OOT3D for **Windows, Mac, Linux, Android, iOS, Apple TV, and experimental Nintendo Switch**. Ocarina ReTuned is a substantially modified fork of TriAevum, integrating **Ocarina ReFramed** gameplay with its own interface, **ROM-preparation flow**, and platform support."（Preview 0.6.5）

**它不是把 TriAevum 源码 fork 进仓库**：仓库本体只有 `src/retuned`、`tools/{frontend,native,reframed}`、`resources/`、`documentation/`、`tests/`（29,651 行 / 389 文件），TriAevum 以**外部引擎目录**形式被消费——`tools/native/rom_setup.py:449,649` 直接操作 `engine / 'tools/triaevum_release'` 与其 `cmake/*.cmake`，产物名 `triaevum_title_aot`（`:664,678`）。版本锚点写在 `resources/source-pins.json`：

```json
"application_base":   { "repository": "https://github.com/OTPR26/DekUI-Fork",     "revision": "1f1e0e2…", "release": "v0.4.2" },
"gameplay_mod":       { "repository": "https://github.com/OTPR26/OOT3DHud",       "revision": "cca2979…", "release": "v2.0.2", "source_directory": "release-source/2.0.2/single" },
"renderer_reference": { "repository": "https://github.com/coccofresco/TriAevum",  "revision": "17218b0…", "status": "native runtime baseline; macOS and Android bring-up" }
```
 也就是说：**UI 基座是 DekUI 的 fork**（3DS 上的 UI 库），**玩法叠加层是 OOT3DHud（即 "Ocarina ReFramed"）**，**引擎是 TriAevum 的某个固定 revision**（与本目录 HEAD `672d13f` 不同，需注意版本差异）。ROM 白名单同样外置：`resources/supported-roms.json` 给出 USA `0004000000033500` / EUR `0004000000033600` / JP `0004000000033400` 及各自 SHA-256；README `:41-45` 要求"decrypted USA or European ROM in `.3ds` or `.cci`"，日/韩/台"planned"。

**三条与上游不同的技术决定**（README `:25-38`）：
1. **本地着色器生成**（对 §6.1 的 `pica_native_fragment.spvpack` 的反向操作）："the upstream embedded title-specific GLSL tables and precompiled SPIR-V pack **were removed**. The **generic shader decompiler** instead processes data from the user's game locally."（对应脚本 `tools/native/apply_neutral_shaders.py`、`apply_spirv_embedding.py`、审计 `audit_neutral_shaders.py`）
2. **准备期（preparation）在用户机器上完成**："First-run preparation uses your ROM locally and can take a while"（`:44`），"No compiler or SDK is needed"（这是 TriAevum 的说法，见 `TriAevum/README.md:210-211`），而 ReTuned 的下载页明确各平台对编译器的要求：Windows 包**内含 compiler and preparation tools**；macOS 需 "Apple C++ command-line tools"，**CMake/Ninja 已内含**；Linux 需 **glibc 2.38+** 与 "a C++20 compiler with system headers"，CMake/Ninja 内含（`:52-56`）。交叉构建脚本 `tools/native/build_windows_cross.py:2-4` 更直接："Build the neutral Windows host using **the official LLVM-MinGW macOS tools**. Supply **LLVM-MinGW 20261006** and **vcpkg ef7dbf94b9198bc58f45951adcf1f041fcbc5ea0**" —— 即从 macOS 交叉产出 Windows 二进制；Apple 平台"需要一台装了 Xcode 的 Mac，Windows/Linux 目前无法准备 Apple 应用"（`:50`）。
3. **平台扩张 + 单屏/双屏可切换**：Android 双屏布局（游戏屏留 ReFramed HUD、副屏放回触屏菜单，另可选底部装备面板 + Shard of Agony 指示）、实验性 "iPhone Duo"、Apple TV 遥控器支持、可自定义触屏布局、**PAK Loader 导入自定义成年/幼年模型**（ModLoader64 生态）、替换式对话字体（基于 Liberation Sans，SIL OFL）、按设备内存预算的纹理包、iCloud 存档同步（仅另行配置的 Apple 构建）。
 Capstone 在本仓库的作用是解析 ROM 代码段：`tools/native/build_android_capstone.py`、`tools/native/android/rom_preparation.py`、`tools/native/rom_setup.py`。

一句话概括与 TriAevum 的关系：**TriAevum 是"引擎 + 预生成产物"，ReTuned 是"引擎引用 + 本地生成 + 多平台发行工具链"**，因此后者的代码量小得多但发布工程更重。

### 6.3 oot3d_practice_menu：留在 3DS 上的 IPS 补丁路线

`oot3d_practice_menu/README.md:3-7`：在 OoT3D 里开一个屏上菜单，"current motivation is for **speedrunners and glitch hunters** to add features which allow them to practice and study the game"，并强调"There is nothing special about OoT3D in the methods used here, though the relevant addresses and size are currently **hard-coded**"；要求配合带关键 game patching 修复的 Luma3DS（`:7`，注明已回流上游）。

**核心内存技巧（这是它能"无限扩展"的原因）**，README `:12-17`：
* 旧补丁只能塞进未修改 OoT3D 代码段的**剩余 padding**；现在"the bulk of the code is located on **extra pages appended at the end of the program image**"，自定义 "loader" 放在代码段 padding 里，**由它把新增的数据页重新授权为可执行** —— "custom code and data can have an effectively unlimited size"。
* 代码授权策略致谢 RicBent 的 Magikoopa；并承认"be a noob"式的 libctru/gz/decomp z3D 头文件借用（`:9-17`）。

硬证据：

```c
/* src/loader.c:13,27 */
Result svcControlProcessMemory(Handle process, u32 addr0, u32 addr1, u32 size, u32 type, u32 perm);
res = svcControlProcessMemory(getCurrentProcessHandle(), address, address, neededMemory, 6, 7); // MEM_MAP, RW+X
```
 `src/svc_loader.s:38` 另有 `SVC_BEGIN svcControlProcessMemory`（直接手写 SVC trampoline，绕开 libctru 与游戏自身符号冲突——README `:12` "I can't just use all of libctru, because it conflicts with the base game"）。落点选择在 `patch.py:20`：`if vaddr >= 0x4CA000 and vaddr < 0x5C7000: #this is not good lol just trying to avoid __GNU_EH_FRAME_HDR section` —— 即**在 ELF 程序头的空洞里挑一段**，`0x5C7000` 是上界而非"固定地址"（此处更正常见的转述误差）。

**补丁点用链接脚本硬绑**：`oot.ld`（`OUTPUT_FORMAT("elf32-littlearm")`、`ENTRY(loader_patch)`、`.patch_loader 0x100000`）共 **38 处 `.patch_*` 段**，19 个唯一名，节选：`.patch_InstantTextFirstLine 0x2E049C`、`.patch_InstantTextBoxBreak 0x2E0664`、`.patch_InstantTextRemoveOff 0x2E06C8`、`.patch_Gfx_Update 0x310254`、`.patch_SetBGMEntrance 0x331048`、`.patch_TurboTextSignalNPC 0x3469F4`、`.patch_PlaySound 0x35C528`，另有 `after/before_GlobalContext_Update`、`OverrideSceneSetup`、`ItemUsability(_Shield)`、`SetBGMDayNight/Event`、`SleepQueryCallback`、`TurboTextAdvance/Close`。区域变体 `oot_e.ld`、`oot_j.ld` 与 IPS 产物 `code.ips/code_e.ips/code_j.ips/code_citra_{us,eur,jp}.ips`（`Makefile:168-171` 生成 `basecode.ips`）成对出现。
 **功能面**：`src/{cheats,commands,entrances,equips,file,inventory,scene,srv,srvpm,gspgpu,menu,menus,draw,input,loader,debug,printf}.c` + `hooks.s/patches.s`，界面头 `include/menus/{cheats,commands,debug,equips,file,inventory,scene,warps,watches}.h`。其中 `watches.h` 是**内存观察器**（`WATCHES_MAX 15`、`WATCHES_MAXNAME 6`，`WatchType{S8,U8,X8,S16,U16,X16,S32,U32,X32,F32}`），`warps.h` 对应入口/传送，其余按名即功能。规模 16,872 行 C/汇编/头。**它给"PC 移植"命题提供的价值是反向的：说明 3DS 版本的运行时同样可被挂钩与替换，这也是 ReTuned/ReFramed 一类 3DS 侧 mod 能存在的底层前提。**

### 6.4 同一命题下的其他项目（本次未克隆，只作定位）

| 项目 | 路线 | 与本报告语料的关系 |
|---|---|---|
| Hookshot（HarbourMasters） | Switch 1/2 版 *OoT (Masterquest)* 的 AOT/重编译前端 + 运行时 | 与 TriAevum 同属"二进制 AOT"家族，但目标是 Switch  remake 而非 3DS；共享 HarbourMasters 生态（与 SoH 同源组织） |
| ProjectReLink | Switch *OoT* 移植路线的社区实现 | 同上；本次未取证 |
| oot-pc-recomp 等零散重编译 | 静态重编译/AOT | 本次未取证 |
> 上表仅给出定位，**未对这三个仓库做任何源码断言**（未克隆，故不引用行号）。若需要该分支的证据，应先补 `git clone --depth 1`。

---

## 7. 衍生生态与增强叠加层

### 7.1 随机化器：从 Python 外挂变成引擎内建模块

| 面向 | 证据 | 结论 |
|---|---|---|
| Python 侧 | `oot-ports-repos/OoT-Randomizer/`：`OoTRandomizer.py`、`Rom.py`、`Patches.py`、`World.py:570-574 load_regions_from_json`、`data/World/*.json`（26 个，含 `… MQ.json`）、`data/Glitched World/`、`ASM/`、`N64Patch.py`、`MBSDIFFPatch.py` | 世界/区域数据是 **JSON**（"YAML 描述世界数据"的说法不成立；YAML 只出现在 Torch 的资产工厂那边）；产物是** patched ROM + `.zpf`**（`.zpf` 是随机化器输出包格式，**不是** SoH 的输入） |
| C++ 侧移植 | `Shipwright/soh/soh/Enhancements/randomizer/`（113 个条目）内含 `3drando/`（`fill.cpp/hpp`、`item_pool.*`、`hints.*`、`playthrough.*`、`spoiler_log.*`、`shops.*`、`starting_inventory.*`、`menu.*`、`pool_functions.hpp`、`LICENSE.md`） | SoH 把随机化器的**算法层整体搬进引擎**（3drando = "third dimension randomizer" 的 C++ 版），运行时直接在游戏里做物品填充逻辑 |
| 契约面 | `SeedContext.cpp:407-417 Context::ParseSpoiler()` 用 `nlohmann::json` 读 **spoiler 文件**；`OTRGlobals.cpp:2362-2380` 导出 `extern "C" Randomizer_ParseSpoiler / SpoilerFileExists / Randomizer_GetSettingValue / Randomizer_GetCheckFromActor / Randomizer_IdentifyShopItem` | **进程内交换靠 spoiler JSON + 一组 `extern "C"` 查询函数**：decomp 侧 C 代码（actor/场景）问 C++ 侧"这个 check 是什么物品"，不需要重新实现逻辑 |
| ROM 侧 | `Shipwright/docs/supportedHashes.json`（按 `name/sha1` 列出 PAL 1.0/1.1/PAL GC/… 全版本 SHA-1） | SoH 以 SHA-1 白名单校验输入 dump，与 §7.4 的 ReTuned SHA-256 白名单是同族做法、不同哈希算法 |

2ship2harkinian 里没有对应的 3drando（其 Enhancements 面以 `Cycle` 系统/`seqplayer` 为主，见 §2.8），因此**随机化器内建是 SoH 独有**。

### 7.2 四种 modding 姿态（同一生态里并存、互不兼容）

| 姿态 | 代表 | 机制证据 | 作者需要什么 |
|---|---|---|---|
| **源码分叉重编译** | SoH | `Shipwright/docs/MODDING.md:1-30` 整篇是 "fork → clone → build → 从 develop 切分支" 的流程 | 会 C++、要装完整工具链、每次改动重新编译整引擎 |
| **资产替换（免代码）** | SoH 自定义音乐、rt64/TriAevum 纹理包 | `docs/CUSTOM_MUSIC.md:3-12`：每个序列需 `<name>.seq`（Seq64，作者署名指向 sauraen/seq64）+ `<name>.meta`（两行：显示名 + **base16** 乐器集号），可递归嵌套目录；`rt64/TEXTURE-PACKS.md:1-14` 定义 texture pack 配置格式与官方 Texture Pack Tools（同 release 页分发） | 只要会导出素材；不需编译器 |
| **运行时脚本/内存补丁** | LUS + SoH TCC mods | §3.6：TCC 内存内脚本 + `SafeLevel` + 6 个 key；CVar 落 JSON（`CVars.` 前缀）；`ShipInit.hpp:11-60` 的初始化注册 | 少量 C++/脚本知识，仍受引擎 API 面限制 |
| **容器化 mod（含原生码）** | Zelda64Recomp / NRM | `.nrm` = ZIP + `mod.json`（含 `config_schema`）+ `mod_syms.bin`/`mod_binary.bin`/`thumb.dds`；内容类型可注册扩展（`mods.hpp:303,377-378`）；`.rtz` 纹理包容器（`Zelda64Recomp/src/main/main.cpp:722-732`）；执行路径二选一：sljit 现场重编译或 `.offline.nrm` 离线编译（`mods.cpp:2378-2379`）；函数替换写 `movabs rax;jmp rax`（`:558-590`） | 可以只给素材，也可以给"MIPS 补丁码"，安装即用 |

TriAevum/Ocarina-ReTuned 走的是**策划式分发**：mod 由 Forge/准备流程叠加（`TriAevum/README.md:59,80-83,127`：Vanilla+ Link  remodel "downloaded from GameBanana only when you turn it on, never redistributed by TriAevum"；`LICENSES/Vanilla-Plus-Link-Remodel.md` 单独立档），ReTuned 额外提供 **PAK Loader** 在应用内导入玩家模型（`README.md:12,34`）。**两者都没有向第三方开放代码 mod API 的证据**（未找到 hook/export 面）。

### 7.3 周边工具的定位（有镜像的给一行证据，没有的明确标注）

| 仓库 | 定位 | 本次状态 |
|---|---|---|
| `fast64`（Blender 插件） | **正向**创作 dlist/actor/collision/skeleton/cutscene，`fast64_internal/z64/exporter/*`；微码含 `F3DZEX2`（README 注释 "used in OoT and MM"） | 镜像 `oot-ports-repos/fast64`；与 ZAPD 反向导出互补（§4.7） |
| `z64rom` | ROM 打包/重组 CLI；`README.md:1` "**This repo is archived.** Newer fork: z64utils/z64rom" | 镜像存在；已被 z64utils 系列取代 |
| `gz`（practice rom） | `README.md:2-4` "a trainer / practice / testing utility…formally known by its codename **gz**" | 镜像存在；与 §6.3 的 practice menu 是同趣味的**实体机**方案，`oot3d_practice_menu/README.md:16` 明确致谢 gz |
| `splat` / `decomp-toolkit` / `decomp-permuter` | 现代 split→重建工具链，逐步替代 `z64rom`/`ido-static-recomp` 时代的 split 流程 | **本次语料内无镜像，未取证**（只作谱系定位） |
| `SharpOcarina` | C# 编辑器套件（模型/时序/音乐） | 镜像 `oot-ports-repos/SharpOcarina` **只含 `MapDev/`+`MapDev.sln`**，README/Languages 缺失 → 不作源码断言 |
| `Ocarina-Atlas`、`HackerOoT`、`oot-gc`、`oot-dc` | 关卡/3D 编辑与 GC 版移植尝试 | **本次语料内不存在，未取证** |
| `OOT3D_Randomizer` | 3DS 版随机化器（Python），是 ReFramed/ReTuned 生态的上游之一 | 仅 `extra-repos/n64-oot/OOT3D_Randomizer` 有副本；§6.2 的 `OOT3DHud`/`DekUI-Fork` 未克隆，未取证 |

### 7.4 法律与合规姿态对照（工程报告的必要项）

| 项目 | 输入要求 | 仓库内含什么 | 许可声明 |
|---|---|---|---|
| Shipwright / 2ship2harkinian | "You must own a copy of the game" 类声明 + SHA-1 白名单（`docs/supportedHashes.json`） | 不含 ROM/资产；`docs/MODDING.md` 教分叉构建 | 顶层无独立 LICENSE（散落于文件头/README）——**许可可发现性弱** |
| libultraship / Torch / ZAPDTR / OTRExporter / rt64 / n64-fast3d-engine | 无游戏内容 | 纯代码 | MIT（ZAPDTR 版权行署 "Zelda Reverse Engineering Team"）/ rt64 `LICENSE` |
| Zelda64Recomp | 需自备 MM US 1.0；`README.md:9` "**This repository and its releases do not contain game assets. The original game is required to build or run this project.**" | 含 `patches/*.c`（引用 decomp 头，需 submodule `lib/mm-decomp`）与 `mods/*.nrm` 内置 mod | `COPYING`；另附 **No-AI 声明**（`:11-12`） |
| TriAevum | "your own decrypted dump"，区域白名单 | 不含 ROM/资产；含生成 C++（按 `SOURCE_OFFER.md` 随发布提供） | **GPL-3.0-or-later + 混合来源说明**（`LICENSE_SCOPE.md`）+ 逐部件 `LICENSES/` + `THIRD_PARTY_NOTICES.md` + 书面源码要约 |
| Ocarina-ReTuned | `.3ds`/`.cci` 解密 ROM，USA/EUR，SHA-256 白名单（`resources/supported-roms.json`） | 只含 tools/src/resources，引擎外部拉取并固定 revision | 继承上游声明："Retained runtime components and the included ReFramed/Project Restoration-derived UI resources keep their original licenses and attribution notices."（`README.md:38`） |
| oot3d_practice_menu | 需要 3DS 实体机 + Luma3DS（`:7`） | 只含 IPS 补丁与 arm 源码，无游戏码 | **顶层无 LICENSE**（同 SoH 的弱可发现性），README 逐条致谢/声明借用（`:9-17`） |

一句话：**同样"不含资产"的底线，五个项目给了五种不同强度的合规文档**；TriAevum 最完整（因为它是 GPL 链条最复杂的那个），SoH/practice menu 最弱。

---

## 8. 横向对比与工程评价

### 8.1 路线矩阵

| 维度 | ① decomp 源码（SoH/2SoH） | ② 静态重编译（Zelda64Recomp） | ③ 二进制 AOT（TriAevum/ReTuned） | ④ 掌机 IPS（practice menu） |
|---|---|---|---|---|
| 输入 | 解密 ROM → Torch/ZAPDTR 抽资产；代码来自 decomp 工程 | 解密 ROM → ELF + syms.toml | 解密 `.3ds/.cci` → Ghidra 库存 → 生成 C++ | 实体机 ROM + IPS |
| 游戏码是否被重写 | **是**（人工/半自动重写为可移植 C） | 否（逐指令机械翻译） | 否（机械翻译 + 少量 host boundary） | 否 |
| 可否改逻辑细节 | 最容易（源码在手，`VB_*` 590+ 处） | 需 MIPS 侧 patch 或 mod 事件 | 需回到 AOT 生成物/`environment` 层 | 只有 hook 点 |
| 渲染 | LUS Fast3D（GBI 直译，无 RSP/RDP 周期模型） | rt64（HLE，含 TMEM/组合器回卷/compute RSP） | 原生 PICA→Vulkan/NRI + PBR/阴影叠加 | 原版 PICA 不变 |
| 帧率增强 | 渲染侧子帧重提交（逻辑仍 20 Hz）+ 逐 bug 补丁 | rt64 帧权重插值 + `gEXSetRefreshRate` | 60/90 FPS 视觉插值（"not 60/90 Hz physics"） | 不适用 |
| 存档 | 引擎自实现（`SaveManager`） | `librecomp` eep/flash/sram + quicksave/rewind | 自建 + save states | 游戏原生 |
| mod 面 | 源码分叉 + 资产包 | `.nrm`（含原生码）/`.rtz` | Forge 叠加（无公开 code API） | IPS/Luma |
| 规模（源码行） | 720,798 + 71,969（基座） | 45,929 + 57,186 + 14,355 + 81,519 | 882,749 | 15,789 |

### 8.2 Fast3D 后端精度对照

| 后端 | 是否建模 TMEM | 组合器求值 | 色彩回卷 | 整数精度来源 | 关键代价 |
|---|---|---|---|---|---|
| LUS `Fast3D`（§3.5） | **否**（`G_RDP*SYNC` stub，`interpreter.cpp:4686-4718`） | CPU 侧按 combiner key 生成 specialize shader（`GenerateCC` + `do_single/multiply/mix`） | 近似（WRAP 三家后端不一致，§3.5 片段） | 靠定点数移植 + `Mtx*→MtxF` | 依赖游戏码"可重写"，无法用于原版二进制 |
| rt64（§5.4） | **是**（`RDP_TMEM_WORDS 512`，`:21`） | GPU 端 ubershader + 5 个 spec constant 折叠（`rt64_raster_shader.cpp:115-134`） | 真 2's-complement（`rt64_color_combiner.h:210,558-567`） | HLE 状态机 + compute RSP | 复杂度/显存；Plume 后端不可读 |
| GLideN64（参考，未取证） | 是 | 特化 shader | 是 | 插件级模拟 | 面向模拟器，需 RSP 插件接口 |
| n64-fast3d-engine（§3.5 祖先） | 否 | 特化 shader 生成 | 近似 | 纯 C 参考实现 | 2021 年 10 月停更，作教材用 |
| TriAevum PICA 路径 | 不适用（3DS 无 TMEM 概念） | TEV→着色器程序（`pica_tev_program.cpp`） | 3DS 语义 | 语料 trace→pipeline manifest | 仅适用于 3DS 标题 |

### 8.3 增强特性可行性边界（选型硬指标）

1. **想让游戏"像原生 PC 游戏"（宽屏/高帧/材质/手柄/存档/UI 缩放）**：①②③ 都能做，但**改动成本随路线递增**。SoH 有现成 CVar 面（`gSettings.InterpolationFPS`、`gAdvancedResolution.*` 等）；Zelda64Recomp 有 rt64 的配置枚举 + 事件系统；TriAevum 直接给了成品预设。
2. **想在不重编引擎的前提下发布 mod**：只有 **② `.nrm`** 和 **④ IPS** 成立；SoH 需要分叉重编（`MODDING.md`），TriAevum 需要走 Forge 流程。
3. **想做逐指令级的行为改写（如帧完美机制）**：① 最容易（C 源码）；② 靠 `[[patches.instruction]]` + `RECOMP_PATCH` + 事件；③ 只能改生成物/环境层，README 也承认存在"gameplay differences and native prerogatives"（`docs/OOT3D_GAMEPLAY_DIFFERENCES_AND_NATIVE_PREROGATIVES.md`）。
4. **想用原版 N64 ROM 而不做反编译**：只有 ②。③ 需要 3DS remake；① 需要 decomp 工程覆盖该版本。
5. **想要 OoT 而非 MM 的 ②**：**目前不可行**——本快照 `supported_games` 只有 MM 一项且 `is_enabled = false`，OoT 的符号/配置尚未落地（§5.5）。
6. **想要 MM 的高帧玩法一致性**：② 有 MM 产品化；① 有 2ship2harkinian（Cycle 系统）；③ 有 Moonfall Curse（首 alpha，仅 EUR dump）。

### 8.4 工程评价（可作为技术选型的直接结论）

1. **Shipwright 的价值不在"移植"而在"可写性"**：72 万行代码里 487 个 `.cpp` + 590 个 `libultraship` include + 1,242 处 `GameInteractor` 调用，把一个反编译工程改造成带 hook 表、CVar、ImGui 菜单与随机化器的现代应用；代价是它**永远依赖 decomp 工程的正确性**（版本/哈希白名单）。
2. **libultraship 是"共享底座"，但底座本身正在换代**：epoch 2（`src/fast/`、`Ship::`）与 `port-maintenance`（SoH 所 pin）并存（§3.7），跨版本迁移文档 `docs/migration/EPOCH-2.md:7-19` 存在，说明**API 稳定性目前不是它的强项**；评估下游项目时要先看它 pin 的是哪条线。
3. **静态重编译路线在"保真"与"可增强"之间取得最好平衡**：指令表 157 项 + 三张符号名单 + `.recomp_*` 段与链接顺序 + sljit 运行时 detour，构成一套自洽且**不依赖反编译**的完整工程；短板是 FPU/协处理器面（无 `bc0`/`fcr31`）与产品端目前只 MM。
4. **rt64 是本报告里唯一给出"RDP 语义级证据"的渲染器**（TMEM 512、组合器回卷、compute RSP、`GameFrameMap` 跨帧身份映射、8 管线 × 5 spec constant），因此它是"高保真 + 高帧插值 + 宽屏"三者同时成立的那个后端；但它的 RHI 在 Plume submodule 里，本快照读不到，评估时应另行获取。
5. **TriAevum 展示了 AOT 路线的真实成本形态**：12,419 个生成函数只是最不需要人管的部分，实际投入在 239 篇文档、35 个 renderer3ds 文件、逐特性的 visual-parity 实验与 GPL 合规链上；它的"entirely AI-assisted" 自述与 Zelda64Recomp 的"blanket ban"自述同时存在，是当下这类大型移植工程治理差异的最好样本。
6. **资产格式的真实抽象只有一层：64 字节资源头 + 路径哈希**。MPQ/ZIP/文件夹/.ootrs 都是可替换外壳；把精力花在容器格式上价值有限，**资源类型 magic + 工厂注册 + 覆盖优先级**才是移植生态的可扩展点（§3.3/§3.4/§4.5）。

---

## 附录 A：关键源文件索引

### A.1 Shipwright / SoH（§2）

| 文件 | 作用 |
|---|---|
| `soh/src/code/z_bgcheck.c`、`z_arms_hook.c`、`z_view.c:…` | decomp→SoH 源码改动样本（include 替换 / `ActorInit` / `0x01FF`） |
| `soh/src/code/graph.c:509` | `RunFrame`：渲染主循环入口 |
| `soh/src/gameframe/game.c:259`、`soh/src/play/z_play.c:1650,1682,1687` | 生命周期链 |
| `soh/src/soh/GameInteractor.{h,cpp}`、`GameInteractor_Hooks.h:5-8` | `extern "C"` + Hook 表（约 140 个 hook，X-macro） |
| `soh/src/fast/...`/`soh/src/soh/GbiWrap.cpp:8-35` | GBI 包装层 |
| `soh/src/soh/ShipInit.hpp:11-60` | 初始化注册宏 |
| `soh/src/soh/ActorDB.cpp` | 反射式 actor 数据库 |
| `soh/src/overlays/z_game_dlftbls.c:9-14` | overlay 表 |
| `soh/src/soh/OTRGlobals.cpp:1826-1873,2362-2380,281-282,418-419` | 帧插值提交 / `Randomizer_*` 导出 / `.o2r` 文件名 |
| `soh/src/soh/Enhancements/randomizer/SeedContext.cpp:407-421` | spoiler JSON 解析 |
| `soh/src/soh/Enhancements/*.cpp`、`mod_menu.cpp:194-210` | CVar 名与 mod 容器收集规则 |
| `CMakeLists.txt:309-349,327-335`、`.github/workflows/generate-builds.yml:35-43` | 资产目标与 CI |

### A.2 libultraship（§3）

| 文件 | 作用 |
|---|---|
| `include/ship/resource/archive/Archive.h:17` | `OTR_HEADER_SIZE 64` |
| `src/fast/resource/OtrArchive.cpp:22-62,71,108-111` | MPQ 只读容器 |
| `src/fast/resource/O2rArchive.cpp:41-96,143-194` | ZIP 可写容器 |
| `src/fast/resource/Archive.cpp:38-103,187-300` | 版本/manifest + BLAKE2b-512 + Ed25519 |
| `src/fast/resource/ResourceLoader.cpp:314-338` | 64 B 头字段读取 |
| `src/global/StrHash64.cpp:163-174,176-183` | CRC64 路径哈希（两处取反差异） |
| `src/fast/interpreter.cpp`（5,546 行）、`include/.../lus_gbi.h:39-71` | Fast3D CPU 侧 + OTR GBI opcode |
| `src/fast/backends/gfx_{opengl,direct3d11,metal}.cpp` | 三后端；WRAP 不对称片段 `:133 / :188 / :137` |
| `src/fast/ui/Gui.cpp`、`src/fast/Context.cpp:129` | ImGui 集成 / 加载线程数 |
| `src/fast/libultra/os/*`（impl 372 行 / headers 7,126 行） | libultra 兼容面 |
| `src/fast/controller/*`（7,714 行）、`SDLGyroMapping`、`SimulatedInputLag` | 输入/陀螺仪/输入延迟 |
| `src/crash/CrashHandler.cpp`（477 行）、`docs/migration/EPOCH-2.md:7-19` | 崩溃处理 / 换代文档 |

### A.3 工具链（§4）

| 文件 | 作用 |
|---|---|
| `ZAPDTR/ZAPD/ZFile.h:12-24`、`ZFile.cpp:280-290,389-440,1332-1340` | 模式枚举、`REGISTER_ZFILENODE` 分派、自定义导出、otrMode 短路 |
| `ZAPDTR/ZAPD/ZResource.h:25-68,246,262`、`Main.cpp:432-433,469-490,568-571,585-588` | 资源类型/导出器注册/CLI flag |
| `ZAPDTR/ZAPD/ZRom.cpp:1-40` | ROM 内 DMA 偏移表 |
| `OTRExporter/OTRExporter/{Exporter.cpp:4-21,TextureExporter.cpp:2,Main.cpp:432-437,56-75,extract_assets.py:12-53}` | 写出侧与插件注册 |
| `Torch/{README.md:1,TemplateFactory.cpp:13-20,src/factories/BaseFactory.h:30,46-52,99-105,src/Companion.h:51-56,src/archive/SWrapper.cpp:17-30,69-80,src/archive/ZWrapper.cpp:33-46,78-101,src/main.cpp:139-156,229-236,269-293,Companion.cpp:1905-1912,1930-1933,2560-2578,docs/otr_format/mainpage.md:9-12}` | YAML 工厂 + 双容器写出 + CLI |
| `retro/{pubspec.yaml:1-2,lib/arc/arc.dart:24-40,lib/otr/resource.dart:17,26-41,lib/otr/resource_type.dart:1-27,README.md:5-11}` | Flutter 打包器与退休声明 |
| `neoretro/{CMakeLists.txt:190-246,252-265,src/otr/Types.h:5-6,src/otr/Archive.h:13-40,src/features/games/GameScreens.h:5-30,src/app/Routes.cpp:26-37,cmake/patches/torch-swrapper-close.patch}` | C++ 后继者（raylib+ImGui+clay，Torch 固定 revision） |

### A.4 重编译路线（§5）

| 文件 | 作用 |
|---|---|
| `N64Recomp/include/recomp.h:95-121,437-443` | `MEM_*` 宏 + `recomp_func_t` ABI |
| `N64Recomp/include/recompiler/{operations.h:12-186,generator.h:28-107,context.h:26-56,593-595,live_recompiler.h:67,90,146}` | 操作表/双 Generator/JumpTable |
| `N64Recomp/src/{operations.cpp（4 张表=157）,recompilation.cpp:215-266,837-875,cgenerator.cpp:469,513-600,symbol_lists.cpp:3,143,564,main.cpp}` | 延迟槽、跳表、符号名单 |
| `N64ModernRuntime/README.md:5-27` | 运行时职责切分 |
| `N64ModernRuntime/ultramodern/include/ultramodern/{events.hpp:7-20,extensions.h:18-45,ultramodern.hpp:104,115}` | VI 回调 / dlist 事件 / 倍速 |
| `N64ModernRuntime/librecomp/src/{mods.cpp:253,558-598,900-919,2378-2379,vi.cpp,heap.cpp,eep.cpp,flash.cpp}` | mod 容器/detour/内容类型/离线模式/存档/堆 |
| `rt64/src/hle/{rt64_rdp.h:21,72,105,rt64_game_frame.h:17-106,rt64_interpreter.h:26-29,rt64_present_queue.h:20-43,rt64_workload.h}` | TMEM/跨帧映射/解释器/呈现线程 |
| `rt64/src/render/{rt64_raster_shader.h:54,81-99,rt64_raster_shader.cpp:115-134,rt64_*_processor.h(curFrameWeight)}` | spec constant 特化 + 帧权重 |
| `rt64/src/shaders/{RSPProcessCS.hlsl:17-18,44,66,92,RSPModifyCS/RSPWorldCS/RSPSmoothNormalCS(GROUP_SIZE 64),Fb*CS,RasterPS/RasterVS}` | compute RSP / framebuffer 再解释 / ubershader |
| `rt64/include/rt64_extended_gbi.h`（88 id / 61 宏 / `RT64_EXTENDED_OPCODE 0x64`）、`src/common/rt64_{user,enhancement}_configuration.h` | 扩展命令与配置 |
| `Zelda64Recomp/{README.md:2,9,11-12,patches/Makefile:5-16,patches/patches.ld:1-20,patches/patches.h:4-32,patches/*.c(20,524 行/189 RECOMP_PATCH/24 事件),us.rev1.toml:14-64,patches.toml,src/main/main.cpp:347-362,722-732,mods/mm_recomp_dpad_builtin.nrm}` | 前端与补丁/mod 体系 |

### A.5 掌机 AOT 路线（§6）

| 文件 | 作用 |
|---|---|
| `TriAevum/{README.md:7-12,86-97,205-236,380-394,LICENSE_SCOPE.md:1-22,SOURCE_OFFER.md:1-18,docs/239 篇}` | 定位/AI 立场/合规 |
| `TriAevum/runtime/three_ds_recomp/{README.md:1-24,src/fast/renderer3ds(35 files/9,182 行),src/fast/backends(32 files),src/fast/{dlss,immerse,extension/pica_texture_decode.cpp:50-70},include/fast/renderer3ds}` | vendored LUS + PICA→Vulkan/NRI |
| `TriAevum/tools/{ctr_title/generate_whole_aot_cpp.py,ghidra_inventory.py,match_functions.py,oot3d/native_a32_runtime/upstream/src/oot3d_pack/{arm_lift.py,arm_ir.py},oot3d/third_party/azahar_pica}` | AOT 流水线与参考实现 |
| `TriAevum/docs/OOT3D_LINUX_WHOLE_AOT_PORT.md:5-18`、`OOT3D_NRI_PICA_AOT_RENDERER_PARITY.md:19-50` | 12,419 函数 / 语料→管线一致性 |
| `Ocarina-ReTuned/{README.md:3,25-56,resources/source-pins.json,resources/supported-roms.json,tools/native/{rom_setup.py:449-678,build_windows_cross.py:2-4,apply_neutral_shaders.py,apply_spirv_embedding.py,build_android_capstone.py}}` | 引擎引用/本地着色器生成/多平台准备 |
| `oot3d_practice_menu/{README.md:3-17,oot.ld(38 段/19 名),src/loader.c:13,27,src/svc_loader.s:38,patch.py:20,include/menus/watches.h,code*.ips}` | IPS + 重授权 + hook 点 |

---

## 附录 B：证据标记说明与复核建议

**B.1 证据等级**。本报告正文只用三档表述：① **硬证据**＝给出 `文件:行号` 且本次实读到该行（可 `sed -n` 复现）；② **统计结论**＝由本次命令产出的计数（`grep -c`、`find|xargs awk END{NR}`、`unzip`/`python -c`），换 HEAD 会变；③ **推断**＝由依赖关系/命名/文档措辞得出，正文一律以"推断/应记录为已知问题/需另行获取"标注（典型：§4.6 的工具链取代叙事、§3.7 的换代风险、§4.7 的 fast64↔ZAPD 互补关系）。

**B.2 本次会话的下载/修复记录**。修复：`$P/retro`（原 HEAD 指向 `refs/heads/.invalid`、objects≈12 KB、工作树为空 → 删除后重克隆，`43c8e91`）。新增（均 `git clone --depth 1`）：`Zelda64Recomp`(b65c482)、`N64ModernRuntime`(e4342ae)、`N64Recomp`(598ce20)、`rt64`、`n64-fast3d-engine`(881eb68)、`OTRExporter`(c5465ba develop)、`neoretro`。尝试失败并纠正认知：`HarbourMasters/N64Recomp`、`UltraLibGL`、`HarbourMasters/Ship64`（404，正确 org 为 `N64Recomp/*`；`Ship64` 是 SoH 的旧名而非独立仓库）。

**B.3 未初始化子模块清单（影响可读范围）**。`Shipwright/.gitmodules`（libultraship、torch 等）、`2ship2harkinian`、`TriAevum`；`Zelda64Recomp`：`lib/{N64ModernRuntime,rt64,RmlUi,mm-decomp,lunasvg,freetype-windows-binaries}` + `Zelda64RecompSyms`（全空）；`rt64`：`src/contrib/{plume,dxc,re-spirv,spirv-cross,imgui,implot,im3d,hlslpp,xess,zstd,xxHash,stb,ddspp,nativefiledialog-extended,mupen64plus-core,mupen64plus-win32-deps}`（全空）。**复核建议**：`git submodule update --init --recursive` 后重跑 §5.4/§5.5 的断言，尤其是 rt64 的 Plume 后端与 Zelda64Recomp 的 `mm-decomp` 头文件依赖。

**B.4 已被本报告否证的流传说法**（逐条给出否证位置）：
1. "TriAevum 是把 PC 移植跑在 3DS 上" → 无 CIA/devkitARM 痕迹，方向相反（§6.1）。
2. "Ocarina-ReTuned 是音乐重制项目" → 是 TriAevum 的发行分叉（§6.2）。
3. "Shipwright 含 MM 代码" → 无 MM 代码；MM 在 2ship2harkinian（§2.8）。
4. "Zelda64Recomp 已支持 OoT" → 本快照 `supported_games` 只有 MM 且 `is_enabled=false`（§5.5）。
5. "OoT-Randomizer 世界数据是 YAML" → `data/World/*.json`（§7.1）。
6. "`.zpf` 是 SoH 的输入格式" → 是 Python 随机化器的输出包；SoH 吃 spoiler JSON（§7.1）。
7. "`0x5C7000` 是 practice menu 代码页的固定地址" → 它是 `patch.py:20` 的空洞**上界**（§6.3）。
8. "ZAPDTR 有 `XmlWriter`/`BoundsChecker`"、"`--otr`/`SoHExport`/`n64pngs` 等 CLI flag"、"MTex/MaterialList 资源节点" → HEAD 零命中（§4.1）。
9. "rt64 配置有 `targetRate/frameLimit`" → 实际字段为 `RefreshRate/refreshRateTarget` + `PresentWait` + `Presentation::Mode`（§5.4）。
10. "LUS 有 `Fast3D::ProcessDisplayList`/`GbiRenderer`/`Sched`/`Mesh` 这些名字"、"LUS 支持 Vulkan/D3D12" → 均无（§3.5）。
11. "Torch 文档说 `.otr` 是 ZIP" → 与 `SWrapper` 的 MPQ 实现矛盾，属**仓库内文档不一致**（§4.3，已标注）。

**B.5 版本漂移提醒**。`N64Recomp` 已从"每指令一个 emitter"重构为"rabbitizer + 操作表 + Generator 接口"；`rt64` 已把后端搬进 Plume 并把 README 标为 WIP（模拟器插件/RT 未提供）；`TriAevum` 已从单作品（OoT3D）扩为双作品（+Moonfall Curse）；`retro` 已退休并由 `neoretro` 取代；`neoretro` 内部把 `Ocarina-ReTuned` 上游 `retro` 的 Flutter 实现换成 raylib。因此凡引用行号，复核时**必须先比对 HEAD**：`git -C <repo> rev-parse HEAD`。

**B.6 复现用最小命令集**。

```bash
P=/root/wsl-demo/oot-port-projects
# 规模统计（不受 wc 批次截断影响）
find $P/<repo> -type f \( -name '*.c' -o -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 awk 'END{print NR}'
# 指令表条目
for m in unary_ops binary_ops conditional_branch_ops store_ops; do \
  awk "/const std::unordered_map<InstrId, [A-Za-z]+> $m/,/^    }/" $P/N64Recomp/src/operations.cpp | grep -c 'InstrId::'; done
# 补丁与事件计数
grep -rho RECOMP_PATCH $P/Zelda64Recomp/patches | wc -l
grep -rho RECOMP_DECLARE_EVENT $P/Zelda64Recomp/patches | wc -l
# mod 容器内容
python3 -c "import zipfile;print(zipfile.ZipFile('$P/Zelda64Recomp/mods/mm_recomp_dpad_builtin.nrm').namelist())"
# 子模块状态
git -C $P/rt64 submodule status | awk '{print $1,$2}'
```

**B.7 与既有报告的衔接**。`02_oot_decomp_analysis.md` 覆盖 decomp 工程（`oot`、`OOT3D_DECOMP`、`OoT_decomp`、`oot-ms`、`oocarina`、`oot-z3d`、随机化器脚本层）；本篇覆盖**可运行引擎/渲染后端/资产容器/增强特性**。两篇的重叠面只在"资产与随机化器契约"，交叉引用点在 §2.1（decomp 源码来源）、§4.1（ZAPDTR 与 ZAPD 的差集）、§7.1（3drando C++ 移植）。
