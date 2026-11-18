# 05 补充开源工具库分析报告

- 生成时间：2026-11-18
- 下载位置：`/root/wsl-demo/extra-repos/`
- 本次新增：**16 个上游仓库 / 792 MB 磁盘 / 10,187 个源码文件 / 2,605,240 行源码**
- 分析方式：自研 Python 结构扫描器 + 分组人工细读（按 GTA / 反编译工具链 / 模拟器与随机器三组）

---

## 1. 任务范围与方法

### 1.1 目标
为工作区已有的 GTA 系列与《塞尔达传说：时之笛》(OoT) 仓库群补充**工具链层与运行环境层**的开源项目，使本地具备：可编译的游戏本体反编译参考、匹配反编译（matching decomp）流水线工具、N64 硬件行为参照实现、以及 GTA 侧的网络/钩子/资源格式基础设施。

### 1.2 去重基线
选型前先枚举工作区已有仓库并抓取其 `origin` 上游，避免重复下载。报告生成时的基线：

| 目录 | 仓库数 | 内容 |
| --- | --- | --- |
| `gta-projects/` | 10 | CodeWalker、fivem、mtasa-blue、openrw、scripthookvdotnet、Sollumz、gta-reversed、GTAIV.EFLC.FusionFix、gtamaplib、gta6-leonida-atlas |
| `oot-decomp-projects/` | 18 | zeldaret/{oot,oot-vc,oot-gc,oot3d,ZAPD}、n64decomp/oot、oot-dc(jnmartin84)、oot-pc-recomp、ProjectReLink、HackerOoT、Hookshot、OoT-Randomizer、fast64、N64Recomp、N64ModernRuntime、Zelda64Recomp、Shipwright、TriAevum |
| `oot-port-projects/` | 15 | 2ship2harkinian、libultraship、Shipwright、Torch、ZAPDTR、OTRExporter、retro、rt64、n64-fast3d-engine、Ocarina-ReTuned、oot3d_practice_menu、TriAevum、N64Recomp、N64ModernRuntime、Zelda64Recomp |
| `gta-repos/` | 5（符号链接） | 指向 `/root/orca-workspaces/gta-repos/`，内容为 gta-projects 的子集，非独立仓库 |

> 注：`oot-decomp-projects/` 与 `oot-port-projects/` 在本次任务执行期间被其他进程扩充（初测 7/9 项，终测 18/15 项）。上述基线为**终测快照**；本次新增的 16 个仓库与基线无任何上游重叠。

### 1.3 选型与验证流程
1. GitHub REST `search/repositories` + `repos/{owner}/{name}` 校验存在性、★ 数、语言、最近推送、许可证、是否 archived；
2. `git ls-remote --heads` 校验实际可克隆（排除已删除/改名的候选，如 `GTAmodding/re3`、`queue-mode/splat`、`openrw` 旧命名）；
3. 与 1.2 基线做上游 URL 去重；
4. `git clone --depth 1 --single-branch --no-tags`（不取子模块，控制体积）。

被剔除的候选及原因：`project64/project64`（与 mupen64plus-core + GLideN64 作用重叠，省 100 MB+）、`zeldaret/swsh`（不存在）、`simple64`/`oottracker`（上游已改名或下架）、`HarbourMasters/MipTools`（不存在）。

### 1.4 统计口径（重要，影响数字解读）
- 只统计源码扩展名（C/C++/C#/Python/TS/JS/Lua/Rust/ASM/Shader/CMake/Shell 等）；`.md/.json/.yml/.png` 等不计入代码行；
- 遍历跳过 `.git`、`node_modules`、`build`、`dist`、`out`、`bin`、`obj`、`vendor`、`__pycache__`；
- 含 NUL 字节判定为二进制并排除；单文件上限 60 MB（早期 3 MB 上限曾漏计 YimMenuV2 的 `IconFont.cpp`，已修正）；
- 根级 `libraries/`、`third_party/`、`deps/`、`external/` 及非根级同名目录单独计为 vendored，不计入自研代码；
- 行数 = `splitlines()` 物理行，未做注释剔除。

---

## 2. 下载清单

### 2.1 GTA 组（6 个，`extra-repos/gta/`）

| 本地路径 | 上游 | 钉定 commit | ★ | 许可证 | 磁盘 | 代码文件 | 代码行 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `gta/re3` | hottabxp/re3 | `e7903d7` (2026-11-16) | 514 | **无 LICENSE 文件** | 56 M | 523 | 235,219 |
| `gta/YimMenuV2` | YimMenu/YimMenuV2 @enhanced | `f0c352a` (2026-10-24) | 1,837 | GPL-2.0 | 20 M | 690 | 272,742 |
| `gta/open.mp` | openmultiplayer/open.mp | `2b4a69d` (2026-11-16) | 674 | MPL-2.0 | 3.2 M | 230 | 59,415 |
| `gta/SampSharp` | ikkentim/SampSharp | `e3aa1ea` (2026-10-19) | 233 | Apache-2.0 | 17 M | 1,346 | 170,549 |
| `gta/gta-toolkit` | Neodymium146/gta-toolkit | `f287c66` (2017-06-17) | 71 | 根目录无 LICENSE（文件头 MIT） | 9.0 M | 395 | 43,827 |
| `gta/WidescreenFixesPack` | ThirteenAG/WidescreenFixesPack | `be2a42b` (2026-11-18) | 3,336 | GPL-3.0（+CLA 再许可条款） | 487 M | 1,710 | 431,268 |

### 2.2 N64 / OoT 组（10 个，`extra-repos/n64-oot/`）

| 本地路径 | 上游 | 钉定 commit | ★ | 许可证 | 磁盘 | 代码文件 | 代码行 |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `n64-oot/mm` | zeldaret/mm | `3be2e24` (2026-11-04) | 1,790 | **无 LICENSE 文件** | 56 M | 2,251 | 704,422 |
| `n64-oot/splat` | ethteck/splat | `7548b97` (2026-10-10) | 385 | MIT | 1.8 M | 124 | 15,216 |
| `n64-oot/m2c` | matt-kempster/m2c | `c632946` (2026-10-22) | 682 | GPL-3.0 | 12 M | 1,214 | 96,230 |
| `n64-oot/decomp-permuter` | simonlindholm/decomp-permuter | `860b6d9` (2026-11-10) | 242 | MIT | 12 M | 221 | 30,682 |
| `n64-oot/decomp-toolkit` | encounter/decomp-toolkit | `3eb7314` (2026-11-15) | 333 | Apache-2.0 / MIT 双证 | 5.0 M | 75 | 31,110 |
| `n64-oot/GLideN64` | gonetz/GLideN64 | `a04081a` (2026-10-11) | 891 | GPL-2.0 | 15 M | 473 | 220,560 |
| `n64-oot/mupen64plus-core` | mupen64plus/mupen64plus-core | `daae68e` (2026-11-10) | 1,571 | GPL-2.0 | 9.4 M | 244 | 111,579 |
| `n64-oot/OoT3D_Randomizer` | gamestabled/OoT3D_Randomizer | `d695cf5` (2026-11-01) | 232 | MIT（含 OoT-Randomizer 派生声明） | 9.9 M | 539 | 115,244 |
| `n64-oot/wwrando` | LagoLunatic/wwrando | `99281a3` (2026-10-22) | 427 | MIT | 6.0 M | 78 | 30,094 |
| `n64-oot/Ocarina-Atlas` | antonioalvromero/Ocarina-Atlas | `fae037e` (2026-11-17) | 60 | AGPL-3.0 | 78 M | 74 | 37,083 |

---

## 3. 量化汇总

### 3.1 分组规模

| 分组 | 仓库 | 代码文件 | 代码行 | 磁盘 |
| --- | --- | --- | --- | --- |
| GTA | 6 | 4,894 | 1,213,020 | 602 M |
| N64 / OoT | 10 | 5,293 | 1,392,220 | 205 M |
| **合计** | **16** | **10,187** | **2,605,240** | **792 M** |

### 3.2 语言分布（代码行）

