# 《塞尔达传说：时之笛》逆向反编译、静态重编译与 Mod 工具链技术分析报告

> **分析范围**：基于本地工作区 `/root/wsl-demo/oot-decomp-projects/` 下 18 个开源项目的源码与文档实证分析。

---

## 1. 开源项目全景与生态图谱

本地 `/root/wsl-demo/oot-decomp-projects/` 已完整收录《塞尔达传说：时之笛》（OoT）及其衍生技术栈的核心开源仓库，按技术路线可分为五大类：

| 类别 | 本地目录 | 上游仓库 | 核心定位与技术特征 |
| :--- | :--- | :--- | :--- |
| **反编译 (Decomp)** | `oot` | `zeldaret/oot` | **主线 100% 匹配反编译项目**。将 N64/GC/iQue 多版本 ROM 还原为可编译出字节一致 ROM 的 C 源码。 |
| | `n64decomp-oot` | `n64decomp/oot` | 早期 Debug ROM（Master Quest）反编译仓库，现已迁移归档至 `zeldaret/oot`。 |
| | `oot-gc` | `zeldaret/oot-gc` | 面向 GameCube 版（Master Quest / Collector's Edition）的分支反编译。 |
| | `oot-vc` | `zeldaret/oot-vc` | 面向 Wii Virtual Console 版本的反编译。 |
| | `oot3d` | `zeldaret/oot3d` | 面向 3DS《时之笛 3D》的 WIP 反编译（目标重建 `oot3d.axf` / `code.bin`）。 |
| **N64 静态重编译** | `N64Recomp` | `N64Recomp/N64Recomp` | **核心 MIPS-to-C 静态重编译器**。含 `RSPRecomp`、`LiveRecomp`、`OfflineModRecomp`、`RecompModTool`、`RecompModMerger`。 |
| | `N64ModernRuntime` | `N64Recomp/N64ModernRuntime` | 静态重编译产物运行时，由 `librecomp`（内存/Overlay/Mod）与 `ultramodern`（libultra OS/硬件抽象）组成。 |
| | `Zelda64Recomp` | `Zelda64Recomp/Zelda64Recomp` | 基于 `N64Recomp` + `N64ModernRuntime` + `lib/rt64` 的塞尔达 64 原生 PC 重编译框架（当前主支持 `mm.n64.us.1.0`《梅祖拉的假面》，为同引擎基座）。 |
| **原生源码移植** | `Shipwright` | `HarbourMasters/Shipwright` | **Ship of Harkinian (SoH)**。基于 `zeldaret/oot` 的原生 C/C++ PC 移植，搭配 `libultraship` 底层库与 `torch` 资源提取器。 |
| | `oot-dc` | `jnmartin84/oot-dc` | 基于 `zeldaret/oot` 的 Sega Dreamcast 原生移植（SH4 + KallistiOS + PowerVR Fast3D 解释器 + AICA 音频）。 |
| **跨平台重编译探索** | `TriAevum` | `coccofresco/TriAevum` | 面向 3DS《时之笛 3D》与《梅祖拉的假面 3D》的 PC 重编译引擎（NRI/Vulkan 渲染）。 |
| | `Hookshot` | `Z-TOON/Hookshot` | 面向 Switch 2《时之笛》重制版的 AArch64 -> C 静态重编译与 NVN2 -> Vulkan 回放探索（基于 `mkw-recomp`）。 |
| | `ProjectReLink` | `pixl27/ProjectReLink` | 面向 Switch 2《时之笛》重制版的非官方 Windows 静态重编译项目。 |
| | `oot-pc-recomp` | `Chiuauaa/oot-pc-recomp` | 基于 DKB Recomp / Lichii 管线的 Switch 2 OoT 1.1.1 实验性重编译分支（重点攻关 Ampere SASS 着色器降级翻译）。 |
| **资源提取与 Mod** | `ZAPD` | `zeldaret/ZAPD` | **Zelda Asset Processor & Disassembler**，通过 XML 配置将 ROM 二进制资产提取为 C 源码/头文件。 |
| | `fast64` | `Fast-64/fast64` | Blender 插件（`fast64_internal/z64`），直接向 `oot` / `HackerOoT` 导出场景、房间、Actor、碰撞、骨骼动画与 F3D 材质。 |
| | `OoT-Randomizer` | `OoTRandomizer/OoT-Randomizer` | Python 随机化器，结合 `ASM/` 下的 `armips` 汇编与 `glankk/n64` C 工具链对原版 ROM 进行逻辑/物品/贴图注入。 |
| | `HackerOoT` | `HackerN64/HackerOoT` | 基于 `zeldaret/oot` 定制的 N64 ROM Hack 开发基底（不面向 PC 移植，专注 N64 实机/模拟器 Mod 开发）。 |

