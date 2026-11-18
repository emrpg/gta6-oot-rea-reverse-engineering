# GTA 6（GTA VI）及 GTA 系列核心开源项目源码架构深度分析报告

> **分析范围**：基于本地 `/root/wsl-demo/gta-projects/` 下 10 个核心开源仓库的源码实证分析，涵盖 **GTA VI 社区测绘与三维重建**、**RenderWare 引擎重实现与二进制逆向**、**大型多人联机框架**、**RAGE 资产解析与 3D 工具链** 以及 **RAGE 运行时修复与托管脚本引擎** 五大领域。

---

## 一、 开源生态全景与项目矩阵

由于 **GTA VI（GTA 6）** 本体尚处于未发售的闭源商业开发阶段，当前 GitHub 开源社区中围绕 GTA VI 的项目主要集中在**基于预告片与官方截图的摄影测量三角定位、交互式地图测绘与 WebGL 3D 场景重建**；而针对底层引擎、渲染管线、网络同步与资源格式的深度开源实现，则高度集中于 GTA III、GTA:SA、GTA IV 及 GTA V（含 Gen9 增强版）生态中。

| 项目名称 | 目标游戏 / 引擎 | 核心语言 / 技术栈 | 开源协议 | 核心定位 |
| :--- | :--- | :--- | :--- | :--- |
| **gta6-leonida-atlas** | GTA VI (Leonida) | TypeScript 6 / React 19 / Vite 8 / Leaflet / Three.js | AGPL-3.0 | 社区驱动的 Leonida 交互式地图与程序化 3D 街景探索器 |
| **gtamaplib** | GTA VI (摄影测量) | Python / NumPy / SciPy / Pillow | MIT | 基于 GTA ENU 坐标系的 3D 相机标定与射线三角测量库 |
| **openrw** | GTA III (RenderWare) | C++ / OpenGL 3.3 / Bullet / SDL2 / OpenAL | GPL-3.0 | 跨平台开源 GTA III 游戏引擎重实现 |
| **gta-reversed** | GTA:SA (RenderWare) | C++20/23 / CMake / Conan / ReversibleHooks | - | 增量式 GTA:SA 二进制逆向与函数级源码替换工程 |
| **fivem** | GTA V / IV / RDR2 (RAGE) | C++ / C# / Lua / V8 / D3D11 / CEF | CitizenFX | 模块化多人联机框架（OneSync 同步、多语言 ScRT、VFS） |
| **mtasa-blue** | GTA:SA (RenderWare) | C++ / Lua / Direct3D 9 / RakNet / CEF | GPL-3.0 | 老牌 GTA:SA 多人联机平台（D3D9 代理 Hook、元素树同步） |
| **CodeWalker** | GTA V / Gen9 (RAGE) | C# / .NET / SharpDX (Direct3D 11) | MIT | RPF7 解密、全格式 RAGE 资产解析与 3D 世界编辑器/渲染器 |
| **Sollumz** | GTA V (RAGE) | Python / Blender 4.2+ / `szio` / `pymateria` | GPL-3.0 | Blender 平台 GTA V 资产导入/导出与着色器节点建模套件 |
| **GTAIV.EFLC.FusionFix** | GTA IV / EFLC (RAGE) | C++20 Modules (`.ixx`) / HLSL / D3D9On12 / DXVK | MIT | 现代化的 GTA IV 引擎修复、后处理管线与内存 RPF 加载器 |
| **scripthookvdotnet** | GTA V (RAGE) | C# / C++/CLI / .NET AppDomain / Roslyn | zlib | 基于 ScriptHookV 的 .NET 托管脚本运行时与 API 封装 |

---

## 二、 GTA VI（GTA 6）开源项目深度分析

