## 3.8 GTAIV.EFLC.FusionFix（ThirteenAG）—— RAGE 时代的 PC 移植修复

### 项目定位

本项目是 GTA IV / Episodes From Liberty City（EFLC）PC 移植的 `.asi` 修复/还原插件，作者 ThirteenAG，GPL-3.0，本地快照 HEAD 提交时间 2026-11-17。与 re3/openrw 这类 RenderWare 时代的"重写引擎"不同，它**不重写任何引擎**：RAGE 的可执行文件保持原样，插件在运行时把逆向出来的类布局、指令模式与着色器字节码接进去。因此它是本报告中最纯粹的"活体逆向"样本——每一个 fix 都必须回答"这条指令在哪个函数里、这个字段在对象第几字节"。仓库 3,362 个文件 / 约 314 MB（不含 `.git`），本地为 shallow clone（`git log` 只有 1 条 squash 提交，含全部 3,368 个跟踪文件），因此无法按提交历史统计作者分布。

### 元数据与授权真相

`license` 为完整 GPLv3 正文（35,149 字节），`premake5.lua:65` 把 `rsc_LegalCopyright="GPL-3.0 license"` 写进 PE 版本资源。但 `contributing.md:7-25` 另附一份 Fusion Fix CLA：贡献者授予 Owner "可再许可、可出售、**包括开源与专有及商业条款**"的不可撤销版权许可（`contributing.md:11`），即 GPL 并非不可绕过的边界。法律自述集中在 `readme.md:72`：直接引用 Take-Two 的 PC 单人模组声明，明确"多人/在线服务不受保护"，并规定"分叉本專案或任何用于多人的衍生软件均为禁止"（`readme.md:72`）；`readme.md:75` 声明不为盗版提供支持。项目必须有正版游戏本体才能运行（安装到 exe 同级目录，`readme.md:32`），但它**同时重新分发了大量由游戏资源衍生并修改后的 RAGE 二进制资产**（`data/` 734 个文件 182 MB 的 `.wdr/.sco/.wft/.wad/.wtd`，`textures/` 410 个 `.dds` 57 MB，见 `buildwtd.bat`、`buildimg.bat`），这部分才是真正有版权风险的资产，README 未就此单独免责。

### 源码 vs 二进制：一张诚实的表

| 类别 | 路径 | 文件数 | 规模 | 可读性判定 |
|---|---|---|---|---|
| C++20 模块接口（fix 实现主体） | `source/*.ixx`（含 `source/seasonal/`、`source/aa/`） | 62 | **50,222 行 / 2.6 MB** | 自有源码，信息密度最高 |
| 插件 C++ 主体 | `source/dllmain.cpp`(187)、`Overlay.cpp`(286)、`d3d9/d3d9.cpp`(153) | 3 | 626 行 | 自有源码 |
| 项目自有头 | `source/maths.hxx`(603)、`upscaler/*`、`aa/Upscaler.hpp`+`UpscalerD3D9.inl` | ~15 | ~4,400 行 | 自有源码 |
|  vendored 微软 DXSDK 头 | `source/dxsdk/*.h` | 10 | 10,886 行 | 第三方，非 RE 产物 |
| vendored GXT 工具源码 | `source/gxt/src/*` | 19 | 1,827 行 | 第三方工具 |
| fxc 反汇编的着色器字节码 | `shaders/.../win32_30_nv8/**/*.asm` | 1,749 | **312,287 行 / 8.9 MB** | 机器生成，但带人工 `// Summary:` 说明 |
| HLSL / .fx 自研着色器 | `source/resources/*.{hlsl,fx,ps,vs}`、`shaders/.../*.asm` 源 | 27 | 6,087 行 | 自有源码 |
| 配置/本地化（数据即代码） | `settings/*.json` 8 份 2,915 行、`text/*FF.txt` 7 份 | 15 | — | 自有 |
| 二进制游戏资产 | `data/`（182 MB）、`textures/`（57 MB） | 1,144 | 239 MB | 不可读 |
| PE 二进制与工具 | `tools/**/*.exe|dll`、`source/dxsdk/lib/**`、`premake5.exe` | 22 | ~40 MB | 不可读（含 `RageShaderEditor.exe`、`satup.exe`、`gxt.exe`、`fftdc.exe`、`fxc.exe`、`fastman92ImgConsole32.exe`） |
| **空 submodule（关键缺失）** | `external/{injector,hooking,inireader,FusionFixUtils,Vulkan-Headers}`、`shaders/GTAIV.EFLC.FusionShaders` | **0** | 0 | `.gitmodules:1-20` 声明 6 个子模块；本快照全部为空目录 |