---

## 2. 《时之笛》反编译核心架构深度剖析 (`zeldaret/oot`)

### 2.1 ROM 分段规格与构建体系 (`spec/spec`)

`oot` 通过自定义规格文件 `spec/spec` 描述整张 ROM 的物理与虚拟内存布局：
- **`boot` 段**：包含早期硬件初始化与 DMA/解压模块（`boot_main.o`、`idle.o`、`z_std_dma.o`、`yaz0.o`、`cic6105.o`、以及 `libultra` / `libu64` / `libc64` 基础库），随系统启动常驻内存。
- **`code` 主段**：包含游戏核心循环、场景/房间管理、碰撞检测、Actor 管理器、音频引擎及常驻系统。
- **动态覆盖层（Overlays）**：为受限于 N64 4MB/8MB RDRAM 容量，绝大多数 `GameState`（除 `Setup` 与 `Play` 外）以及 470+ 个 Actor 均被编译为独立的 ELF/Overlay 段，运行时按需通过 DMA 载入并执行重定位。

### 2.2 GameState 状态机与主图形循环

核心状态机定义于 `oot/include/game.h` 与 `oot/include/tables/gamestate_table.h`，由图形线程 `Graph_ThreadEntry`（`oot/src/code/graph.c`）驱动：

1. **六大顶层状态（`gamestate_table.h`）**：
   - `GAMESTATE_SETUP`（常驻内部，初始化系统）
   - `GAMESTATE_MAP_SELECT`（Overlay `select`，调试地图选择）
   - `GAMESTATE_CONSOLE_LOGO`（Overlay `title`，N64 Logo）
   - `GAMESTATE_PLAY`（常驻内部，核心 3D 游玩状态 `PlayState`）
   - `GAMESTATE_TITLE_SETUP`（Overlay `opening`，标题过场）
   - `GAMESTATE_FILE_SELECT`（Overlay `file_choose`，存档选择）

2. **状态生命周期与内存隔离（`GameState` 结构体，`0xA4` 字节）**：
   - 每个状态持有独立的 `TwoHeadArena tha`（双端竞技场分配器）、`GameAlloc alloc`、`Input input[MAXCONTROLLERS]` 以及 `init` / `main` / `destroy` 函数指针。
   - 状态切换通过宏 `SET_NEXT_GAMESTATE(curState, newInit, newStruct)` 设置下一状态的 `init` 与实例大小，并将 `state->running = false`。
   - `Graph_ThreadEntry` 在外层 `while (nextOvl != NULL)` 循环中完成：**加载新 Overlay (`Overlay_LoadGameState`) -> `SYSTEM_ARENA_MALLOC` 分配状态结构体 -> `GameState_Init` -> 内层帧循环 `while (GameState_IsRunning(gameState)) { Graph_Update(&gfxCtx, gameState); }` -> `GameState_Destroy` -> `SYSTEM_ARENA_FREE` -> `Overlay_FreeGameState`**。
   - 在 `PlayState` 内部，`Play_Main`（`oot/src/code/z_play.c`）每帧依次执行 `Play_Update(this)` 与 `Play_Draw(this)`。

### 2.3 Actor 系统与 C 语言面向对象状态机

OoT 的所有交互实体（玩家、敌人、NPC、宝箱、机关、特效）均统一抽象为 **Actor**（`oot/include/actor.h`、`oot/include/actor_profile.h`、`oot/src/code/z_actor.c`）：

1. **分类链表管理（`ActorContext`）**：
   - `ActorContext`（`0x140` 字节）维护 `actorLists[ACTORCAT_MAX]`（共 12 个分类：`ACTORCAT_SWITCH`、`BG`、`PLAYER`、`EXPLOSIVE`、`NPC`、`ENEMY`、`PROP`、`ITEMACTION`、`MISC`、`BOSS`、`DOOR`、`CHEST`）。
   - 每个 `ActorListEntry` 保存该分类的 `length` 与双向链表头指针 `head`，通过 `Actor.prev` / `Actor.next` 串联。

