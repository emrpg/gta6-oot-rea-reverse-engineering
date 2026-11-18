
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