| 语言 | 文件 | 行数 | 主要来源 |
| --- | --- | --- | --- |
| C++ | 2,372 | 894,081 | WidescreenFixesPack、re3、YimMenuV2、OoT3D_Randomizer |
| C | 1,928 | 719,123 | mm（546 k）、mupen64plus-core、re3 头文件 |
| C/Header | 2,152 | 312,589 | GLideN64（126 k）、mm |
| C++/Header | 801 | 192,986 | YimMenuV2、WidescreenFixesPack |
| C# | 1,675 | 173,529 | SampSharp、gta-toolkit |
| Python | 394 | 143,163 | m2c、wwrando、splat、permuter、mm 工具 |
| Assembly | 530 | 49,604 | m2c 测试夹具、wwrando `asm/patches`、mm `rsp` |
| TypeScript | 75 | 38,505 | Ocarina-Atlas |
| Lua | 12 | 36,377 | YimMenuV2 `docs/natives.lua`（生成物） |
| Rust | 86 | 32,934 | decomp-toolkit |

### 3.3 有效代码量校正（避免高估）

| 仓库 | 统计行 | 主要虚高来源 | 估计自研有效行 |
| --- | --- | --- | --- |
| YimMenuV2 | 272,742 | `src/game/frontend/fonts/{IconFont,MainFont}.cpp` 共 166,393 行字体数组；`docs/natives.lua` 30,760；生成的 `Natives.hpp` 等 ~20,063 | **≈ 59 k** |
| m2c | 96,230 | `tests/end_to_end/` 1,007 个夹具文件（170 组用例）、vendored `m2c_pycparser/` 146 文件 | **≈ 24.5 k**（`m2c/*.py`） |
| WidescreenFixesPack | 431,268 | `LightPack.hpp` 等内嵌数据头、`.ixx` C++20 模块体量 | ≈ 250 k（98 个补丁工程摊薄） |
| decomp-permuter | 30,682 | `perm_pycparser/tests/` 139 个 C 头夹具（942 行但占文件数） | ≈ 27 k Python |
| Ocarina-Atlas | 37,083 | `launcher/OcarinaAtlas.cs` 7,766（单文件 WinForms 启动器） | 前端 24,411 行（`src/` 61 文件，含 18 个 `*.test.ts`） |
| mm | 704,422 | 基本无虚高：`src/overlays` 1,248 文件为反编译产物本体 | ≈ 700 k |

### 3.4 工程化水平

- 有 GitHub Actions：15 / 16（缺 `gta-toolkit`，已停更项目）；CI 最重的是 `SampSharp`（8 条 workflow）、`re3`/`OoT3D_Randomizer`/`open.mp`/`YimMenuV2`/`wwrando`（3–4 条）。
- 有**自研**单元测试：**7 / 16** —— `SampSharp`(161 个测试文件 / 9 个测试工程)、`m2c`(37 个单测文件 + **170 组 end-to-end 用例**)、`Ocarina-Atlas`(18 个 `*.test.ts`)、`decomp-permuter`(仅 4 个：`test/test_perm.py`、`test_deduplicate_typedefs.py` 等)、`splat`(3 个入口脚本：`test.py`、`test_n64_entrypoints.py`，另有 `test/basic_app` 构建夹具)、`mm`(25，多为 `tools/` 内脚本)、`wwrando`(4 个 `test/*.py`)。C++ 组（re3 / YimMenuV2 / open.mp / GLideN64 / mupen64plus-core / WidescreenFixesPack / OoT3D_Randomizer / decomp-toolkit）**均无真正单测**，质量主要靠 CI 构建 + clang-format 检查保证。
  > ⚠️ 扫描器口径修正：`decomp-permuter` 与 `m2c` 会把 **vendored 的 `*_pycparser/tests/`（各 140 / 147 个文件）**计入，原始自动统计的"145 / 1161 个测试文件"是虚高值，上表已按实际归属校正。
- 风格约束：`.clang-format` 见 re3、YimMenuV2、open.mp、SampSharp、mm、OoT3D_Randomizer、WidescreenFixesPack(部分)；`.editorconfig` 见 SampSharp、re3；`mypy.ini`/`.pre-commit-config.yaml` 见 m2c、decomp-permuter、splat。
- CI 与测试的错配点：`decomp-permuter` 唯一的 workflow `systray.yml` **只构建 Rust systray，不跑 Python 测试**（需手动 `./run-tests.sh`）；`splat` 的 `unit_tests.yml` 覆盖 Python 3.9–3.14 矩阵，是本批 CI 测试最扎实的仓库。

---

## 4. 逐库结构分析

### 4.1 GTA 补充库（6）

#### `gta/re3` — GTA III / Vice City 静态反编译（C++）
- **定位与补充价值**：工作区内唯一"可编译的 GTA 3D 时代游戏本体全量源码"。`openrw` 只做 San Andreas 且远未完成，`gta-reversed` 仅是反编译类头文件库（不可编译成游戏）。
- **构建**：`CMakeLists.txt`（cmake ≥3.14）+ `conanfile.py`（Conan 1，license 字段为 `"??"`）；并存 `premake5.lua`（VS，选项 `--with-librw/--with-asan/--with-lto`）与 `codewarrior/re3.mcp.xml`（PS2 目标）。`src/CMakeLists.txt:23` 为 `add_executable(re3 WIN32)`，**未声明 C++ 标准**。
- **结构**：`src/` 21 个子系统约 231 k 行 —— `core` 91 / `extras` 85 / `renderer` 62 / `control` 51 / `audio` 31 / `peds` 27 / `vehicles` 27 / `collision` 22 / `rw` 20 / `modelinfo` 19 / `fakerw` 12（RenderWare 头存根）/ `math` 12。
- **匹配反编译特征**：`src/core/config.h:145-190` 用 `GTA_PC/GTA_PS2/GTA_XBOX` × `GTA3_PS2_140/160/PC_10/11/STEAM` 的版本宏矩阵对齐地址与行为；`src/renderer/Renderer.cpp:446` 等处保留原程序疑点 TODO。与 `zeldaret/oot` 的方法论同源，可作跨项目对照。
- **入口**：`src/skel/glfw/glfw.cpp:1861 main()`、`src/skel/skeleton.cpp`、`src/core/main.cpp`。
- **可抽取部件**：`src/math`、`src/core/IniFile.*`、`src/rw/MemoryHeap.cpp`、`src/collision`。
- **风险（最高）**：`README.md:1-2` 自述 "THIS IS NOT MINE，original RE3 was created by GTAModding"；本地 remote 为个人 fork，实际链条 **hottabxp/re3 ← NetheriteOfficial/re3 ← GTAmodding/re3（原仓库已下架）**，浅克隆只有 1 个 commit，无法审计篡改；**仓库根无 LICENSE**（CMakeLists 内注释 "FIXME: missing license"）；`vendor/milessdk` 内嵌专有 MSS `.lib`；`gamefiles/` 含 R\* 原始资产（`models/menu.txd` > 5 MB）。

#### `gta/YimMenuV2` — GTA V: Enhanced D3D12 注入式菜单（C++）
- **补充价值**：提供 CodeWalker / ScriptHookVDotNet / FiveM 这些"官方友好"路线之外的**私有实现**：rage `netSyncTree` 结构、native invoker、D3D12 hook、minhook/VMTHook 体系。
- **构建**：`CMakeLists.txt:3` cmake ≥3.20，`:22` **CXX_STANDARD 23**，`MODULE` 方式产出 DLL，PCH `src/common.hpp`；依赖全走 FetchContent（`cmake/{imgui,minhook,json,luajit,async-logger}.cmake`），无 vcpkg/conan，**无子模块**。
- **分层**：`src/core`（11 模块 / 138 文件：memory 12、hooking 10、scripting 33、frontend 27、commands 22）= 与游戏无关的通用层；`src/game`（275 文件：features 126、frontend 109、gta 53、backend 33、hooks 29）= 偏移与功能层；`src/types`（196 文件 / 15 命名空间）= rage 类型定义。
- **关键文件**：`src/main.cpp`、`src/core/hooking/Hooking.cpp`、`src/core/memory/PatternScanner.cpp`、`src/core/renderer/D3D12Hook.cpp`、`src/game/pointers/Pointers.hpp`、`src/game/gta/invoker/{Invoker.cpp,Crossmap.hpp,natives_gen9.json,GenerateNatives.py}`、`src/game/backend/NativeHooks.cpp`、`docs/lua-api.md`。
- **可抽取部件**：`src/core/memory/*`（模式扫描 + BytePatches）、`src/core/logger`、`src/core/scripting`（LuaJIT 资源与沙箱）、`src/core/commands`（命令—设置绑定）。
- **合规/性质判定（如实记录）**：GPL-2.0；README 要求注入器 Xenos、外部 FSL `WINMM.dll`，并明确需**关闭 BattlEye**；存在 `src/game/backend/AnticheatBypass.cpp`、`src/game/hooks/Anticheat/{GetAnticheatInitializedHash,GetThreadContext,PrepareMetricForSending}.cpp`、`src/game/hooks/Spoofing/{WriteNodeData,WriteSyncTree}.cpp`、`src/types/battleye/*` —— 属于**在线反作弊对抗与身份伪造代码**，仅适合在自有账号/离线环境做机制研究，不可用于对外分发或他机使用。

