## 3.5 Multi Theft Auto（multitheftauto/mtasa-blue）

### 项目定位
MTA:SA 是"寄生式"多人改造：它不重写 GTA:SA 引擎，而是把自身作为引擎扩展注入零售可执行文件，复用 R\* 的 RenderWare 渲染、物理、池管理与任务系统，仅替换网络、脚本、GUI 与实体同步层。`README.md:13-19` 自述 "based on code injection and hooking techniques whereby the game is manipulated without altering any original files"（此句与下文 PE 就地改写自相矛盾，见注入节）并强调 "the class design of our game framework is based upon Grand Theft Auto's design, we are able to insert our code into the original game"——这正是本报告的 RE 方法论核心：**镜像类布局以换取二进制兼容**。交付物为客户端注入链（`mtasa.dll`/`core.dll`/`game_sa.dll`/`multiplayer_sa.dll`/`mods/deathmatch/client.dll`）与专用服务器（`mta-server` + `mods/deathmatch/deathmatch.dll|so`）。

### 元数据与授权真相
- 浅克隆：`git rev-list --count HEAD` = 1，无 tag；HEAD `70b0b402`（2026-11-17，"Fix various crash handler failures"）。版本 `Shared/sdk/version.h:45-49` = **1.7.0**，`MTASA_VERSION_TYPE = VERSION_TYPE_CUSTOM`。
- `LICENSE` 是逐字 **GPLv3**（`LICENSE:1`），`README.md:127` 为 "Unless otherwise specified, all source code … GPLv3"。真相有三层例外：
  1. **网络层不在源码内**。`net.dll`/`netc.dll`/`net.so` 由 `utils/buildactions/install_data.lua:10-15` 从 `mirror-cdn.multitheftauto.com/bdata/` 下载；仓库只有纯虚接口 `Client/sdk/net/CNet.h:63-137`（含 `VerifySignature`、`CheckFile`、`DeobfuscateScript`、`EncryptDumpfile`、`GetSerial`、`GetNetRev`）。树中不存在 `CNetworkClient`/`NetPacket`/`NetworkTaskQueue`/`Synchronize` 模块（grep 为空）。
  2. `Shared/publicsdk/LICENSE:1-20` 是 zlib 风格但加了 **"excluding commercial applications"**，即非自由许可证，覆盖 `include/ILuaModuleManager.h`（服务端 native 插件 ABI）。
  3. 仓库直接提交 **75 个二进制**（40 个 dll/exe：bass 全家桶、D3DX9_42、tags.dll、预编译 launcher）与 Rockstar 衍生数据 `sa.dat`(1.16 MB)、`data/gta_sa_diff.dat`(4.95 MB)，并有 MD5 完整性白名单 `Client/loader/MainFunctions.cpp:1646-1662`。
- 题设的 `shared/ client/ server/ tester/ contrib/ extern/ cmake/ installer/ AdminTable/` 目录名**均不存在**；现名为 `Shared/ Client/ Server/ Tests/ vendor/`，安装器在 `Shared/installer/nightly.nsi`（NSIS），`AdminTable`、`CClientPluginManager`、`ModDetectionSystem`、`ASCSecurity` 已全部消失（推断：老版本被重构/删除，插件机制改由 client.dll mod + 服务端 Lua module 承担）。

### 模块清单与规模
非 vendor C/C++ 共 **2,145 文件 / 611,185 行**；vendor 3,489 文件 / 1,863,894 行。

| 模块 | 文件 | 行 | 职责 |
|---|---|---|---|
| `Client/mods/deathmatch` | 459 | 174,990 | `client.dll`：CClientEntity 体系、CPacketHandler 5,930、CNetAPI 2,490、luadefs 36,774、rpc 20 |
| `Client/core` | 190 | 82,363 | core.dll：启动/GUI/下载/崩溃转储/DXHook(32 文件)/图形批处理 |
| `Client/game_sa` | 296 | 65,001 | **RE 主战场**：`CxxxSAInterface` 布局镜像 + StaticSetHooks |
| `Client/multiplayer_sa` | 50 | 34,297 | 引擎扩展/修复/键弹道同步（34 个 `CMultiplayerSA_*.cpp`） |
| `Client/sdk` | 186 | 15,486 | 跨 dll 纯虚接口 + `sdk/game/*.h` 抽象类 |
| `Client/loader`(+proxy/launch/cef*) | 57 | 19,858 | 注入链：PE 改写、`mtasa.dll` 代理、launcher、CEF |
| `Client/gui` | 52 | 8,472 | CEGUI 0.4 定制封装 |
| `Server/mods/deathmatch` | 554 | 143,356 | `deathmatch.dll/so`：CElement 树、150 packet 文件、CSim* 解同步 |
| `Shared`（sdk/mods/XML/publicsdk） | 229 | 53,992 | SString/CVector、`sdk/net` 4 头、SyncStructures 2,462、luascripts |
| `Tests` / `vendor` | 22 / 3,489 | 6,062 / 1,863,894 | gtest；第三方 |