结论：**可自有源码约 55,000 行，仅占文件数的 2.1%、体积的不到 1%**；仓库 97% 是二进制资产、第三方工具和机器生成的着色器反汇编。更关键的缺失是 hook 基础设施本身（`injector`、`Hooking.Patterns`、`FusionFixUtils` 里的 `gameref.hpp`、`asi_settings`、`upscaler_bridge_client.h`、ImGui overlay）都在子模块里，浅克隆后一行都看不到——本报告只能从调用点反推其语义「推断」。

### 语言、标准与构建系统

`premake5.lua:46-57`：`architecture "x86"`、`kind "SharedLib"`、`targetextension ".asi"`、`cppdialect "C++latest"`，即 **MSVC x64 主机交叉出 32 位插件 + C++20/23 前沿标准 + 模块（modules）**。这是本仓库最反直觉的工程选择：每个 fix 是一个 `export module xxx;`（59 个，`grep '^export module' source/*.ixx`），通过 `import common/comvars/natives/settings;` 组成显式依赖图，而非传统头文件包含；`.ixx` 是 MSVC 模块接口文件后缀，**与"自定义着色器容器"无关**（任务描述中的这一假设需要纠正）。着色器另有两条独立管线：HLSL 由 `fxc.exe` 编译（`tools/compileshaders.ps1`，`source/resources/shaders.lock` 用 SHA-256 记录输入/输出哈希，CI 用 `-Force -Verify` 保证字节码与源码一致，`appveyor.yml:13`），RAGE 着色器由 `RageShaderEditor.exe` 从 `.xml + .asm` 重新汇编为 `.fxc`（`buildshaders.bat:8-14`）。构建系统为 premake5 → `.slnx`（`appveyor.yml:15`，VS2026）+ GitHub Actions `.github/workflows/msvc_x86.yml` + AppVeyor。语言构成：C++（约 55k 行）、内联汇编 `__declspec(naked)` + `_asm`（`source/d3d9/d3d9.cpp:34-101`，17 个转发桩）、HLSL/`.fx`（ps_3_0/vs_3_0/fx_2_0）、PowerShell/Batch/Lua。依赖清单见 `external/README.md:1-7`：crosire 系 `injector`、ThirteenAG 的 `Hooking.Patterns`/`IniReader`/`FusionFixUtils`、`Vulkan-Headers`、内嵌的 `safetyhook` + `Zydis`（`premake5.lua:141-150`）、DXVK 相关头（`source/upscaler/dxvk_interop.hpp`）、`LogitechLEDLib.lib`、ImGui（经 FusionFixUtils）。

### RE 产物一：注入机制

`.asi` 就是改名 DLL，由 **Ultimate ASI Loader 的 `dinput8.dll`** 载入：CI 下载 `dinput8-Win32.zip` 解到 `data/`（`appveyor.yml:21`、`msvc_x86.yml:82,147`），legacy 版本则把它复制为 `xlive.dll`（`appveyor.yml:28`）。插件本身不代理任何系统 DLL，而是导出 `InitializeASI`：`source/dllmain.cpp:165-175` `void __declspec(dllexport) InitializeASI()` → `CallbackHandler::RegisterCallback(Init, hook::pattern("F3 0F 10 44 24 ? ..."))`，即**延迟到某条特征码命中时才初始化**；`source/dllmain.cpp:181` 的 `if (!IsUALPresent()) { InitializeASI(); }` 处理无 loader 的情况。唯一的 `.def` 是 `source/d3d9/d3d9.def`（18 个 `@ord` 转发导出，含 `FusionFixPassThroughD3D9 = _FusionFixPassThroughD3D9 @100`），它**已经不再承担注入**：`source/d3d9/d3d9.cpp:3-11` 注释说明这只是把旧版图形 API 代理 `d3d9.dll` 覆盖成无害转发件（17 个 `__declspec(naked)` + `_asm { jmp [...] }` 桩，`:34-101`，全部 `GetProcAddress` 到 System32/SysWOW64 的真 `d3d9.dll`）。真正的 API 切换走 **IAT 改写**：`source/upscaler/graphics_api.h:1650-1681` 遍历每个已加载模块的 `IMAGE_DIRECTORY_ENTRY_IMPORT`/`OriginalFirstThunk`/`FirstThunk`，在 `:1586-1592` 用 `VirtualProtect` + 覆写 `u1.Function` 把所有 `Direct3DCreate9(Ex)` 指向插件函数——这与 MTA 的离线 PE 导入表重写是同一种技术在运行时版本。`source/compat.ixx:34-49` 用 `ModuleList` 枚举模块、检测 `dsound.dll` 是否被 `0x25FF`（`jmp [imm32]`）挂钩，是典型的"代理 DLL 冲突自检"。