#### `gta/open.mp` — SA-MP 兼容多人服务端（C++）
- **补充价值**：补齐工作区缺失的**服务端 / 网络层**（此前只有 FiveM 客户端与服务端混合体、MTA 客户端）。
- **构建**：`CMakeLists.txt:9-13` **仅支持 Clang**、`:15` C++17、Win32 加 `/LARGEADDRESSAWARE`；Conan 2 经 `lib/cmake-conan/conan-omp.cmake` 接入。9 个子模块（`SDK`、`CAPI`、`Shared/Network`、RakNet、pawn、pawn-natives、ttmath、cpp-httplib）本次**均未取，故当前不可编译**。
- **架构（重点参考价值）**：稳定 ABI 边界设计。`Server/CMakeLists.txt:6` 定义 `add_server_component()`，把 27 个组件各编为 **SHARED 插件**，只链接 `OMP-SDK` / `OMP-NetCode`；组件实现 SDK 纯虚接口，如 `Server/Components/Checkpoints/checkpoints_component.cpp:15 class CheckpointsComponent final : public ICheckpointsComponent, public PlayerConnectEventHandler`。`README.md:5-15` 用表格明确标注 stable / NOT stable 头文件边界；`CODING.md` 禁止 STL 进入 SDK 头并给出 `API_SCRIPT → PAWN` 参数映射表。
- **规模**：59,415 行；`Server/Source` 仅 7 个文件（`server.cpp:99 main()`），`Server/Components` 193 文件，`Shared/NetCode` 4,099 行（RPC + packet）。
- **可抽取**：`Shared/NetCode/*`、`Tools/abi-check/main.cpp` + `docker/abicheck.sh`（ABI 回归检查，思路可直接搬到 libultraship/Shipwright）、`add_server_component` 模板。
- **合规**：MPL-2.0（文件级 copyleft，可商用），无作弊代码。

#### `gta/SampSharp` — open.mp / SA-MP 的 C# 游戏模式框架
- **补充价值**：与 `scripthookvdotnet` 形成清晰对照（同为"CLR 宿主 + 原生插件"，一个面向 GTA V 客户端脚本、一个面向 SA-MP/open.mp 服务端）；并且是本批**工程化最好**的仓库。
- **构建**：`Directory.Build.props`（net10.0、Nullable、`EnableNETAnalyzers` + `AnalysisMode=Recommended`）、`Directory.Packages.props`（集中版本：xunit.v3 3.2.2、Microsoft.CodeAnalysis 5.3.0、Verify.SourceGenerators）、新式 `SampSharp.slnx`；原生插件侧 `src/sampsharp-component/{CMakeLists.txt,CMakePresets.json}` 链接 `external/dotnet/{hostfxr.h,coreclr_delegates.h}` 与预编译 `nethost.lib/.a`。子模块 `external/sdk`（open.mp SDK）未取。
- **双轨结构（重要发现）**：`src/legacy`（561 文件，v0.x，SA-MP + 反射式互操作，README:11 声明仅安全修复）与新线 `src/SampSharp.OpenMp.{Core,Entities,Entities.Commands}`（合计 ~530 文件，**Roslyn Source Generator 生成 P/Invoke**）。生成器：`src/SampSharp.SourceGenerator/Generators/{OpenMpApiSourceGenerator,OpenMpEventHandlerSourceGenerator,EntryPointSourceGenerator,NumberedTypeSourceGenerator}.cs` + `Marshalling/ShapeDetector.cs`，按 partial type + attribute 产出互操作代码。
- **入口**：`src/sampsharp-component/main.cpp`、`managed-host.cpp`、`proxy-api.hpp`、`crash-handler.cpp`、`proxies/{api,extension,pool,robin-hood}.cpp`；托管侧 `OpenMp.Core/{IStartup,StartupContext,LaunchInstructions}.cs`。
- **可抽取**：Source Generator 的 marshalling 形状引擎、`OpenMp.Core/RobinHood`（无锁结构）、Analyzer/CodeFixes 骨架、`crash-handler.cpp`。
- **质量**：8 条 workflow（component-win64/linux、dotnet-libraries、dotnet-legacy、release…）、9 个测试目录 161 个测试文件、`.editorconfig` + `.DotSettings`；仓库还带 `AGENTS.md`/`CLAUDE.md`，说明其已接入 AI 协作流程。
- **小瑕疵**：`LICENSE` 为 Apache-2.0，但 `Directory.Build.props` 内声明 MIT —— 元数据不一致。

#### `gta/gta-toolkit` — RageLib R\* 资源格式库（C#，已停更）
- **定位**：`README.txt:1` 仅一行 "THIS PROJECT IS DISCONTINUED."（HEAD 提交名 "Last commit"，2017-06-17）。价值在于它是**纯 .NET、无 GUI 的格式库形态**，与 CodeWalker（运行时 + 编辑器）、Sollumz（Blender 插件）互补，适合做格式交叉验证与批处理参考实现。
- **结构**：`RageLib/`（平台无关：`Data/{DataReader,Writer,Endianess,PartialStream}`、`Archives`、`Compression/Deflate`、`Cryptography/Aes`、`Hash/Jenkins`、`Resources`、`ResourceWrappers/{Drawables,Texture,Particles,Fragments}`）→ `RageLib.GTA5/`（`PSO`+`PSOWrappers`、`RBFWrappers`、`ArchiveWrappers/RageArchiveWrapper7.cs`(848 行，RSC7)、`ResourceWrappers/PC`、`Resources/PC` 15 类）→ `Tools/{ArchiveTool,TextureTool(WPF),MetaTool}`。
- **关键文件**：`RageLib.GTA5/PSO/PsoFile.cs`（分节 magic 枚举）、`PSOWrappers/{PsoReader,PsoTypeBuilder,PsoXmlExporter}.cs`、`ResourceWrappers/PC/Drawables/DrawableFileWrapper_GTA5_pc.cs`、`Tools/MetaTool/Program.cs` + `PsoDefinitions.xml` + `Lists/{PsoTypeNames,PsoFieldNames,PsoEnumValues}.txt`（PSO 字段符号字典，可与 CodeWalker 交叉校验）。
- **可抽取**：PSO 读写/导出三件套 + MetaTool 符号字典；`Jenkins`/`AES`/`Deflate` 小组件。
- **构建/风险**：`Toolkit.sln` 12 个**旧式 csproj**，`TargetFrameworkVersion v4.6`（.NET Framework，Linux 下不可直接构建，需 netstandard 化）；`packages.config` 依赖 HtmlAgilityPack 1.4.9.5、NUnit 3.4.1；`Libraries/DirectXTex` 内嵌微软源码（须保留版权头）；根目录**无 LICENSE**；17 文件 / 20,573 行为 vendored。

#### `gta/WidescreenFixesPack` — 单仓库 98 补丁工厂（C++）
- **定位**：ThirteenAG 的跨游戏补丁集（PC / PS2 / PSP / PCSX2 / PPSSPP / RPCS3），本批体积与代码量最大。与已有 `GTAIV.EFLC.FusionFix`（单游戏）互补：这里提供**跨 98 个游戏的 pattern 数据库与多平台构建编排**。
- **构建**：`premake5.lua`（`require "vstudio"`、注册 ps2/psp 平台、98 个 project、`envdir()` 从**未入库的 `.gitignore` 后 `.env`** 读取游戏安装路径、`settings_check` 调用 `tools/asi-settings-validate.exe`）；CI 三重（`appveyor.yml` + `azure-pipelines.yml` + `.github/{main,all,tag}.yml`）。9 个子模块（injector、IniReader、**Hooking.Patterns**、spdlog、ModUtils、ps2sdk、pspsdk、minidx9、FusionFixUtils）未取。无 CMake/vcpkg。
- **规模**：431,268 行；`source/` 96 M（98 个补丁，GTA 系 15 个：GTA3/VC/SA/GTA2/GTA1/DE/Driv3r/RDR\*/GTASA.UWP.Test/PCSX2F/PPSSPP），`includes/` 2.7 M 通用层，`data/` 50 M 配置，`text/` 3.7 M 本地化，`textures/` 125 M。`.ixx`（C++20 模块接口文件）1,173 个是主体。
- **核心抽象（最高复用价值）**：
  - `includes/stdafx.h:128-180` — `pattern_str / find_pattern / find_module_pattern / find_range_pattern`，支持**多版本 pattern 回退 + count_hint 命中数校验**，是跨版本补丁的通用解法；
  - `includes/gameref.hpp` — `GameRef<T, AutoUnprotect>` 延迟地址解析（未解析即抛异常），避免硬编码地址静默错误；
  - `includes/{usercall.hpp,callargs.h}` — 原样 cdecl/stdcall 原函数重调用；`iathook.hpp`、`raw_mem.hpp`、`d3dvtbl.h`。
