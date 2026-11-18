## 3.1 OpenRW（rwengine/openrw）

### 项目定位

OpenRW（"Open ReWrite"）是 GTA III 可执行文件的跨平台开源重写，GPL-3.0-only。它的技术路线与 re3 / reVGS（GTA-Mods 系）根本不同：**不做二进制等价（binary-identical）改写，也不注入原 exe**，而是把 RenderWare 的*资产文件格式*（DFF / TXD / IMG / COL / IFP）与 GTA 的*数据/脚本/存档格式*（IDE / IPL / GXT / SCM / .b save）作为公开规格重新实现，再在其上搭一层游戏逻辑。渲染后端替换为 OpenGL 3.3 core，物理替换为 Bullet，音频替换为 OpenAL + FFmpeg，窗口/输入替换为 SDL2。

需要先纠正本报告任务书中的一处前提：本仓库当前 HEAD（`5c5f266`）的目录结构是 `rwcore/ rwengine/ rwgame/ rwviewer/ rwtools/ tests/`，**不存在** `source/`、`engine/`、`game/` 三层；也没有 "game profile" 抽象层，README.md 首段自我描述为"a cross-platform, open source re-implementation of Rockstar Games' Grand Theft Auto III"，对 Vice City / San Andreas 未作任何承诺（全仓库对 VC/SA 的引用只有注释与少量枚举残留）。因此下面所有关于"三作共用"的结论都是**否定性**的。

### 元数据表

| 项 | 值 | 证据 |
|---|---|---|
| 上游 | rwengine/openrw | `git remote` |
| 本地 HEAD | `5c5f266`，日期 2025-06-18；本克隆为 squash/浅克隆，仅 1 个 commit，作者信息不可用于统计 | `git log` |
| 许可 | GPL-3.0（`COPYING` 35 KB）+ README 第三方许可清单 | `README.md` |
| 语言标准 | C++17（`CXX_STANDARD 17`，`CXX_EXTENSIONS OFF`）；C11 | `cmake_configure.cmake:198-204` |
| CMake 最低版本 | `3.8...3.28` | `CMakeLists.txt:1` |
| 文件总数 | 353（`git ls-files \| wc -l`）；工作区 3.3 MB（不含 `.git`） | — |
| 手写 C++ 规模 | 约 53.9k 行（`.cpp` 27,819 / `.hpp` 13,124 / `.inl` 12,928） | `find` + `wc -l` |
| 生成代码 | `rwcore/gl/gl_core_3_3.{c,h}` 共 8,478 行（glad 风格无 `dlopen` 加载器），已从覆盖率中排除 | `rwcore/CMakeLists.txt:70-74` |
| 目标产物 | 静态库 `rwcore`→`rwengine`；库 `librwgame` + 可执行 `rwgame`；可选 `rwviewer`（Qt5）、`rwfontmap`（Qt5+FreeType）、`rwtests` | 各 `CMakeLists.txt` |
| 依赖 | Boost（program_options/system/test）、Bullet、GLM、OpenAL、SDL2、FFmpeg（avcodec/avformat/avutil/swresample）、Freetype（仅 tools）、Qt5（仅 viewer/tools）；子模块 imgui、microprofile | `CMakeLists.txt:31-52`、`conanfile.py:31-45` |
| CI | GitHub Actions（Arch Linux / Fedora / Ubuntu 三容器）、AppVeyor（MSVC 2017 + Conan）、Travis（macOS，仅 badge）、CDash（`my.cdash.org/index.php?project=OpenRW`）、codecov | `.github/workflows/build-and-test.yml`、`.appveyor.yml`、`CTestConfig.cmake` |

### 目录与模块清单（含行数统计）

统计口径：该目录**本层**文件（不含子目录）的 `.cpp/.c` 与 `.hpp/.h` 行数。

| 路径 | 文件 | .cpp 行 | 头文件行 | 合计 | 职责 |
|---|---|---|---|---|---|
| `rwcore/rw` | 5 | 16 | 167 | 183 | RW 基础宏：`RW_CHECK`/`RW_ASSERT`/`RW_UNIMPLEMENTED`、`bit_cast`、前向声明 |
| `rwcore/data` | 2 | 152 | 287 | 439 | `Clump` / `Atomic` / `Geometry` / `ModelFrame` / `GeometryVertex` |
| `rwcore/loaders` | 9 | 968 | 604 | 1,572 | **RenderWare 格式层**：`LoaderDFF`、`LoaderTXD`、`LoaderIMG`、`LoaderSDT`、`RWBStream` |
| `rwcore/gl` | 8 | 5,786 | 2,914 | 8,700 | `GeometryBuffer`（VBO+语义化属性）、`DrawBuffer`（IBO）、`TextureData`；含 8,478 行生成 loader |
| `rwcore/fonts` | 8 | 412 | 510 | 922 | `FontMap`/`FontMapGta3`（GSChar↔Unicode）、`GameTexts`、`Unicode` |
| `rwcore/platform` | 5 | 161 | 147 | 308 | `FileIndex`（大小写归一 + IMG 归档统一寻址）、`FileHandle`、`RWWindows` |
| `rwengine/src/engine` | 17 | 4,612 | 1,769 | 6,381 | `GameData`、`GameWorld`、`GameState`、`SaveGame`、`Animator`、`Garage`、`Payphone`、`ScreenText` |
| `rwengine/src/script/modules` | 3 | 923 | 10 | **13,861** | `GTA3Module.cpp`（904 个 `bind()`）+ `GTA3ModuleImpl.inl`（12,928 行 opcode 实现） |
| `rwengine/src/objects` | 16 | 2,962 | 1,379 | 4,341 | `GameObject`/`VehicleObject`/`CharacterObject`/`InstanceObject`/`PickupObject`/`ProjectileObject`/`CutsceneObject` |
| `rwengine/src/render` | 20 | 2,413 | 1,424 | 3,837 | `Renderer` 抽象 + `OpenGLRenderer`、`GameRenderer`、`ObjectRenderer`、`WaterRenderer`、`MapRenderer`、`TextRenderer`、`DebugDraw`、`VisualFX`、`ViewFrustum` |
| `rwengine/src/ai` | 12 | 1,722 | 576 | 2,298 | `AIGraph`/`AIGraphNode`、`CharacterController`、`PlayerController`、`DefaultAIController`、`TrafficDirector` |
| `rwengine/src/data` | 19 | 804 | 1,341 | 2,145 | `ModelData`、`CollisionModel`、`PedData`、`VehicleInfo`、`AnimGroup`、`ZoneData`、`Weather`、`Chase`、`PathData` |
| `rwengine/src/audio` | 14 | 1,394 | 388 | 1,782 | `SoundManager`/`Sound`/`SoundSource`（FFmpeg→OpenAL）、`SoundBuffer(Streamed)`、`SfxParameters` |
| `rwengine/src/script` | 10 | 613 | 1,116 | 1,729 | `ScriptMachine`（VM）、`SCMFile`、`ScriptModule`、`ScriptTypes`、`ScriptFunctions` |
| `rwengine/src/loaders` | 16 | 1,089 | 300 | 1,389 | `LoaderIDE`/`LoaderIPL`/`LoaderCOL`/`LoaderGXT`/`LoaderIFP`/`LoaderCutsceneDAT`/`GenericDATLoader`/`WeatherLoader` |
| `rwengine/src/dynamics` | 5 | 207 | 140 | 347 | `CollisionInstance`（Bullet body 装配）、`HitTest`、`RaycastCallbacks` |
| `rwengine/src/items` | 2 | 83 | 64 | 147 | `Weapon` |
| `rwengine/src/core` | 4 | 45 | 94 | 139 | `Logger`、`Profiler`（microprofile 包装） |
| `rwgame` | 24 | 2,437 | 1,507 | 3,944 | `main.cpp`、`RWGame`、`GameBase`、`GameWindow`(SDL)、`GameInput`、`RWConfig(.inc)`、`HUDDrawer`、`MenuSystem`、`RWImGui`、`State*` |
| `rwgame/states` | 12 | 1,379 | 244 | 1,623 | `LoadingState`、`IngameState`、`PauseState`、`MenuState`、`DebugState`、`BenchmarkState` |
| `rwviewer`（+models/views/widgets） | 35 | 1,667 | 747 | 2,414 | Qt5 资产浏览器（DFF/TXD/IMG/GXT/动画） |
| `rwtools/rwfont` | 1 | 272 | 0 | 272 | FreeType→字体图集离线生成器 |
| `tests` | 43 | 3,439 | 135 | 3,574 | Boost.Test 单元测试 |

