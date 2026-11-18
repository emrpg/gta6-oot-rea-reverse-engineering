## 3.3 ScriptHookVDotNet（crosire/scripthookvdotnet）

### 项目定位

ScriptHookVDotNet（下称 SHVDN）是 GTA V 的 ASI 插件，本身不含任何游戏侧 hook：它通过 Alexander Blade（crosire）闭源二进制 `ScriptHookV.dll` 暴露的 `scriptRegister` / `scriptWait` / `keyboardHandlerRegister` 三个入口挂进游戏，再把 CLR 与一套 C# API 塞进去（`README.md:8,14`；`source/loader/DllMain.cpp:207-209`）。因此本仓库是"**闭源逆向成果的最小暴露面**"：ScriptHookV 负责 native 哈希→函数指针的查表与脚本 fiber 调度，SHVDN 负责把以 `ulong` 哈希寻址的能力翻译成可编译、可热重载的托管对象模型。现行维护者为 kagikn（版权头 `Copyright (C) 2015/2026 crosire & kagikn & contributors`）。

### 元数据表

| 项 | 值 | 证据 |
|---|---|---|
| 许可 | zlib | `LICENSE.txt`、`README.md:53` |
| 第三方代码 | SlimDX（向量/矩阵）、chrono（游戏历法） | `COPYRIGHT.md:13,38` |
| 第三方包 | Fody、InlineIL.Fody、Costura.Fody、Roslyn | `THIRD-PARTY-NOTICES.md:8,32`；`source/core/ScriptHookVDotNet.csproj` |
| 目标框架 | 全部 `net48`（.NET Framework 4.8），`LangVersion 9.0`，`Platforms x64` | 五个 `.csproj` 均一致 |
| 原生工程 | C++/CLI（`CLRSupport=true`，v143/v145，输出 `ScriptHookVDotNet-Loader.asi`） | `source/loader/ScriptHookVDotNet.Loader.vcxproj:29-55` |
| 程序集版本 | v3 API `3.6.0`，v2 API `2.11.6`，core 默认 `3.0.0.0`（CI 覆盖） | 各 csproj `<Version>` |
| 运行时依赖 | ScriptHookV SDK、.NET Framework ≥4.8、VC++ Redist | `README.md:12-16` |
| 变更日志 | 无 `CHANGELOG.md`；版本历史仅存于 GitHub Releases / nightly 仓库 | 根目录文件清单 |

### 目录清单与规模

全仓 605 个文件、约 11 MB（含 `.git` 1.5 MB），559 个 `.cs` 文件共 200 248 行。

| 模块 | 文件数 | C# 行数 | 职责 |
|---|---|---|---|
| `source/loader` | 11 | 0（`DllMain.cpp` 241 行 C++/CLI） | ASI 入口、TLS 存取器、CLR 线程与 fiber 握手 |
| `source/core`（程序集 `ScriptHookVDotNet.dll`，命名空间 `SHVDN`） | 78 | 15 011 | 宿主内核：`ScriptDomain`、`Script`、`NativeFunc`、`NativeMemory`、`MemScanner`、编译器、控制台、日志、INI |
| `source/scripting_v2`（`ScriptHookVDotNet2.dll`） | 114 | 62 920 | 已废弃 v2 API 面（`[Obsolete]`） |
| `source/scripting_v3`（`ScriptHookVDotNet3.dll`） | 360 | 116 540 | 现行 v3 API 面（`GTA`/`GTA.Native`/`GTA.UI`/`GTA.Math`…） |
| `source/scripting_v3_tests` | 11 | 4 939 | xUnit，仅数学/时钟测试 |
| `examples` | 6 | 838 | 示例脚本 |
| `tools` | 4 | 0 | `NativeGen.exe`、`NativeTemplate.txt`、`NetJSON.dll`、`install_sdk.ps1` |

`NativeMemory.cs` 单文件 6 634 行、`GTA.Native/Native.cs` 3 394 行、`NativeHashes.cs` v3 6 607 行 / v2 8 623 行，构成体量主体。