- **入口样例**：`source/GTA3.FusionFix/dllmain.cpp`（导出 `AsiSettings_GetApi`，`safetyhook::create_mid` 挂在 pattern 命中处）、`Main.ixx`、`FixSettings.hpp`、`settings.json`（`"format":"asi-settings"`，声明 exe 指纹 `gta3.exe`）。
- **风险**：`license` 为 **GPL-3.0**，但 `contributing.md` 的 CLA 第 1/3 条授予 Owner "以任意（含专有/商业）条款再许可且不可撤回" —— 与 GPL 存在张力，二次分发前需评估；体积 487 M（其中 196 M 是 `.git`、125 M 是 `textures/`、50 M 是 `source/SplinterCell.FusionFix/sccompile.bin`），若仅做代码分析可 sparse-checkout 裁到 ~100 M。补丁只 hook 单机可执行文件，**无在线反作弊绕过**。

### 4.2 N64 / GC 匹配反编译工具链（5）

**先说清接入关系（全库 grep 核实，勿凭直觉）**：

- `oot`(zeldaret/oot) 与 `mm` 的 `requirements.txt`/`Makefile` 中**没有任何 splat、ZAPD、decomp-toolkit 引用** —— 它们用自研 `tools/disasm` + `spimdisasm`/`rabbitizer`/`pygfxd`/`crunch64`；但对 m2c / permuter 有**硬接口**：`oot/tools/m2ctx.py`、`oot/tools/permuter_settings.toml`（`build_system="make"`、`compiler_type="ido"`）、`oot/Makefile:820` 的 `PERMUTER` 开关、`oot/diff.py:2901` 明确注明复制了 permuter 的 `src/scorer.py` 打分逻辑。`mm` 完全同源（`mm/tools/m2ctx.py`、`mm/tools/permuter_settings.toml`、`Makefile:488`）。
- 唯一真正引用 decomp-toolkit 的是已有仓库 `oot-gc`：`oot-gc/tools/download_tool.py:43,55-56,87` 从 `encounter/decomp-toolkit` 拉取 **`dtk` 预编译二进制**，`oot-gc/README.md:97-98` 使用 `dtk dol diff` / `dtk dol apply`。本次取源码 = 把"下载来的黑盒二进制"变成可审计、可自建。
- 5 个仓库的 `.gitmodules` 与 gitlink（`git ls-files -s` 中 mode 160000）**均为 0** —— 浅克隆不带子模块在这批上**没有信息损失**；真正缺的是外部产物（IDO 编译器归档、baserom `.z64`）与第三方 Python 包。

#### `n64-oot/splat` — 二进制拆分器（Python，包名 `splat64` 0.50.1）
- **定位**：ROM/ELF → `.s` + YAML 工程的通用拆分器（N64/PSX/PS2/PSP）。相对 oot/mm 的自研 `tools/disasm` 是**平行工具链**，不共享工程配置，只共享 `spimdisasm`/`rabbitizer`/`pygfxd`/`crunch64` 依赖谱系 —— 适用于把**尚未反编译的新 ROM**（如 oot 美版其它 revision、其它 N64 游戏）快速起工程。
- **安装/入口**：`pyproject.toml` name=`splat64`，`[project.scripts] splat = "splat.__main__:splat_main"`，`requires-python >=3.9`，构建后端 hatchling；extras `mips`（spimdisasm≥1.42.1,<2、rabbitizer≥1.12、pygfxd≥1.0.5、n64img、crunch64）与 `dev`（ruff、mypy）。
- **结构**：`src/splat` 104 py / 12,555 行 —— `segtypes/`（common 27、n64 22、psx、ps2、psp + `segment.py`）、`util/`（`options.py` 690、`symbols.py` 749、`cache_handler`、`vram_classes`、`compiler`）、`platforms/`（n64/psx/ps2/psp）、`disassembler/`（`spimdisasm_disassembler`）；另有 `docs` 12 md、`test/basic_app`。
- **核心抽象**：YAML 配置模型 `options / segments / subsegments / vram / symbol_addrs`（样例见 `test/basic_app/splat.yaml`）；类型分派链 `Segment`(`segtypes/segment.py` 893 行) → `CommonSegment` → `CommonSegCode/Group/Rodata/Bss/Img/Gfx/Vtx/Ipl3` 注册式分派。
- **关键文件**：`split.py`（兼容壳）、`src/splat/scripts/split.py`(700)、`segtypes/common/code.py:15`、`util/options.py`、`create_config.py`。
- **测试/CI**：6 条 workflow —— `unit_tests.yml`（Python 3.9–3.14 矩阵，`python3 test.py` + `test_n64_entrypoints.py` + `make -C test/basic_app`）、`lint.yml`、`format.yml`、`pypi.yml`、`test_lib.yml`、`publish_docs_to_wiki.yml`。17 个测试文件。**本批工程化最规整的 Python 库。**
- **可用性**：❌ 本机 `import colorama`/`yaml` 失败且无 pip，需 `pip install -e '.[mips]'` 后才能运行。

#### `n64-oot/m2c` — MIPS/ARM/PPC/SH 汇编→C 反编译器（Python）
- **定位**：`oot/tools/m2ctx.py` 与 `mm/tools/m2ctx.py` 的**直接下游**，是"从汇编到可编译 C"的唯一自动化环节。
- **安装/入口**：`[project.scripts] m2c = "m2c.main:main"`；依赖仅 `graphviz~=0.20.1`（可选），`requires-python >=3.8`（poetry 段 `^3.10`）；pycparser 已 **vendored** 为 `m2c_pycparser/`。
- **结构**：`m2c/` 20 py / **24,497 行**（本批统计的 96,230 行含 1,007 个 `tests/end_to_end` 夹具与 vendored pycparser）；`tests/end_to_end` **170 组用例**、`tests/unit/test_arm.py`、`browser/` + `website.py`（Web UI）、`tools/`、`stubs/`。
- **管线**：`main.py:123 run()` → `asm_file.py` 解析 → `arch_*.py`（`Arch(ArchFlowGraph, ArchC, ArchAbi)`，`translate.py:83`）→ `flow_graph.py:1746 build_flowgraph`（`Block:109`、`BlockBuilder:143`）→ `translate.py`（`PersistentFunctionState:257`、`StackInfo:277`、`Var:881`、`Expression:901`）→ `if_statements.py` 结构化。
- **"候选搜索"的真实位置**（修正常见误解）：不在所谓 assemble_json —— 而是 `types.py:570-632` 的结构体字段偏移候选递归择优，加上 `main.py:231-267` 的 `options.passes` 多趟预解析（丢弃前几趟结果只为喂类型）与 `typepool.prune_structs()`；跳转表歧义在 `flow_graph.py:1012` 报错。m2c **无** `assemble_json` 模块或 `--assemble-json` 选项，仅 `asm_file.py:761,767` 兼容 dtk 的 `.rel` 宏与标签格式。
- **测试/CI**：`run_tests.py`（`python-app.yml`，py3.8 + `poetry run ./run_tests.py`）、`pages.yml`。
- **可用性**：✅ **本批 16 仓中唯一零依赖即可运行的工具**，实测 `python3 m2c/m2c.py --help` 正常退出。