### 1. `gta6-leonida-atlas`（Leonida 交互式地图与 3D 街景重建）
- **仓库路径**：`/root/wsl-demo/gta-projects/gta6-leonida-atlas`（`navanem/gta6-leonida-atlas`，版本 `v0.8.2`）
- **目录架构**：
  - `src/app/`：应用壳层、路由（`routes.ts`）、侧边栏（`Sidebar.tsx`）与状态栏。
  - `src/features/map/`：基于 **Leaflet** 的二维地图核心，包含 `coordinates.ts`、`layers.ts`、`spatial.ts`、`MapView.tsx`。
  - `src/features/street-leonida/`：基于 **Three.js (`0.186.1`)** 的程序化 3D 漫游与街景探索子系统（`walk-engine.ts`、`walk-region-streaming.ts`、`walk-atmosphere.ts`、`walk-postprocessing.ts`、`walk-water-surface.ts`、`walk-road-geometry.ts`、`walk-window-interiors.ts` 等 30+ 模块）。
  - `src/db/` & `src/stores/`：采用 **Dexie (IndexedDB)** 实现本地优先（Local-first）数据持久化与备份，使用 **Zustand** 管理全局状态。
- **坐标系统与空间管线**：
  - 在 `src/features/map/coordinates.ts` 中，地图采用 Leaflet 的无投影平面坐标系 `CRS.Simple`，将游戏世界坐标 `(x, y)` 映射为 `[lat, lng] = [y, x]`；SVG 底图 `viewBox` 采用 `world x * 2, y * -2` 缩放约定。
  - 预设社区测绘边界 `INITIAL_BOUNDS` 覆盖 `west: -10000, east: 3300, south: -8000, north: 9200`，缩放层级 `MIN_ZOOM = -7` 至 `MAX_ZOOM = 3`。
- **3D 渲染与流式加载**：
  - `street-leonida` 模块并非简单展示静态全景图，而是构建了一套轻量级 WebGL 程序化街景引擎：支持按区域流式加载建筑轮廓（`walk-region-streaming.ts`、`walk-footprints.ts`）、立面与室内视差窗户（`walk-facade-shell.ts`、`walk-window-interiors.ts`）、水面反射（`walk-water-surface.ts`）、大气散射与后处理（`walk-atmosphere.ts`、`walk-postprocessing.ts`）。

### 2. `gtamaplib`（GTA VI 三维摄影测量与三角定位引擎）
- **仓库路径**：`/root/wsl-demo/gta-projects/gtamaplib`（`rolux/gtamaplib`，版本 `v0.1.0`）
- **核心文件**：
  - `gtamaplib.py`（核心 `Camera` 与 `Map` 类及三角测量/优化器，3875 行）
  - `gtamapdata.py`（社区标注的 GTA VI 预告片/截图相机参数、逐帧标注与地标坐标数据库）
  - `gtamaputils.py`（实用工具与用户脚本集）
- **几何与坐标约定**：
  - 严格遵循 GTA 内部使用的**右手 ENU（East-North-Up）坐标系**（见 `gtamaplib.py` 头部注释）：`+X` 向东、`+Y` 向北、`+Z` 向上；偏航角（Yaw）绕世界 `+Z` 轴（0° 为正北，逆时针递增），俯仰角（Pitch）绕局部 `+X` 轴，滚转角（Roll）绕局部 `+Y` 轴。图像原点位于左上角，像素 `(x, y)` 中心为 `(x + 0.5, y + 0.5)`。
- **核心算法架构**：
  - **消失点 FOV 反推**：`Camera._get_fov_from_lines()` 利用画面中正交的水平线族（`_get_hvps_from_hlines`）与垂直线族（`_get_vvps_from_vlines`）计算消失点，从而自动求解未知截图的水平/垂直视场角（HFOV/VFOV）。
  - **多进程参数扫描定位**：`find_camera` 方法借助 Python `multiprocessing`、`NumPy` 与 `scipy.spatial.transform.Rotation`，在给定搜索空间内执行并行多维参数扫描，结合多个已知相机观测到的共享地标射线交点，最小化重投影对数损失（log loss），实现未知镜头机位与姿态的精确标定。

---

## 三、 经典 RenderWare 引擎重实现与逆向工程（GTA III / SA）

