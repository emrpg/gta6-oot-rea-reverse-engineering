
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