#### `n64-oot/decomp-permuter` — 匹配迭代器（Python + 部分 Rust）
- **定位**：对 m2c 产出的 `.c` 做语句/声明/表达式随机重排 + 编译打分，直到与目标二进制匹配；与 m2c 构成"反编译 → 匹配"闭环。
- **安装/入口**：`pyproject.toml` **不含包元数据**（仅 pyright/black 配置），无 console_scripts；入口 `permuter.py` → `src.main:main`，另有 `import.py`（脚手架）、`pah.py`（网络客户端）。README:18 要求 `pip install pynacl toml Levenshtein`，pycparser 走 vendored `perm_pycparser/`。
- **结构**：`src` 61 文件 / 9,764 行 —— `randomizer.py`(2,704：`Region:113`、`random_weighted:430`、`get_insertion_points:515`)、`perm/perm.py`（Perm 代数族：Root/Text/Ignore/Pretend/Combine/Randomizer/General/Once/Var/LineSwap…）、`candidate.py`(`Candidate:30`、`score:93`)、`scorer.py`(326 行 diff 打分，被 oot 的 `diff.py` 复制)、`compiler.py`（`class Compiler` 以 `subprocess` 驱动 make/ninja）、`preprocess.py`、`ast_types/ast_util`、`net/`（分布式 server/controller，含 `controller/Cargo.toml`）。**注：全库无 "scooter" 标识符。**
- **配置契约**：`default_weights.toml`（按 `compiler_type` 的 `perm_*` 权重）、`example_settings.toml` / `permuter_settings_example.toml`（`build_system = "ninja"`）—— `oot/tools/permuter_settings.toml` 与 `mm` 同名文件正是此格式，含 `[decompme.compilers]` 的 ido7.1/5.3 映射。
- **测试/CI**：`run-tests.sh` = `python3 -m unittest discover -s test/`（`test/test_perm.py`、`test_deduplicate_typedefs.py`，共 145 个测试文件）；⚠️ 唯一 workflow `systray.yml` **只构建 Rust systray，不跑 Python 测试**。
- **可用性**：❌ 实测 `python3 permuter.py --help` 失败于 `import toml`；且即使装齐依赖，也需要 IDO 编译器才能产出匹配。

#### `n64-oot/decomp-toolkit` — GC/Wii 拆分与 ldscript 生成（Rust，二进制 `dtk` 1.9.1）
- **定位**：DOL + REL → 可重定位目标对象 + `.lcf` ldscript（CodeWarrior），服务 GC/Wii 匹配工程。**与 N64 侧 oot/mm/Shipwright/TriAevum 无接口**，但被已有 `oot-gc` 直接依赖（见本节开头）。
- **构建**：`Cargo.toml` edition 2024、`rust-version 1.85`、`[[bin]] dtk`，**无 `[features]` 段**（CI 仍传 `--all-features`）；profile `release` / `release-lto`；关键依赖 `objdiff-core=2.2`(feature `ppc`)、`ppc750cl`、`cwextab`/`cwdemangle`、`nodtool`、`argp`、`zerocopy`，以及 **git 依赖 `ar`（rust-ar 分支，需联网）**。
- **结构**：`src` 32,778 行 —— `cmd/` **21 个子命令**（dol/rel/disc/elf/elf2dol/map/dwarf/extab/rarc/yaz0/wad…）、`obj/`（`ObjInfo`、sections、splits、symbols、relocations）、`analysis/`（cfa、vm、executor、signatures、slices）、`util/`（`split.rs:1305 split_obj`、`dol.rs:207 process_dol`/`704 write_dol`、`rel.rs:372 process_rel`/`759 write_rel`、`lcf.rs:10 generate_ldscript`、`config.rs`、`toposort.rs`）、`vfs/`（disc/u8/rarc/wad）。
- **模板与文档**：`assets/ldscript.lcf`、`ldscript_partial.lcf`、`macros.inc`、`assets/signatures`；`docs/terminology.md`、`docs/other_approaches.md`。
- **测试/CI**：`build.yml` = check + clippy + fmt + cargo-deny + `cargo test --release --all-features`（**未用 nextest**）。
- **可用性**：❌ 本机无 cargo/rustc/rustup，且有 git 依赖需联网；`oot-gc` 默认走"下载预编译 dtk"，故短期不阻塞。

#### `n64-oot/mm` — 梅祖拉的假面匹配反编译（C，700 k 行）
- **定位**：`oot` 的姊妹工程，方法论与工具链**完全同源**，可复用 m2c/permuter/ZAPD 全链；同时是"如何用 overlays 组织 600 k 行反编译产物"的最大范例。
- **结构**：3,315 文件。`src` 1,644（`src/overlays/{actors,effects,gamestates,fbdemos,kaleido_scope}` 631 个 `.c` 且 **0 个 `.s`** → 该目录已 100% 反编译完成；另有 `code`/`boot`/`libultra`/`makerom`/`dmadata`/`audio`）；`tools` 499；`include` 250；`assets` 835；`rsp` 7；`spec/spec`；`linker_scripts/` 4 个 `.ld`（`data_with_rodata.ld`、`final/extra.ld`、`final/undefined_syms.ld`、`soundfont.ld`）。最大文件 `src/overlays/actors/ovl_player_actor/z_player.c` 21,278 行。
- **Makefile 目标**：`rom compress o_files asset_files schedule_inc_files clean assetclean distclean venv setup assets disasm diff-init init all run`；`PYTHON ?= $(VENV)/bin/python3`，`venv` 由 `python3 -m venv` 创建。
- **外部工具（注意：全部是仓库内实文件，非子模块）**：`tools/ZAPD` 222 文件 + `tools/ZAPDConfigs/MM`（与已有 `oot-decomp-projects/ZAPD` 是同一上游的 vendored 副本，`mm/tools/Makefile` 会 `make -C ZAPD`）、`tools/fado` 33（音频）、`tools/asm-processor` 57、`tools/asm-differ` 15、`tools/disasm` 19、`tools/m2ctx.py`、`tools/permuter_settings.toml`、`tools/extract_yars.py`(pyelftools)。
- **依赖**：`requirements.txt` = crunch64、ipl3checksum、pyyaml、rabbitizer、spimdisasm、pyelftools、mapfile-parser、watchdog、argcomplete、Levenshtein、cxxfilt、pycparser、toml。`Dockerfile` 用 ubuntu 24.04 + python3-pip/venv —— **本机无编译器时，这是最省事的构建路径**。
- **测试/CI**：`ci.yml`（构建 + 匹配校验）、`format.yml`。
- **可用性**：❌ 双重阻塞 —— `baseroms/n64-{us,jp-1.1}/` 只有 checksum/config（`.z64` 本体依版权不可入库，需自备），`tools/ido_recomp/` 缺失（由 `tools/Makefile` 联网下载 v1.2）；Python 包与 pip 亦不可用，`make setup` 无法完成。


### 4.3 模拟器、随机器与追踪器（5）

#### `n64-oot/GLideN64` — HLE N64 视频插件（C++）
- **补充价值**：工作区唯一**完整的 RSP 微码族表**（gSP/fSP 到 OpenGL 的 HLE 翻译），是理解 OoT 渲染行为的参照实现。
- **构建**：仅 `src/CMakeLists.txt`（+`projects/cmake`、`projects/msvc`）；开关 `MUPENPLUSAPI` / `MUPENPLUSAPI_GLIDENUI` / `GLIDENUI_QT6` / `ANDROID` / `DEBUG_DUMP`；依赖 OpenGL、Qt5/6，内置 `src/GLideNHQ`；**无子模块**。CI `.github/workflows/build.yml` 覆盖 Linux(Mupen CLI/Qt) 与 Windows(MSVC+vcpkg / MinGW，Project64 ABI)。
- **结构**：`src/uCodes` 54 文件（F3D\*/F5\*/L3D/S2D）、`src/Graphics` 101 文件（`Context.h`、`CombinerProgram`、`ShaderProgram` 后端抽象 + `Graphics/OpenGLContext/opengl_*`，**仅 OpenGL 后端**）、`src/GLideNHQ` 59（高清纹理替换）、`src/GLideNUI` 32、`src/mupenplus` 6（胶水层）。
- **插件边界事实**：`src/PluginAPI.h` 用 `#ifdef MUPENPLUSAPI` 在 `m64p_plugin.h` 与 Project64 `windows/*-spec` 间切换；`src/mupenplus/GLideN64_mupenplus.h` 只 include `m64p_{common,config,vidext,frontend}.h`。
- **与 Zelda64Recomp / N64ModernRuntime 的关系**：**并行方案而非替代**。recomp 走 RT64（`oot-decomp-projects/Zelda64Recomp/lib/rt64`，其 `contrib/mupen64plus-core/doc` 与 `GLIDEN64-LICENSE` 表明只借鉴语义、不加载插件 ABI）；`N64ModernRuntime/README.md` 要求外部注册 renderer（推荐 RT64），本身无插件 ABI。
- **可复用**：`uCodes` 微码表、`Combiner` 合成器、`DepthBuffer`、`GLideNHQ`。