关键观察：**单一文件 `GTA3ModuleImpl.inl`（12,928 行）就占全项目手写代码的 24%**，是脚本层重写的绝对主体；而 `rwcore` 去掉生成代码后只剩约 220 行手写 GL 包装 —— 说明 RenderWare 镜像的工作量集中在*格式解析*与*逻辑*，不在图形后端。

### 构建系统与依赖图

```
rw_interface (INTERFACE, openrw::interface)   ← cmake_configure.cmake：所有警告/宏/平台定义
rw_checks     (INTERFACE, openrw::checks)     ← sanitizer 专用，与警告分离
   └─ rwcore  ← Boost::boost, glm::glm, OpenGL::GL(private)
       └─ rwengine  ← bullet::bullet, ffmpeg::ffmpeg, OpenAL::OpenAL, [microprofile]
           └─ librwgame (STATIC)  ← SDL2::SDL2, Boost::program_options, imgui::sdl_gl3
               ├─ rwgame (executable, main.cpp only)
               └─ rwtests (executable)
       └─ rwfontmap (rwtools) ← Freetype, Qt5::Gui
   └─ rwviewer ← rwengine, Qt5::OpenGL/Widgets
```

设计要点：

1. **依赖来源双轨制**。`USE_CONAN` 为真时走 `conan_basic_setup(TARGETS NO_OUTPUT_DIRS)`；否则走 `find_package`。两条路径通过 `cmake/modules/WrapTargets.cmake` 归一化为同一套 namespaced target（`rwdep_wrap_conan_targets()` 把 `CONAN_PKG::bullet3` 包装成 `bullet::bullet`），因此业务 `CMakeLists.txt` 完全不感知依赖来源。注意 `conanfile.py` 中 `stdcpp = 14` 与实际 `CXX_STANDARD 17` 已不同步（陈旧声明，构建时由 CMake 覆盖）。
2. **统一选项注入函数** `openrw_target_apply_options(TARGET x CORE COVERAGE COVERAGE_EXCEPT ...)`（`cmake_configure.cmake:182-245`）：一次性完成 C++ 标准设定、IWYU 挂接、clang-tidy 挂接、覆盖率注册、install/PDB 安装。所有目标必须经此函数，这是"约定即架构"的做法。
3. **平台探测即编译期硬失败**：`cmake_configure.cmake:82-96` 对 Linux/Darwin/FreeBSD/NetBSD/OpenBSD/Windows 逐一 `RW_LINUX`/`RW_OSX`/… 定义，`else()` 直接 `FATAL_ERROR`，无 CMake 平台抽象层。
4. **Git SHA 内嵌**：`include(GetGitRevisionDescription)` + `configure_file(rwgame/GitSHA1.cpp.in ...)`（`rwgame/CMakeLists.txt:1`）。
5. **CTest 驱动 CI**：所有平台 job 都只跑一句 `ctest -VV -S cmake/ctest/script_ci.ctest`，由 `configure_{linux,windows,darwin}.ctest` 决定生成器/选项，`build.ctest` 复用，实现"CI 逻辑放 CMake 而非 YAML"。
6. **无代码生成的 CMake 集成**：`scripts/generate_scripts_functions.py` 与 `rwtools/rwfont` 都是**人工离线执行**，构建期不跑（Python 2 + lxml 脚本，已在 `generate_scripts_functions.py:1` 标注 `#!/usr/bin/env python2`）。因此 `GTA3Module.cpp`/`.inl` 与 `FontMapGta3.cpp` 是"提交进仓库的生成物"。
7. `RW_FAILED_CHECK_ACTION`（IGNORE/ABORT/BREAKPOINT）与 `ENABLE_SCRIPT_DEBUG`、`ENABLE_PROFILING`、`TEST_DATA`、`SEPARATE_TEST_SUITES` 等构成完整的调试开关矩阵（`cmake_options.cmake`）。

### 逆向机制核心：RenderWare 类布局镜像（带代码证据）

OpenRW 的镜像发生在**三个不同层面**，必须区分开：

**层面 A —— RenderWare 二进制流（BS）chunk 树：这是主战场。**
`rwcore/loaders/RWBinaryStream.hpp:89-114` 用 `namespace RW` 复刻了 chunk type ID 常量表：

```cpp
namespace RW {
enum {
    SID_Struct = 0x0001,  SID_String = 0x0002,  SID_Extension = 0x0003,
    SID_Texture = 0x0006, SID_Material = 0x0007, SID_MaterialList = 0x0008,
    SID_FrameList = 0x000E, SID_Geometry = 0x000F, SID_Clump = 0x0010,
    SID_Atomic = 0x0014, SID_TextureNative = 0x0015, SID_TextureDictionary = 0x0016,
    SID_GeometryList = 0x001A,
    SID_HAnimPLG = 0x011E,
    SID_BinMeshPLG = 0x50E,
    SID_NodeName = 0x0253F2FE
};
```

紧随其后的注释直接给出来源（`RWBinaryStream.hpp:82-87`）：

```
 * @file rwbinarystream.h
 * Deprecated RenderWare binary stream interface.
 * ...
 * Many thanks to
 * http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file
```

