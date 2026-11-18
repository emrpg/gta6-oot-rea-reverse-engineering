## 3.7 librw（aap/librw）—— RenderWare 本体的重实现

本节分析 `/root/orca-workspaces/gta-repos/librw`（远端 `https://github.com/aap/librw`，浅克隆仅 1 个提交 `18532c2`，日期 2026-08-26，作者 `aap <aap@papnet.eu>`，`LICENSE` 为 MIT/(c) 2014 aap）。librw 的目标写在 `README.md:4-5`：「a re-implementation of RenderWare graphics, or a good part of it anyway」——它不是游戏引擎，而是 **RenderWare 3 库 API 本身**的重写，是 re3/reVC 那条线的图形依赖底座。`ARCHITECTURE.MD:3-4` 作者自陈「Some of these design decision were taken over from original RW, some are my own. I only take partial responsibility.」，这句话是评估「镜像保真度」的钥匙。

### 3.7.1 目录与模块清单

仓库无 `librw/`、`gc/`、`examples/`、`tests/` 目录；核心全在 `src/`，共 306 个受版本管理文件，`src/` 含 64 个 `.cpp` + 49 个 `.h`。

| 模块 | 行数 | 内容 |
|---|---:|---|
| `src/*.{cpp,h}`（平台无关核心） | 15,410 | `base.cpp` 1156、`geometry.cpp` 1053、`image.cpp` 1324、`clump.cpp` 669、`matfx.cpp` 641、`texture.cpp` 596、`engine.cpp` 573、`raster.cpp` 559、`camera.cpp` 524、`uvanim.cpp` 502、`skin.cpp` 486、`frame.cpp` 470、`tristrip.cpp` 682、`hanim.cpp` 414、`userdata.cpp` 364、`geoplg.cpp` 339、`anim.cpp` 293、`world.cpp` 238、`plg.cpp` 246、`light.cpp` 163、`charset.cpp` 244、`prim.cpp` 70、`render.cpp` 96、`pipeline.cpp` 198 |
| `src/gl/`（不含 glad 4,688） | 6,795 | `gl3device.cpp` 2415、`gl3raster.cpp` 1045、`wdgl.cpp` 875、`gl3skin.cpp` 366、`gl3shader.cpp` 363 |
| `src/d3d/` | 13,061 | 其中 `shaders/*.h` 为生成的 VS 汇编 4,181；手写：`d3ddevice.cpp` 2022、`d3d.cpp` 1087、`xbox.cpp` 1005、`d3d9.cpp` 853、`d3d8.cpp` 678 |
| `src/ps2/` | 7,005 | `ps2raster.cpp` 2238、`ps2.cpp` 1586、`ps2device.cpp` 1252、`ps2immed.cpp` 345、`ps2skin.cpp` 334；另有 `vu1/*.vu,*.dsm` 微码 1,654 行 |
| `skeleton/`（不含 imgui 69,408） | 2,315 | `win.cpp`/`glfw.cpp`/`sdl2.cpp`/`sdl3.cpp`/`ps2.cpp` |
| `tools/` | 12,091 | `playground` 3399、`ps2test` 1343、`im3d` 1324、`im2d` 1156、`demoreel` 1018、`dumprwtree` 145 |
| 第三方 vendored | 82,451 | glad 4,688 + lodepng 8,355 + imgui/ImGuizmo 69,408 |

一手代码（剔除 vendored 与生成的着色器汇编）约 **4.2 万行**（`src/` 手写 38,090 + skeleton 2,315）。

### 3.7.2 构建系统、标准与依赖