#### `n64-oot/mupen64plus-core` — N64 模拟器核心（C）
- **补充价值**：同时收录 `pure_interp` / `cached_interp` / `x86_64` dynarec / `new_dynarec{arm,arm64,x86}` 四种 CPU 路径，以及 RDP/RSP/VI/AI/PI/SI/MI 硬件时序、64DD、netplay —— 是 recomp 类项目最完整的**行为对照基准**。
- **构建**：`projects/unix/Makefile`（OS 探测）+ `projects/msvc/mupen64plus-core.vcxproj`；第三方在 `subprojects/{md5,minizip,oglft,xxhash}`；外部依赖 SDL2、libpng。CI `build.yml`（Linux 多 cc/arch、MSYS2、mac）+ 遗留 `.travis.yml`/`appveyor.yml`。
- **结构**：`src/device` 127 文件（`r4300`、`rcp/{rdp,rsp,vi,ai,pi,si,mi}`、`cart`、`controllers`、`dd`、`memory`、`pif`、`device.c`）、`src/api` 18（ABI 头 + `frontend.c`/`callbacks.c`/`config.c`/`vidext.c`）、`src/main` 26、`src/plugin`（加载器 + `dummy_*`）、`src/backends` 17。入口 `src/main/main.c`。最大文件 `src/device/r4300/new_dynarec/new_dynarec.c` 12,040 行。
- **与 GLideN64 的关系**：两者构成"核心 + 视频插件"闭环（`src/api/m64p_plugin.h` 定义函数指针表，`src/plugin/plugin.c` 装载，`src/api/api_export.ver` 控制导出）。可用于对 recomp 产物做**逐帧一致性验证**。

#### `n64-oot/OoT3D_Randomizer` — OoT3D 道具随机器（C/C++）
- **定位**：与已有 `oot-decomp-projects/OoT-Randomizer`（N64 / Python）**同语义、异技术栈**。
- **构建**：根 `Makefile`（需 DEVKITARM/`3ds_rules` + `ootrando.rsf`，产出 `.cia` 与 `.3dsx`）、`code/Makefile`（补丁 ELF → `code/patch.py` 手写 IPS）、`generate_patch_symbols.py`（由 `code/basecode_USA.elf` 生成 `source/patch_symbols.cpp`）、`linux_build_rando.sh`。CI：`create_build.yml`、`checks.yml`、`clang_format_code.yml`。
- **双层结构**：`source/` 119 文件 = 主机/3DS 侧逻辑层（`logic.cpp` 1,438、`fill.cpp` 1,175、`hints.cpp` 1,167、`item_pool.cpp` 1,151、`location_access.cpp` + `location_access/locacc_*.cpp` 区域图、`entrance.cpp` 传送层、`spoiler_log.cpp`、`patch.cpp` 715 行写 `/luma/titles/0004000000033500/{code.ips,exheader.bin,romfs}`）；`code/` 400 文件 = 3DS 补丁端（`code/src/{item_override,entrance,hints,enemizer,loader}.c`、`code/src/multiplayer.c` 2,821）；`shared/` 21 个 `s_*.h` = **双端 ABI 契约层**（N64 Python 版没有，其常量直接写在 Python 里）。
- **对照 OoT-Randomizer**：Python 版为 `World.py`/`Region.py` + `Rules.py`/`RuleParser.py` + `Search.py`（`reachable_regions`、`RewindableSearch`）+ `ItemPool/Fill/HintList/Hints/Spoiler`，输出 `.zpf/.zpfz` 或 `ZOOTDEC.z64`（`Main.py:413,468,542`）。
- **可复用**：`shared/` 头契约的组织思路、`locacc_*` 分文件区域图；`oot-decomp-projects/oot3d`（zeldaret/oot3d）可直接为其提供符号源。

#### `n64-oot/wwrando` — 风之杖随机器（Python 3.12）
- **构建**：`build.py`/`build.bat` + `wwrando.spec`（PyInstaller）；`requirements.txt`：PySide6 6.11、QtPy、ruamel.yaml、pillow、appdirs（另有 `requirements_qt5*.txt`）；子模块 `gclib`（DOL/REL/ARC/JPC 读写，本次未取）；补丁汇编用 **devkitPPC**（`asm/assemble.py` 调 `powerpc-eabi-as` + `ld` + `asm/linker.ld`，`asm/elf2rel.py`）。
- **前提校正**：全仓 grep **无 `decomp-toolkit`/`dtk` 引用** —— 它并不使用本次一并下载的 decomp-toolkit，二者是并行生态。
- **布局**：`logic/`（`logic.py` 1,311、`item_locations.txt`、`macros.txt`）、`asm/` 59 文件（`patches/*.asm` 20 个特性补丁、`patch_diffs/*_diff.txt`、`patcher.py` 按符号写入 `sys/main.dol` 追加 Text2 段或任意 `.rel` 并重建重定位表）、`randomizers/`（items/entrances/enemies/hints/charts/palettes/music/boss_reqs）、`options/`、`wwr_ui/`、`data/`、`seedgen/`、`keys/`；主文件 `randomizer.py` 1,107 行（permalink 编解码）。
- **测试/CI**：`pytest.ini` + `test/{test_dry,test_gui,test_save,test_helpers}.py`；`test.yml` 四平台矩阵、`python-app.yml`、`publish-site.yml`(mkdocs)。
- **向 OoT 流水线迁移的判定**：其"每特性 `.asm` + 符号 diff + DOL 自由段"模型与 OoT-Randomizer 现栈同构（`OoT-Randomizer/ASM/Makefile` 用 mips64-gcc/ld，`ASM/build.py` 调 armips 产 `asm_symbols.txt`，`Rom.py:205 revert_patch` 按 `data/generated/patch_symbols.json` 回滚；**注意 OoTR 现已不再使用 ZPSD**）。迁移成本集中在 ISA/汇编器（PPC ↔ MIPS），逻辑层（`Entrances.py`/`Hints.py` 算法）可直接对照移植。

#### `n64-oot/Ocarina-Atlas` — OoT 实时地图与存档遥测（TypeScript + C#）
- **定位**：TS/React 前端 + 平台启动器，通过 **debug/RPC 遥测**读取运行中游戏（OoT3D Remake、Longshot/ReLink/Triforce 等 PC 重编译与 Switch 模拟）的内存状态。补上工作区缺失的"外部追踪器"一环。
- **构建**：`package.json`（vite 8 + `@tanstack/react-start` + react 19 + tailwind 4 + vitest 5 + electron 45/electron-builder）；`server:build` 用 rolldown 产出 `dist/standalone-server.mjs`；`scripts/build-win.mjs` 以 .NET Framework `csc.exe` 编译 `launcher/OcarinaAtlas.cs`；`scripts/build-linux.mjs` 用 gcc 编译 `launcher/linux/longshot_rpc_bridge.c` 并打 AppImage。CI：`ci.yml`（Node 22/24 typecheck + vitest + build）、`release-windows.yml`。
- **结构**：`src/` 61 个 ts/tsx 共 24,411 行（含 18 个 `*.test.ts`）；最大文件 `src/lib/lichii-server.ts` ≈ 4,484 行（TCP `:6545` debug/RPC 客户端 + HTTP `/api/lichii/state` + `handleLichiiStreamRequest` SSE 推流）；另有 `sync-server.ts`（多世界同步）、`coords.ts`/`floorGrid.ts`/`prerequisites.ts`/`progress-stats.ts`、`src/data/oot.json`(208 K)、`src/components/{WorldMap,MapViewport}.tsx`（`<img>` 图块 + pin 绝对定位）。
- **与已有"地图库"的判定**：与 `gta-projects/gta6-leonida-atlas`（React19 + leaflet + three + dexie/IndexedDB 的静态社区地图，Playwright e2e，**无遥测/WebSocket**）以及 `gtamaplib`（纯 Python 坐标脚本）**不同源**，仅信息架构（图块 + pin + 前置清单）相似，数据源与技术栈完全不同。
- **可复用**：`launcher/linux/longshot_rpc_bridge.c` 的客户机地址翻译（`GUEST_CODE_BASE 0x80000000`、`GUEST_WINDOW_BASE 0x100000000000`）与 `lichii-server.ts` 状态机，可直接服务于 `Zelda64Recomp` / `ProjectReLink` 的调试遥测。