这是全仓库**唯一一处**明确点名的逆向来源（详见"逆向来源标记"节）。`RWBStream`（`RWBinaryStream.hpp:22-79`）实现 `{id,size,version}` 三元组游标与 `getInnerStream()` 递归下降；`RW::BinaryStreamSection`（同文件 `289-356`）则是另一套基于 `reinterpret_cast` 的 section 读取器。**两者并存且分工不同**（DFF 用前者、TXD 用后者，见 `LoaderDFF.cpp:460-484` 与 `LoaderTXD.cpp:42,179`），属可识别的技术债。

**层面 B —— 偏移保真的 POD 结构（仅用于文件解析，不用于内存对象）。**
`RW::BSGeometry`、`RW::BSGeometryTriangle`、`RW::BSTextureNative`、`RW::BSMaterial` 等（`RWBinaryStream.hpp:144-279`）是逐字段对齐磁盘布局的 POD；`RW::BSGeometryTriangle` 的四个 `uint16_t`（`first/second/attrib/third`）尤其说明问题，作者还留下情绪化注释：`uint16_t attrib;  // Who designed this nonsense.`（同文件 `180`）。DFF 侧最典型的是 `rwcore/loaders/LoaderDFF.cpp:38-51`：

```cpp
// These structs are used to interpret raw bytes from the stream.
/// @todo worry about endianness.
struct RWBSFrame {
    BSTMatrix rotation;   // glm::mat3
    BSTVector3 position;  // glm::vec3
    int32_t index;
    uint32_t matrixflags;  // Not used
};
```

此处有一条**隐蔽但关键**的构建期依赖：全项目**没有**任何 `#pragma pack` / `__attribute__((packed))` / `alignas`（已用 grep 确认为零命中），`RWBSFrame` 之所以恰好 68 字节、可被 `headerPtr += sizeof(RWBSFrame)` 直接推进，全靠 `cmake_configure.cmake:75` 的 `GLM_FORCE_PURE` 全局宏强制 GLM 使用纯 C++ 实现（`mat3` 存为 3 个 `vec3`，无 SIMD 补齐）。**「推断」：这是一个未写成文档、一旦移除即静默破坏所有 DFF 解析的隐式不变量**，比 `#pragma pack` 更脆弱。

**层面 C —— 运行时对象模型是干净重写，不是内存镜像。**
`rwcore/data/Clump.hpp` 中的 `Clump`/`Atomic`/`Geometry`/`ModelFrame` 使用 `std::shared_ptr`（`rwcore/rw/forward.hpp:62-66` 定义 `ClumpPtr` 等别名）、`std::vector`、`std::string`、`GLuint EBO`，与 RenderWare 的 `RpClump`/`RpAtomic`/`RpGeometry` + `RwLink` + memNode 池毫无对应关系。**任务书要求查找的 `rpBaseClass`、`rpIntObj`、`GetT<T>()` downcast、`Rp*` C API shim、plugin registration、`RW::` 核心 chunk 之外的 vtable 镜像，在 OpenRW 中一律不存在**：
- `grep -rn "rpBaseClass\|rpIntObj\|GetT\b"` → 0 命中；
- `grep -rni "plugin"` → 0 命中（仅 `SID_HAnimPLG`/`SID_BinMeshPLG` 两个 chunk ID 名字里残留 "PLG"，且 HAnim plugin 未被解析）；
- 唯一的 `static_assert` 只有 1 处，见下节；
- 无自定义分配器：`grep "operator new\|allocator\|RwMalloc\|MemoryPool"` → 0 命中，全部依赖 `std::unique_ptr`/`std::shared_ptr`/`std::vector`。

**唯一真正的 sizeof 断言在存档层**：`rwengine/src/engine/SaveGame.cpp:549-551`

```cpp
    static_assert(sizeof(BasicState) == 0xBC,
                  "BasicState is not the right size");
    READ_VALUE(state.basic)
```

`BasicState` 定义在 `rwengine/src/engine/GameState.hpp:41-76`，是**全项目最纯粹的偏移镜像**：成员顺序、`_align0.._align5` 填充、以及 `uint8_t dateTime[24]{0};  // Unused` 全部为对齐 0xBC 而存在。同文件 `PlayerInfo`（78-95）带 `uint8_t unknown5[70]{0}`，`SaveGame.cpp:39-116` 的 `Block0RunningScript`/`Block0ScriptData`/`StructPed` 更是把 `unknown[0x100]`、`unknown1[640]`、`unknown3[348]` 这类"逆向未归类区段"原样写进类型系统：

```cpp
struct StructPed {
    uint8_t unknown0_[52];
    glm::vec3 position{};
    uint8_t unknown1[640];
    float health;
    float armour;
    uint8_t unknown2[148];
    std::array<StructWeaponSlot, kNrOfWeapons> weapons;
    uint8_t unknown3[348];
};
```

读档侧配 `CHECK_SIG(expected)` 与 `BLOCK_HEADER(sizevar)` 宏做游标跳转（`SaveGame.cpp:519-534`），共 22 个块（`grep -c BLOCK_HEADER` = 22）。

**结论**：OpenRW 的逆向机制是**"格式级镜像 + 逻辑级清洁重写"**，而非 re3 式的"内存布局/函数地址镜像"。它的可验证性来自"能否正确解析官方数据文件并跑通官方任务脚本"，而非"某函数是否在原 exe 的某地址"。

### game 层与 engine 层分工

| 层 | 目录 | 抽象层级 | 与 GTA 的关系 |
|---|---|---|---|
| RW/图形基元 | `rwcore/` | 最底层，无游戏语义 | 只认识 DFF/TXD/IMG/SDT 字节；唯一含 GTA 特例的是 `fonts/FontMapGta3.cpp` 与 `types.hpp` 里的 `WATER_*`/`WORLD_GRID_*` 常量 |
| 引擎+玩法 | `rwengine/` | 领域对象与系统 | 认识 `VehicleObject`、`AIGraph`、`SCMThread`，但**不含窗口/GL 上下文** |
| 前端/外壳 | `rwgame/` | 平台与状态机 | 唯一链接 SDL2/imgui 的层；持有 `main()` |

依赖严格单向：`rwgame → rwengine → rwcore`。`rwengine/CMakeLists.txt:156-163` 明确不链 SDL，`rwgame/CMakeLists.txt:66-72` 才引入 `SDL2::SDL2` 与 `Boost::program_options`。

入口：`rwgame/main.cpp:12-33` —— 先建 `Logger`/`StdOutReceiver`，再 `RWArgumentParser::parseArguments`（`SDL_MAIN_HANDLED` + `SDL_SetMainReady()` 避开 `SDLmain`），然后 `RWGame game(logger, argLayerOpt); game.run();`。`main` 只 catch `std::runtime_error` 并用 `SDL_ShowSimpleMessageBox` 呈现（`main.cpp:34-52`），注释解释了为何不吞 `bad_alloc`。