两套并行构建：`CMakeLists.txt`（主推，CI 全量使用）与 `premake5.lua` + 仓库内自带的 `premake5.exe`（PS2 路径唯一可用）。CMake 侧 `src/CMakeLists.txt:140-148` 强制 `CXX_STANDARD 11` / `C_STANDARD 11`、`EXTENSIONS OFF`。平台矩阵由 `CMakeLists.txt:16-31` 决定：Windows = `NULL/GL3/D3D9`，Nintendo Switch = `NULL/GL3`（`cmake/nx/NXFunctions.cmake`），`PS2` = 仅 `PS2`（`cmake/ps2/`），其余 = `NULL/GL3`。CI（`.github/workflows/build.yml`）覆盖 win/macOS/Ubuntu 的 D3D9+GL3(GLFW/SDL2/SDL3)+NULL、`ps2dev/ps2dev:latest` 容器与 `devkitpro/devkita64`。依赖极少：OpenGL(≥2.1/ES 2.0)、d3d9+xinput、GLFW/SDL2/SDL3、vendored lodepng/glad/imgui；PS2 走 `ps2dev/ps2sdk`（`cmake/ps2/cmaketoolchain/toolchain_ps2_ee.cmake:16-50`，`mips64r5900el-ps2-elf-g++` + `dvp-as`，并自定义了一门 `DSM` 语言：`enable_language(DSM)`，`CMakeLists.txt:54`）。GameCube 在 `src/rwbase.h:560` 只是平台枚举里的一行注释占位，无实现。

### 3.7.3 核心问题：librw 有多像真 RW？

**关键反直觉发现：librw 完全不使用 `Rw*`/`Rp*` C 符号命名，也不使用 `naRw*` 枚举名。** 全仓 `Rp[A-Z]*` 唯一符号数 = **0**；`Rw[A-Z]*` 只有 2 个唯一名（`RwStateCache`、`RwRasterStateCache`，即各后端的状态缓存结构体，见 `src/ps2/rwps2impl.h:37`、`src/d3d/d3ddevice.cpp:61,72`、`src/gl/gl3device.cpp:146,156`），是全仓仅存的前缀残留。`naRw*`、`rpBaseData`、`rpBaseClass`、`GetT()`、`StreamCallback`、`RhDevice*`、`RpEngineRegisterNamespace`、`RpPLUGINS*`、`RWPLUGINENTRY` 在 librw 中**一个都不存在**。

它的策略是**语义同构、命名重构**：`namespace rw` 下的 C++ `struct` + 成员函数，逐条对应 RW 的 `Rp*Get/*Set/*StreamRead`。规模可用几个可测量指标刻画：168 处 `struct/class` 声明（含前向声明）、公共头文件中 609 条函数声明、434 条 `.cpp` 里的 `Class::func` 定义、191 个十六进制枚举常量（这些常量的**数值**必须与 RW 一致）。

真正被一比一复刻的是 RW 的**内部机制**而非符号名：