### RE 产物二：Hook API 与数量

三套并存的安装器：`safetyhook::create_mid`（**290** 处，指令级中段 hook）、`safetyhook::create_inline`（**125**）、`injector::MakeCALL`（**89**）、`injector::MakeInline`（**59**）、`injector::MakeJMP`（**12**），合计 **575 个 hook 安装点**；另有 `injector::MakeNOP` 292、`injector::WriteMemory` 132 处纯字节补丁。MinHook、MS Detours、Hunt 均**未使用**（仓内 `MH_CreateHook`/`SetHooks`/`DetourAttach`/`Hunt::` 命中数为 0）；`safetyhook` + `Zydis`（指令长度解码）随 injector 子模块内嵌（`premake5.lua:141,149-150`）。特征码侧：`hook::pattern(` 827 次、`find_pattern(` 408 次；按 `"[0-9A-F]{2}( ?|\?\?|hex)"` 三段以上严格匹配统计，源码中共 **1,703 处特征码字符串字面量、去重后 1,681 条**，分布在 52 个模块。密度最高的模块：

| 模块 | 特征码 | hook 点 | 说明 |
|---|---|---|---|
| `settings.ixx` | 176 | 76 | 把新选项注入游戏自己的菜单页 |
| `comvars.ixx` | 140 | 10 | 解析并导出全局变量/单例地址（类布局总表） |
| `episodiccontent.ixx` | 137 | 4 | DLC 内容与脚本解锁（`CEpisodes::isAvailable` 改写，`:593-605`） |
| `fixes.ixx` | 135 | 46 | 玩法/脚本级修复集合 |
| `nightshadows.ixx` | 95 | 56 | 夜间动态阴影 |
| `rawinput.ixx` | 62 | 56 | Raw Input / 鼠标灵敏度 |
| `frameratevigilante.ixx` | 68 | 57 | 帧率无关的物理/动画/计时 |
| `limits.ixx` | 47 | 13 | pool/budget 扩容 |
| `memory.ixx` | 42 | 35 | 地址空间与流送预算 |

### RE 产物三：签名机制的可迁移性

三种风格：(a) 单一模式 + 直接取位移，`source/dllmain.cpp:56-57` `hook::pattern("E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? ...")` 命中后 `injector::MakeCALL(pattern.get_first(0), CGameProcessHook, true)`——用一串 CALL 的"骨架"定位 `CGame::Process`；(b) **多候选 `find_pattern`**（同一逻辑点在不同 build 有不同字节），`source/fixes.ixx:512` `find_pattern("8B 88 ? ? ? ? 8D 49 ? E8 ? ? ? ? 8B 4E 04 89 44 24 0C", "8B 80 24 02 00 00 8D 48 44 E8 ? ? ? ? 8B 4E 04 89 44 24")`；(c) **把模式当作反查索引**，`source/postfx.ixx:1964` 用 `hook::pattern("6A 00 FF B7 ? ? ? ? FF 77 18 E8 ? ? ? ? FF 77 18 8B 4F 60")` 在偏移 11 处取 CALL 替换成 `DrawCallDownsample`。最精巧的是 `source/shaders.ixx:29-48`：插件对**已编译的着色器字节码**做 `hook::range_pattern` 搜索自家人工植入的 `"FusionShader"` 标记，读出紧跟其后的 int 作为 shader ID（VS 再减 2000），从而在不依赖符号的情况下识别"这是第 27 号 gta_default PS"——这是把签名扫描反向用于自我标识。注意：**全仓没有 `0x140xxxxxx` 形式的 x64 模块基址+RVA 常量**——24 处 `0x14xxxxxx` 字面量全部是 `natives.ixx` 里的 32 位脚本 native 哈希（如 `ARE_MEASUREMENTS_IN_METRES = 0x145D8F6D`）与 CRC32 表项，因为游戏是 32 位、ASLR 关闭；插件也**几乎不写死绝对地址**（非哈希的 `0x0xxxxxxx` 常量只有 5 个），而是把 IDA 名保留为变量名、在运行时由签名解析出真地址（`source/comvars.ixx:2521-2524` 声明 `CReplayMgr::dword_11F7060 = nullptr`，`:3511` 用 `find_pattern("83 3D ? ? ? ? ? F3 0F 10 05 ...")` 的位移 `get_first<uint32_t*>(2)` 填它）。位移靠 `pattern.get_first<k>()` 与 `resolve_next_displacement()`（`source/autoladder.ixx:643`）现场解出。**这套方法完全可迁移到 x64**：多候选模式、位移提取、`range_pattern` 自标记都不依赖 ISA，唯一要补的是 64 位绝对寻址的 RIP-relative 解码。