主循环 `RWGame::run()`（`rwgame/RWGame.cpp:415-457`）：`while (stateManager.currentState() && running)` → `tickWorld(deltaTime, accumulatedTime)`（固定步长累加器，`RWGame.cpp:459-478`）→ `SDL_PollEvent` 分发（489）→ `stateManager.draw`。`RWGame::tick`（529-592）依次推进 chase、游戏内时钟（`state.basic.gameMinute/gameHour`）、脚本计时器 `timerClockRate`、`tickObjects(dt)`（遍历 `world->allObjects` 调 `object->tick(dt)`）、`vm->execute(dt)`。

状态机：`State`（`rwgame/State.hpp:20-67`）纯虚 `enter/exit/tick/draw/handleEvent/getCamera/shouldWorldUpdate`，`StateManager` 管理栈；6 个实现见 `rwgame/states/`。

### 关键子系统逐个拆解

**渲染管线**（`rwengine/src/render/`，3,837 行）。抽象接口 `Renderer` 定义在 `OpenGLRenderer.hpp:73`（无独立 `Renderer.hpp`），内含 `DrawParameters`（`86-110`：count/start/`Textures`/`BlendMode`/`DepthMode`/colour/ambient/diffuse/visibility）与 `RenderInstruction`（118+）。三段式：
1. `ObjectRenderer`（`ObjectRenderer.cpp`）把 `GameObject` 展开为 `RenderList`，用 `RenderKey createKey(float normalizedDepth, Textures&)` 做 3-pass 排序键打包：`(uint32_t(0x7FFFFF * normalizedDepth) << 8 | uint8_t(0xFF & textures[0]))`（`ObjectRenderer.cpp:38-41`）—— 深度占高位、纹理 ID 占低位，实现"按深度排序 + 同纹理合并"。LOD 距离常量 `kMagicLODDistance = 330.f`、`kVehicleLODDistance = 70.f`、`kVehicleDrawDistance = 280.f`（34-36）。
2. `GameRenderer`（`GameRenderer.hpp:29-`）持有 `std::unique_ptr<Renderer> renderer = std::make_unique<OpenGLRenderer>()`，编排 world/sky/water/particle/post 五套 shader（`worldProg`、`skyProg`、`particleProg`、`ssRectProg`、`postProg`）。
3. `OpenGLRenderer` 实现 `pushDebugGroup/popDebugGroup` + `ProfileInfo` 计数（draws/primitives/buffers/textures/uploads，`OpenGLRenderer.hpp:226-376`）。

Shader 全部以 R"()" 字面量内联于 `GameShaders.hpp`（`namespace GameShaders`，GLSL `#version 330`，`layout(std140) uniform SceneData` 含 projection/view/ambient/dynamic/fogColor/campos/fogStart/fogEnd），**运行时不读磁盘 shader 文件**。GL 顶点布局通过"结构体自带 `static const AttributeList vertex_attributes()`"这一约定实现零反射：`GeometryVertex`（`Clump.hpp:114-137`）声明 position/normal/texcoord/colour 的 stride+offset，并在注释里写出目标偏移 `/* 0 */ /* 24 */ /* 48 */ /* 64 */`；`VertexP2`/`VertexP3`（`OpenGLRenderer.hpp:28-57`）同理。`GeometryBuffer::uploadVertices` 靠 `attributes = T::vertex_attributes();` 提取（`GeometryBuffer.hpp:68-73`）。

剔除：`ViewFrustum`（`ViewFrustum.hpp:11-`）为 6 个 `ViewPlane`；`rwcore/rw/types.hpp:26-29` 的 `WORLD_GRID_SIZE 4000 / WORLD_CELL_SIZE 100 / WORLD_GRID_CELLS` 同时服务于 AI 图与剔除网格。水面 `WaterRenderer` 用数据纹理（`setDataTexture(GLuint fbBinding, GLuint dataTexture)`）实现投影网格 HQ 水面，参数来自 `types.hpp:12-20`。

**碰撞/物理**（`rwengine/src/dynamics/`，仅 347 行 + 使用点）。`CollisionInstance::createPhysicsBody(GameObject*, CollisionModel*, DynamicObjectData*, VehicleHandlingInfo*)` 把 `CollisionModel`（`data/CollisionModel.hpp:14-51`：`Sphere/Box/Triangle` + 每面 `Surface{material,flag,brightness,light}`）转成 `btCompoundShape` + `btTriangleIndexVertexArray`，并挂 `GameObjectMotionState : btMotionState` 回写 `GameObject::position/rotation`。`HitTest`（`HitTest.hpp:17-35`）只暴露 `sphereTest` / `boxTest`。车辆用 Bullet 内置 `btRaycastVehicle` + `btVehicleRaycaster`（`VehicleObject.hpp:66-67`），参数来自 `data/handling.cfg`。角色用 `btKinematicCharacterController` + `btPairCachingGhostObject` + `btCapsuleShapeZ`（`CharacterObject.hpp:20-22`）。浮力是自写的解析式，直接用 `types.hpp` 常量：`WATER_BUOYANCY_K * x + -WATER_BUOYANCY_C * ...`（`VehicleObject.cpp:810-811`、`InstanceObject.cpp:131-132`）。世界侧 `GameWorld::dynamicsWorld`（`GameWorld.hpp:329`）+ 静态回调 `PhysicsTickCallback(btDynamicsWorld*, ...)`（343），`GameWorld.cpp:711-766` 是带 microprofile 探针的 `physicsTick`。

**AI 与路径**（`rwengine/src/ai/`，2,298 行）。`ai::AIGraphNode`（`AIGraphNode.hpp:13-33`）字段 `type/position/size/leftLanes/rightLanes/external/flags/nextIndex/disabled/connections` 是 IPL PATH 段的直接映射；`enum { None = 0, CrossesRoad = 1  /// No documentation for other flags yet, but this is mentioned. }` 是一条诚实的逆向缺口记录。`AIGraph::createPathNodes(...)` / `gatherExternalNodesNear(center, radius, nodes, type)`；`TrafficDirector`（`TrafficDirector.hpp:17-39`）做 `setDensity(NodeType, float)` / `setPopulationLimits(maxPeds, maxCars)` / `populateNearby`；生成半径常量 `kMaxTrafficSpawnRadius = 100.f`、`kMaxTrafficCleanupRadius = *1.25f`（`GameWorld.cpp:54-55`），并用视锥避开"玩家眼前刷车"。寻路最近节点的距离函数是逆向结论的实现（`ScriptFunctions.hpp:263-269`）："This is how the original game calculates distance, weighted manhattan-distance where the vertical distance has to be 3x as close"，即 `|dx| + |dy| + |dz|*3`。