### 3. `openrw`（GTA III 开源引擎重实现）
- **仓库路径**：`/root/wsl-demo/gta-projects/openrw`（`rwengine/openrw`）
- **目录架构**：
  - `rwcore/`：底层 RenderWare 格式加载器（`loaders/LoaderDFF.cpp`、`LoaderTXD.cpp`、`LoaderIMG.cpp`、`LoaderSDT.cpp`）、OpenGL 封装（`gl/GeometryBuffer.cpp`、`DrawBuffer.cpp`、`TextureData.cpp`）、字体与平台抽象。
  - `rwengine/src/`：引擎核心子系统，划分为 `render/`、`script/`、`loaders/`（IDE/IPL/COL/IFP/GXT/Weather）、`dynamics/`（Bullet Physics 碰撞与射线检测）、`ai/`、`audio/`（OpenAL + FFmpeg）、`objects/`（车辆/角色/拾取物/投射物）。
  - `rwgame/`：游戏状态机（`MenuState`、`LoadingState`、`IngameState`、`PauseState`、`DebugState`）与 ImGui 调试界面。
- **渲染管线（OpenGL 3.3 Core）**：
  - 在 `GameRenderer`（`rwengine/src/render/GameRenderer.hpp` & `.cpp`）中，底层通过 `OpenGLRenderer` 管理帧缓冲（FBO）与 Uniform Buffer Object（UBO）：
    - 绑定 `SceneData`（binding 1）与 `ObjectData`（binding 2）两个 UBO。
    - FBO 挂载两个颜色附件：`fbTextures[0]`（`GL_RGBA` 主色彩缓冲）和 `fbTextures[1]`（`GL_R16F` 数据纹理，专门传递给 `WaterRenderer` 用于水面深度/效应计算），以及 `GL_DEPTH24_STENCIL8` 深度模板缓冲。
    - 渲染流程依次调度 `MapRenderer`、`ObjectRenderer`、粒子系统（`particleProg`）、`WaterRenderer`、天空穹顶（`skyProg`）及全屏后处理（`postProg`）。
- **脚本虚拟机（SCM VM）**：
  - `ScriptMachine`（`rwengine/src/script/ScriptMachine.hpp`）实现了原版 `main.scm` 字节码的取指-执行循环。
  - 每个协程由 `SCMThread` 表示：拥有独立的 `baseAddress`、`programCounter`、4 层调用栈（`SCM_STACK_DEPTH = 4`）、256 个 4 字节局部变量槽位（`SCM_THREAD_LOCAL_SIZE = 256`）、条件组合计数与掩码（`conditionCount`、`conditionResult`、`conditionAND`、`conditionMask`）以及毫秒级唤醒计数器 `wakeCounter`。

### 4. `gta-reversed`（GTA:SA 增量式二进制逆向工程）
- **仓库路径**：`/root/wsl-demo/gta-projects/gta-reversed`（`gta-reversed/gta-reversed`）
- **目录架构**：
  - `source/game_sa/`：按原版游戏模块严格划分，涵盖 `Animation/`、`Audio/`、`Collision/`、`Entity/`、`Fx/`、`Models/`、`Pipelines/`（`CustomBuilding`、`CustomCarEnvMap`）、`RenderWare/`、`Scripts/`（`CRunningScript`、`CTheScripts`、`CommandParser`）、`Tasks/` 等，目前已逆向约 50%–60% 代码。
  - `source/reversiblehooks/`：自研热替换 Hook 框架（`RHManager`、`VMTInfo`、`HookCategoryItem`）。
  - `source/reversiblebugfixes/` & `source/toolsmenu/`：可开关的原版 Bug 修复与基于 ImGui 的运行时调试菜单。
- **逆向注入与 Hook 机制**：
  - 编译为注入 GTA:SA Compact 可执行文件（`5,189,632 bytes`）的 DLL/ASI。
  - 在 `ReversibleHooks.h` 中定义了高度声明式的宏体系：`RH_ScopedClass` / `RH_ScopedVirtualClass` 绑定目标类与虚函数表（VMT）地址，`RH_ScopedInstall` 替换静态/成员函数地址，`RH_ScopedVMTInstall` 替换虚函数表项，`RH_ScopedInstallScriptCommand` 接管 SCM 脚本指令。这使得开发者可以在运行时自由切换“原版二进制逻辑”与“逆向 C++ 重写逻辑”以进行行为比对。

