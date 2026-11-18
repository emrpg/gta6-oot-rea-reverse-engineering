
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