### 构建系统与依赖
**不是 CMake**：`premake5.lua` + `utils/premake5.exe`，生成 vs2026 解决方案（`win-create-projects.bat`）、gmake、xcode；`cppdialect "C++23"`（`premake5.lua:51`），Allman/160 列（`.clang-format:41,48`）。平台矩阵：Windows x86/x64/arm64、Linux x64/arm/arm64、macOS arm64（`premake5.lua:33-39`）；**客户端仅 Windows 构建**（`premake5.lua:161`），故 Linux/macOS 只能产出服务端。工具集 `v145`、`/MT` 静态 CRT（Debug 也用 Release runtime）、`_WIN32_WINNT=0x601`、`GLIBC_COMPAT` 走 `/compat` 强制旧 glibc、`MTA_MAETRO` 出 Win7–8.1 变体（`MAETRO.md`）。vendor 依赖：`lua`(源码即 **Lua 5.1.5**，`vendor/lua/src/lua.h:20`)、`cegui-0.4.0-custom`、`cef3`(仅 premake 桩，二进制另装)、`curl`、`cryptopp`、`mbedtls`、`sqlite`、`json-c`、`pcre2`、`zlib`/`zip`/**`unrar`**、`freetype`/`libpng`/`jpeg-9f`/`lunasvg`、`bass`(音频，闭源 dll)、`libspeex`/`portaudio`(语音)、`google-breakpad`、`googletest`、**`detours` 4.0.1**、`bochs`(x86 指令长度分析)、`nvapi`、`discord-rpc`、`ksignals`、`sparsehash`、`tinygettext`/`tinyxml2`、`pthreads`、`mysql`、`glob`、`pme`。**未见 RenderWare/D3D9 SDK 头来自外部**：RW 结构是手抄的（下一节），D3D9 靠 `DXSDK_DIR` 环境变量。

### 注入与 hook 基础设施
注入是**离线 PE 改写 + 导入表劫持**，而非 CreateRemoteThread：`Client/loader/GameExecutablePatcher.cpp:18-21` 用 SHA256 + `SIZE_GTA_EXE{0x00db7a00}` 锁定唯一受支持的零售 32 位 exe，然后就地打四种补丁：`IMAGE_FILE_LARGE_ADDRESS_AWARE`、`NX_COMPAT`、把 `TimeDateStamp` 改成 `0x437101ca` 骗 Win7 走 Aero（`WindowsAeroPatch`）、以及 **`LibraryRedirectionPatch`：把导入项 `"WINMM.dll\x00\x02"` 原地改名成 `mtasa.dll`**（`GameExecutablePatcher.cpp:154-200`，`Client/loader/Main.h:14-16` 定义 `gta_sa.exe`/`gta-sa.exe`(Steam)/`proxy_sa.exe`）；另有在 `.rdata`/`.data` 空闲区伪造导出目录（`EXPORT_DIRECTORY_OFFSET 0x4b2b0`）以声明高性能 GPU 偏好。运行时链：`mtasa.dll`(loader-proxy) → `loader.dll` → `core.dll` → `GetGameInterface()`（`Client/game_sa/gamesa_init.cpp:29`，导出 `CGameSA`+`g_pNet`+`g_pCore`）→ `multiplayer_sa.dll` → `CModManager` 载入 `mods\deathmatch\client.dll`（`Client/core/CModManager.cpp:157`），后者以 `DoPulsePreFrame/DoPulsePreHUDRender/DoPulsePostFrame` 挂进帧循环（`Client/core/CModManager.h:37-39`）。

字节级 hook：`HookInstall<T>(dwInstallAddress, handler, iJmpCodeSize)` 用 `CreateJump` 生成 rel-jmp 并以 `0x90` 填满 5–9 字节（`Client/game_sa/HookSystem.h:56-70`），全部目标以 `#define HOOKPOS_xxx 0x……` + `HOOKSIZE_xxx` 书写（全树 **599 处 HOOKPOS**）；`EZHookInstallChecked` 先 `HookCheckOriginalByte` 校验原字节，`EZHookInstallRestore` 保存原字节以便卸载（`Client/multiplayer_sa/multiplayer_hooksystem.h:41-58`）。所有 hook 函数为 `__declspec(naked)`，并用 `MTA_VERIFY_HOOK_LOCAL_SIZE` 埋 `_localSize` 标签，由 `utils/src/hookcheck/hookcheck.cpp:6-11` 事后**读 PDB 断言 `__LOCAL_SIZE == 0`**（MSVC C++17 naked 函数陷阱）——这是同类项目罕见的编译器级自检。D3D9/DInput8 则不改字节，而是整套 COM 代理对象（`Client/core/DXHook/CProxyDirect3D9.cpp` 1,445 行、`CProxyDirect3DDevice9.cpp` 等 32 文件）。`CMultiplayerSA_*` 即"自定义引擎扩展"落地点：LOD 系统（`CMultiplayerSA_ObjectLODSystem.cpp:107-331` hook `CRRenderer::SetupEntityVisibility@0x554230`）、object streamer 池分配器替换、自定义动画、崩溃/帧率修复、`OnPreCreateDevice@0x007F675B`。

### 逆向类布局镜像证据
范式是三段式：`Client/sdk/game/CXxx.h`（纯虚抽象）→ `Client/game_sa/CXxxSA.h` 内的 `CXxxSAInterface`（**按零售布局逐字段复刻的 POD**）+ `CXxxSA`（只持 `internalInterface` 指针的薄封装）。

| 证据类型 | 数量 | 代表位置 |
|---|---|---|
| `static_assert(sizeof(...) == 0xNN)` 布局断言 | 107 处 / 46 文件 | `CDoorSA.h:21-35`(0x18)、`CFxSystemSA.h:34-137`(0x24/0x88/0x104/0x38/0x40)、`CPedSoundSA.h:81-120`(0x100/0x15C/0xA8)、`CPlaceableSA.h:42`(0x18)、`CBuildingSA.h:20`(0x38)、`CIplSA.h:37`(0x34)、`CWeaponInfoSA.h:37`(0x2c) |
| `*SAInterface` 镜像类型声明 | 217 | `CVehicleSA.h:228-369`（含 `// 312`、`// 1220`、`// 1224` 偏移注释） |
| `#pragma pack` | 14 | `CWaterSA.h`、`CPlayerInfoSA.h`、`CProjectileInfoSA.h`、`Shared/sdk/net/SyncStructures.h` |
| `#define FUNC_xxx 0x……` 直接调零售函数 | 481 | `CEntitySA.h:21-31`、`CDoorSA.h:16-19`、`CKeyGenSA.h:16-19`；内联形如 `((void(__thiscall*)(CVehicleSAInterface*, RwFrame*, uint32_t))0x6D2700)(...)`（`CVehicleSA.h:287`） |
| RenderWare 函数地址表 | 92 | `Client/game_sa/gamesa_renderware.hpp:12-103`（`RpAtomicCreate=0x00749C50` 等），头 `Client/sdk/game/RenderWare.h` 577 行手抄 `RwFrame/RpAtomic/RpGeometry/RpHAnimHierarchy` 结构并注 `// 56`、插件扩展 `0x253F2FE` |
| 零售数据段地址引用（0xB7????–0xBC????） | 324 | `CPoolsSA.cpp:37-39`（`0xB74490/94/9C` 即 ped/veh/object 池）、`CClockSA.h:16-18`、`CPlayerInfoSA.cpp:30-42`、`CRopesSA.cpp:17` |
| 作弊/属性字节表 | 16 | `CGameSA.h:47-65`（`VAR_HoveringCarsEnabled 0x969152` …）→ `CGameSA.cpp:192-212` 建表 |
| 零售 exe 指纹自检（字符串常量在指定 VA） | 12 | `CGameSA::VerifySADataFileNames()` `CGameSA.cpp:952-959`：`*(char**)0x5B65AE == "DATA\\CARMODS.DAT"` |
| 直接借用零售 CRT | 3 | `MemSA.h:17-35`：`malloc@0x824257`/`free@0x82413F`/`msize@0x828C4A`（保证与游戏池混用不崩） |
| vtable/指针合法性范围校验 | – | `CTaskManagementSystemSA.cpp:115-118` `GTA_BASE_MIN 0x400000 / MAX 0x900000` |

命名沿袭 re3/openrw 的 R\* 内部符号（`CPed`/`CAutomobile`/`CBike`/`CPhysical`/`CHandling`/`CTaskSimple*`/`CVisibilityPlugins`/`CStreaming`/`CReference`+`CReferences m_refs[3000]`），并直接改游戏池容量（`CGameSA.cpp:215-218` 把 TASK_POOL 500→5000、OBJECT 350→`MAX_OBJECTS`）。

### 网络与元素复制模型
服务端权威元素树：`CElement`（`Server/mods/deathmatch/logic/CElement.h`，`EElementType` + `ElementID GetID()` @:101）派生 `CPed`/`CVehicle`/`CObject`/`CBlip`…；ID 由 `CElementIDs`（`CElementIDs.h:16-27`）在 `SFixedArray<CElement*,131072>` 上分配，空闲 ID 走 `CStack`；`ElementID` 是 32 位值对象（`Shared/sdk/Common.h:40-84`，`MAX_SERVER/CLIENT_ELEMENTS = 131072` @:33-34，静态断言 2 的幂）。客户端镜像为 `CClientEntity`（`eClientEntityType`，同 ID：`CClientEntity.h:155,201,347`），再由 `CClientPed/CClientVehicle` 绑定到 `game_sa` 的真实游戏实体（`SClientEntity<>` 池包装）；`CClientStreamer` 负责按距离流入/流出。传输与低层可靠性在闭源 `netc.dll`，开源侧边界是 `CNet::RegisterPacketHandler/SendPacket/AllocateNetBitStream`（`CNet.h:72-90`）+ `NetBitStreamInterface`（`Shared/sdk/net/bitstream.h` 421 行，含 `eBitStreamVersion` 版本协商）。**协议编码本身是开源的**：111 个 `ePacketID`（`Packets.h:16`，服务端 `logic/packets` 150 文件/73 个 handler）、220 个 RPC 枚举（`rpc_enums.h`，注释强制"只能追加到 `NUM_RPC_FUNCS` 之前"）、`PACKET_ID_RPC` 单包承载全部 RPC；量化同步结构 `Shared/sdk/net/SyncStructures.h` 2,462 行（`SPositionSync` 用 `SFloatSync<14,10>` 压 X/Y、Z 走裸 float，`SYNC_POSITION_LIMIT` 做夹紧与合法性复核，@:222-278），客户端 `CNetAPI.cpp`(2,490) 读写 puresync/keysync/bulletsync/camerasync，服务端 `logic/net`（20 文件，其中 9 个 `CSim*.cpp`）做解算与权威校验；`CLatentTransferManager`/`CElementGroup`/`CCustomData` 负责分片与可见性。兼容常量：默认端口 **22003**（`Client/core/CCore.cpp:1861`）、LAN 广播 **34219**（`Shared/sdk/Common.h:89`）、ASE/GTASA 服务器浏览器协议（`Server/.../ASE.h`、`ASEQuerySDK.h`）。

### Lua 绑定层
无代码生成器，全部手写 constexpr 表：每个 `CLuaXxxDefs::LoadFunctions()` 内 `static const std::pair<const char*, lua_CFunction> functions[]{ {"createBlip", ArgumentParserWarn<false, CreateBlip>}, … }` 后循环 `CLuaCFunctions::AddFunction`（`Client/mods/deathmatch/logic/luadefs/CLuaBlipDefs.cpp:137-162`）；全局函数另有 `CLuaManager.cpp:239-241` 的表。规模：**唯一注册名约 2,050**（client 1,097 / server 771 / shared 75），分布在 169 个 luadefs 文件、71,079 行。现代 marshalling 用 `Shared/mods/deathmatch/logic/lua/CLuaFunctionParser.h` 的 `ArgumentParser<>`/`LuaArg` + `std::optional`/`std::variant`/`std::tuple`/`CLuaMultiReturn`，业务函数直接写成强类型 C++ 函数、非法参数 `throw std::invalid_argument("Invalid icon")` 由解析层转成脚本错误（`CLuaBlipDefs.cpp:20-22`）；旧式 `CLuaArgument/CLuaArguments` 仍用于 RPC 与自定义事件。元素 OOP 由 `lua_newclass` + `AddClass`（56 处）建立 `element.*` 元表继承；内嵌 **Lua 5.1.5**（无 LuaJIT），意味着重建工程需保住 5.1 语义（`lua_getfenv` 等）。

### 反作弊与兼容性
开源侧只是骨架：`CAntiCheat` 每周期跑 5 个 `CAntiCheatModule`（`CAntiCheat.cpp:16,66-92`，文件用 `#pragma optimize("", off)` 包裹"because we're encrypting stuff" @:14）；`CClientPed` 同时继承 `CAntiCheatModule`（`CClientPed.cpp:79,91`），`PerformChecks()` 比较"引擎内血量/护甲是否超过 MTA 记账值"并回调 `g_pGame->PerformChecks()`（`CClientPed.cpp:4409-4437`），后者遍历 16 个零售作弊字节地址看是否被外部改写（`CGameSA.cpp:942-951`）。真正的签名校验、文件校验、脚本解混淆、序列号与 dump 加密都在闭源 netc（`CNet.h:121-135`）。启动期另有进程黑名单 `{"CheatEngine","PCHunter"}`（`Client/loader/Utils.cpp:2126`）与 `integrityCheckList` MD5 白名单（`MainFunctions.cpp:1646`）。`FileGenerator.cpp:162-172` 用 SHA256 校验零售 `audio/CONFIG/PakFiles.dat`（468 B）并解 `gta_sa_diff.dat`（RAR，4.95 MB）重建补丁数据。fork 兼容策略写在 `version.h:5-40`：`VERSION_TYPE_RELEASE/UNSTABLE/CUSTOM` 三态 + 官方/fork 专用 netc.dll + 服务端 `<disableac>5,6,21</disableac>`（`CMainConfig.cpp:304`）。

### 工程质量
CI 全在 GitHub Actions：`build.yaml`（windows-2025-vs2026 × win32/x64/arm64、macOS-15、ubuntu + `utils/docker/Dockerfile` 交叉编译）、`tests.yml`（msbuild `Build\Tests_Client.vcxproj` →跑 gtest 并上传 XML）、`clang-format.yml`（格式化即失败）、`codeql.yml`、crowdin 翻译流水线（129 个 `.pot`）。测试 `Tests/client` 22 文件 6,062 行、**313 个 gtest**，覆盖 CVector/CMatrix/哈希/**Sync 结构往返**（`SyncMovement_Tests.cpp:6-13` 明确记录量化位宽），但无任何 hook/布局回归测试（推断：布局只能靠编译期 assert 兜底）。崩溃遥测很重（breakpad + `CCrashDumpWriter` + 3,200 行 `CrashHandler.cpp` + OOM 自定义异常码 `0xE03B10C0`）。头文件统一 "PROJECT/LICENSE/FILE/PURPOSE" 横幅并指向顶层 LICENSE；`AGENTS.md`/`.clang-tidy` 极简，正式编码规范外迁至 `mtasa-docs`。

### GTA 特有的 RE 技术含量评估
属于**"二进制兼容镜像"级 RE**，比 re3（重建引擎）保守、比 openrw（重实现 API）更依赖硬地址：不还原算法，只保证布局与入口正确，再用 hook 把控制权搬进自己的 C++ 对象图。技术密度集中在约 10 万行的 `game_sa` + `multiplayer_sa`；真正的网络与反作弊内核**不在仓库**，因此 mtasa-blue 单独不足以复刻一个可连服的 MTA——它给出的是客户端注入/引擎镜像/元素与同步编码/API 层的完整蓝图，加上一个明确定义的 ABI 缺口（`CNet`）。

### 对 GTA VI 的启示
1. 可移植的是与设计而非地址：`ElementID`+`CElementIDs` 分配器、`SyncStructures` 量化位宽、2,050 个 Lua API 名与 5.1 语义、latent 分片/元素分组/可见性模型——这些与 GTA 无关，可近乎照搬。
2. `HOOKPOS/FUNC/VAR` 三张表必须替换成"符号解析层"：x64 + ASLR/CFG/CET + Online 反作弊下硬地址即刻失效，需要特征码/模式匹配（库内已有 `detours 4.0.1` 与 `bochs` 指令长度分析可作起点），并把 `static_assert(sizeof)` 换成"运行时字段探针 + 服务端下发偏移表"。
3. `hookcheck` 的 PDB 后审计思想值得放大：任何布局/偏移假设都应有构建期或首次运行期的自动验证闸门，否则升级即雪崩。
4. 授权与架构教训：MTA 把 netcode 与 AC 放进闭源 dll，在 GPLv3 下保住了协议与反作弊控制权，代价是 master 不能独立跑通。复刻 GTA VI 联机应**第一天就定义纯虚 ABI（`CNet.h` 是范本）并把兼容性契约（端口、包 ID 追加规则、RPC 枚举顺序）写成显式不变量**。