### 版本处理：没有版本表

本项目**不存在**按 build number 索引的偏移表。版本区分靠三层：(1) 签名存在性分派——`if (!pattern.empty()) // 1.2.0.32, 1.2.0.43, 1.2.0.59` / `else // 1.0.7.0, 1.0.8.0, 1.1.2.0, 1.1.3.0`，共 223 处 `pattern.empty()` 分支、456 条版本注释（`source/shadows.ixx:233,249`、`source/frameratevigilante.ixx:725,749`、`source/natives.ixx:6487,6501` 等），本质是"用字节指纹代替版本号"；(2) 结构体布局的 `legacy` 布尔，如 `source/comvars.ixx:791-803` 用 `preRender.empty()` 判定老 build，再切换 `0x5B/0x56`、`0x1EE/0x1E0` 等字段偏移；(3) 运行时环境探测，`source/compat.ixx:34-80` 检查 UAL 是否存在、`xlive` 是否为 UAL 改名（决定是否需要 Legacy Addon），`source/extrainfo.ixx:11-45` 才用 `GetFileVersionInfoW` 读出 `x.x.x.x-GitHash`——但**仅用于 overlay 显示，不参与分派**。完整 Edition 与 Steam 版被视为同一目标（只有 1.2.0.x 系列），差异只体现在 `.def`/`dinput8.dll` 的装载路径上「推断」。

### 修复内容的三个深入样本

**（1）帧率解除与时间步长统一**：`source/framelimit.ixx:19-60` 实现高精度帧限（`CreateWaitableTimerExW(CREATE_WAITABLE_TIMER_HIGH_RESOLUTION)` + 尾部自旋，槽位相互锚定防漂移）；`source/frameratevigilante.ixx` 才是主体——它不改帧率上限，而是**逐个修正假设 30 FPS 的代码**：`:41-42` `CPhysics::PreSimUpdate(*CTimer::fTimeStep * v3, i)`，`:97,103` 把水体速度按 `fTimeStep / (1/30)` 归一，`:247-248,328-332` 用 `fCamTimeStep * 30.0f` 修正相机，`:121` 给 `m_fTimeStepAccumulator` 补累积器，覆盖过场动画同步、警察掩护 AI、脚本计时器（README `:169-173`）。

**（2）内存/地址空间（不是 LAA 补丁）**：源码中没有任何 PE 头 `IMAGE_FILE_HEADER.Characteristics` 修改（`grep LARGE_ADDRESS_AWARE` = 0），`source/memory.ixx:243-258` 解释原因——"GTA IV 是 32 位进程，先耗尽的是地址空间而非显存；游戏每张流送的贴图都在系统内存留一份副本"，于是插件**在运行时按空闲地址空间动态缩放流送预算**：单独线程遍历内存区域（`:334` 注释"约一万个 region"），维护 `nCriticalRoom/nTightRoom/nRaiseRoom` 三档（`:264-266`），`GetAvailableFreeMemory` 被 inline hook（`:496-501`），配合 `:899-903` 把大贴图系统内存副本换成按需映射的 page-file section（保留 64 KB 永不映射的空洞以防误判为指针）。README `:186` 的"约 500 MB 更多流送内存"即此项；另有 `source/limits.ixx:144-202` 扩 pool/car budget（注释明确警告"Increasing all pools is not safe"）。