### 原生侧与托管侧架构

原生侧被刻意削到极薄（无 `native/main.cpp`、无 `ScriptInterface`、无 `nativeinvoker.cpp`——旧版的这些组件已删除）。`DllMain.cpp` 只做四件事：

1. 暴露 TLS 读写器：`__writegsqword(0x58, …)` / `__readgsqword(0x58)`（`DllMain.cpp:19-26`），并把函数指针与主线程 TLS 地址、ThreadId 传给托管侧（`DllMain.cpp:48-55`）。
2. 用独立线程跑 CLR：`ClrThreadProc`（`DllMain.cpp:99-128`）。注释明确解释原因——在 ScriptHookV 的自定义 fiber 里跑托管代码会让 .NET 的 VEH 依据缓存栈边界误报栈溢出（`DllMain.cpp:97-98,211-218`，并直接引用 coreclr `excep.cpp#L7763`、`threads.cpp#L7912` 与 issue #976）。
3. `ScriptMain`（ScriptHookV fiber 内）用两个 `Event` 与 CLR 线程乒乓同步，并在检测到 `GetCurrentFiber()` 变化时判定"游戏重建会话"从而请求域重载（`DllMain.cpp:133-173`）。
4. `keyboardHandlerRegister` 把键消息转发进托管域（`DllMain.cpp:178-193`）。

托管侧 `ScriptHookVDotNet.Init/Tick/KeyboardMessage`（`source/core/ScriptHookVDotNet.cs`）承担旧版 ScriptInterface 的角色：清日志→读 INI→`ScriptDomain.Load(".")`→注入 TLS 函数指针→`InitNativeMemoryMembers()` 并行触发 `NativeMemory`、`NativeMemory.Vehicle`、`NativeMemory.Ped` 静态构造做全量特征码扫描（`ScriptDomain.cs:180-191`）→创建 `AppDomain` 内的控制台→`domain.Start()`。

### native hash ABI 调用链详解

* **寻址方式**：native 只有一个 64 位哈希，没有符号、没有地址。哈希以编译期枚举固化：`public enum Hash : ulong`，文件头写明 `// This file was generated with https://github.com/scripthookvdotnet/scripthookvdotnet-nativegen`（`source/scripting_v3/GTA.Native/NativeHashes.cs:1-7`）。v3 有 **6 460** 个具名成员、**0** 个占位名；v2 有 8 483 个成员、其中 5 212 个是 `_0x43A66C31C68491C0` 这类未命名占位（`source/scripting_v2/GTA.Native/NativeHashes.cs:16`）——v3 已把全部保留 native 的名字解析完。
* **仓库内不存在 `natives.json`/`.dat`/`.db`**：唯一的数据库是外部 `alloc8or/gta5-nativedb-data`，由 `tools/NativeGen.exe` 在 `NativeGen|x64` 配置的 PreBuildEvent 拉取并套模板生成枚举（`source/loader/ScriptHookVDotNet.Loader.vcxproj:93-97`；模板 `tools/NativeTemplate.txt`）。
* **ABI**：`Function.Call<T>(Hash, params InputArgument[])`（`Native.cs:486-501`）把每个参数展平成 `stackalloc ulong[]`，上限 `MaxArgCount = 63`（`Native.cs:478`），交给 `SHVDN.NativeFunc.Invoke`。`NativeFunc` 用 7 个 `DllImport("ScriptHookV.dll")` 直接按 **MSVC mangled name** P/Invoke：`?nativeInit@@YAX_K@Z`、`?nativePush64@@YAX_K@Z`、`?nativeCall@@YAPEA_KXZ`（`source/core/Game/NativeFunc.cs:23-41`），执行体即经典三段式：

```csharp
NativeInit(hash);
for (int i = 0; i < argCount; i++) NativePush64(argPtr[i]);
return NativeCall();          // NativeFunc.cs:318-327
```