2. **C 语言“继承”与虚表（`Actor` + `ActorProfile`）**：
   - 基类 `Actor`（`0x14C` 字节，Debug 含 `0x10` 字节 `dbgPad`）包含：`id`、`category`、`room`（`-1` 表示跨房间不销毁）、`flags`（注意力/锁定、敌我、剔除开关、真实之镜响应 `ACTOR_FLAG_REACT_TO_LENS`、钩爪交互等）、`home/world/focus` 位置旋转、物理属性（`velocity`/`speed`/`gravity`）、地面/墙面碰撞状态（`bgCheckFlags`、`floorPoly`/`wallPoly`）、与玩家相对距离/偏航角、`colChkInfo`、`ActorShape`、视锥剔除参数（`cullingVolumeDistance/Scale/Downward`）以及 `init`、`destroy`、`update`、`draw` 四个生命周期函数指针。
   - 派生类通过首成员嵌入实现内存布局兼容，例如：`Actor` -> `DynaPolyActor`（带动态地形碰撞 `bgId`）-> `EnBox`（宝箱，`z_en_box.h`）。
   - 每个 Overlay 导出静态描述结构体 `ActorProfile`（`0x20` 字节：`id`、`category`、`flags`、`objectId`、`instanceSize`、`init`、`destroy`、`update`、`draw`）。

3. **Overlay 动态加载策略（`z_actor_dlftbls.h`）**：
   - `ActorOverlay` 定义了三种内存分配策略：
     - `ACTOROVL_ALLOC_NORMAL`（`0`）：按需动态分配，无实例使用时释放；
     - `ACTOROVL_ALLOC_ABSOLUTE`（`1 << 0`）：加载至 `ActorContext.absoluteSpace` 固定共享缓冲区（同一时刻仅允许加载一个）；
     - `ACTOROVL_ALLOC_PERSISTENT`（`1 << 1`）：持久驻留直至当前 `GameState` 结束。

4. **帧更新与渲染管线（`Actor_UpdateAll` / `Actor_DrawAll`）**：
   - **`Actor_UpdateAll`**：
     1. 生成待实例化列表（`play->actorEntryList`）；
     2. 依据玩家状态计算分类冻结掩码（`sCategoryFreezeMasks`），在吹奏时之笛（`PLAYER_STATE2_USING_OCARINA`）或对话时仅放行豁免 Actor（如带有 `ACTOR_FLAG_UPDATE_DURING_OCARINA` 或当前对话对象）；
     3. 对 `actor->init != NULL` 的实体，检查其依赖的资产 Bank 是否就绪（`Object_IsLoaded(&play->objectCtx, actor->objectSlot)`），就绪后才调用 `actor->init` 并置空；
     4. 若 `actor->update == NULL` 则触发 `Actor_Destroy` / `Actor_Delete` 回收；否则更新与玩家的距离/角度，在未冻结且位于剔除体积内（或禁用更新剔除）时执行 `actor->update(actor, play)`。
   - **`Actor_DrawAll`**：
     - 将世界坐标投影到屏幕空间（`SkinMatrix_Vec3fMtxFMultXYZW`），执行 `Actor_CullingCheck` 更新 `ACTOR_FLAG_INSIDE_CULLING_VOLUME`；
     - 若带有 `ACTOR_FLAG_REACT_TO_LENS` 且当前需延迟渲染，则暂存至 `invisibleActors[INVISIBLE_ACTOR_MAX]`（上限 20 个）延后绘制；其余调用 `Actor_Draw(play, actor)`。

5. **典型 `actionFunc` 状态机（以 `ovl_En_Box` 为例）**：
   - 在 `EnBox` 中，内部状态机通过函数指针 `EnBoxActionFunc actionFunc` 实现，`EnBox_SetupAction(this, newFunc)` 完成状态迁移（如 `EnBox_FallOnSwitchFlag` -> `EnBox_AppearInit` -> `EnBox_AppearAnimation` -> `EnBox_WaitOpen` -> `EnBox_Open`）。`EnBox_Update` 每帧只需调用 `this->actionFunc(this, play)`，极大降低了分支复杂度且天然适配 Overlay 代码组织。