**脚本 VM / opcode**（`rwengine/src/script/`，1,729 + 13,861 行）。**不是 CLEO，也不是 Lua**；是原版 SCM 字节码 VM。
- 线程模型 `SCMThread`（`ScriptMachine.hpp:79-106`）：`char name[17]`、`baseAddress`/`programCounter`、`conditionCount/Result/Mask/AND`、`wakeCounter`、`locals`（`SCM_THREAD_LOCAL_SIZE 256` × `SCM_VARIABLE_SIZE 4`，注释直言 "Changing this will break saves"）、`std::array<pc_t, SCM_STACK_DEPTH 4> calls`。
- 操作数类型直接抄磁盘编码（`ScriptTypes.hpp:178-186`）：`EndOfArgList=0x00, TInt32=0x01, TGlobal=0x02, TLocal=0x03, TInt8=0x04, TInt16=0x05, TFloat16=0x06, TString=0x09`。
- 取指循环 `ScriptMachine::executeThread`（`ScriptMachine.cpp:15-186`）：`auto opcode = file.read<SCMOpcode>(pc)`；条件取反位 `#define SCM_NEGATE_CONDITIONAL_MASK 0x8000`（`ScriptMachine.hpp:20`）；`TFloat16` 解码为 `file.read<std::int16_t>(pc) / 16.f`；`TString` 固定 8 字节。未注册 opcode 抛 `IllegalInstruction(opcode, pc, t.name)`。线程计时器直接写死在局部槽 16/17（`ScriptMachine.cpp:179-183`）：`p.globalPtr = (t.locals.data() + 16 * sizeof(SCMByte) * 4); *p.globalInteger += msPassed;`。硬编码 `opcode != 0x00D6`（"if and"）以维护条件链，并带 `@todo add conditional flag to opcodes`。
- **绑定层是模板元编程的亮点**：`script_bind::arg_traits<T>::arg_size`（`ScriptModule.hpp:12-35`）声明 `ScriptVec2/Vec3/RGB/RGBA` 在字节流里占 2/3/3/4 个槽；递归 `call_unpacked<depth, script_arg, Tret, Targs...>`（72-96）在编译期展开参数游标；`binder<bool, Targs...>` 特化（111-119）自动把返回值写入 `args.getThread()->conditionResult` —— 于是"条件型 opcode"与"动作型 opcode"共用一个 `bind(id, argc, fn)`（`ScriptModule.hpp:163-171`）。
- **生成器**：`scripts/generate_scripts_functions.py`（Python 2 + lxml）读 `SCM.ini`（人类可读签名）+ `SCM.xml`（参数类型），按 `function_template`（29-38 行）与 `binding_template`（40 行）产出 `binding.cpp` + `functions.cpp`，`type_names` 字典（42+）把 `ENTITY/CAR → ScriptVehicle`、`TYPE/FLOAT → ScriptFloat` 映射过去。产物落进仓库：`GTA3ModuleImpl.inl` 每个函数头都是模板展开的注释块（如 `@brief wait %1d% ms` / `opcode 0001` / `@arg time Time (ms)`），`GTA3Module.cpp:19-` 则是 904 行 `bind(0x0000, 0, opcode_0000);`。`.inl` 被 `#include` 进 `.cpp`（`GTA3Module.cpp:14`），从而共享同一 TU 的符号可见性 —— 这是刻意的"生成物内联"手法而非疏忽。
- 桩体只有一行 `RW_UNIMPLEMENTED_OPCODE(0x0003);`（`.inl:64`），而 `RW_UNIMPLEMENTED_OPCODE` 在 `GTA3Module.cpp:7-12` 里被定义为**仅 debug 构建**下打印消息的空宏 —— 生产构建中未实现 opcode 完全静默 no-op。

**音频**（`rwengine/src/audio/`，1,782 行）。`SoundManager` 构造时打开 `audio/sfx.SDT` + `audio/sfx.RAW`（`SoundManager.cpp:44-51`），`LoaderSDT` 解析 SDT 索引并在内存里即时拼 WAV 头（`LoaderSDT.cpp:63-80`：`header->chunkSize = sizeof(WaveHeader) - 8 + assetInfo.size;`）。解码统一交给 FFmpeg（`SoundSource.cpp:7-9,34,85-101,119,157`，含 `avformat_open_input(&formatContext, "SDT", ...)` 这种自定义 IO 名的技巧），送 OpenAL buffer；`SoundBuffer` 与 `SoundBufferStreamed`（按 `kSizeOfChunk` 分块 `alBufferData`）二分。`SfxParameters`（`SfxParameters.cpp`）是 `audio/sfx.dat` 的每音效参数。**未见游戏电台/流式音乐轨（UAUDIO 循环）完整实现**（「推断」，基于 `SoundManager.cpp:36` 的 `// @todo reloading, how to check is it wav/mp3?`）。

**存档**（`SaveGame.cpp/.hpp`，1,420 行）。读档 22 块齐全，**写档整体未实现**（`SaveGame.cpp:499-503`）：

```cpp
void SaveGame::writeGame(GameState& state, const std::string& file) {
    RW_UNUSED(state); RW_UNUSED(file);
    RW_UNIMPLEMENTED("Saving the game is not implemented yet.");
}
```

`getAllSaveGameInfo()` 直接扫原游戏的 `GTA3 User Files` 目录取 `*.b`；Windows 下用注册表 `Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders` 的 `Personal` 值定位"我的文档"（`SaveGame.cpp:1325-1345`），POSIX 下退回 `$HOME`。

**文本 / 字体**（`rwcore/fonts/`）。原版把 HUD 文本存成"16 位字体图集索引"而非 ASCII，OpenRW 忠实保留：`using GameStringChar = std::uint16_t; using GameString = std::basic_string<GameStringChar>;`（`GameTexts.hpp:13-19`），三套字体 `FONT_PAGER 0 / FONT_PRICEDOWN 1 / FONT_ARIAL 2`。`LoaderGXT::load` 跳 4 字节 `TKEY`、按 `blocksize/12` 迭代 `{offset,id[8]}` 表项（`LoaderGXT.cpp:11-31`）。`FontMap` + `Unicode` 负责 GSChar↔U+XXXX，`Unicode.hpp:300` 甚至含 `UNICODE_SHIELD = 0x1f6e1`。字体图集不是运行时渲染的，而是 `rwtools/rwfont/rwfontmap.cpp` 用 FreeType + `QImageWriter` 离线烘成贴图（`FT_Init_FreeType`，`m_glyph_width = width / 16`）。