* **参数编组**：`InputArgument` 只有 `internal ulong _data`，靠 30 余个隐式转换构造（`Native.cs:119-311`）：`float` 直接位打包进 64 位槽（`Native.cs:225-233`）、`double` 先降为 `float`（`Native.cs:234-243`）、`string` 经 `ScriptDomain.PinString` 转 UTF-8 CoTaskMem 指针（`Native.cs:287-290`，`ScriptDomain.cs:1790`）、对象走 `ObjectToNative`，仅接受 `null`/`string`/`INativeValue`，否则抛 `InvalidCastException`（`Native.cs:3028-3048`）。出参用 `OutputArgument`（默认 `Marshal.AllocCoTaskMem(24)`，正好容纳一个 Vector3 的 0x18 字节，`Native.cs:327-330`），并提供 `DiscardPtr64` 静态可写指针（`Native.cs:136-151`）。
* **Vector3 的按值约定**：返回值读取按 `data[0] / data[2] / data[4]`（float 索引）取 X/Y/Z（`Native.cs:3069-3079`），即每个分量间隔一个 64 位槽；对应写入侧 `internal struct NativeVector3 { [FieldOffset(0x00)] X; [FieldOffset(0x08)] Y; [FieldOffset(0x10)] Z; }`、`Size = 0x18`（`source/scripting_v3/GTA.Math/Vector3.cs:767-782`）。而对内的几何传递用紧凑 12 字节 `FVector3`（`Size = 0xC`，`source/core/Game/Structs/FVector3.cs:12`）。两套布局的差异常被忽略，是 native ABI 与 C ABI 的分界实证。
* **未知/新哈希**：托管侧不做存在性校验，也不维护哈希→地址映射（映射完全在闭源 ScriptHookV 内部），只把哈希原样推给 `nativeInit`。可检测的失败只有返回值：`nativeCall` 返回 `null` 时抛 `"Native.Function.Call can only be called from the main thread."`（`Native.cs:1766-1777`）；对"该版本不存在的 native"没有任何兜底路径。因此 SHVDN 只能靠**编译期**版本门控来规避——`ExeVersions`（`source/scripting_v3/GTA/ExeVersions.cs:14-80`，列举 b335_2…b3407+）、`MinimumRequiredGameBuildAttribute`（`GTA/Shvdn/Script/MinimumRequiredGameBuildAttribute.cs`）以及在调用点显式比对 `Game.FileVersion`（例：`Entities/Entity.cs:232-235`）。脚本全局变量同理：`getGlobalPtr` 的文档注释直言 "ids may differ between patches"（`source/loader/sdk/inc/main.h:92-94`；`NativeFunc.cs:69-71`），托管侧只做 `IndexOutOfRangeException`（`Native.cs:3165-3175`）。
* **字符串型 native**：`PushLongString` 按 UTF-8 码点长度把长串切 ≤99 字节分块，逐块调 `0x6C188BE134E074AA /* ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME */`（`NativeFunc.cs:79-196`），是"哈希 + 行内注释"命名风格的典型样本（全仓 18 处）。
* **joaat 盐**：`StringHash` 在托管侧重实现了引擎的 jenkins-one-at-a-time：查表归一化（大写→小写、`\`→`/`、首字符为 `"` 时只取引号内子串）+ `hash += hash<<10; hash ^= hash>>6`，finalize 为 `+<<3, ^>>11, +<<15`（`StringHash.cs:375-402`），文档明确说明这就是 `GET_HASH_KEY` 使用的算法及其 256 项查找表；`AtHashValue`/`AtLiteralHashValue`/`IJoaatHashValue` 把 32 位哈希做成强类型（`GTA/AtHashValue.cs`、`GTA/IJoaatHashValue.cs`）。

### RAGE 内部布局泄露点