---

## 四、 大型多人联机框架（Multiplayer Frameworks）

### 5. `fivem`（CitizenFX：GTA V / GTA IV / RDR2 多人联机平台）
- **仓库路径**：`/root/wsl-demo/gta-projects/fivem`（`citizenfx/fivem`）
- **目录架构（基于组件化微内核）**：
  - `code/client/citicore/` & `citigame/` & `launcher/`：客户端启动器、COM/对象模型（`om`）、安全上下文（`se`）与控制台。
  - `code/components/`：超过 100 个独立组件，通过前缀区分公共层、服务端（`citizen-server-*`）及游戏特定层（`-five` 对应 GTA V，`-ny` 对应 GTA IV，`-rdr3` 对应 RDR2）。
- **网络同步机制（OneSync 架构）**：
  - 原版 GTA Online 采用 P2P 网状同步，FiveM 通过 `gta-net-five`（客户端）与 `citizen-server-impl`（服务端 `ServerGameState`、`ServerGameState_SyncTrees.cpp`）将其改造为**服务端权威/中继同步**：
  - 在 `gta-net-five/src/netSyncTree.cpp` 中，通过特征码扫描（`hook::get_pattern`）与跨版本运行时抽象（`xbr::IsGameBuildOrGreater<2802>()`）Hook RAGE 引擎的 `rage::netSyncTree::ReadFromBuffer`、`CanApplyToObject`、`PrepareObject` 及 `GetForType`。
  - 客户端实体的创建、状态位（`datBitBuffer`）序列化与反序列化均被拦截并路由至 `CloneManager` / `CloneObjectManager`，服务端解析同步树节点后按空间兴趣范围（OneSync）分发给周边玩家。
- **渲染管线 Hook 与 UI**：
  - `rage-graphics-five/src/RenderHooks.cpp` Hook 了 Direct3D 11 / DXGI（`d3d11_1.h`、`dxgi1_4.h`、`dxgi1_5.h`）以及 RAGE `grc`（Graphics Core）内部状态（`OnGrcCreateDevice`、`OnBackbufferCreation`、`OnPostFrontendRender`、`_grcBindTexture`、`_grcLighting`、`flushRenderStates`），同时集成 Flip Model 优化、ReShade/AMD 驱动兼容性修复，并将 `nui-core`（基于 CEF 的 HTML5 UI）叠加渲染至游戏后缓冲。
- **多语言脚本运行时（ScRT）与 VFS**：
  - `citizen-scripting-core` 定义统一的脚本运行时接口，分别由 `citizen-scripting-lua`（Lua）、`citizen-scripting-v8` / `v8-v12.4` / `node`（JavaScript/Node.js）、`citizen-scripting-mono` / `mono-v2`（C#/.NET）实现，统一调用 `rage-scripting-five` 暴露的 RAGE Native 函数。
  - `vfs-core` 与 `vfs-impl-rage` 接管 RAGE `fiDevice`（`rage-device-five`），实现无需修改原版 RPF 的虚拟文件系统流式覆盖（`gta-streaming-five`）。

### 6. `mtasa-blue`（Multi Theft Auto：GTA:SA 多人联机平台）
- **仓库路径**：`/root/wsl-demo/gta-projects/mtasa-blue`（`multitheftauto/mtasa-blue`）
- **目录架构**：
  - `Client/core/`：核心框架、D3D9/DInput8 代理 Hook（`DXHook/`）、图形渲染管理器（`Graphics/`）、CEF 浏览器（`cefweb/`）。
  - `Client/game_sa/` & `Client/multiplayer_sa/`：GTA:SA 内存结构封装与底层补丁/Hook（`multiplayer_hooksystem.h`、`multiplayer_keysync.cpp`、`multiplayer_shotsync.cpp`、`CMultiplayerSA_Streaming.cpp` 等）。
  - `Client/mods/deathmatch/` & `Server/mods/deathmatch/`：核心联机逻辑、元素树（Element Tree）、Lua 脚本虚拟机与安全反作弊（`CAntiCheat`、`CAccessControlList`）。