### 2.4 RSP/RDP 微码与显示列表管线

- **微码选型（`oot/src/code/sys_ucode.c`）**：
  - 引导微码：`rspboot`（`SysUcode_GetUCodeBoot()`）
  - 图形微码：`gspF3DZEX2_NoN_fifo`（或 `PosLight` 变体 `gspF3DZEX2_NoN_PosLight_fifo`，不支持近平面裁剪 `NoN`）与 2D 精灵微码 `gspS2DEX2d_fifo`。
- **双缓冲与四通道 Arena（`oot/src/code/graph.c`）**：
  - `Graph_InitTHGA` 使用双缓冲 `gGfxPools[gfxCtx->gfxPoolIdx & 1]`，为每帧初始化四个 `TwoHeadGfxArena`（`TwoHeadGfxArena` 头部向后写 GBI 指令，尾部向前分配矩阵/顶点动态数据）：
    - `work`（`WORK_DISP`）：根显示列表与帧缓冲/视口初始化；
    - `polyOpa`（`POLY_OPA_DISP`）：不透明几何体；
    - `polyXlu`（`POLY_XLU_DISP`）：半透明几何体；
    - `overlay`（`OVERLAY_DISP`）：HUD、UI 与覆盖元素。
  - `Graph_Update` 结束时通过 `gSPBranchList` 将四条链首尾相接（`work -> polyOpa -> polyXlu -> overlay`），追加 `gDPPipeSync`、`gDPFullSync`、`gSPEndDisplayList`，检查 `GFXPOOL_HEAD_MAGIC`/`TAIL_MAGIC` 与 Arena 溢出后，由 `Graph_TaskSet00` 打包为 `M_GFXTASK` 类型的 `OSScTask`（含 `OS_SC_NEEDS_RSP | OS_SC_NEEDS_RDP | OS_SC_SWAPBUFFER`），发送至 `gScheduler.cmdQueue`。

---

## 3. N64 静态重编译管线架构 (`N64Recomp` + `N64ModernRuntime` + `Zelda64Recomp`)

与传统模拟器的动态重编译（JIT）或逐句解释不同，`N64Recomp` 生态采用 **AOT（Ahead-Of-Time）静态重编译 + 原生现代图形/OS 运行时** 方案：

### 3.1 `N64Recomp`：MIPS-to-C 静态翻译引擎

基于 `rabbitizer`（MIPS 反汇编/分析）、`ELFIO`（ELF 解析）、`toml11` 与 `fmt` 构建，核心流水线如下（`N64Recomp/src/recompilation.cpp`）：

1. **元数据与分段解析**：读取配置文件（TOML）与目标 ELF 符号表，将静态段与可重定位 Overlay 段切分为独立函数单元。
2. **指令级字面翻译**：逐条将 MIPS 指令转换为操作 `recomp_context* ctx` 的 C 语句（例如 `addiu $r4, $r4, 0x20` 翻译为 `ctx->r4 = ADD32(ctx->r4, 0X20);`），并复制处理分支延迟槽（Branch Delay Slots）。
3. **控制流与函数调用解析（`resolve_jal`）**：
   - `jal` 若命中同一 Section 或唯一非重定位目标，直接生成静态 C 函数调用；
   - 若目标位于可重定位 Overlay 且存在地址二义性，或遇到 `jalr` 虚函数/函数指针调用，则生成 `LOOKUP_FUNC(ctx->r25)(rdram, ctx)`，交由运行时按当前已加载的 RAM 地址动态查表。
   - `jr` 配合特征识别（`jtbl_lw_instructions`）可自动恢复为 C 的 `switch-case` 跳转表。
4. **Overlay 重定位宏注入**：针对可重定位段中的 `lui`、`addiu` 及访存指令，注入 `RELOC_HI16(section_idx, offset)` 与 `RELOC_LO16` 宏，在运行时根据 `section_addresses[section_idx]` 动态修正基址。
5. **Single File Output 模式**：可将补丁函数（如帧率解锁、宽屏修正、Bug 修复）编译为单文件 C 代码并置于链接器优先顺序，利用标准链接器符号覆盖机制直接替换原版重编译函数，无需重新重编译整个游戏。