`NativeMemory.cs` 是全仓信息密度最高的逆向产物：静态构造里 **186 处** `MemScanner.FindPatternBmh/FindPatternNaive` 特征码扫描调用，再用 `Rel32<T>(address, disp)` 或 `*(int*)(address + k)` 从指令流里抽出偏移/绝对地址（`NativeMemory.cs:91-390` 起）。`MemScanner` 自带 Boyer–Moore–Horspool 实现与 `?` 通配掩码（`MemScanner.cs:69-135`），未命中只在 DEBUG 记日志（`MemScanner.cs:139-152`），生产环境静默降级为 null 指针——这是"新版游戏更新后功能悄悄失效"的根因。

泄露的结构布局包括：

* `rage::fwBasePool`：0x30 字节、无 vtable；字段 `PoolAddress@0x00 / Flags@0x08 / Capacity@0x10 / SlotSize@0x14 / FirstEmptySlot@0x18 / LastEmptySlot@0x1C / SlotsUsed@0x20(低 30 位)`，并用 `flags[i] & 0x80` 做有效性掩码、`& 0x7F` 做 handle 版本号，`handle = (index << 8) | counter`（`source/core/Game/Structs/FwBasePool.cs:12-120`）。注释还给出调试版 0x68 字节的差异，以及在**未装 ScriptHookV** 时用魔数 `D7 A8 11 73`（0x7311A8D7）反查默认容量的方法。
* `rage::sysMemPoolAllocator`：`PoolAddress@0x00`、`Size@0x08`、`BitArray@0x30`、`ItemCount@0x60`，并注明 `atIteratablePool` 占 0x60、`inlist<FreeNode,8>` 占 0x18（`RageSysMemPoolAllocator.cs:11-45`）。CVehicle 池就是这种分配器而非 `fwPool`。
* **句柄合成**：`FwScriptGuidPoolTask.Run` 直接遍历 ped/object/vehicle/building/interior 池，逐项调用逆向得到的 `rage::fwScriptGuid::CreateGuid`（`s_createGuid`，由特征码 `\x48\xF7\xF9\x49\x8B\x48\x08…` 前推 0x68 得到，`NativeMemory.cs:136-139`）现造脚本句柄；池将满时抛 `InvalidOperationException`（`NativeMemory.cs:3774-3872`）。这解释了大量 `World.Get*Handles` 的实现（`GTA/World.cs:473-765`），也说明 SHVDN 早已不依赖 ScriptHookV 的实体枚举 API。
* 指针链与裸偏移：`CPed+0x30 → phInstGta`、`CPed+0x50 → unkStruct`、`unkStruct+0x40 → animDirector`（`NativeMemory.cs:6460`；`Entities/Entity.cs:241-247`）、`CVehicle+0xD8` 的 "IsConsideredDestroyed" 3 位域（`Entities/Vehicles/Vehicle.cs:1665,1673-1680`）。
* 类族/内部符号名：`CGameScriptResource`（`Next@0x18`、链表头在 `CGameScriptHandler+48`，`NativeMemory.cs:5405`）、`YscScriptHeader`（`CodeBlocksOffset@0x10`、`NativeCount@0x2C`、代码页 0x4000，`Structs/Ysc/YscScriptHeader.cs:12-47`——被用来在游戏自身的 YSC 代码页里定位 `shop_controller`（表项哈希 `0x39DA738B`）中"解锁全部 DLC 载具"的 `GLOBAL_U24` 操作数字节序列，取到脚本全局索引后写 `GetGlobalPtr(idx) = 1`，并按 SHV 版本 `>=80 / >=46 / else` 分三套特征码，`NativeMemory.cs:932-985`）、`CPathNode`/`CPathRegion`（导航网格）、`CrSkeleton`/`CTask`/`FragInst`/`GtaFragType`（破坏件）、`PgHashMap`（`rage::pgHashMap`）、`RageAtArrayPtr`、`CModelList`、`GridMonitor` 等，全部以 `LayoutKind.Explicit` + `FieldOffset` 逐字节钉死。
* 直接函数指针调用：`delegate* unmanaged`（Stdcall/Cdecl）声明与使用共 98 行，绕过 native 表调用游戏内部函数（如 `GetPtfxAddress`、`getPlayerPedAddress`、`CNm` 消息发送，`NativeMemory.cs:6442-6456`）。