- **`RwObject` 头布局**：`src/rwobjects.h:5-10` 的 `struct Object { uint8 type; uint8 subType; uint8 flags; uint8 privateFlags; void *parent; }` 与 RW 的 `RwObject` 字段顺序/宽度完全相同。
- **插件偏移表**：`src/rwplg.h:64-82` 的 `PLUGINBASE` 宏为每个可插插件的对象注入 `static PluginList s_plglist` 与 `registerPlugin/registerPluginStream/setStreamRightsCallback/setStreamAlwaysCallback/getPluginOffset`——这就是 RW `_rpXXXRegister` + `RpEngineRegisterNamespace` 的等价物；共 **11 个对象**嵌入了它（`rwobjects.h:31 Frame, 225 Raster, 367 Texture, 430 Material, 515 Geometry, 610 Atomic, 672 Light, 734 Camera, 805 Clump, 855 World, 881 TexDictionary`）。`src/rwplg.h:3-4` 的 `PLUGINOFFSET(type, base, offset) ((type*)((char*)(base)+(offset)))` 在代码中被用 **133 次**，与 RW 尾部附加数据的做法一致。类型 ID 也照抄：`Geometry` `enum { ID = 8 }`（`rwobjects.h:516`）、`Atomic` `ID = 1`（`:612`）、`Clump` `ID = 2`（`:806`）、`Light` `ID = 3`、`World` `ID = 7`。
- **chunk ID 数值**：`src/rwbase.h:579-637` 用 `MAKEPLUGINID(v,id) (((v&0xFFFFFF)<<8)|(id&0xFF))` 重建 `Vend_core/CriterionTk/CriterionInt/CriterionWorld`，并列出 `ID_NAOBJECT/STRUCT/STRING/EXTENSION/CAMERA/…/UVANIMDICT` 与 `ID_MESH/NATIVEDATA/VERTEXFMT`；厂商号 10/11 是 librw 自定义（用于原生 raster）。`CoreModuleID`（`:648-668`）复刻了 RW 的 module 插件号（`ID_FRAMEMODULE`、`ID_RASTERMODULE`…）。
- **library version 编码**：`src/rwbase.h:690-725` 的 `libraryIDPack/UnpackVersion/UnpackBuild` 是 RW 把 `3.4.0` 打包进 chunk header 版本字段的位运算逆向，上方还留了 `// 0x1C000000  3.7` 的注释表；`src/rwbase.h:726-731` 的 `ChunkHeaderInfo{type,length,version,build}` 即 `RwChunkHeader`。
- **`RwStream`**：`src/rwbase.h:486-546` 改用 C++ 虚基类 `Stream/StreamMemory/StreamFile`，取代 RW 的函数指针 vtable，但 `read8/write32/seek/tell` 语义与 `RwStreamMacros` 一一对应。
- **`RwEngineInstance`**：`src/rwengine.h:158-206` 的 `struct Engine` + 全局 `Engine *engine`（`src/engine.cpp:34`）；`Engine::open()` 上方注释直接写「This is where RW allocates the engine and e.g. opens d3d」（`engine.cpp:244`）。`Engine::init()` 里 `Frame/Image/Raster/Texture::registerModule()` 后紧接 `for(i<NUM_PLATFORMS) new(&Driver::s_plglist[i]) PluginList(sizeof(Driver))` 与六个 `registerPlatformPlugins()`（`engine.cpp:212-235`），是 RW `RpEngineInit` 阶段的忠实重演。
- **`RhDevice*` 的对应物**：`src/rwengine.h:48` `Device`（渲染设备函数表：`beginUpdate/endUpdate/setRenderState/im2D*/im3D*/system`）与 `:74` `Driver`（平台相关但可移植：`rasterCreate/rasterLock/imageFindRasterFormat/rasterFromImage/rasterToImage`）。`ARCHITECTURE.MD:86-88` 明说「The second category does not exist in original RW」——`Driver` 层是 librw 为「任意平台读写任意平台文件」新增的抽象。`DeviceReq` 枚举（`rwengine.h:4-36`）复刻了 RW 那个奇怪的单一 `RpEngineSystem(req,arg,n)` 入口，作者旁注「uhhhm..... why are these not actual functions?」。
- **`RwRenderState`**：`src/rwrender.h:4-41` 顺序与 RW 完全一致（`TEXTURERASTER…STENCILFUNCTIONWRITEMASK`），只额外加了 `GSALPHATEST/GSALPHATESTREF` 用于 GS 语义模拟。
- **atomic→geometry→clump 层级**：`Atomic` 持 `Geometry*`+`Clump*`+`ObjPipeline*`+`renderCB`（`rwobjects.h:623-633`），`Clump` 用三条 `LinkList atomics/lights/cameras`（`:808-810`），`Frame` 有 `matrix`/`ltm` 双矩阵与 `HIERARCHYSYNC*` 私有标志（`:34-46`）——与 RW 的 frame/clump/atomic/geometry + LTM 脏标记模型一致。