**资产装载 / "流式"**（`rwengine/src/engine/GameData.cpp`、`rwcore/platform/FileIndex.cpp`）。`GameData::load()`（44-87）一次性顺序加载 `models/gta3.img`、`anim/cuts.img`、5 个 TXD 槽（particle/icons/hud/fonts/generic+misc）、`carcols.dat`、`timecyc.dat`、`handling.cfg`、`waterpro.dat`、`weapon.dat`、`pedstats.dat`、`ped.dat`、`ped.ifp`，再按 `data/default.dat` 与 `data/gta3.dat` 逐行解释 `IDE/SPLASH/COLFILE/IPL/TEXDICTION/MODELFILE`（`loadLevelFile`，110-190）。`FileIndex` 建立"小写相对路径 → 磁盘路径 or IMG 内部条目"的统一映射（`FileIndex.hpp:66-101`，`enum IndexedDataType { FILE, ARCHIVE }`），从而同时满足大小写敏感文件系统。
**没有 RenderWare 风格的流式引擎（CStreaming）**：`ModelData.hpp:84-88` 写着 `/// @todo replace with proper streaming implementation` 的 `virtual bool isLoaded()`/`unload()`，而 `GameWorld.cpp:141` 在 `createInstance` 里同步补载：`/// @todo implment streaming properly`。`LoadingState::tick` 只是 `done(); complete();`（`LoadingState.cpp:15-20`），即"假异步、同步阻塞"。

**内存分配**。无 RW 的 `RwMalloc`/memNode 池等价物，全部标准容器 + 智能指针；`GameWorld` 用 5 个 `ObjectPool`（`GameWorld.hpp:282-288`：`allObjects` + `pedestrianPool/instancePool/vehiclePool/pickupPool/cutscenePool`）与 `GameObject::Lifetime` 分层（配合 `destroyObjectQueued`/`destroyQueuedObjects` 的延迟销毁，避免迭代中删除）——**这是唯一近似"池化"的结构，但不涉及内存复用**。

**平台抽象**。`GameWindow`（`rwgame/GameWindow.hpp:12-38`）极薄：`SDL_Window*` + `SDL_GLContext` + `swap()`。输入 `updateGameInputState(GameInputState*, const SDL_Event&)`（`GameInput.hpp:9`）。宽窄字符只在 Windows 路径存在：`rwcore/platform/RWWindows.{hpp,cpp}` 与 `rwengine/src/rw_mingw.hpp`（三行 `#undef IGNORE/near/far`）。配置 `RWConfig.inc` 用 **X-macro** 单点声明同时生成结构体成员、`boost::program_options` 选项、INI 键映射与 help 文本（`RWConfig.hpp:16-80` 连续 4 次 `#include "RWConfig.inc"` 配不同宏定义）。

### 三作共用代码与 profile 机制

**不存在 profile 机制**。证据链：
1. `grep -rni "profile"` 在 `rwengine/rwgame` 的命中全部是性能剖析（`Renderer::ProfileInfo`、`RW_PROFILE_*` 宏、microprofile），无一处指游戏配置档；
2. 只存在一个 opcode 模块 `GTA3Module`（`rwengine/src/script/modules/`），`RWGame.hpp:44` 直接 `GTA3Module opcodes;` 作为成员，构造 `ScriptMachine(file, &opcodes)` —— 无虚接口、无注册表、无按游戏分派；
3. `RWGame.cpp`/`RWGame.hpp` 里没有 `class GTA3Scene`/`ViceCityScene`/`SanAndreasScene` 之类；`rwgame/CMakeLists.txt` 也不产多目标；
4. `GameData::getPlatformString()` 硬编码 `return "PC";`（`GameData.hpp:66-68`），无 mobile/GOTY/10th-anniversary 变体分支；
5. 三作差异只以**注释与枚举残根**存在：`SCMFile.hpp:18` 的 `enum SCMTarget { NoTarget = 0, GTAIII = 0xC6, GTAVC = 0x6D, GTASA = 0x73 }` 是 SCM 头里的游戏目标字节，`SCMFile.cpp:14` 会把它读进 `_target`，**但 `_target` 从未被读取**（grep 确认仅两处赋值/声明）；`LoaderIMG.hpp:27-32` 的 `enum Version { GTAIIIVC, GTASA, GTAIV }` 明确注为 "GTA III and GTA VC archives -- only this one is implemented"；`GameData.cpp:234-235` 认得 `CAR3, ///< Used in GTASA, ... CAR4`；`RWGame.cpp:253-377` 的作弊码注释统一写 `// III / VC: ...`。

因此"三作共用"的正确表述是：**`rwcore` + `rwengine` 的格式解析与通用玩法层是可复用的，但缺少把这些零件按游戏装配起来的抽象层**。真正的多游戏抽象需要（a）`ScriptModule` 派生出 `VCModule`/`SAModule`、（b）`GameData` 文件清单参数化、（c）`SCMFile::_target` 参与分派、（d）`getPlatformString()` 真实化。这四点在 HEAD 均缺失 —— 与上游公开路线图一致。

### 测试与工程质量

- 框架：**Boost.Test**（非 gtest/catch）。`tests/main.cpp:1-2` 是 `#define BOOST_TEST_MODULE openrw` + `unit_test.hpp`。41 个 suite、141 个 `BOOST_*_TEST_CASE`。
- fixture：`tests/test_Globals.hpp:99-141` 定义单例 `Global`，构造时 `SDL_Init(SDL_INIT_VIDEO)` → 建 800×600 窗口 → `GameData(getGamePath())->load()` → `GameWorld` → `GameState` → 把重力设为 0。**因此"单元测试"实际要求 SDL2 视频 + OpenAL + 完整配置**，CI 用 `ALSOFT_DRIVERS: "null"` + `dbus-run-session` + 后台 `pipewire` 来喂平头环境（`.github/workflows/build-and-test.yml:11-14,38`）。
- 数据驱动测试：`DATA_TEST_PREDICATE`（`test_Globals.hpp:32-33`）= `label("data-test") * disabled()`，共 26 处（分布在 22 个文件）。**只有显式 `-DTEST_DATA=ON` 时才通过 `--run_test=@data-test` 跑**（`tests/CMakeLists.txt:98-102`），而 `getGamePath()` 从用户 `openrw.ini` 里取 `game.path`（`test_Globals.cpp:5-11`），CI 上没有正版数据 → **绝大多数格式解析正确性（DFF/TXD/GXT/SDT/IDE 实际字节）在 CI 里不被覆盖**，CI 覆盖的是合成对象逻辑。
- `SEPARATE_TEST_SUITES` 可把每个 suite 注册成独立 CTest（`tests/CMakeLists.txt:77-96`，`TIMEOUT 300`）；CI 里设 `SEPARATE_TEST_SUITES FALSE`。
- 静态分析：IWYU（含 2.7 KB 定制映射 `openrw_iwyu.imp`，为 GLM/Boost/Bullet/libav 的传递包含归一化）与 clang-tidy（`CHECK_CLANGTIDY`/`CHECK_CLANGTIDY_FIX`，`FORMAT_STYLE "file"`）均可选挂载；UB/ASan/LSan/TSan 通过 `rw_checks` 目标按需注入。
- 风格：`.clang-format` = Google 基线 + 4 空格 + `ColumnLimit 80` + `BreakConstructorInitializersBeforeComma`；`.editorconfig` 无；`scripts/verify-commit` 提交信息检查。文档 Doxygen（`Doxyfile`，`INPUT=.`、`EXCLUDE=tests`）。
- 警告强度很高：`-Wall -Wextra -Wdouble-promotion -Wpedantic -Wmissing-braces -Wold-style-cast`（`cmake_configure.cmake:8-16`）。**「推断」`-Wold-style-cast` 与解析器里大量 `reinterpret_cast`/`static_cast` 并存，靠 `RW_CHECK` 与严格审查而非编译期保证布局安全。**
- 覆盖率：`TEST_COVERAGE` + `cmake/modules/CodeCoverage.cmake` + llvm-cov wrapper，直传 codecov（`.codecov.yml` 关掉 `require_ci_to_pass`），生成物 `gl_core_3_3.*` 被 `COVERAGE_EXCEPT` 排除。
- 脚手架 vs 逻辑比例：`rwengine/src/data`（804 cpp : 1,341 hdr）、`script`（613 : 1,116）、`engine`（4,612 : 1,769）、`objects`（2,962 : 1,379）—— 数据/类型定义层头文件偏重（正常），而真正的"空壳布局镜像"极少；换言之 OpenRW 头文件多为**接口与 POD 数据定义**，不像 re3 那样是"为了对齐偏移而写的死结构体墙"。