- **渲染管线（Direct3D 9 代理模式）**：
  - 不同于 FiveM 的特征码内联 Hook，`mtasa-blue` 在 `Client/core/DXHook/` 中实现了完整的 COM 代理包装器（`CProxyDirect3D9`、`CProxyDirect3DDevice9`、`CProxyDirect3DEffect`、`CProxyDirect3DTexture` 等）。
  - `CGraphics` 与 `CRenderItemManager`（`Client/core/Graphics/CGraphics.h`）在此基础上构建了 `CTileBatcher`、`CLine3DBatcher`、`CPrimitive3DBatcher` 等批处理器，并支持通过 Lua 创建自定义 HLSL 着色器（`CRenderItem.Shader.cpp`）、离屏渲染目标（`RenderTarget`）、屏幕源（`ScreenSource`）、SVG 矢量图（`VectorGraphic`）和 CEF 网页纹理（`WebBrowser`）。
- **网络同步与脚本机制**：
  - 底层基于 **RakNet** 封装数据包传输（`CPacketTranslator`、`CBitStream`），服务端针对不同实体类型划分了精细的同步管理器：`CLightsyncManager`（远距离轻量同步）、`CPedSync`、`CObjectSync`、`CUnoccupiedVehicleSync`（无人驾驶载具同步）。
  - 脚本层采用 **Lua**，以树形 `Element` 模型组织所有玩家、载具、建筑、碰撞体（`CClientColShape`）与自定义数据，并配合服务端 `CAccessControlList`（ACL）与资源沙盒机制运行。

---

## 五、 RAGE 引擎资产解析与 3D 管线工具（GTA V / Gen9）

### 7. `CodeWalker`（GTA V RAGE 资产解析与 3D 世界编辑器）
- **仓库路径**：`/root/wsl-demo/gta-projects/CodeWalker`（`dexyfex/CodeWalker`）
- **目录架构**：
  - `CodeWalker.Core/`：纯 .NET 核心解析库，包含 `GameFiles/`（`RpfFile.cs`、40+ 种 `FileTypes/`、18 种 `Resources/`、`MetaTypes/`）与 `World/`。
  - `CodeWalker/`：基于 **SharpDX (Direct3D 11)** 的实时 3D 地图/模型/时间周期渲染器与编辑器（`Rendering/Renderer.cs`、`Shaders/`、`ShaderManager.cs`、`RenderableCache.cs`）。
  - `CodeWalker.Gen9Converter/`、`CodeWalker.RPFExplorer/`、`CodeWalker.ModManager/`、`CodeWalker.Peds/`、`CodeWalker.Vehicles/`：面向不同工作流的独立工具集。
- **资源解析架构**：
  - `RpfFile.cs` 完整逆向了 **RPF7** 归档格式，支持 AES 与 NG（Next-Gen）密钥解密、递归嵌套 RPF 挂载以及资源文件头（Resource Header）的虚拟/物理页分段解压。
  - `FileTypes/` 与 `Resources/` 覆盖了 RAGE 几乎全套二进制资产：`YdrFile`（Drawable 模型）、`YddFile`（模型字典）、`YftFile`（破碎/载具骨架 Fragment）、`YbnFile`（物理碰撞 Bounds）、`YtdFile`（纹理字典）、`YmapFile` & `YtypFile`（地图实体放置与原型定义）、`YcdFile`（动画剪辑）、`YndFile`/`YnvFile`（路网与导航网格）、`AwcFile`（音频容器）、`FxcFile`（编译后着色器）、`HeightmapFile` & `WatermapFile`。`CodeWalker.Gen9Converter` 则进一步支持将资产转换为 GTA V 增强版（Gen9）格式。