**偏移/尺寸断言：全仓 `static_assert` 仅 2 条，且都被注释掉**（`src/skin.cpp:438-439`：`//static_assert(sizeof(Skin::RLEcount) == 2, "RLEcount size");`）。没有 `RWOFFSET` 式宏，没有 `leftover`/`padding` 字段（`leftover` 出现 0 次）。原因是设计目标不同：librw **不追求内存布局兼容真 RW**，只追求**磁盘 chunk 布局兼容**，因此布局校验靠手工逐字段偏移读写完成，最典型是 `src/d3d/d3d9.cpp:172-192`：

```c
header->serialNumber = *(uint32*)p; p += 4;
header->numMeshes    = *(uint32*)p; p += 4;
header->indexBuffer  = nil;         p += 4;
p += 16*2;  // skip vertex streams, they're repeated with the vertex buffers
header->useOffsets   = *(bool32*)p; p += 4;
```

这段「按 4 步进、显式跳过未知字段」的写法，本身就是反出的字节偏移量的化石记录。对内存布局真正敏感的地方（`struct InstanceDataHeader : rw::InstanceDataHeader` 在 `d3d/rwd3d9.h:39`、`ps2/rwps2.h:77`；`Texture` 的 `char name[32]; char mask[32];` `rwobjects.h:371-372`）靠字段顺序自然对齐而非断言。唯一一处显式命名的 pad 字段是 `src/ps2/rwps2.h:23` `Im2DVertex` 里的 `float32 u, v, q, PAD;`（GS 顶点要求 128 位对齐）。另有 31 处 `padding` 命中全在 PNG/BMP 注释里，与 RW 无关。

### 3.7.4 平台后端三分割

同一套 API 由 5 个后端分别实现，`Engine::open` 通过编译期 `#ifdef` 挑一个 `Device`（`engine.cpp:266-275`），但**所有平台的 `Driver` 同时在场**（`engine.cpp:280-296`），这正是 librw 能跨平台转换 DFF/TXD 的根因。

| 后端 | 目录/行数 | Driver 注册 | 状态 |
|---|---|---|---|
| OpenGL 3 | `src/gl/` 6,795 | `gl3.cpp:48 registerPlatformPlugins()` → `PLATFORM_GL3` | 最完整；有 GLSL 源（`shaders/` 714 行）+ skin/matfx 管线 |
| D3D9/D3D8/Xbox | `src/d3d/` 13,061 | `d3d.cpp`/`d3d9.cpp`/`d3d8.cpp`/`xbox.cpp` | 读写原生数据完备；`xbox.cpp:202` 注「xbox can't render」 |
| PS2 | `src/ps2/` 7,005 + 1,654 微码 | `ps2.cpp:56` | 「working as a test only」（`README.md:17`） |
| WarDrum GL | `src/gl/wdgl.cpp` 875 | `wdgl.cpp:327`「wdgl can't render」 | 仅数据侧 |
| NULL | `src/engine.cpp:470-545` | — | 10 条 `assert(0 && "... not implemented")`（`:478,485,492,498,505,511,519,526,533`） |

五套 `InstanceDataHeader`/`InstanceData`/`AttribDesc` 与五套 `readNativeData/writeNativeData/getSizeNativeData`（如 `ps2.cpp:84,129`、`d3d8.cpp:104`、`xbox.cpp:133`）意味着作者必须**分别反出 PS2 VIF 包、D3D8/D3D9 FVF+顶点流、Xbox 顶点格式、GL 原生扩展**的私有二进制布局。这本身就是「RW 内部抽象层被恢复了多少」的直接度量：文件读写侧（`Driver`）与渲染侧（`Device`）被彻底解耦，而真 RW 是把两者焊在同一个 `RwDevice` 里的。

### 3.7.5 PS2 与「完全用自由工具链构建」