**（3）后处理与着色器替换**：`source/postfx.ixx` 在 `PostFX()` 构造里（`:1954-1985`）hook 游戏的 postfx 绘制调用链（`DrawPrimitivePostFX`/`DrawSky`/`DrawCallPostFX`/`DrawCallDownsample`/`DrawCallFog`），并在 `:240-260` 用 `FindResourceW(hm, MAKEINTRESOURCEW(id), RT_RCDATA)` + `LoadResource`/`LockResource` + `pDevice->CreatePixelShader` 把内嵌的 FXAA/SMAA/DOF/sunshafts/AO 字节码建为真 D3D9 shader；`.fx/.pso/.vso/.fxo` 由 `source/resources/Shaders.rc` 以 `IDR_*` 资源 ID 打进 `.asi`。`source/shaders.ixx:264-338` 则用 `rage::grmShaderInfo::registerShaderParam("gta_atmoscatt_clouds.fxc", "AzimuthColor")` **按名字向游戏的 shader 参数表注册槽位**，再 `SetPixelShaderConstantF(212, ...)` 直接写入寄存器——等于接管了 RAGE 的常数寄存器分配。`source/hdr.ixx:26-42` 用同样思路做 HDR 输出（A16B16G16R16F 反 buffers + DXVK swapchain 扩展 + scRGB 色调映射），`source/temporal.ixx:813-814` + `source/aa/Upscaler.hpp:22-25` 通过命名管道 `Local\GTAIV.EFLC.FusionFix.Upscaler` 把 DLAA/FSR 外包给 FusionFixHub 进程（厂商运行库不在本仓，`external/README.md:5-6`）。其余子系统按模块一一对应：`widescreenfix.ixx`（FOV/边框/网页视口，`:12-45`）、`rawinput.ixx`、`windowed.ixx`、`skipintro.ixx`、`led.ixx`、`turnindicators.ixx`、`cheats.ixx`、`seasonal/*`、`gxtloader.ixx`/`imgloader.ixx`/`rpfloader.ixx`（经 UAL 的 `GetOverloadPathW`/`AddVirtualFileForOverloadW` 做虚拟文件覆盖，`source/comvars.ixx:3076-3079`、`source/imgloader.ixx:1082`），以及完整的 RPF 容器解析/TOC 加解密（`source/rpfloader.ixx:443,523,603,910`）。配置不是 `Common.ini`（**IV 系没有该文件，那是 GTA V 的**；仓内 `grep Common.ini` = 0），而是 `GTAIV.EFLC.FusionFix.cfg` + `settings/GTAIV.EFLC.FusionFix.json`（863 行 schema，8 种语言各 393 行译文）经 ASI Settings 暴露，构建期由 `asi-settings-validate.exe` 校验（`premake5.lua:166-167`），`source/contributing.ixx:10-39` 给出四步"加一个设置"的标准流程——这是"配置驱动的 offset/功能开关"的制度化。

### `Update postfx.ixx` 到底改了什么

HEAD 提交信息是上游最后一次提交的标题（浅克隆 squash 成单提交，`git show --stat` 显示其包含全部 3,368 个跟踪文件），所指文件是 `source/postfx.ixx`——**C++20 模块源文件，不是着色器数据文件**。这澄清了任务预设：`.ixx` 在本仓没有"自定义着色器容器"含义。真正的"着色器/内嵌资源管线"有四条：① HLSL → `fxc.exe` → `.pso/.vso/.fxo` → `Shaders.rc` 内嵌 → `postfx.ixx:245-258` 运行期 `CreatePixelShader`（自研新效果）；② RAGE `.xml`+`.asm` → `RageShaderEditor.exe` → `.fxc` → `data/update/common/shaders/win32_30/` → 游戏自己的着色器加载器（替换游戏原版字节码，`buildshaders.bat:8-14`；仓库里 `data/update/common/shaders/` 目前只有 `.gitkeep`，说明该目录由构建/安装包填充）；③ `shaders.ixx:444-465` 的运行期劫持：直接读磁盘上的 `rage_perlinnoise.fxc`，再用模式 `"A1 ? ? ? ? A3 ? ? ? ? C7 05 ..."` 枚举 exe 镜像内的"嵌入式着色器 {名字, 数据}"注册表，比对名字后 `WriteMemory(&entry[1], ...)` 把数据指针改指自己的副本；④ `GetFusionShaderID` 的 `"FusionShader"` 自标记反查。四条合起来就是"对引擎着色器路径的逆向知识"的完整表达。

