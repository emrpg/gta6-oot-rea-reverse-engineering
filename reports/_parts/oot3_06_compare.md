
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