HEAD 提交正是这条前沿：`build the ps2 platform entirely with freesce`。做法是在 `premake5.lua:41-44` 加 `--freesce` 选项，`:91-116` 把 PS2 平台一分为二：默认走 SCE SDK（`gccprefix 'ee-'`、`$(PS2SDK)/ee/include`），加上 `--freesce` 则 `gccprefix '../tools/freesce/ee-'` + `$(FREESCE)/ee/include`，并注释解释「a premake-time choice because the 2.9 and 3.2 C++ ABIs don't link」。`tools/freesce/ee-g++`、`ee-gcc`、`ee-ar` 是三个 sh 包装器，专门剥掉 premake 生成的 gcc-3 风格依赖开关（`-MD/-MP/-MF`），因为 freesce 的 **ee-gcc 2.9** 驱动会把 `-MF` 的参数当第二个输入文件。

真正的硬核证据在 `tools/demoreel/Makefile:66-125`：`demoreel_cd_freesce.elf` 用 `-nostdlib` 显式列出 `crt0/crti/crtbegin/crtend/crtn` + freesce 自己的 6 个库（`libgraph/libdma/libpc/libpad/libcdvd/libkernl`）+ newlib 的 `-lc -lm` + `app.cmd`，「Nothing comes from /usr/local/sce」。因为 gcc 2.9 在 EE 上只生成 `_GLOBAL_$I` 构造函数而不注册它们，还要跑 `ctorgen` 生成 `ctorreg.s`——Makefile:93-97 的注释写道：「Without it librw's global ctors (**the PluginList statics**) silently never run」。这句话反过来印证了 §3.7.3 的机制：librw 的插件系统是靠 C++ 全局静态对象自注册的。此外 `includedirs` 备注 freesce 头文件自带本 vintage 的 `IOP_IMAGE_FILE`（`IOPRP242.IMG`），与 SCE 头文件同构。CMake 侧的 PS2 路径仍依赖 `ps2dev/ps2sdk` 且要求 `GSKIT`（`toolchain_ps2_ee.cmake:21-35`），freesce 路线目前只在 premake 分支可用。

另一处前沿证据是手写的 **VU1 微码**（`src/ps2/vu1/`：`defaultPipe.dsm`、`cliptri.vu`、`lighting.vu`、`LL/LS/TL/TSclip.vu` 共 1,654 行），以及 `src/ps2/ps2device.cpp:387-416` 那段注释——先写「It would be nice to be able to test against RW's microcode」，再列出 RW 的 VU 常量内存表 `0x3F0 vuSDmat0 … 0x3FF vuSDVUSwitch`，右侧还标出 RW 原始符号名 `vu1DataNearClip`、`xMaxYMax128`、`vu1DataOffset3D`、`skyClipVect1`。这是一份从二进制里抄出来的 RW PS2 内部 ABI。

### 3.7.6 完成度与子库成熟度

没有 `RW_UNIMPLEMENTED` 宏（0 次）；替代形态是 148 条 `TODO`、20 条 "not implemented"、以及 `TODO` 文件（`tristrips / morphing / Pipelines (PDS, Xbox, PC) / bsp / ps2 rendering / ADC conversion`）。