### 成熟度与缺口表

| 子系统 | 状态 | 证据 / 缺口 |
|---|---|---|
| DFF/Clump 解析 | **可用**，含 BinMesh PLG、MatList、NodeName、法线重建 | `LoaderDFF.cpp:386-419,452-510`；缺 HAnim 插件（`SID_HAnimPLG` 定义但 0 引用）、缺 morph target、未处理端序（`LoaderDFF.cpp:39` `@todo worry about endianness.`） |
| TXD/DXT/调色板纹理 | **可用** | `LoaderTXD.cpp:16-96`（含 4 张纯色 debug 纹理兜底） |
| IMG 归档 | **部分**：仅 III/VC 版 | `LoaderIMG.hpp:27-32`；线程不安全（`LoaderIMG.hpp:47-49`） |
| IDE / IPL / COL / GXT / IFP / waterpro | **可用**（GTA III 方言） | `rwengine/src/loaders/*`；`CollisionModel.hpp:22` `@todo give shapes surface data` |
| 渲染 | **可用**：GL3.3、雾、alpha/additive 双队列、水面 HQ/LQ、minimap、文字、letterbox、splash | `GameRenderer.cpp`、`MapRenderer.cpp`、`WaterRenderer.*`；无阴影、无法线映射、材质属性硬编码为默认（`OpenGLRenderer.hpp:108` `// Default state -- should be moved to materials`） |
| 物理 | **可用**：车辆 `btRaycastVehicle`、角色胶囊、自写浮力、crane | `VehicleObject.cpp:810`、`CharacterObject.cpp`；碰撞面 `material/flag` 未反馈伤害差异 |
| 脚本 VM | **结构完整**，opcode **约 57% 实现**：904 个 handler 中 389 个含 `RW_UNIMPLEMENTED_OPCODE` | `grep -c` 于 `GTA3Module.cpp`(bind=904) 与 `.inl`(389)；Release 下桩体**静默无输出**（`GTA3Module.cpp:7-12`） |
| AI / 交通 | **基础可用**：图寻路、`TrafficDirector` 密度/上限、车道 `leftLanes/rightLanes` | 缺警车战术、缺 ped 反应/群组行为细化 |
| 音频 | **可用**：SFX(SDT/RAW+FFmpeg)、流式 buffer | 电台/音乐轨不完整（`SoundManager.cpp:36` `@todo reloading`） |
| 存档 | **只读不写** | `SaveGame.cpp:499-503` `RW_UNIMPLEMENTED("Saving the game is not implemented yet.")`；`SaveGame.cpp:1262-1263` `// TODO restore garage data`；`:1289` `@todo determine where the remainingSpawns should be` |
| 流式加载 | **未实现** | `ModelData.hpp:84`、`GameWorld.cpp:141`、`GameData.hpp:42` 三处 `@todo`；`LoadingState.cpp:15-20` 假异步 |
| 多游戏（VC/SA） | **未实现** | 见"三作共用"节 |
| 移动端资产变体 | **未实现** | `GameData.hpp:66-68` `return "PC";` |
| RenderWare 内存模型镜像 | **主动不做**（设计选择） | 0 处 `rpBaseClass`/plugin/自定义分配器；仅 1 处 `static_assert` |
| 整体 TODO 密度 | 245 处 `@todo|TODO|FIXME|XXX`；`RW_UNIMPLEMENTED` 家族共 404 处（其中 389 在 opcode 表） | grep 统计 |

结论：以"能跑通 GTA III 主线"衡量，静态资产管线与玩法核心已达**可玩级**（README 有开场动画/驾驶/城景截图佐证），最大缺口是**写档、流式加载、约 43% 的 opcode**，其次是 VC/SA 抽象层完全缺席。

### 资产依赖

OpenRW 必须外挂正版 GTA III PC 数据目录，运行期零内嵌资产（`assets/` 目录只有 `logo_box.svg` 一个构建产物图标）。

- 入口：`--gamedata PATH`（`RWConfig.inc` 的 `RWCONFIGARG(std::string, gamedataPath, ..., "game.path", CONFIG, "gamedata", "PATH", "Path of gamedata")`）→ `GameData(Logger*, const std::filesystem::path&)`；配置文件本身按 XDG/macOS `~/Library/Preferences`/Windows `%APPDATA%` 定位（`RWConfig.cpp:177-208`）。
- 目录合法性只检查"是目录且无 error_code"（`GameData.cpp:768-775`），**不做版本/校验和判定**，也不检测 Steam/GOG/Rockstar 安装器路径 —— 装错版本只在后续 `logger->error("Data", "Failed to open game file " + path)` 暴露（`GameData.cpp:354-356`）。
- 具体读取清单见"关键子系统 → 资产装载"节。`FileIndex::indexTree` 先递归建盘上索引，之后 `indexArchive` 把 IMG 内成员并入同一命名空间（`FileIndex.hpp:28-60`），使 `index.openFile("landstal.dff")` 与 `findFilePath("audio/sfx.SDT")` 同构 —— 这是它对"大小写敏感文件系统 + 归档内文件名"两个问题的一次性解法（`normalizeFilePath`）。
- 存档目录复用原游戏的 `GTA3 User Files/*.b`；Windows 经注册表解析"我的文档"（`SaveGame.cpp:1325-1364`）。
- 因 `getPlatformString()` 恒为 `"PC"`，移动版（`.opcodes`/`.aif`/DDS 纹理/`mobile` 布局）**不受支持**。

### 逆向来源标记

OpenRW 不用地址/符号做标注，而用**文献引用 + 常量自证 + 未归类区段命名**三种方式：

