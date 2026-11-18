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