### 3.2 `N64ModernRuntime`：内存虚拟化与硬件抽象

由 `librecomp` 与 `ultramodern` 两层构成：

- **`librecomp`（内存、Overlay 与 Mod 层）**：
  - **地址空间映射（`addresses.hpp`）**：预留 4GB 虚拟地址空间与 512MB KSEG0 物理映射窗口，在扩展 RDRAM 中划出专用区域：`cart_handle = 0x80800000`、`patch_rdram_start = 0x80801000`、`mod_rdram_start = 0x81000000`，确保原版游戏指针与扩展内存/Mod 数据互不冲突。
  - **动态 Overlay 管理（`overlays.cpp`）**：提供 `load_overlays`、`unload_overlays`、`load_overlay_by_id`、`unload_overlay_by_id`。当游戏通过 DMA 搬运代码段或卸载内存区域时，运行时同步更新 `section_addresses[]` 并维护全局 `func_map`（`VRAM -> recomp_func_t*`）。
- **`ultramodern`（libultra 与系统调度层）**：
  - 原生实现 OS 线程队列（`threads.cpp` / `threadqueue.cpp`）、消息队列（`mesgqueue.cpp`，对应 `osSendMesg`/`osRecvMesg`）、高精度定时器（`timer.cpp`）、VI/AI/PI/SI 寄存器模拟及音频/输入后端。

### 3.3 RSP 微码的双轨处理：`RSPRecomp` 与 `RT64`

在 `Zelda64Recomp/src/main/main.cpp` 与 `rt64_render_context.cpp` 中，RSP 任务按 `OSTask.t.type` 分流为两条完全不同的执行路径：

1. **音频与 JPEG 微码 -> `RSPRecomp` 静态重编译（LLE 级还原）**：
   - `N64Recomp/RSPRecomp/src/rsp_recomp.cpp` 将 `M_AUDTASK`（`aspMain`）与 `M_NJPEGTASK`（`njpgdspMain`）等 RSP 微码直接静态重编译为 C++ 函数（`RspUcodeFunc`）。
   - 运行时通过 `RSP_MEM_B` / `RSP_MEM_W_LOAD` 处理 4KB `dmem` 的大端异或寻址（`^ 3`），通过 `dma_rdram_to_dmem` / `dma_dmem_to_rdram` 完成 DMA 搬运，并通过 `rsp_vu.hpp` 模拟 128-bit 向量单元指令（`vmacf`、`vmadh`、`vadd` 等）。
2. **图形微码 -> `RT64` HLE 现代渲染管线**：
   - `RT64Context` 将 `RDRAM`、`DMEM`、`IMEM` 以及 `DPC_*` / `VI_*` 寄存器指针直接绑定至 `RT64::Application::Core`。
   - `RT64` 的 `Interpreter`（`lib/rt64/src/hle/rt64_interpreter.cpp`）识别加载的微码类型（如 F3DZEX2），直接解析内存中的 GBI 显示列表（`processRDPLists`），在 **D3D12 / Vulkan / Metal** 上实现原生高分辨率渲染、MSAA 抗锯齿、高刷新率矩阵/骨骼插值、宽屏视口扩展、高精度帧缓冲与高清纹理包替换。

---

## 4. 两条 PC 化路线对比：`Shipwright` (源码移植) vs `Zelda64Recomp` (静态重编译)

| 维度 | `Shipwright` (Ship of Harkinian) | `Zelda64Recomp` (`N64Recomp` 体系) |
| :--- | :--- | :--- |
| **代码来源** | 基于 `zeldaret/oot` 人工反编译的 C 源码，重构并原生编译为 x64/ARM64 可执行文件。 | 直接输入原版 N64 ROM 与 ELF 符号，经 `N64Recomp` 自动翻译为 C 代码后编译。 |
| **资产提取** | 使用 `torch` 在首次启动时从 ROM 提取资产并打包为 `.otr` / `.o2r` 归档文件。 | **无需预提取资产**，运行时直接挂载压缩/解压后的 ROM 并通过 `RDRAM` 模拟 DMA 读取。 |
| **图形后端** | `libultraship` 内的 Fast3D 转换器（支持 OpenGL / DirectX 11 / Metal）。 | `RT64` 现代 HLE 引擎（Vulkan / D3D12 / Metal），原生支持帧插值、光线增强与高精度 FB。 |
| **兼容性与 Mod 生态** | 可深度修改 C 源码实现大规模玩法魔改（Built-in Randomizer、多人联机、自由视角），但与 N64 原版 ROM Hack 不兼容。 | **天然兼容 N64 原生 ABI**；配合 `RecompModTool` 与 `LiveRecomp` 支持动态加载 Mod，且补丁开发周期极短。 |
| **可移植性** | 高（只要有对应平台 C/C++ 编译器与 `libultraship` 后端即可，如 `oot-dc` 甚至移植到了 Dreamcast）。 | 高度依赖 `N64ModernRuntime` 与 `RT64`，但可快速复用到任意已具备符号的 N64 游戏。 |