---

## 5. 与既有仓库的互补关系

### 5.1 直接填补的缺口

| 已有仓库/链路 | 缺失环节 | 本次填补 |
| --- | --- | --- |
| `oot`(zeldaret/oot)、`oot-vc`、`oot-gc`、`HackerOoT`、`oot-dc` | `requirements.txt` 已声明 `# decomp-permuter → pycparser, toml`，并自带 `tools/m2ctx.py`、`tools/permuter_settings.toml` 包装脚本，但**外部工具本体不在本地** | `m2c`、`decomp-permuter`（包装脚本 → 工具本体的闭环） |
| `oot`、`n64decomp-oot`、`oot3d` | 缺少同代际 N64 项目的横向对照 | `mm`（zeldaret 同族方法论，700 k 行，含 `tools/` 全套脚本） |
| `Zelda64Recomp`、`N64Recomp`、`N64ModernRuntime` | 缺少硬件行为真值参照（recomp 只做指令翻译，不含 RDP/RSP 时序） | `mupen64plus-core` + `GLideN64`（可作逐帧对照基准） |
| `OoT-Randomizer`(Python/N64) | 缺少 3DS 平台同语义实现、缺少双端 ABI 契约设计参考 | `OoT3D_Randomizer`（`source/` 逻辑 + `code/` 补丁 + `shared/` 契约） |
| `oot-gc`（zeldaret GameCube 版） | `tools/download_tool.py:43,55-56,87` 从 GitHub Releases 拉 **`dtk` 预编译二进制**（黑盒），`README.md:97-98` 依赖 `dtk dol diff/apply` | `decomp-toolkit`（本地源码化，可审计、可自建） |
| `oot-gc`、未来 GC/Wii 工程 | 缺少 DOL/REL 拆分的通用参考实现 | `decomp-toolkit` 的 `util/{split,dol,rel,lcf}.rs` |
| （无现成消费方） | zeldaret 系**不使用** splat（自有 `tools/disasm`） | `splat` 定位为"新 ROM 起工程"的平行工具链，需自写 `splat.yaml` 迁移 |
| `CodeWalker`、`Sollumz`、`gta-reversed` | 缺少独立的 R\* 资源格式纯库形态、缺少 pattern 扫描/pso 符号字典交叉验证 | `gta-toolkit`（PSO↔XML + `PsoDefinitions.xml` 符号字典）、`WidescreenFixesPack`（多版本 pattern 回退） |
| `fivem`、`mtasa-blue`、`openrw` | 缺少独立多人**服务端**与 stable-ABI 插件框架范例 | `open.mp`（27 个 SHARED 组件 + `Tools/abi-check`）、`SampSharp`（CLR 宿主 + Source Generator） |
| `gta-projects`（无游戏本体可编译源码） | GTA III/VC 全量可编译源码 | `re3` |
| `OoT-Randomizer`、`oot3d_practice_menu` | 缺少运行时外部实时追踪器 | `Ocarina-Atlas` |

### 5.2 补全后的完整链路

```
ROM/ISO ──► 拆分:  splat · decomp-toolkit · ZAPD/OTRExporter(已有)
        ──► 反编译: m2c ──► 匹配迭代: decomp-permuter + asm-differ
        ──► 可编译源码: oot / oot-vc / oot-gc / oot3d / mm (zeldaret 系)
                        re3 (GTA III/VC)
        ──► 资产往返: ZAPD · ZAPDTR · fast64 · Sollumz · gta-toolkit(PSO)
        ──► 运行时:   N64Recomp · Zelda64Recomp · N64ModernRuntime · rt64
                      ‖ 对照基准: mupen64plus-core + GLideN64
        ──► MOD/逻辑: OoT-Randomizer(Py) · OoT3D_Randomizer(C++) · wwrando(Py)
        ──► 遥测/追踪: Ocarina-Atlas · oot3d_practice_menu
GTA:  钩子/pattern: WidescreenFixesPack · YimMenuV2(core/memory)
      脚本运行时:   scripthookvdotnet ↔ SampSharp
      服务端/netcode: fivem · mtasa-blue ↔ open.mp
```

---

## 6. 横向技术主题（可直接移植的部件 Top 清单）

| 主题 | 来源文件 | 可移植到 |
| --- | --- | --- |
| 多版本 pattern 回退 + 命中数校验 | `WidescreenFixesPack/includes/stdafx.h:128-180` | CodeWalker / 自研 GTA 补丁工具 / FusionFix 系列 |
| 延迟地址解析（未解析即抛） | `WidescreenFixesPack/includes/gameref.hpp` | 所有基于偏移量的 hook 项目 |
| 原样 cdecl/stdcall 重调用 | `WidescreenFixesPack/includes/{usercall.hpp,callargs.h}` | re3 对照调试、gta-reversed 工具化 |
| 内存扫描与 hook 抽象 | `YimMenuV2/src/core/memory/*`、`src/core/hooking/*` | 通用 Windows 注入工具骨架 |
| Lua 沙箱与资源系统 | `YimMenuV2/src/core/scripting/*` | FiveM/MTA 侧脚本加载器对照 |
| Stable-ABI 组件插件架构 | `open.mp/Server/CMakeLists.txt:6 add_server_component()` + `CODING.md` | libultraship / Shipwright 的插件边界设计 |
| ABI 回归检查 | `open.mp/Tools/abi-check/main.cpp`、`docker/abicheck.sh` | ZAPD / libultraship 的 CI |
| Roslyn Source Generator 互操作 | `SampSharp/src/SampSharp.SourceGenerator/**` | 任何 C# ↔ native 绑定（含 CodeWalker 脚本层） |
| PSO/RSC7 读写 + 符号字典 | `gta-toolkit/RageLib.GTA5/{PSO,PSOWrappers}/**`、`Tools/MetaTool/**` | Sollumz / CodeWalker 交叉验证 |
| RSP 微码 → GL 合成器 | `GLideN64/src/uCodes/**`、`src/Graphics/Combiner*` | RT64 / recomp 渲染后端调参 |
| 双端共享 ABI 契约头 | `OoT3D_Randomizer/shared/s_*.h` | OoT-Randomizer 的 ASM 符号管理 |
| DOL/REL 自由段注入 | `wwrando/asm/patcher.py`、`decomp-toolkit/src/util/split.rs` | GC/Wii 版 Zelda 项目（oot-gc） |
| 客户机↔主机地址翻译 | `Ocarina-Atlas/launcher/linux/longshot_rpc_bridge.c` | Zelda64Recomp / ProjectReLink 遥测 |

---

## 7. 本机可用性评估

### 7.1 工具链现状（实测）

| 工具 | 状态 | 影响 |
| --- | --- | --- |
| `python3` | ✅ 3.13.5 | 可跑 m2c |
| `pip` / `ensurepip` | ❌ 均不存在（`venv` 模块在但无法装包） | splat / decomp-permuter 无法启动 |
| `pyyaml` / `toml` / `colorama` | ❌ 缺失 | 同上（实测报错见 7.2） |
| `node` / `npm` | ✅ v24.22.3 / 11.19.1 | Ocarina-Atlas 前端可 `npm i && npm run dev` |
| `gcc` `clang` `make` `cmake` | ❌ 全部缺失 | 所有 C/C++ 仓库（re3、open.mp、YimMenuV2、WFP、GLideN64、mupen64plus、mm、OoT3D）**只能做静态阅读，无法构建** |
| `dotnet` | ❌ | SampSharp、gta-toolkit 不可构建 |
| `cargo`/`rustc` | ❌ | decomp-toolkit 不可构建 |
| `apt-get` | ✅ 可用 | 具备一键补齐条件 |
| `git` | ✅ 2.47.3（`gh` CLI 不在） | — |

### 7.2 实测运行结果

