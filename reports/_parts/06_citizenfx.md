## 3.6 CitizenFX / FiveM（citizenfx/fivem）

### 项目定位
`README.md:9-13` 自述本仓库包含 Cfx 全部产品：FiveM（GTA V Legacy 单机/多人双用途框架）、RedM（RDR2）与 FXServer。`docs/layout.md:9-13` 给出四段式根结构：`code/`（原生代码与构建根）、`data/`（运行期数据）、`ext/`（辅助工程）、`vendor/`（依赖子模块）。它是 GTA V 生态中唯一把 RAGE 多人内核知识以**生产级 C++ 双端（客户端注入件 + 服务端）一致实现**公开出来的项目。

克隆状态：656 MB、depth 1，HEAD `8cbec36`（2026-11-17 16:00 +0100，`chore(scrt-js): NodeJS update (again)`）；151 个 submodule 全部指向 github.com（30 citizenfx/*、120 第三方），均未 checkout。规模：`code/components` 117 目录、112 个 `component.json`、组件 529,742 行 + `code/client` 141,895 行；3,229 处 `hook::pattern|get_pattern|get_address`、457 处 `IsGameBuild*` 门控、GTA V native 面 5,300 条声明 / 4,777 个唯一哈希（`ext/natives/natives_stash/gta_universal.lua`）、876 个 CFX 自有 native、847 个 `ext/native-decls/*.md`。

### 授权真相与多许可分拆
FiveM 并非"开源"。`LICENSE:1-9` 为 © 2023 Take-Two，适用 **Rockstar Games Creator Platform License Agreement**（`fivem.net/terms`、`redm.gg/terms`）——非 OSI 的"源码可见"许可；`LICENSE:11-22` 只对少量路径授予 LGPL-2.0。`LICENSES/` 另有 10 份第三方许可证文本。

| 许可 | 覆盖范围 | 证据 | 含义 |
|---|---|---|---|
| R* Creator Platform | 其余一切（含 `rage-*`、`gta-*`、`citizen-server-impl`、`data/`） | `LICENSE:1-9` | 可读不可自由再分发 |
| LGPL-2.0 | `code/client/{shared,citicore,launcher,common}/`、`code/tools/`、`code/components/citizen-*/`、`font-renderer`、`vfs-core`、`code/vendor/*.lua` | `LICENSE:11-22` | 仅"框架层"自由；**逆向核心不在内** |
| 第三方 MIT/BSD/Apache/BSL/Zlib/ISC/FTL/LGPL-2.1/curl | `vendor/`、`ext/` | `LICENSES/`、`THIRD_PARTY_NOTICES.md` | 各自原许可 |
| 仅二进制 | `legitimacy.dll`、`libnode22.*`、CEF | `legitimacy/component.json` `"bundled": true`、`code/vendor/libnode.lua:5-15` | 无源码 |

关键点：`citizen-*` 是 LGPL，而 `gta-*`/`rage-*`（真正的 RE 成果）落在 R* 许可下——逆向知识被刻意排除在"自由软件"之外。「推断」对应 2023 年 Take-Two 与 Cfx 的授权重整。

### 顶层结构与构建编排
构建图由 Premake5 驱动，`code/premake5.lua:11-33` 的 `PROGRAM_DETAILS` 声明 `--game=five|rdr3|ny|server|launcher`，其中 `['ny'] = { publicName = 'LibertyM', … NativesNY.cs }` 是一个未公布的第三目标（`GTA_NY`）；`code/premake5_builds.lua` 为构建矩阵，`code/premake5_layout.lua` 定义 CfxPrebuild/CfxPostbuild。`fxd.ps1` 只是 pwsh-7 分发器：`$CommandScript = "$PSScriptRoot\code\tools\fxd\$Command.ps1"` → `Invoke-Expression`，子命令 build/dev-sdk/gen/get-chrome/natives/run-sdk/vs/wsl-build；`gen.ps1` 调 `premake5.exe vs2022 --game=$Game`，`build.ps1` 做 nuget restore + `msbuild CitizenMP.sln`，`get-chrome.ps1` 从 `cfx-content.akamaized.net` 拉 CEF v144。私有层由 `code/tools/build/privates.lua` + `privates_config.lua` 叠加 `fivem-private`（`private_repo()`、`_G.inPrivates`、`delete_component_by_name`）；`code/tools/ci/build_server_docker_alpine.sh:11-22` 直接克隆私有仓并写 `privates_config.lua`——公开树 = 私有树减叠加层。题设的 `code/global/`、`code/postbuild/` 不存在，实际为 `code/shared/` 与 `code/tools/build/run_postbuild.ps1`。

### 组件框架架构
当前树中**已无** `Framework::`、`INITIALIZE`、`StartupArgvHack`、`GET_SERVICE`（grep 零命中，题设假设需修正）。现模型为：每个组件目录一份 `component.json`（`name`/`loadAfter`/`deps`/`optionalDeps`/`gameEntrance`/`bundled`），由 `code/tools/build/components.lua:25-72,357,453-477` 归并成 `data/client/components.json`（65 条）与 `data/server/components.json`（26 条）；运行期 `code/client/citicore/ComponentLoader.h` 提供 `Component`/`ComponentData`/`ComponentLoader`、拓扑排序 `SortDependencyList` 与 XPCOM 式 QI `dynamic_component_cast`（对 `boost::type_index` 名去前缀 `substr(6)` 后哈希）；单例走 `Instance<T>` + `fwEvent` + `DeclareInstanceType`。注册惯例是 `static InitFunction initFunction([](){…})`（`code/client/shared/Utils.h:53-92`，在 `Component::Initialize()` 中 `RunAll()`）与 `static HookFunction`（在 `DoGameLoad()` 执行），入口 `extern "C" DLL_EXPORT Component* CreateComponent()`，样板见 `code/components/template/Component.cpp`。XPCOM 遗产仍在：`fxIBase.idl`/`fxcore.idl`/`fxScripting.idl` 经 `code/tools/idl/xpidl` 派生 `citizen-scripting-core/include/fxScripting.h`。同一棵树出两端靠 `IS_FXSERVER`/`game` define 分叉：客户端以游戏 DLL（类 `.asi`）自举（`code/client/citigame/Launcher.cpp:32-39`），服务端复用组件但不装游戏钩。

| 组件 | 总行 | `src/` 行 | 职责 |
|---|---|---|---|
| `citizen-scripting-mono` | 71,675 | 854 | C# ScRT；绝大多数为 vendored Mono 头 |
| `voip-mumble` | 51,353 | 4,848 | 客户端语音（Mumble/Opus） |
| `citizen-server-impl` | 41,496 | 26,622 | FXServer 核心（`ServerGameState.cpp` 8,151） |
| `voip-server-mumble` | 26,489 | 26,478 | 服务端语音中继 |
| `glue` | 25,876 | 14,514 | 官方脚本胶水 + `PackfileSigning.cpp` |
| `conhost-server` | 25,764 | 426 | 终端 UI；`include/menlo.h` 25,273 行是字形表 |
| `extra-natives-five` | 21,294 | 20,574 | 876 个 CFX 扩展 native 实现 |
| `gta-core-five` | 20,795 | 14,098 | GTA V 对象/池/RTTI（`gta_vtables.h` 5,773） |
| `gta-net-five` | 17,483 | 15,910 | netsync（`CloneManager.cpp` 2,544） |
| `gta-streaming-five` | 16,772 | 13,339 | 内容流送 |
| `rage-formats-x` | 16,366 | 2,449 | RAGE 资源格式重建 |
| `gta-net-rdr3` | 14,536 | 2,116 | RDR2 复制树 |
| `nui-core` | 12,524 | 10,005 | CEF NUI 桥 |
| `rage-graphics-five` | 9,974 | 9,231 | 渲染 hook |
| `rage-scripting-five` | 8,932 | 1,629 | 脚本 VM 桥（本文核心） |
| `citizen-scripting-core` | 7,387 | 5,684 | ScRT 基座 + PAS `ScriptInvoker` |
| `debug-net` / `citizen-resources-core` / `citizen-scripting-lua` | 5,906 / 5,883 / 5,329 | 4,987 / 4,238 / 4,864 | 抓包 / resource 系统 / Lua 绑定 |

原始 LOC 因内嵌 blob 严重失真，故并列 `src/` 列作为手写量指标。

### natives 数据库与绑定生成
链路为 **DSL → stash → codegen → 落位**：
1. 定义格式（`ext/natives/README.md`）：`native "GET_PLAYER_PED"` / `hash "0x43A66C31C68491C0"` / `jhash (0x6E31E993)` / `arguments { Player "playerId", }` / `ns "PLAYER"` / `returns "Ped"`——双哈希（64 位游戏 FNV + 32 位 Jenkins）。README 明确格式"defined most clearly in native-doc-tooling's views/lua.twig"。
2. 取得：`prebuild_natives.cmd` 用 MSYS2 pacman 后 `call:UpdateToLatest inp\natives_global.lua https://static.cfx.re/natives/natives.lua`（另有 rdr3/ny_tmp/*_client_compat 变体）。`ext/natives/natives_stash/` 提交生成库：`gta_universal.lua` 1,330,565 B / 5,180 native、按构建切分的 `gta_0193D0AF.lua`/`gta_21E43A33.lua`；而 `rdr3_universal.lua`、`ny_universal.lua` 为 0 字节「推断」RDR3/NY 数据库仅私有可得。`get_natives.ps1` 用 `nativemoon` + SHA-256[0:8] 命名，注释引用 `dev-c.com/nativedb/func/info/…`（哈希与签名的外部信用来源）。
3. 生成：`ext/native-doc-gen`（PHP/Twig）+ `ext/natives/Makefile:100-172`（如 `out/NativeTypes.h: inp/natives_global.lua … codegen_out_pointer_args.lua`），发射器 `codegen_out_{cs,cs_v2,dts,enum,js,json,lua,markdown,native_lua,pointer_args,rpc,slua}.lua`；`Makefile:24-58` 注入的 `NATIVES_FIVE_HEADER` 含 `#if USE_HYPERDRIVE using ContextType = CitizenFX.Core.RageScriptContext;`。
4. 落位（`prebuild_natives.cmd` xcopy）：`out/*.lua|js|d.ts|rpc_natives.json` → `data/shared/citizen/scripting/{lua,v8}`，`out/*.cs` → `code/client/clrcore`，`out/v2/*.cs` → `clrcore-v2/Native`，`out/Natives*.h` → `citizen-scripting-lua/src`，`out/NativeTypes*.h` → `citizen-scripting-core/src`。
5. 类型元数据：`ext/natives/codegen_out_pointer_args.lua:1-59` 输出逐参标志，如 `'PED/GET_PED_DRAWABLE_VARIATION/1' = PAS_ARG_BOUND | 11`。

### rage 脚本 VM 调用 ABI
被逆向的上下文布局在 `code/components/rage-scripting-five/include/scrThread.h:76-135`：`rage::scrNativeCallContext { void* m_pReturn; // +0 ; uint32_t m_nArgCount; // +8 ; void* m_pArgs; // +16 ; uint32_t m_nDataCount; // +24 ; scrVectorSpace m_vectorSpace; uint8_t pad[96]; }`，同文件对 `scrThreadContext` 用 `static_assert(sizeof == 168)` 锁定。FiveM 自己的 VM 中立壳 `scrEngine.h:123-187`：`MaxNativeParams = 32`、`ArgSize = 8`、`uint8_t m_TempStack[256]`，`m_pReturn`/`m_pArgs` 别进入临时栈；`:189-211` 是 `NativeInvoke::Invoke<Hash,R,…>`，`:64` 为 `typedef void(__cdecl * NativeHandler)(scrNativeCallContext*)`。`scrEngine.h:8-31` 保留 2009 年 GTA IV 时代 "Aru" 授权声明，直接把血统连到 x87/yim 期。

游戏的 native 注册表被混淆，`scrEngine.cpp:107-179` 重建为 `struct NativeRegistration_obf`（`handlers[7]`、`hashes[7*2]`，`getNextRegistration()/getHash()` 做 XOR 解码），`:369-407 RegisterNativeDo` 注释直言"re-implemented here as the game's own function is obfuscated"，`:484-519` 是哈希快速查表；`:583-688` 为签名扫描（如 `hook::pattern("48 8B C8 EB ? 33 C9 48 8B 05")`、`registrationTable` 取 `hook::pattern("76 32 48 8B 53 40")+9`），`:659` 承认解混淆器在别处："scrObfuscation in fivem-private can handle opcode 0x2C (NATIVE)"；`:105` 引用 `ivanmeler/OpenVHook` 的 ScriptEngine 作为出处。`:34-90` 的封禁表（`{0x213AEB2B90CBA7AC, INT_MAX} // _COPY_MEMORY`、DATAFILE/DATAARRAY 全禁）标出哪些 native 在 MP 中不可暴露。

指针安全层（PAS）在 `citizen-scripting-core/src/ScriptInvoker.cpp`：`:32-48` 标志、`:63-64` `g_IsolatedBufferSize = 7*4096` × 4 缓冲、`:393-565 CheckArguments`（`0xDEADBEEF7FEDCAFE` 投毒 + `IsolatePointer`）、`:296-345 PushMetaPointer`、`:567-643 CheckResults`、`:703-757` 在 `OnScriptInit` 时 `VirtualAlloc(..., PAGE_NOACCESS)` 加 guard；`ScriptInvoker.h:29-92` 定义 `MetaField`、`ScrString`（magic `0xFEED1212`）。服务端复用同 ABI：`fxNativeContext.h:10-17` `uintptr_t arguments[32]; … uint64_t nativeIdentifier;`，`ScriptInvoker.cpp:98-104` 在 `NativeTypes.h`/`NativeTypesServer.h` 间切换，`Invoke()`（`:212-237`）以 `__try/__except` 兜底。跨版本哈希由 `TableBuilder.cpp` 承担：`:16-19 CrossMappingEntry{uint64_t entries[28]}`、`:27-62 rage::MapNative/ReviveNative`（并 `DLL_EXPORT`）、`:191-193 #include "CrossMapping_Universal.h"`（6,565 行）、`:69-100` 用 `__DATE__` 与特征串识别构建（`"Jul  9 2026"`→`Summer_2026` … `"Jun  9 2015"`→372，否则 `FatalError("No native mapping information found…")`）。

### netsync 复制模型与 RAGE 内部结构泄露
`gta-net-five/include/netObject.h:61-126` 给出 `CNetworkSyncDataULBase`（`ownerId`、`nextOwnerId`、`wantsToDelete : 1 // netobj+76`、`creationAckedPlayers // netobj+112`）与 `netObject{ uint16 objectType; uint16 objectId; CNetworkSyncDataULBase syncData; }`，`GetBlender()` 以 `*(netBlender**)((uintptr_t)this + 88)` 取；`:8-54` 的 `template<int Offset> inline int MapNetObjectMethod()` 按构建做增量位移（`>=0x2B8` +0x8 for Winter_2025、`>=0xA8` +0x8 for 3258、`>=0x8` +0x30 for 2802、`>=0x78` +0x10 for 2545、`>=0x330` +0x8 for 2189）——一个偏移维护 DSL。`src/netSyncTree.cpp:34-90` 解 bitField 并按构建 `hook::get_pattern` 打桩 `getSyncTreeForType`/`ReadFromBuffer`/`CanApplyToObject`/`PrepareObject`；`CloneManager.cpp`（2,544 行）负责克隆与主机迁移（`:1803` MIGRATE_NODE→UPDATE_CREATE_NODE；`:2349` 还留着注释掉的裸地址调用 `((void(*)(rage::netSyncTree*, rage::netObject*, …))0x1415D94F0)`）。

服务端是 RAGE 同步协议的**位精确重实现**：`citizen-server-impl/include/state/SyncTrees_Five.h` 4,929 行 / 81 个 `struct C*Node`，逐字段声明位宽（`s.Serialize(32,m_model) … s.Serialize(19,m_maxHealth); s.Serialize(32,m_creationToken);`），由 `SyncTrees_Header.h`（715 行，`ParentNode`/`NodeWrapper`/`ParseSerializer`/`UnparseSerializer`）驱动，`src/state/ServerGameState_SyncTrees.cpp:1-105` 的 `MakeSyncTree()` 分派到 `fx::sync::CAutomobileSyncTree` 等；`ServerGameState.cpp`（8,151 行）拥有实体/克隆/重放，`ServerGameState_SyncTrees` 之外的 `ServerGameState_Scripting.cpp`（2,702 行）把它暴露给 native。这意味着 CPed/CVehicle/CObject 状态被逐位公开，是现存最深的 RAGE MP 实体布局披露；RDR3 分支额外含动物/群体/incident 树。类型层泄露见 `gta-core-five/include/gta_vtables.h`（5,773 条 RTTI 串）+ `src/CustomRtti.cpp:26-38 ReadHashUnsafe`（识别 `mov eax, cs:dword_*; retn`：`funcByteCode[0]==0x8B && [1]==0x05 && [6]==0xC3`），供 `src/PoolManagement.cpp:169` 分配游戏池。取证工具链也在树内：`code/tools/dbg`（116 MB）22 份 `dump_<build>_<n>.txt`（约 4.9 MB/份，`dump_2944_0.txt` 含 303 个 `rage::` 名，如 `140001000 UserName _gtaMalloc`）+ `pe_debug/main.cpp:20-27`（"PEframework … written by The_GTA"，wordwhirl@outlook.de）+ `retarget_pe/`「推断」用于从 PE 符号转储再生偏移。

### 语言运行时层
Lua 5.4（子模块 `citizenfx/lua`，tag `luaglm-548`）：`citizen-scripting-lua/src/LuaScriptNatives.cpp:254/278/298/336` 注册 `Citizen.InvokeNative/InvokeNative2/GetNative/LoadNative`，`data/shared/citizen/scripting/lua/natives_loader.lua:4-14` 为高层入口。JS 走双引擎：内嵌 V8（9.3/12.4）与 Node 22（`code/vendor/libnode.lua:5-15` 拉 `libnode22.dll/.lib/.pdb/.so`，配合 `libuv.lua:12-15`、`v8-monolith.lua:43`），绑定见 `V8ScriptRuntime.cpp:1259/1326/1340/1546-1547`。C# 由 Mono 承载：`clrcore`（旧 `CitizenFX.Core`）与 `clrcore-v2`，`code/premake5.lua:570` 的 `USE_HYPERDRIVE` 把 `ContextType` 切成托管直读的 `RageScriptContext`「推断」为绕过 C++ 转发、直跑游戏 VM 的路径。NUI/GUI 用 CEF（`vendor/cef/PLACE_CEF_HERE.txt`）与 bgfx/ImGui（`conhost-v2`）。第三方 mod 兼容层是完全开源的 ABI 复刻：`scripthookv/src/VishCompat.cpp` 实现 `scriptWait`(:198)、`scriptRegister`(:221)、`getGlobalPtr`(:248，空实现)、`eGameVersion` 映射(:256-269)、`nativeInit/nativePush64/nativeCall`(:312/318/323)、SET_PLAYER_MODEL 守卫(:332-349) 与 `ShAllowed`(:354)；`asi-five/src/Component.cpp:106-238` 加载 `.asi`，带 `FX_ASI_BUILD` 校验和 `FatalError("You cannot use OpenIV with FiveM…")` 黑名单。

### 开源边界表
| 能力 | 公开树状态 | 真实载体 | 证据 |
|---|---|---|---|
| 组件框架 / launcher / tools | 源码（LGPL） | 本仓库 | `LICENSE:11-22` |
| natives 数据库 | 生成器公开、DB 从 CDN 取 | `static.cfx.re/natives/natives.lua` | `prebuild_natives.cmd` |
| 游戏 VM 去混淆 | 仅 hook 公开 | `fivem-private` | `scrEngine.cpp:659` |
| 反作弊 / 合法性 | 无源码 | `legitimacy.dll`（debug 13,486,080 B / release 12,721,664 B） | `legitimacy/component.json` `"bundled": true`、`.gitignore` `!debug/!release` |
| 客户端保护件 | 无源码 | `adhesive.dll`（下载进内存）/ `libsvadhesive.so` | `citicore/EarlyMode.Win32.cpp:12-18`、`launcher/Download.cpp:518-519`、`run_postbuild.ps1:172-192`、`ci/wsl-build.sh:30-32` |
| RDR3 / NY natives DB | 0 字节占位 | 私有 | `natives_stash/rdr3_universal.lua` |
| 服务端同步树位域 | 全量源码 | R* 许可 | `SyncTrees_Five.h` |
| Mono/V8/CEF/botan | 二进制或 vendored 源 | `code/vendor/*.lua` + CDN | `libnode.lua`、`get-chrome.ps1` |
| 正式发版流水线 | 仅描述 | 自建 GitLab | `.gitlab-ci.yml`（`CFX_RETAIL_BUILD=true`） |

`legitimacy` 是 117 个组件中唯一没有 `src/` 的；`Utils.h:206-207` 的 `#define _CFX_TRACE_FILE "adhesive"` 表明公开代码把调试轨直接接进闭源模块。

### vendor 与私有件
信任根仍公开：`glue/src/PackfileSigning.cpp:10-35` 内嵌 RSA DER + `Botan::…create("SHA-384")` 校验 packfile；`launcher/ExecutableLoader.Snapshot.cpp:11-28` 以 RVA `0x14020b70c` + Launcher.exe sha1 `f259de45…` 触发工具模式，并列出各构建 RVA（`0x14187F48C` Summer_2026 … `0x1417D3600` 2612）。`code/vendor`（122 MB）中 `botan`（6.7 MB 源）、`chromium/ipc`（4,036 文件）随树发布，`mojo` 目录空但有 `mojo.lua`，151 个 submodule 无一私有——私有性靠"仓库切分 + CDN 工件 + 二进制捆绑"实现。

### 工程质量
GitHub Actions 只做公开构建（`ci_build.yml` 四作业矩阵、`pr_opened.yml`），真正的产物链在 `.gitlab-ci.yml`（master|beta|production 才 `CFX_RETAIL_BUILD=true`，`.only_for_public_build` 条件、缓存生成的 `fxScripting.h`）。测试极弱：`code/tests/premake5.lua` 以 `vendor:catch2` 建 `CitiTest`，但仅 4 个组件有 `tests/`（`rage-formats-x`、`http-client`、`font-renderer`、`voip-mumble`），RE 面几乎无自动化验证，靠真机逐构建试。风格由 `code/.clang-format`（`BasedOnStyle: WebKit`）统一。活跃度证据：`data/client/citizen/platform-{2060…3889}` 15+ 个构建目录，映射表覆盖 372(2015)→Summer_2026 约 11 年不断档；`crash-data.json` 把崩溃码译成玩家可读提示。

### 逆向技术含量评级
**A+（本系列最高，唯一满分）**。依据：① 双端一致实现 RAGE MP 协议 —— 服务端同步节点结构共 26 个（`citizen-server-impl/include/state/SyncTrees_Five.h` 15 个 `*Node : GenericSerializeDataNode<T>`，其余 `SyncTrees_*.h` 11 个），客户端侧 `gta-net-five/include/netSyncTree.h` 直接镜像 `rage::netSyncTree` 并用 `MapNetSyncTreeMethod()` 做**构建号相关的偏移修正**（`offset >= 0xD0 && IsGameBuildOrGreater<2545>()` 时 `+0x10`）；② 对抗**主动混淆**（`NativeRegistration_obf` XOR 表，见 `rage-scripting-five/src/scrEngine.cpp` + 私有 `scrObfuscation`）而非仅签名扫描；③ ABI 体系化建模（`rage-scripting-five/include/scrThread.h:191` 的 `static_assert(sizeof(scrThreadContext) == 168, "scrThreadContext has wrong size!")`、28 列 CrossMapping、`MapNetObjectMethod` 偏移 DSL）；④ 类型/RTTI/池/资源格式整层重建（`gta_vtables.h`、`rage-formats-x` 的 pgBase 28/36 位指针编码）；⑤ 把 RE 结果产品化为对抗恶意脚本的沙箱（PAS + `PAGE_NOACCESS` + SEH）；⑥ 可再生的取证工具链（`dump_*.txt`、`pe_debug`、`nativemoon`、Twig/Lua codegen）。native 面实测：`ext/natives/natives_stash/gta_universal.lua` 有 **5,300 条函数声明、4,777 个唯一 8 位哈希**，另有 876 个 CFX 自有 native 与 847 份 `ext/native-decls/*.md`。相较 ScriptHookV（单机、零协议）在广度与深度上高一个量级；扣分项是零测试覆盖与核心保护件闭源导致知识不可完全审计。

### 对 GTA VI 的启示
框架侧几乎可原样迁移：`PROGRAM_DETAILS` 的 `--game=` 维度本就是多游戏抽象，新增目标只需一份 CrossMapping 列与新 natives DB——`ny`/LibertyM（`NativesNY.cs`、`natives_ny_tmp.lua`、`*ny` 组件）证明该抽象仍在为未公布标题铺路「推断」其真实动机即下一代 RAGE。真正要在 VI 上重做的是：x64/ARM64 下新的 `scrNativeCallContext` 与 scrThread 布局、更强 VM 混淆（V 代已需 XOR 解表 + 私有解混淆器，VI 预期更严）、从 `netSyncTree`/`netObject` vtable 重取同步树位域、以及 `ReadHashUnsafe` 依赖的 RTTI 哈希发射模式（随工具链升级而变）。方法论上有三点可直接借用：把偏移表驱动化（集中 DSL 而非散落硬编码）、把 native 元数据外置为可再生 DB（CDN 拉取，游戏更新后数小时内出兼容层）、把 RE 知识局限在极少数组件（`rage-scripting-*`、`gta-net-*`）以便快速重编。边界警示：本树已确立"核心保护件无源码"的先例（`legitimacy.dll`、`adhesive`），Rockstar 若加固，公开树可审计性还会继续收缩。