- **离线重渲染管线**：
  - `Renderer.cs` 使用 SharpDX D3D11 重建了 GTA V 的部分着色与环境系统，内置 `Timecycle`（时间周期）、`Weather`、`Clouds`、`ShaderGlobalLights`（自然环境光与人工光源）以及基于视距的动态 LOD 流式调度（`usedynamiclod`、`RenderableCache`）。

### 8. `Sollumz`（Blender 平台 GTA V 资产建模套件）
- **仓库路径**：`/root/wsl-demo/gta-projects/Sollumz`（`Sollumz/Sollumz`，版本 `2.9.1`，支持 Blender 4.2+）
- **目录架构**：
  - 按 RAGE 资产类型高度模块化：`ydr/`、`ydd/`、`yft/`、`ybn/`、`ycd/`、`ytd/`、`ymap/`、`ymap_next/`（新一代地图编辑器，含 `grass`、`lod_lights`、`occluders`、`overlays`）、`ytyp/`、`ynv/`。
  - `shared/shader_expr/` & `shared/presets/`：将 RAGE 引擎材质参数与着色器名称映射为 Blender Principled BSDF / Shader Node 网络。
- **双后端序列化架构（`dependencies.py`）**：
  - **XML 互通后端（`szio`）**：必需依赖（v1.4.0），用于读写与 CodeWalker 兼容的 `.xml` 中间格式（`.ydr.xml` 等）。
  - **原生二进制后端（`pymateria`）**：可选依赖（来自 Cfx.re 官方源 `https://static.cfx.re/whl/`），允许在 Windows 环境下直接导入/导出原生 RAGE 二进制资产，并支持自动生成载具碎裂贴图（vehicle shattermap）。

---

## 六、 RAGE 运行时增强与脚本桥梁（GTA IV / GTA V）

### 9. `GTAIV.EFLC.FusionFix`（GTA IV 现代化引擎修复与渲染增强）
- **仓库路径**：`/root/wsl-demo/gta-projects/GTAIV.EFLC.FusionFix`（`ThirteenAG/GTAIV.EFLC.FusionFix`）
- **目录架构**：
  - 全面采用 **C++20 Modules (`.ixx`)** 组织源码：`source/shaders.ixx`、`shadows.ixx`、`sunshadows.ixx`、`temporal.ixx`、`timecyc.ixx`、`timecycext.ixx`、`rpfloader.ixx`、`seasonal/`（季节性积雪/万圣节效果）、`upscaler/`（`D3D9On12Extensions.h`、`dxvk_interop.hpp`、`vulkan_minimal.h`）。
  - `source/resources/` & `shaders/`：包含大量自定义或反编译重构的 HLSL/ASM 着色器（`FXAA3_11.h`、`SMAA.hlsl`、`SSAO3_ps.hlsl`、`SunShafts_PS.hlsl`、`DeferredShadowBlur.hlsl`）。
- **核心渲染与资源技术**：
  - **着色器预热（`ShaderWarmUp`）**：针对 D3D9 驱动首次绘制时同步编译着色器导致的 20–90ms 卡顿，FusionFix 在加载界面后台使用零面积三角形预先触发所有 Effect Pass 编译（`nLoadingScreenPairs = 600`、`fLoadingScreenMs = 1500.0ms`），彻底消除游戏内着色器编译卡顿。
  - **字节码特征识别（`GetFusionShaderID`）**：通过在编译后的 DXBC 字节码中嵌入/扫描 `"FusionShader"` 签名，精确识别并替换原版像素/顶点着色器，注入 SMAA/FXAA、SSAO、体积雾、日落射线与延迟阴影模糊等现代效果，并兼容 D3D9On12 与 DXVK/Vulkan 互操作层。
  - **内存虚拟 RPF 加载器（`rpfloader.ixx`）**：完整实现 `RPF_VERSION_0` 至 `RPF_VERSION_3`（GTA IV）TOC 目录树解析与重建，支持无需修改原始 `.rpf` 文件的 Mod 虚拟挂载。