### 脚本域与生命周期

`ScriptDomain` 是 `MarshalByRefObject`，在 `AppDomain.CreateDomain` 中以 **ShadowCopy + Unrestricted PermissionSet + 无限生存期** 创建，域名单 `SHVDN_ScriptDomain_<hex>`（`ScriptDomain.cs:406-448`）。注意：仍是 .NET Framework 的 `AppDomain`，**不是** ALC（`AssemblyLoadContext` 在 net48 不可用）；隔离粒度是"整域卸载"，靠 `AppDomain.Unload` 释放脚本程序集（`ScriptDomain.cs:383-399`）。

* **加载**：源文件（`.cs`/`.vb`，可按文件名后缀 `.2.cs`/`.3.cs` 指定 API 版本）用进程内 Roslyn（`Microsoft.CodeAnalysis.CSharp` 5.9.0）编译，`allowUnsafe: true`（`Compiler/Presets/CsharpSourceFileCompilerPreset.cs:26-27`）；也可直接加载 DLL。API 版本由脚本基类 `GTA.Script` 所属程序集版本推导，v2 视为 deprecated（`ScriptDomain.cs:777-870, 2163`）。
* **调度**：默认每个脚本一个后台 `Thread`（`Script.cs:444-466`）。域主线程 `DoTick` 逐个 `SignalAndWait(continue, wait)` 唤醒脚本线程，脚本 `Wait(ms)` 就是 `_waitEvent.Release(); _continueEvent.Wait()` 的循环（`Script.cs:536-554`），即协作式让出；让出期间脚本线程阻塞，托管侧回到自己的循环。`NoScriptThread=true` 的脚本退化为域线程上的 `script.DoTick()`（`ScriptDomain.cs:1643-1665`）。跨线程 native 调用通过 `_taskQueue` + 信号量交回域线程（`ExecuteTaskInScriptDomainThread`，`ScriptDomain.cs:1540-1565`）。
* **超时/安全**：`CheapThreadSafeStopwatch` 度量每个脚本单 tick 耗时，超过 `ScriptTimeoutThreshold`（INI，默认 5000 ms）就 `Abort()` 并写 "Blocking script!"（`ScriptDomain.cs:1683-1700`）；附加调试器时豁免。native 调用是否重置超时由 `ScriptAttributes.NativeCallResetsTimeout` 控制，v3.7 起默认不重置（`ScriptDomain.cs:91`，`GTA/Shvdn/Script/ScriptAttributes.cs:20-40`）。
* **异常策略**：Key 事件异常只记日志并跳出该事件循环、脚本继续运行；`Tick` 异常判为致命 → `HandleUnhandledException` + `Abort()`（`Script.cs:395-439`）。处理策略是记录完整堆栈、提示 `SupportURL`、并在游戏 HUD 推一条红色 ticker（`ScriptDomain.cs:2101-2137`），**不吞异常、也不吞游戏**：所有 native 入口都在 `ExecuteTaskWithGameThreadTlsContext` 的 try/finally 中恢复 TLS（`ScriptDomain.cs:1508-1527`）。`Abort()` 使用 `Thread.Abort()`（net48 才允许），注释坦承在专用线程模型下无法消除竞态（`Script.cs:467-501`）。
* **热重载**：`ReloadKeyBinding`（INI）或控制台 `Reload()` 置位 `SharedVariables.DidRequestReloadResetFlag`（`Console/InbuildCommands.cs:51-63`），原生 `DidScriptRuntimeRequestReload` 读到后跳出 tick 循环，`ClrThreadProc` 重新 `Init` → 卸旧域、建新域（`DllMain.cpp:57-61,114-125`）。控制台（F4）本身也实例化在脚本域内以便调用 API，命令历史在重载间被"寄存"（`ScriptHookVDotNet.cs:35-133`）。日志写到程序集同目录 `.log`，每次 `Init` 清空（`Logging/Log.cs:26-40`）。

### 工程质量