| 类型 | 位置 | 内容 |
|---|---|---|
| 显式致谢 wiki | `rwcore/loaders/RWBinaryStream.hpp:85-86` | `Many thanks to http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file` |
| 显式 wiki  opcode 页 | `rwengine/src/render/MapRenderer.cpp:199-200` | `// Colours from http://www.gtamodding.com/wiki/0165 (colors not specific to that opcode!)` |
| 显式 wiki 引用 | `rwengine/src/engine/GameWorld.cpp:988` | `// According to http://www.gtamodding.com/wiki/014C the spawn limit doesn't work.`（随后 `#if 0` 掉原逻辑） |
| 显式论坛文档 | `rwengine/src/engine/SaveGame.cpp:1263` | `// http://gtaforums.com/topic/758692-gta-iii-save-file-documentation/` |
| 行为复刻声明 | `rwengine/src/script/ScriptFunctions.hpp:263-265` | `// This is how the original game calculates distance, weighted manhattan-distance where the vertical distance has to be 3x as close` |
| 数值取自原版 | `rwengine/src/engine/Garage.cpp:120-121`、`Garage.hpp:17,56-57`、`PlayerController.cpp:182`（`// Original game uses 4000.f radius`）、`RWGame.cpp:562`（`// Original game uses milliseconds`） | `// back to non swing just like in original game / Values are from original game`、`// Garage types are from original game` |
| 魔数（chunk ID） | `RWBinaryStream.hpp:113`、`LoaderDFF.cpp:19-35` | `SID_NodeName = 0x0253F2FE`（RW 的 Atomic/NodeName 插件 ID）、`CHUNK_NODENAME` |
| 魔数（文件签名） | `LoaderCOL.cpp:13` | `constexpr uint32_t kCollMagic = 0x4C4C4F43;`（即 `"COLL"`） |
| 魔数（版本比较） | `LoaderDFF.cpp:179` | `if (geomStream.getChunkVersion() < 0x1003FFFF) { headerPtr += sizeof(RW::BSGeometryColor); }` —— RW 3.3→3.5 的几何头差异 |
| 硬编码文件偏移 | `GameData.cpp:323-327` | `ifstr.seekg(0x03C4);` 之后 `read(&visibleWater, 64*64)`、`read(&realWater, 128*128)`，`waterHeights` 固定 48 float |
| 硬编码块大小 | `SaveGame.cpp:571` | `if (scriptDataBlockSize != 0x03C8) { ... }` |
| 尺寸断言 | `SaveGame.cpp:549` | `static_assert(sizeof(BasicState) == 0xBC, ...)` |
| IFP 四字符码 | `LoaderIFP.cpp:65-120`、`LoaderIFP.hpp:39` | `ANPK`/`NAME`/`DGAN`/`CPAN`/`ANIM`/`KFRM` + `"KR00"/"KRT0"/"KRTS"` → `enum Data { R00, RT0, RTS }` |
| 脚本编码常量 | `ScriptMachine.hpp:20-28`、`ScriptTypes.hpp:178-186`、`ScriptMachine.cpp:157,179-183` | `0x8000`/`0xFF`/`SCM_THREAD_LOCAL_SIZE 256`/`SCM_VARIABLE_SIZE 4`（"Changing this will break saves"）、`0x00D6` |
| 存档枚举目标字节 | `SCMFile.hpp:18` | `GTAIII = 0xC6, GTAVC = 0x6D, GTASA = 0x73` |
| 逆向诚实标注 | `AIGraphNode.hpp:16-17`、`SaveGame.cpp:59,81,86,92` | `/// No documentation for other flags yet, but this is mentioned.`；`unknown[0x100]` / `unknown1[640]` |

**未出现**：任何 `0x00[0-9A-F]{6}` 形态的 exe 函数地址、`// ida`、`ghidra`、`decompile`、原二进制符号名（`grep -rni "IDA Pro\|ghidra\|decompil"` → 0 命中）。这与"不注入原 exe、不要求原版二进制"的许可定位一致（GPL 洁净室边界，`CONTRIBUTING.md` 只指向 wiki）。

### 对本报告其他项目的启示

1. **两条逆向路线可直接对照**：OpenRW = *格式驱动 / 数据可验证 / 无需原始二进制*；re3 系 = *内存布局驱动 / 偏移可验证 / 需要原始 exe*。前者产出可自由分发的 GPL 引擎，后者产出必须搭配原程序的二进制补丁。`rwcore/loaders/RWBinaryStream.hpp` 与 re3 的 `Rp*` 头文件对同一 RW 概念的处理差异，是报告里最好的"方法论对照"素材。
2. **`GLM_FORCE_PURE` 的隐式布局依赖**（`cmake_configure.cmake:75` ↔ `LoaderDFF.cpp:45-51`）值得作为反面教材：布局不变量应写成 `static_assert(sizeof(RWBSFrame) == 68)`。OpenRW 全项目只有 1 个 `static_assert`，说明"格式镜像"缺一道自动化护栏，而 re3/ReClass 类项目通常大量使用断言。
3. **用生成器 + 外部规格（INI+XML）批量铺 opcode handler** 是可移植的打法：`scripts/generate_scripts_functions.py` + `GTA3ModuleImpl.inl`（12,928 行）+ `.cpp` 侧 904 条 `bind()` 的组合，让"规格更新"与"逐条填充实现"解耦；配合 C++14 模板元 `script_bind::call_unpacked` 自动展开变长参数、并按返回类型 `bool` 特化写入条件结果，是同类脚本 VM（CLEO、FreeRoam、pylama 系）里最干净的样板之一。
4. **`FileIndex` 的双源统一寻址**（磁盘 or IMG 归档，小写归一化）是任何重制项目都会撞上的第一个非趣味性问题，OpenRW 的解法（单一 `unordered_map<string, IndexedData>` + `IndexedDataType{FILE,ARCHIVE}`）简洁且值得抄。
5. **诚实的缺口标记值得制度化**：`RW_UNIMPLEMENTED` / `RW_UNIMPLEMENTED_OPCODE` 与 `unknown[NNN]` 命名法把"逆向未完成"编码进类型系统与构建期日志。但需注意其反模式：**Release 下 `RW_UNIMPLEMENTED_OPCODE` 展开为空**（`GTA3Module.cpp:7-12`），静默 no-op 会让 mod 脚本或任务在发行构建里"无声失败"，这是所有重制项目都应避免的默认值。
6. **CI 依赖数据文件时的分层测试策略**：`DATA_TEST_PREDICATE = label("data-test") * disabled()` + `TEST_DATA` 选项，让"无资产可跑的单元测试"与"需正版资产的格式测试"在同一个 Boost.Test 二进制里共存，可被任何"必须依赖版权数据"的重制项目直接借鉴（搭配 CDash 与 `ALSOFT_DRIVERS=null`/`dbus-run-session` 的无头技巧）。