### 10. `scripthookvdotnet`（GTA V .NET 托管脚本运行时）
- **仓库路径**：`/root/wsl-demo/gta-projects/scripthookvdotnet`（`scripthookvdotnet/scripthookvdotnet`）
- **目录架构**：
  - `source/loader/`：C++/CLI 桥梁层（`DllMain.cpp`），作为 `.asi` 插件由原版 ScriptHookV 加载，并初始化 CLR 托管环境。
  - `source/core/`：核心运行时（`ScriptHookVDotNet.cs`、`ScriptDomain`、基于 Roslyn 的动态脚本编译器 `Compiler/`、日志与控制台）。
  - `source/scripting_v2/` & `scripting_v3/`：面向 Mod 开发者的强类型 .NET API（`GTA.Native`、`GTA.NaturalMotion`、`GTA.Math`、`GTA.Graphics`、`GTA.UI`、`GTA.Chrono`）。
- **运行时隔离与执行机制**：
  - 在 `ScriptHookVDotNet.Init()` 中，通过独立的 `AppDomain`（`ScriptDomain`）加载用户 `.cs`/`.vb`/`.dll` 脚本，支持热重载（`ScriptDomain.Unload` + 重新加载）而不重启游戏进程。
  - 传入并保存原生 TLS（Thread Local Storage）上下文指针（`getTlsContextFunc`、`setTlsContextFunc`、`gameThreadTlsContext`），确保托管脚本线程在调用 RAGE Native 函数时能够正确切换至游戏主线程的 `scrThread` 上下文，避免内存损坏或崩溃。

---

## 七、 横向架构对比与技术演进总结

1. **渲染管线演进路径**：
   - **经典重实现 / 修复层**：`openrw` 使用跨平台 OpenGL 3.3 Core 重建 RW 管线；`mtasa-blue` 通过 D3D9 COM 代理拦截绘制并批处理；`GTAIV.EFLC.FusionFix` 更进一步，利用 C++20 模块在 DXBC 字节码级替换着色器，引入 `ShaderWarmUp` 与 D3D9On12/DXVK 桥接。
   - **次世代 Hook 与离线重渲染**：`fivem`（`rage-graphics-five`）通过特征码与 MinHook 接管 D3D11/RAGE `grc` 状态机；`CodeWalker` 则在 SharpDX D3D11 上完整复现了 RAGE 的 Timecycle 与动态 LOD 渲染；`gta6-leonida-atlas` 在浏览器端以 Three.js/WebGL 探索轻量级程序化街景重建。
2. **网络同步架构对比**：
   - `mtasa-blue` 采用 **RakNet + 分类同步器**（`CLightsyncManager`/`CPedSync` 等）配合客户端元素树，兼顾低带宽与高自由度。
   - `fivem` 则直接逆向并劫持 RAGE 底层的 **`netSyncTree` 与 `datBitBuffer`**，构建了 **OneSync** 服务端权威状态机，突破了 GTA Online 原生 P2P 的人数与作弊瓶颈。
3. **脚本引擎与扩展性演进**：
   - 从 `openrw` / `gta-reversed` 还原的 **原生 SCM 字节码虚拟机**，演进到 `mtasa-blue` 的 **Lua 沙盒**、`scripthookvdotnet` 的 **`.NET AppDomain` + TLS 上下文切换**，最终发展为 `fivem` 的 **多语言 ScRT 微内核**（Lua / V8 / Node / Mono 统一绑定 RAGE Native 与 `scrBind`）。
4. **对未来 GTA VI（RAGE 9）开源生态的启示**：
   - 短期内（发售前），以 `gtamaplib`（摄影测量三角定位）和 `gta6-leonida-atlas`（Web GIS + Three.js 3D 地图）为代表的**社区空间测绘工具**仍是主力。
   - 长期来看（PC 版发售后），参考 `CodeWalker.Gen9Converter` 与 `Sollumz` 引入 `pymateria` 的模式，社区大概率会沿用 **“特征码运行时 Hook（FiveM 模式）+ C#/Python 资源格式逆向（CodeWalker/Sollumz 模式）”** 的双轨架构，快速适配新一代 RAGE 引擎的加密归档与渲染管线。