纯 x64/net48/C# 9；核心工程开启 `AllowUnsafeBlocks`（97 个文件出现 `unsafe`）与 `SignAssembly`（`source/loader/PublicKeyToken.snk`）。混合互操作三件套并存：**P/Invoke**（仅 7 个 ScriptHookV 导出，用 mangled EntryPoint）、**`delegate* unmanaged[Stdcall]` 裸函数指针**（游戏内部函数）、**C++/CLI**（仅 loader，`#using "ScriptHookVDotNet.dll"`）。IL 层面还用了 `InlineIL.Fody` 织入以及 v2 工程的 **ildasm/ilasm 后构建重写**——把 `GTA.UI.WIDTH/HEIGHT` 字段改成带 `modopt(IsConst)` 的形式，以匹配当年 C++/CLI 生成的元数据、保持向后兼容（`ScriptHookVDotNet_APIv2.csproj` 末尾 `AfterCompile` target）。Costura.Fody 把 Roslyn 等依赖压进主 DLL。CI 有 4 条 workflow：`checks.yml`（PR 构建）、`build.yml`（版本推导 + 构建）、`nightly-release.yml`（推 main 即发 nightly 到独立仓库 `scripthookvdotnet-nightly`）、`stable-release.yml`（打 `vX.Y.Z` tag 触发）。测试仅覆盖 `Vector2/3/4`、`Matrix`、`Quaternion`、`Plane`、`GameClock*`（11 个文件 4 939 行），**ABI、特征码、域生命周期均无自动化测试**——只能靠实机验证，这也是 README 反复警告"必须 asi 与三个 dll 同版本更新"的原因。`.editorconfig` 统一 4 空格 / 120 列 / UTF-8，且对 SHV SDK 头文件与 README 单独设 CRLF。

### 开源 ↔ 闭源边界

| 层次 | 归属 | 位置/证据 |
|---|---|---|
| 游戏引擎 hook、native 哈希→地址表、脚本 fiber/GtaThread 管理、`getGlobalPtr` 实现、纹理绘制 | **闭源**（`ScriptHookV.dll`/.asi，作者 crosire） | 仅有 SDK 头 `source/loader/sdk/inc/main.h` 与导入库 `source/loader/sdk/lib/ScriptHookV.lib`（7 888 B，被 force-add 进版本控制） |
| native 名称/参数/返回类型数据库 | **第三方开源**（`alloc8or/gta5-nativedb-data` 的 `natives.json`） | `ScriptHookVDotNet.Loader.vcxproj:95`；仓库内无 `.json` |
| ScriptHookV SDK 下载 | 外部，脚本甚至注明镜像站证书会 526 | `tools/install_sdk.ps1:3-6`（`ntscorp.ru/dev-c/…SDK_1.0.617.1a.zip`） |
| CLR 宿主、TLS 窃取、fiber/VEH 规避 | 开源（本仓） | `source/loader/DllMain.cpp` |
| native 调用 ABI 编组、脚本域/线程/超时/异常 | 开源（本仓 `SHVDN`） | `source/core/**` |
| RAGE 内部结构、池、偏移与特征码 | 开源（本仓，SHVDN 自研逆向，与 ScriptHookV 无关） | `source/core/Game/NativeMemory.cs`、`Structs/**`、`MemScanner.cs` |
| 面向 mod 作者的 C# API（6 460 native + 对象模型） | 开源（本仓） | `source/scripting_v3/**` |

关键结论：**闭源部分只剩"哈希→函数指针"这一步查表**；实体池、内部函数调用、内存偏移这些真正的逆向工作反而在 SHVDN 自己手里，`NativeMemory` 就是一份可编译的 RAGE V 布局文档。

### RE 来源标记