### RE 产物四：一张跨代的 key 哈希家族表 + 字典反查

`source/gxt/`（vendored 的 GXT 编译/反编译器，`license.txt` 正文**只有一行 URL** `https://github.com/Sergeanur/GXT-compiler`，没有任何许可条款）顺手替本报告保存了一条别处很难拿到的事实：**Rockstar 系逐代的 key 哈希家族**，被写成可执行参数（`source/gxt/README.MD` 的 `-k`）——

| `-k` 取值 | 用于哪些作品 | 依据 |
|---|---|---|
| `CHAR8` | GTA III / VC 等旧作"其余部分" | README 原文 "CHAR8 - rest of the games"，默认值 |
| `CHAR16` | 需 16 位字符键的版本 | 同上 |
| `CRC32` | **San Andreas** | README 原文 "CRC32 for SA" |
| `JENKINS` | **GTA IV / EFLC** | README 原文 "JENKINS for IV" |
| `CHAR8_TIME` / `CHAR12_TIME` | **Mafia II** | README 原文 "MH2" |

配套还有两个同样有信息量的开关：`-w(0|1)` 单字节/双字节字符（"Only some versions of SA and IV should have this set to 0"，即**同一作的不同零售版连字符宽度都不同**）、`-b` 大端（"Only useful for MH2 Wii"）。键的存储形式由 `source/gxt/src/KeyChar.h:3` 的 `template<size_t count> struct KeyChar` 表达 —— CHAR8/CHAR12 的字面意思就是**定长字符数组**，不是哈希。

真正值得单独记一笔的是**反查手法**。`Hash.h:44-58` 的 `to_wstring(Hash<T>)` 在第一次需要时惰性载入 `static std::map<uint32_t, std::wstring> dictionary = ReadDictionary(T::DictionaryPath)`，命中就返回原文、未命中才退化成 `#%08X`；`Jenkins_dictionary.txt` **91,230 行**、`crc32_dictionary.txt` **16,931 行**（`Hash_Jenkins.cpp:4`、`Hash_CRC32.cpp:5`），再用 `tables/` 下 108 份字符集表（最大 `iii_table_mob_jpn.txt` 11,202 行）把本地化字符写回去。也就是说：**因为 GXT 的键空间是人类词汇而不是随机串，32 位哈希在这里根本不构成单向性**，一份英文词典就足够把整张文本表还原成可读名字。`Jenkins` 实现本身（`Hash_Jenkins.cpp:7-24`）是标准 one-at-a-time，但被模板化到 `wchar_t` 上逐 2 字节累加并 `t_tolower` —— 这是 CodeWalker（`Jenk.cs`）与 CFX（`joaat.ts`）版本里看不到的细节，因为 PC 版 IV 的 key 是 UTF-16。

把这条放进全生态坐标系看，"哈希→名称"这类知识有**三种取得途径**，且成本差一个量级：CodeWalker 用对自家 exe 的 375 项 SHA1 指纹扫描 + PSO 反射把哈希解出来（3.4），CFX 用反汇编与运行时导出维护 6,437 份 native 文档数据库（3.6、3.9），而 `gxt` 只需要**一部字典**。这直接说明：只要键取自自然语言，任何 32 位哈希（含 GTA V 的 32-bit joaat 索引）都可以离线穷举 —— 对第 7 章判断"GTA VI 若沿用 64-bit JOAAT，键名反查的可行性"有直接参考价值（64-bit 只对**随机串**单向，对词典键依然可穷举碰撞）。

### 三方方法论对比