| RW 子库 | librw 对应 | 成熟度 |
|---|---|---|
| rpgeometry / rpatomic / rpclump / rpframe | `geometry.cpp`+`geoplg.cpp`+`clump.cpp`+`frame.cpp` | 完整（含 mesh/tristrip 生成 `tristrip.cpp` 682 行） |
| rpraster / rptexture / rpiimage | `raster.cpp`+`texture.cpp`+`image.cpp`+`{bmp,tga,png}.cpp` | 完整，DFF/TXD 五平台互通 |
| rpcamera / rplight | `camera.cpp` 524 / `light.cpp` 163 | 完整（含 frustum plane、`enumerateLights`） |
| rpskin / rpsplit | `skin.cpp` 486 + 各后端 `*skin.cpp` | 完整，含 split-skin RLE |
| rphanim | `hanim.cpp` 414 | 良（`hanim.cpp:154`「TODO: handle more (all!) cases」） |
| rpanimation | `anim.cpp` 293 + `rwanim.h` | 部分（`anim.cpp:192` 猜指针/padding 尺寸；插值器不全） |
| rpmatfx | `matfx.cpp` 641 + `*matfx.cpp` | 部分（ARCHITECTURE.MD:31「MatFX (only env map so far)」） |
| rpuvanim / rpuserdata / rpanisot / rpcharset | `uvanim.cpp` 502 / `userdata.cpp` 364 / `texture.cpp:565` / `charset.cpp` | 良（`uvanim.cpp:257`「implement this fully」） |
| **rpworld** | `world.cpp` 238 | **stub**：`rwobjects.h:852`「A bit of a stub right now」；无 `RpWorldSector`、无 sector tree，`World::render()` 自认「this is very wrong, we really want world sectors」（`world.cpp:136`）；`world.cpp:181` 局部光排除需要 world sectors |
| rppatch / rpgeommod | 无 | 缺失（`prim.cpp` 70 行仅占位） |
| rpsprite / rpgui / rpu2d | 无 | 缺失（全仓无 sprite GUI 实现） |
| rpbsp / rpeFFECT / rplegacy normal map | 无 | 缺失（`README.md:14`「BSP is not supported at all」） |
| PDS | `ps2/pds.cpp` 203 | 仅注册/查找壳，管线未接 |

管线本身被刻意简化：`ARCHITECTURE.MD:22-28` 说 RW 的 pipeline「severely overengineered」，librw 只保留 `ObjPipeline` 的 `instance/uninstance/render` 三指针（`src/rwpipeline.h:28-32`）而非 RW 的 stage 链。

### 3.7.7 librw 与 OpenRW 的对照

| 维度 | librw | OpenRW |
|---|---|---|
| 许可 | MIT（`LICENSE:1-3`） | GPL-3.0（`COPYING`） |
| 一手 LOC | ≈42,000（`src/` 手写 38,090 + skeleton 2,315） | ≈58,800（`rwcore` 12,124 + `rwengine` 38,399 + `rwgame` 5,567 + `rwtools` 272 + `rwviewer` 2,414）；RW 本体只有 `rwcore/rw`+`data`+`loaders`+`gl` ≈2,300 行手写 |
| 目标 | 替代 RW **库**（可被 re3/reVC 链接） | 替代 GTA III **游戏**，RW 只是它的 loader |
| RW API 命名 | 语义同构、`namespace rw` 重构；`Rp*`=0，`Rw*`=2 | 同样无 `Rw*/Rp*`（`rwcore/` 中唯一符号数 = 0），`openrw::rw::Clump/Geometry/Atomic` |
| 对象插件机制 | **有**：`PLUGINBASE`×11、`PluginList`/`Plugin`、`PLUGINOFFSET`×133、16 个 `register*()` 注册入口 | **无**：`rwcore/` 中 `Pipeline`/`Plugin` 关键字 0 命中 |
| chunk/stream | 复刻 `MAKEPLUGINID`+`Vend_*`+`libraryIDPack`+`ChunkHeaderInfo`，五平台原生数据双向读写 | 只有 `RWBStream::ChunkID`（`rwcore/loaders/RWBinaryStream.hpp:31`）单向读 `LoaderDFF.cpp` |
| 内存布局断言 | 2 个 `static_assert`，全部注释掉（`src/skin.cpp:438`） | 2 个生效的 `static_assert`，且都不是 RW 布局而是自研结构：`rwengine/src/engine/SaveGame.cpp:549 sizeof(BasicState)==0xBC`、`rwengine/src/data/Chase.cpp:39 sizeof(ChaseEntryRecord)==28` |
| 渲染后端 | 5 套（GL3/D3D9/D3D8/Xbox/PS2）+ 微码 | 1 套 OpenGL 3.3（vendored `gl_core_3_3.c` 5,737 行） |
| 测试 | 无 `tests/`，靠 `tools/` 回归 | 有 `tests/` 3,574 行 + CTest/CDash/codecov |
| 上游依赖 | 被 re3/reVC 使用（`README.md:21`） | 自带 `rwgame` 脚本/物理/音频全栈 |