1. 生成器与数据源：`NativeHashes.cs:1` 的 nativegen 链接 + vcxproj 里 `NativeGen.exe https://github.com/alloc8or/gta5-nativedb-data/blob/master/natives.json?raw=true`。
2. 行内"哈希 + 原名"注释：`0x6C188BE134E074AA /* ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME */`（`NativeFunc.cs:93`）、`0x2B3334BCA57CD799 /* RELEASE_SCRIPT_GUID_FROM_ENTITY */`（v2 `RaycastResult.cs:46`）等 18 处。
3. 明确点名 CodeWalker 并附 commit 级 URL：`GTA/GetSafePositionFlags.cs:16-17`（`dexyfex/CodeWalker/…/YnvFile.cs#L756`）、`Entities/Vehicles/Vehicle.cs:283`、`Peds/Task/TaskInvoker.cs:954`、`core/Game/Structs/FwBasePool.cs:37`、`NativeMemory.cs:2308`。
4. 教读者如何自行复核的注释："search the dumped exe for hashed values like `HANDLING_TYPE_FLYING`"（`Entities/Vehicles/HandlingType.cs:12`）、`PV_FLAG_*`（`HelmetPropFlags.cs:17`）、`CPED_RESET_FLAG_*` 与具体哈希 `0x49F290D0`（`PedResetFlagToggles.cs:7-8`）、`BF_*` / `0x61C7368E`（`CombatAttributes.cs:13-14`）、以及直接给出 x64 字节序列让社区找新偏移（`TaskInvoker.cs:1281`）。
5. 版本敏感的硬编码：`ExeVersions`（68 个构建号）、`if (gameVersion >= 80 || gameVersion <= 25)` 之类的 SHV 版本号阈值分支（`NativeMemory.cs:2629-2657`）、"The offset is 0x78 in b2944… 所以只能再加一条特征码"（`NativeMemory.cs:724`）。
6. 未发现 gtaForums / unknowncheats 的直接引用（推断：早期社区出处已被迁移为文档/Wiki 与 CodeWalker/nativedb 两条可溯源链路）。

### 对逆向 GTA VI 的启示

1. **薄原生层 + 厚托管层**是可复制的架构：把原生侧限制在 TLS/线程/fiber 与几个 P/Invoke 入口，其余全部用 `delegate* unmanaged` + `LayoutKind.Explicit` 在 C# 里做，能把迭代周期从"重编 C++"降到"改一个 6 634 行的 cs 文件"。
2. **特征码而非地址**是唯一可行的更新策略，但本仓实证其代价：186 条模式、失败静默、无自动化回归。任何 GTA VI 工具链应从第一天起建立"每条特征码 + 期望版本 + 失败即显式降级并写日志"的清单化机制（`MemScanner.LogMemPatternNotFound` 目前只在 DEBUG 生效）。
3. **把闭源 DB 转成编译期枚举**（NativeGen → `enum Hash : ulong`）既规避运行时解析开销，也把"未知哈希"问题前移为"必须更新 nativedb"。对 VI 而言，若 native 不再是 64 位哈希直查，这一层需要重做；但 `MaxArgCount = 63`、64 位槽、Vector3  stride=8 的 ABI 假设仍需重新验证。
4. **句柄语义可被逆向重建**：`handle = (index<<8)|version` + 自研 `fwScriptGuid::CreateGuid` 调用，证明无需官方 API 也能枚举并接管引擎对象池——这对 VI 的 `CPed`/`CVehicle` 池（布局大概率换代表 `atPool`/`seqPool`）是方法学模板，而非数值模板。
5. **CLR 与游戏主线程的冲突要早处理**：fiber/栈边界误报、TLS `_tls_index` 不匹配（`DllMain.cpp:72-79` 的 `EmptyClrMethodForEagerClrDllLoading` 技巧）、键盘回调线程与 tick 线程的竞态，这三类问题在任何"托管 mod 宿主 + 原生 hook"组合里都会重现。
6. **兼容多 API 版本的域模型**（v2/v3 双程序集 + 按基类版本解析 + ildasm 后处理）说明 mod 生态的向后兼容成本可能高于逆向本身；VI 早期工具链宜一开始就锁定单一版本面，并用 nightly/stable 双通道发布（本仓已如此运作）。