| 维度 | **FusionFix**（本章） | **ScriptHookVDotNet** | **MTA (mtasa-blue)** |
|---|---|---|---|
| 目标引擎 | RAGE IV/EFLC，x86，**无符号** | RAGE V，x64，闭源 ScriptHookV 承担 hook | RenderWare/GTA SA，x86，无符号 |
| 逆向载体 | 1,681 条去重特征码 + 59 个 `.ixx` 内的类布局 | 186 处 `FindPatternBmh/Naive`（`NativeMemory.cs`），失败静默降级 | 481 个 `#define FUNC_xxx 0x…` 硬地址 + 217 个 `*SAInterface` 布局头 |
| 地址策略 | 模式→位移→运行时解引用；版本用"多候选签名"表达 | 模式→`Rel32<T>(addr,disp)`；版本用 `b2699/b2944` 分支表 | **离线改写 PE 导入表 + 直接写死 VA** |
| Hook 层 | 全部自持（safetyhook/injector，575 点） | **外包**给闭源 `ScriptHookV.dll` | 自持 `StaticSetHooks`（vmt/detour） |
| 能否脱离本体分发 | 否，需正版 exe；但附带衍生二进制资产 | 否 | 否 |
| 面向未来的可迁移性 | **最高**：签名+多候选+字节码自标记，换 ISA 即可复用 | 中：模式手法可复用，但 ABI 与 hook 面依赖黑盒 | 低：绝对 VA 与 RenderWare 语义绑定 |

三者构成一条清晰的谱系：MTA 代表"硬地址 + 完整类镜像"的旧范式（对单一 build 最精确、跨版本最脆）；SHVDN 代表"签名扫描 + 把最难的 hook 层让给闭源二进制"的省力范式；FusionFix 代表**当代最完整的开源范式**——签名扫描、hook 安装、类布局、资源格式、着色器管线、配置 schema、CI 校验与更新检查（`source/updatecheck.ixx:34-50`）全在同一仓内闭环。

### 逆向/工程来源标记

反编译伪代码的直接残留最多：64 个 IDA/Ghidra 风格标识符（`dword_11F7060`、`dword_103B984`、`off_1045520`、`byte_103C111`、`sub_7870A0`、`sub_8C5D70`…），并保留 `a1/a2/a3/_this/edx/__fastcall` 参数命名与 `v4/v19/v20` 临时变量（`source/frameratevigilante.ixx:247-251,328-332,481-492`；`source/windowed.ixx:185-193` 直接以 `sub_69F0C0` 为函数名；`source/gxtloader.ixx:39-71` 以 `shsub_8C5D70` 为 hook 句柄；`source/shaders.ixx:439` 引用 `off_1045520`）。这些是"从反编译器输出人工誊写"的铁证，也是本项目最诚实的自述方式。社区出处：`readme.md:166-171` 逐条链接 `GTAmodding/GTAIV-Issues-List` issue（3/5/6/112 等），`fixes.ixx:1445,1522,1571` 在补丁点重复标注；`shaders/GTAIV.EFLC.FusionShaders-temp/README.md:4-9` 致谢 RaphaelK12、AssaultKifle47、robi29、\_CP\_、Shvab、brokensymmetry，并引用 **gtaforums 的 "Rage Shader Editor"**、ShadowResFix 与 LogarithmicDepth 项目。未见 unknowncheats 直接署名「推断」：IV/EFLC 的地址与模式主要来自 GTAmodding 议题列表与 Discord/Patreon 资助的小型协作网，而非 unknowncheats（`readme.md` 顶部即挂 Discord/Patreon/Boosty 链接）。工具侧 `tools/Saturn/scripts_example/compile.bat` 读 `HKCU\Software\ELO\Saturn`，说明 RAGE 脚本编译依赖外部 Saturn 工具而非仓内实现。

### 评估

这个仓库证明：**即使目标是一个完全闭源、无符号、32 位、已停止更新十余年的主机世代引擎移植，只要 (a) 特征码足够、(b) 能把字节指纹当版本键使用、(c) 愿意用 55k 行源码逐函数誊写类布局与着色器参数表，就仍可以在运行时获得接近"有源码"的改造能力**——不仅修 bug，还能重写整个后处理链与内存模型。它的对偶代价同样明确：1,703 条模式与 575 个 hook 点没有任何自动化回归（除 `asi-settings-validate` 与 shader `-Verify` 之外），一旦上游 exe 重编译即成片失效；而真正的 hook 与 overlay 代码在子模块中，主仓的可读性被浅克隆削弱了近一半。