---

## 5. 资源提取与 Mod 工具链 (`ZAPD` / `fast64` / `OoT-Randomizer` / `HackerOoT`)

1. **`ZAPD` (`zeldaret/ZAPD`)**：
   - `oot` 构建链的核心资产提取器。依据 `assets/xml/` 下的 XML 描述文件，从 Baserom 中解析纹理（RGBA16/CI4/CI8/IA8/I4）、DisplayList、顶点缓冲、碰撞网格（`ZCollision`）、骨骼与关键帧动画（`ZSkeleton` / `ZAnimation`）、过场（`ZCutscene`）及房间/场景命令（`ZRoom`），生成供 C 编译器直接 `#include` 的头文件与源数组。
2. **`fast64` (`Fast-64/fast64`)**：
   - Blender 插件（`fast64_internal/z64/`），是当前 N64 OoT 3D 资产与关卡制作的标准生产力工具。
   - 支持 F3DZEX2 节点材质编辑、场景/房间 Empty 层级结构管理、Actor 与参数可视化配置、Room Object 依赖表管理（上限 15 个）、动态材质脚本（`Draw Config` / `Gfx_TexScroll`）、碰撞属性（水体、相机、退出点）以及骨骼/LOD 网格导出，一键输出至 `oot` 或 `HackerOoT` 源码树。
3. **`HackerOoT` (`HackerN64/HackerOoT`)**：
   - 基于 `zeldaret/oot` 的 N64 ROM Hack 专用基底（支持 `ntsc-1.0` / `ntsc-1.1` 等），内置了更友好的调试工具、扩展内存配置与与 `fast64` 的深度集成特性（如动态材料、自定义 Actor 快速注册）。
4. **`OoT-Randomizer` (`OoTRandomizer/OoT-Randomizer`)**：
   - 采用“**Python 逻辑求解器 + 原位二进制/ASM 补丁**”架构：顶层 Python（`World.py`、`Fill.py`、`RuleParser.py`、`EntranceShuffle.py`）计算无死锁的随机化物品与入口映射；底层通过 `ASM/` 目录下的 `armips` 汇编器与 `glankk/n64` MIPS GCC 工具链编译 C/ASM 补丁模块（`ootSymbols.ld` 绑定原版函数地址），直接注入解压后的 N64 ROM（`Rom.py` / `N64Patch.py`），实现实机与全模拟器的无缝兼容。

---

## 6. 跨平台/次世代衍生探索 (`oot3d` / `TriAevum` / `Hookshot` / `ProjectReLink` / `oot-pc-recomp`)

除了成熟的 N64 技术栈外，工作区中还包含两条面向其他官方版本的重编译前沿探索：
- **3DS 版 (`oot3d` / `TriAevum`)**：`zeldaret/oot3d` 致力于还原 3DS 版 `code.bin` 的 ARM C++ 源码；`TriAevum` 则直接将 OoT3D 与 MM3D 静态重编译至 PC，并使用现代 NRI/Vulkan 渲染器提供阴影与光照增强。
- **Switch 2 重制版 (`Hookshot` / `ProjectReLink` / `oot-pc-recomp`)**：围绕 Switch 2 平台（ARMv8.2+ Cortex-A78C、DT_RELR 重定位、NVN2 图形 API 与 Ampere SM86 SASS 着色器），探索将 AArch64 二进制静态提升（Lift）为 C 并把 NVN2/Ampere 着色器降级翻译为 Vulkan SPIR-V 的可行路径（目前仍处于早期渲染管线攻关阶段）。