```
$ python3 n64-oot/m2c/m2c.py --help
usage: m2c.py [-t mips-ido-c] [--context C_FILE] [-f FN ...] filename [filename ...]
Decompile assembly to C.                          ← 开箱可用（仅依赖 graphviz 为可选项）

$ python3 n64-oot/decomp-permuter/permuter.py --help
ModuleNotFoundError: No module named 'toml'        ← 被阻塞

$ python3 n64-oot/splat/split.py --help
ModuleNotFoundError: No module named 'colorama'    ← 被阻塞
```

### 7.3 子模块与外部产物缺口（影响可构建性）

| 仓库 | 子模块数 | 关键缺失 | 后果 |
| --- | --- | --- | --- |
| `open.mp` | 9 | `SDK`、`CAPI`、`Shared/Network`（独立仓库）+ RakNet、pawn | **完全不可编译** |
| `WidescreenFixesPack` | 9 | Hooking.Patterns、IniReader、injector、spdlog、ps2sdk、pspsdk | 不可编译；pattern 核心库正是要看的部分 |
| `re3` | 4 | `vendor/librw`（空） | 需 librw + RenderWare 头才能链接 |
| `SampSharp` | 1 | `external/sdk`（open.mp SDK） | 原生插件不可编译 |
| `wwrando` | 1 | `gclib` | ISO 读写不可用，逻辑层仍可读 |
| `mm` | **0**（无 `.gitmodules`、无 gitlink） | — 其 `tools/{ZAPD,asm-differ,asm-processor,fado,disasm}` 是仓库内实文件副本（222/15/57/33/19 个文件），**未丢失**；真正缺的是 `tools/ido_recomp`（由 `tools/Makefile` 联网取 v1.2）与 baserom `.z64` | 不缺源码，缺构建输入 |
| `splat` / `m2c` / `decomp-permuter` / `decomp-toolkit` | **0** | pycparser 均已 vendored（`m2c_pycparser/`、`perm_pycparser/`） | 信息完整 |
| YimMenuV2 / GLideN64 / mupen64plus-core / OoT3D_Randomizer / Ocarina-Atlas / gta-toolkit | 0 或无关键项 | — | 结构完整 |

> 校正说明：初版把 `mm` 的 `tools/*` 误判为"未取子模块"。实测 `git ls-files -s | grep ^160000` 在 5 个反编译工具链仓库中均为 0 —— 这批浅克隆**没有子模块损失**，只有 GTA 组的 `open.mp`(9)、`WidescreenFixesPack`(9)、`re3`(4)、`SampSharp`(1) 与 `wwrando`(1) 存在真实缺口。

### 7.4 磁盘占用热点

| 路径 | 占用 | 构成 |
| --- | --- | --- |
| `gta/WidescreenFixesPack` | 487 M | `.git` 196 M + `textures/` 125 M + `source/` 96 M（含 50 M `sccompile.bin`）+ `data/` 50 M |
| `n64-oot/Ocarina-Atlas` | 78 M | `.git` 37 M + 542 个 `.webp` 地图图块 |
| `gta/re3` / `n64-oot/mm` | 各 56 M | `gamefiles/` 原始资产、`assets/` |
| 其余 12 仓合计 | 123 M | 纯代码 |

> 全部工作树代码为 494 MB，其余为 `.git` 与资产。若只需代码结构，可对 WFP 做 `git sparse-checkout set source includes` 或使用 `--filter=blob:none` 重新克隆，单仓可省 ~380 MB。

---

## 8. 许可证与合规风险矩阵

| 仓库 | 许可证 | 风险等级 | 说明 |
| --- | --- | --- | --- |
| `gta/re3` | **无 LICENSE** | 🔴 高 | 上游 GTAmodding 原仓库已下架，本地为三重 fork 镜像（浅克隆仅 1 commit，不可审计）；含专有 MSS SDK 与 R\* 原始资产 |
| `gta/YimMenuV2` | GPL-2.0 | 🔴 高 | **含反作弊绕过/网络身份伪造代码**（`AnticheatBypass.cpp`、`hooks/Anticheat/*`、`hooks/Spoofing/*`）；README 要求关闭 BattlEye。仅限自有环境研究，禁止用于他人设备或对在线服务作破坏 |
| `n64-oot/mm`、`oot*`（已有） | 无 LICENSE | 🟠 中 | zeldaret 系惯例（反编译产物，不含任天堂代码即无版权，但也不授许可）；仅内部研究可用 |
| `gta/gta-toolkit` | 根目录无 LICENSE，文件头 MIT | 🟠 中 | 停更 9 年；内嵌 MS DirectXTex 需保留版权头 |
| `gta/WidescreenFixesPack` | GPL-3.0 | 🟠 中 | CLA 授予 Owner 不可撤回的任意再许可权，与 GPL 存在张力；copyleft 会传染衍生补丁 |
| `n64-oot/Ocarina-Atlas` | AGPL-3.0 | 🟠 中 | **网络服务条款**：若把其服务端（含 `/api/lichii/*`）对外提供，需开源全部衍生；本地研究不受影响 |
| `n64-oot/GLideN64`、`mupen64plus-core`、`m2c` | GPL-2.0 / GPL-2.0 / GPL-3.0 | 🟡 低-中 | copyleft；m2c 若嵌入自研闭源工具需隔离进程 |
| `gta/open.mp` | MPL-2.0 | 🟢 低 | 文件级弱 copyleft，修改的文件需开源，可商用 |
| `gta/SampSharp` | Apache-2.0 | 🟢 低 | 与 `Directory.Build.props` 声明的 MIT 不一致，引用前以 `LICENSE` 为准 |
| `splat`、`decomp-permuter`、`OoT3D_Randomizer`、`wwrando`、`decomp-toolkit` | MIT / MIT / MIT / MIT / Apache-2.0+MIT | 🟢 低 | 宽松，可直接复用与再分发 |

> 统计校正说明：本次自动扫描器对许可证关键字的初判存在误报（曾把 `m2c`、`YimMenuV2`、`Ocarina-Atlas` 报成 MIT），上表已**逐一打开各仓库 LICENSE 文件人工复核**修正。引用许可证信息时以本表为准。

---

## 9. 建议后续动作（按优先级）

1. **补齐 Python 依赖，激活匹配反编译闭环**（当前唯一"低成本高收益"阻塞项）：
   `apt-get install -y python3-pip python3-yaml python3-toml python3-colorama python3-graphviz`
   随后 `splat`/`decomp-permuter` 即可运行，`oot` 的 `tools/m2ctx.py`、`tools/permuter_settings.toml` 才能真正生效。
2. **补齐构建链**（若需编译验证）：`apt-get install -y build-essential cmake clang pkg-config libsdl2-dev libqt5opengl5-dev`；.NET 与 Rust 按需通过官方脚本安装。
3. **补取真实缺失的子模块**（仅 GTA 组 + wwrando 有此需要）：`open.mp`（SDK / CAPI / Shared/Network）、`WidescreenFixesPack`（Hooking.Patterns / IniReader / injector / spdlog）、`re3`（vendor/librw）、`SampSharp`（external/sdk）、`wwrando`（gclib）。命令模板：`git -C <repo> submodule update --init --depth 1 <path>`。反编译工具链 5 仓**无需**补子模块。
4. **激活 oot / oot-gc 流水线**：`oot`、`oot-vc`、`oot-gc`、`mm` 的匹配构建还需要 IDO/CodeWarrior 编译器与 baserom —— 编译器可由各仓库 `tools/Makefile` 联网自取（ido_recomp v1.2），**baserom `.z64`/`.iso` 需自行从正版介质提取**（不入版本库，属法务边界）。
5. **体积治理**：对 `WidescreenFixesPack`、`Ocarina-Atlas`、`re3` 采用 sparse-checkout 或 `--filter=blob:none` 重取，可回收 ~500 MB。
6. **合规隔离**：`YimMenuV2` 与 `re3` 建议标注为"仅研究、不外发、不并入任何分发产物"；任何要沉淀到自研工具的代码，优先从 🟢 组（splat / permuter / wwrando / decomp-toolkit / open.mp / SampSharp）取。
7. **下一批候选**（本次未取，供后续扩展）：`project64/project64`（Windows 插件 ABI 另一实现）、`n64decomp/sm64`（最成熟的 diff/工具范式）、`ethteck/m3c`（m2c + permuter 编排壳）、3DS 侧 `ZDS3` 资产工具、以及 GTA 侧 `Neodymium146/Utilising-the-Formats`（R\* 格式文档）。