**谁更「引擎级」？** librw 明显更忠实。它复刻了 RW 的**内部结构机制**（Object 头 + 尾部插件偏移表 + 构造/析构/流三段回调 + rights/always 回调 + module 注册顺序 + 每平台 native raster/geometry 私有格式），这些只有在反复对照 RW 二进制/DFF 才能重建；OpenRW 只到「能读通 DFF 到内存」这一层，其 RW 侧 ≈2.3k 行没有管线、没有插件、没有多平台。LOC 换算是反直觉的：OpenRW 总行数是 librw 的 1.4 倍，但增量全在 `rwengine/rwgame`（游戏逻辑、脚本、物理、AI），RW 本体反而薄得多。代价是 librw 无法直接当游戏跑（`World` 是 stub、无 rpsprite/rpgui），而 OpenRW 能通关 GTA III。

### 3.7.8 溯源标记汇总

`src/ps2/ps2raster.cpp:479`「This code was reversed from RW and is rather complicated…」；`:43`「i don't really understand this, **stolen from RW**」；`:801`「RW's code does not seem to quite work with 24 bit rasters」；`:1001`「but even **gta3's 3.1** seems to??」；`src/ps2/ps2raster.cpp:662`「We use a lookup, RW does bit operations」；`src/ps2/rwps2.h:102`「Not really types as in RW but offsets」；`src/rwobjects.h:311`「// from RW」标注 `PRIVATELOCK_READ/WRITE/…` 私有位；`:376`「actually not in RW」标注主动偏离；`src/skin.cpp:140-141`「numUsedBones and numWeights appear in/after **34003** but not in/before **33002**（probably `rw::version >= 0x34000`）」——用 RW 构建号做版本考古；`src/d3d/xbox.cpp:138`、`ps2.cpp:657` 用 `rw::version >= 0x35000`（RW 3.5）门控写入。RW 原始符号名 `vuSDmat0/vu1DataNearClip/xMaxYMax128/skyClipVect1`（`ps2device.cpp:390-415`）。`src/gl/rwwdgl.h:5`「This is not really RW OpenGL! It's specific to **WarDrum's GTA ports**」（WarDrum 即 GTA III/VC 移动移植方）。`tools/dumprwtree/dumprwtree.cpp:11-71` 保留了 RW SDK 版 `dumprwtree` 的完整 chunk 名称表，并注明区段「From 0x0101 through 0x0135」「From 0x0180 through 0x01c1」，其中 Rockstar 私有扩展 `RSchunks`（`0x0253F2F0+`）列出 `2dfx / Breakable / Collision Model / Environment Material / Specular Material / Extra Normals / Pipeline Set / Node Name`，另有 `0xF21E "ZModeler Lock"`——这份表是 GTA MOD 社区 chunk 知识的权威来源。`skeleton/skeleton.h:7`「same as RW skeleton」表明 `sk::` 应用框架也照 RW SDK 的 skeleton 示例而来。未见 IDA/Ghidra 地址注释或反汇编工具产物引用（推断：主要靠 RW SDK 文档 + DFF/TXD 实测往返，而非仅二进制反出）。

### 3.7.9 结论

librw 与 OpenRW 代表了 RenderWare 复现的两条截然不同的技术路线：librw 是**库级重建**，用约 4.2 万行 MIT 代码把 RW 的对象头、插件偏移表、module 注册时序、chunk/version 编码和五个平台的原生数据格式逐机制复原，并额外做出了一件事——用 freesce（自带 ee-gcc 2.9 + newlib）替换 Sony SDK 完成 PS2 全链路自举；OpenRW 是**应用级重建**，只需要 RW 的磁盘格式可读，因此可以用 ~2.3k 行的 loader 加一套 GL 后端撑起整款游戏。前者是引擎考古，后者是产品复活。
