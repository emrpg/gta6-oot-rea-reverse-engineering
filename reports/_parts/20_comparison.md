## 6. 横向对比：五种逆向路线的技术指纹

### 6.1 分类框架

GTA 生态里的"逆向"其实混着五种完全不同的目标，把它们放进一张表是理解本报告的前提：

| 路线 | 定义 | 代表项目 | 产出物 | 法律暴露面 |
|---|---|---|---|---|
| **A. 引擎重写（reimplementation）** | 不注入原程序，用公开规格重建可运行替代引擎，用户自备正版资产 | OpenRW、librw（RenderWare 本体）、re3/reVC、~~OpenRPA~~（源不可核验） | 可执行程序/库 | **最高**（Take-Two 2021 诉讼即针对此） |
| **B. 二进制等价反编译** | 逐函数重写以匹配原 exe 布局/行为，服务于移植与修复 | re3（部分目标）、RPi/Switch/WiiU/3DS 移植分支 | 源码 + 移植版 | 高（同 A） |
| **C. 运行时接管（runtime hook）** | 保留原引擎，注入 DLL/ASI 读取内部结构并暴露 API | ScriptHookV(闭源) + **SHVDN(开源)**、**FusionFix**、MTA、FiveM 客户端 | 插件/框架 | 中（单人容忍、在线禁止） |
| **D. 资产格式逆向** | 只做文件解析与编辑，不执行程序 | **CodeWalker**、OpenIV(闭源)、dff2gltf | 编辑器/库 | 低—中（2017 OpenIV 停止函证明仍会被追） |
| **E. 多人状态复制** | 在 C/D 之上重建网络同步与脚本 VM | **FiveM/CitizenFX**、MTA、alt:V、RageMP | 服务端 + 客户端 | 已被发行商**收编**（收购 Cfx.re） |

### 6.2 可量化指标（全部本地实测，同一口径）

统计口径：`--depth 1` 浅克隆，剔除 `vendor/ contrib/ extern/ third_party/ node_modules/ deps/ packages/`；行数为物理行。

| 指标 | openrw | mtasa-blue | CodeWalker | scripthookvdotnet | citizenfx-fivem |
|---|---|---|---|---|---|
| 一手源码行数 | 62,959（含 `.inl` 12,928） | 614,522 | 327,165 | 200,663 | 1,275,212 |
| 主语言 | C++17 | C++ | C# | C# (+241 行 C++/CLI) | C++/JS/Lua/C# |
| 仓库总文件数 | 351 | 9,180 | 725 | 605 | 13,425 |
| 仓库内预编译二进制件 | 0 | 84 | 1 | 3 | 319 |
| 含 `static_assert` 的文件数 | 1 | 64 | 1 | 0 | **233** |
| 含 `#pragma pack`/`StructLayout` 的文件数 | 0 | 20 | 1 | 46 | 70 |
| 十六进制常量出现次数（3–8 位） | 2,185 | **123,141** | 2,510 | 22,447 | **276,388** |
| RenderWare/`Rp*`/`Clump` 符号命中文件 | 17 | 64 | 0 | 0 | 15 |
| 许可证 | GPL-3.0 | GPL-3.0 | **无 LICENSE 文件** | Zlib 类（自定义三条件） | **Take-Two + LGPL-2.0 白名单** |
| 最近提交 | 2025-06-18 | 2026-11-17 | 2025-04-11 | 2026-11-15 | 2026-11-17 |
| ★ / fork | 2,275 / 213 | 1,901 / 604 | 773 / 354 | 1,379 / 624 | 4,329 / 2,823 |

第二批四个仓库同口径实测（补齐"接管/数据/工具"三档）：

| 指标 | librw | GTAIV.EFLC.FusionFix | txAdmin | citizenfx-natives |
|---|---|---|---|---|
| 一手代码行数 | 139,843 | 392,909 | 57,519 | —（纯数据） |
| 文件数 | 306 | 3,362 | 729 | 6,449 |
| 主语言构成 | C++ 106,610 / 头 30,979 | **反编译着色器 ASM 315,659** / **C++ 68,497**（62 个 `.ixx` 模块 50,222 + 头 16,911 + cpp 1,364；扣 vendored 的 DXSDK 头 10,886 与 `source/gxt` 1,827 后**自有约 55,800**）/ Lua 5,886 / HLSL 2,867 | TS 36,320 / JS 15,581 / Lua 5,618 | 44 个命名空间目录、6,437 份 Markdown |
| 含 `static_assert` 文件 | 2 | 7 | 0 | 0 |
| 十六进制常量 | 2,600 | **3,846** | 526 | 0 |
| RenderWare/`Rp*`/`Clump` 命中 | **21** | 0 | 0 | 0 |
| 仓库内二进制/打包件 | 1 | **1,788**（含 239 MB 衍生游戏资产） | 0 | 0 |
| 许可 / 活跃度 | MIT / 2026-08-26 | GPL-3.0 / 2026-11-17 | MIT / 2026-10-23 | 见仓库 / 2026-10-27 |

这批数字里最值得写进方法论的一条是：FusionFix 的 68,497 行 C++（自有约 55,800）中有 **50,222 行位于 62 个 `.ixx`（C++20 模块接口文件）**。用常规 `*.cpp/*.h` 口径统计，它会伪装成"千余行胶水 + 一堆反编译着色器"的空壳项目 —— 我第一轮就统计错了。修正口径后它的真实形状是 **3,846 处十六进制常量 / 1,681 条去重特征码 / 575 个 hook 安装点 + 315,659 行 fxc 反汇编**：签名接管与字节码重建两层同时做到极致（详见 6.3 形态五、3.8）。

### 6.3 从指标读出的六种逆向"证据形态"

**形态一：格式解析（最干净的知识）** —— CodeWalker / OpenRW。
它们的知识不体现在断言或偏移上，而体现在**能读也能写回**。CodeWalker 有 0 个 RenderWare 符号、`#pragma pack` 仅 1 处，却有 20,688 行的 `MetaNames.cs` 与 16,524 行的 `PsoTypes.cs` —— 全部知识以**哈希→名称表 + 类型反射表**的形式存在。OpenRW 同理：它镜像 RenderWare 的是**二进制流格式（chunk ID）**而不是内存布局，全仓唯一的 `static_assert` 是 `sizeof(BasicState) == 0xBC`（存档结构）。

这条路线上还有一个内部光谱 —— **"哈希→名称"知识的三种取得途径**，成本差一个量级：① CodeWalker 对游戏 exe 做 375 项 SHA1 指纹扫描 + PSO 反射，把哈希与结构一起解出；② CFX 用反汇编 + 运行时导出维护 6,437 份 native 文档数据库；③ FusionFix 仓内 vendored 的 `source/gxt`（Sergeanur/GXT-compiler）只要**一部字典**：`Jenkins_dictionary.txt` 91,230 行、`crc32_dictionary.txt` 16,931 行，靠 `Hash.h:44-58` 的 `map<uint32_t,wstring>` 惰性载入做原像穷举，未命中才退化为 `#%08X`。原因是这些键取自人类词汇而非随机串，**32 位甚至 64 位哈希在这种键空间上都不具备单向性**。该工具同时留下一张跨代 key 哈希家族表（`-k`：CHAR8 旧作 / CRC32=SA / JENKINS=IV / CHAR8_TIME、CHAR12_TIME=Mafia II，外加 `-w0|1` 字符宽度与 `-b` MH2 Wii 大端），是本地仓库里少见的"一行参数概括一个引擎世代"的样本。（详见 3.1、3.4、3.8）

**形态二：偏移表（最脏也最直接）** —— MTA。
本仓库实测有 123,141 处十六进制常量；其内部更精确的计数是 **481 个 `#define FUNC_xxx 0x…`**、**217 个 `*SAInterface` 头文件**、**107 处 `static_assert(sizeof(...)==0xNN)`**、92 个 RenderWare 函数地址（`gamesa_renderware.hpp`）、324 处 0xB7/0xBC 数据段引用 —— 这是一个"逐地址钉死零售 exe"的项目应有的形状。它的注入方式尤其硬核：**不改运行中的进程，而是离线重写零售 exe 的导入表**（`GameExecutablePatcher.cpp:154-200` 把 `WINMM.dll` 原地改名成 `mtasa.dll`），并用 SHA256+文件长度锁定目标版本（`:18-21`），再补 LAA/DEP/时间戳；naked-hook 甚至写了 `__LOCAL_SIZE` 自检工具（`utils/src/hookcheck` 读 PDB 事后审计），这在整个生态里都少见。代价同样明显：伺候多个 Retail 版本让地址表线性膨胀，且**真正的网络层（`netc.dll`/`net.dll`/`net.so`）不在源码树里，是构建时从 CDN 拉下来的**，仓库只剩纯虚 ABI —— 也就是说 MTA 的"开源"同样是可重写的接管壳，而非全栈。

**形态三：特征码扫描（偏移表的后继演化）** —— SHVDN。
241 行 C++/CLI、零 `static_assert`，但 22,447 处十六进制常量里，`NativeMemory.cs`（6,634 行）内含约 186 处签名扫描，并直接自造 `rage::fwScriptGuid` 句柄 —— 它已不依赖上游闭源 ScriptHookV 的枚举 API，而是**自己从 RAGE 的内存分配器里取对象**。这是接管类项目摆脱"版本脆弱性"的技术转折点。（详见 3.3）

**形态四：符号数据库（规模化的知识）** —— FiveM。
233 个断言文件 + 276,388 处十六进制 + `ext/natives/natives_stash/gta_universal.lua`（76,015 行、5,300 条声明、4,777 个唯一哈希）与整套 `codegen_out_{cs,cs_v2,js,lua,dts,enum,json,doclua,markdown}.lua` 生成器，加上独立的 `citizenfx/natives` 数据仓库（**44 个命名空间目录、6,437 份 Markdown**，NETWORK 824 / VEHICLE 751 / PED 612 为前三）—— native 表不再是"手抄"，而是**数据库 + 多语言代码生成**。知识一旦进数据库，覆盖面就与人力线性解耦，这是它规模比其他项目之和大得多的结构性原因。但数据库化也把"数据库自身会腐"这件事暴露出来：同一个 64-bit 哈希 `0xCFEB8AF24FC1D0BB` 在 `NETWORK/NetworkBlockJoinQueueInvites.md`（带 32-bit `0x8AB4890B`、alias `_NETWORK_BLOCK_INVITES_2`）与 `NETWORK/N_0xcfeb8af24fc1d0bb.md`（无 32-bit、签名写作 `void _0xCFEB8AF24FC1D0BB(BOOL p0);`）里各存在一份；而 CI 只对**本次改动过的文件**跑校验（`.ci/run-pr:29-37`，`changed_docs` 循环内 `node verify-one.js`），校验器本体还在仓外（`.ci/setup-builder` 每次浅克隆 `citizenfx/native-doc-tooling`），全局一致性无人守护；JOAAT 不变式 `jhash == joaat(lower(name))` 只在 4,313/4,881 条记录上成立。（详见 3.6、3.9）

**形态五：签名当版本键 + 字节码层重建（当代最完整的接管范式）** —— FusionFix。
先纠正一个极易误判的表象。本仓 **315,659 行是 `.asm`**（1,763 个文件），第一眼像"汇编 hook 大集合"，实际是 **Microsoft (R) HLSL Shader Compiler 9.26.952.2844 反编译出的 RAGE 着色器字节码**，按平台/模型/显卡厂商分目录（`shaders/GTAIV.EFLC.FusionShaders-temp/win32_30_nv8/<shader名>/<name>PS2.asm`），且每份头部写着改动意图：`// Summary: improved cascaded shadows + added volumetric fog + added unclamped directional toggle`。而它自有的 C++ 并不少，只是**藏在非主流后缀里**：62 个 `.ixx`（C++20 模块接口）50,222 行 + 30 个 `.h` 16,911 行 + 9 个 `.cpp` 1,364 行 = **68,497 行**（扣除 vendored 的 `source/dxsdk` 头 10,886 与 `source/gxt` 1,827 后，真正自有约 55,800 行），其中 59 个 `export module xxx;` 与 fix 子系统一一对应 —— 我第一轮统计漏掉 `.ixx` 而得出"C++ 仅 1,370 行"，这本身就是一条工程观察：**用前沿标准（modules）写插件的老引擎 mod，会被常规统计口径当成空壳。** 地址/签名侧实测：3,846 处十六进制常量、1,703 处特征码字面量（去重 1,681 条，分布 52 个模块）、`hook::pattern(` 832 / `find_pattern(` 408、**575 个 hook 安装点**（`safetyhook::create_mid` 290 + `create_inline` 125 + `injector::MakeCALL` 89 + `MakeInline` 59 + `MakeJMP` 12），另有 `MakeNOP` 292、`WriteMemory` 132 处纯字节补丁；MinHook / Detours 命中为 0。它最独特的知识形态是**把字节指纹当作版本键**：全仓没有版本表，只有 223 处 `pattern.empty()` 分支与 456 条 `// 1.0.7.0 … 1.2.0.59` 注释，多候选 `find_pattern`（`fixes.ixx:512`）承担跨 build 分派；IDA 风格名（`dword_11F7060`、`sub_69F0C0` 等 64 处）被原样保留为**变量名**，真地址在运行时由签名位移解出。再往下才是字节码层：`shaders.ixx:29-48` 用 `hook::range_pattern` 在 DXBC 里反查自己植入的 `"FusionShader"` 标记以识别"第 27 号 gta_default PS"，`:444-465` 用模式 `"A1 ? ? ? ? A3 ? ? ? ?"` 枚举 exe 镜像内的嵌入式着色器注册表并改写数据指针，`shaders.ixx:264-338` 干脆经 `rage::grmShaderInfo::registerShaderParam()` **按名字向 RAGE 的参数表注册槽位**再 `SetPixelShaderConstantF(212, …)` —— 等于接管了引擎的常数寄存器分配。（详见 3.8）

**形态六：库级 API 镜像 vs 格式级镜像 —— RenderWare 的两条路线** —— librw ↔ OpenRW。
两者都自称"重实现 RenderWare"，手法却几乎相反。**librw**（MIT，作者 aap 即 re3 的作者）全仓 `Rp*` 命中 **0 次**、`Rw*` 仅 2 次，也没有 `naRw*`/`rpBaseData`/`GetT`/`RhDevice*`/`RpEngineRegisterNamespace`/`RWPLUGINENTRY` —— 它走的是"**语义同构、命名重构**"：`namespace rw` 下 609 条头文件声明，但**机制层一比一复刻**：11 个 `PLUGINBASE` 对象（`src/rwobjects.h:31/225/367/430/515/610/672/734/805/855/881`）、133 个 `PLUGINOFFSET`、`MAKEPLUGINID`/`Vend_*`/`libraryIDPack`（`src/rwbase.h:579-725`）、`Object{type,subType,flags,privateFlags,parent}` 五元头。它的 `static_assert` 只有 2 条且**全被注释掉**（`src/skin.cpp:438-439`），布局保真靠手工 `p += 4` 偏移解析（`src/d3d/d3d9.cpp:172-192`）—— 说明它**只求磁盘 chunk 兼容，不求内存兼容**。`OpenRW` 则只有 17 个文件命中同类 RW/Clump 符号、**全仓仅 1 条生效的 `static_assert`**（`sizeof(BasicState)==0xBC`），完全把 RW 当"资产流格式规格"处理（`rwcore/loaders` 1,572 行）。体量对照（同口径、本地实测）：librw 的 RenderWare 本体 `src/` **55,597 行**、`skeleton/` 71,724 行、`tools/` 12,092 行；OpenRW 的 RW 层 `rwcore/` **12,124 行**、游戏层 `rwengine/` 38,400 行。差距 4.6 倍，且 librw 额外维护 **5 套渲染后端**与五平台互转工具，HEAD 提交刚把 **PS2 从 SCE SDK 完全解绑**（ee-gcc 2.9 + newlib + `ctorgen` 补 `_GLOBAL_$I`，`tools/demoreel/Makefile:66-125`）—— 这是整个生态里最靠近"库级考古"的一端；而它的 `rpworld` 至今仍是 stub（`src/rwobjects.h:852`，无 sector tree），`rpsprite/rpgui/rpu2d/rppatch/rpbsp` 直接缺失。结论：**"逆向到什么深度才算够"在本报告里有真实对照组**，librw 是引擎级、OpenRW 是可玩级。（详见 3.1、3.7）

### 6.4 完成度、失败模式与共同缺口

| 项目 | 达成 | 关键缺口（代码级证据） |
|---|---|---|
| OpenRW | GTA III 主线可运行；DFF/TXD/IMG/COL/IFP/IDE/GXT/SCM 解析；渲染/物理/音频/窗口全部替换 | **仅 GTA III**（`_target` 解析后从未读取）；脚本 opcode **904 绑定 / 389 桩（约 43%）**，Release 下桩展开为空；**存档写入未实现**；`getPlatformString()` 硬编码 `"PC"`；真实游戏数据的 26 个测试被 `disabled()` + `label("data-test")` 排除在 CI 外 |
| MTA:SA | 客户端+专用服务端的**全部游戏接管层**、Lua 脚本层（`CStaticFunctionDefinitions` 客户端 10,306 行 / 服务端 12,864 行）、实体同步协议（`SyncStructures.h` 2,462 行）+ 313 个 gtest 往返测试 | **netcode 不在开源树内**（`netc.dll`/`net.dll`/`net.so` 由 `utils/buildactions/install_data.lua:10-15` 从 CDN 下载，仓库只剩纯虚 ABI `Client/sdk/net/CNet.h:63-137`）；仅 Windows 出客户端；顶层逐字 GPLv3 但 `Shared/publicsdk/LICENSE` 禁商用；仓库内嵌 75 个二进制与 Rockstar 衍生数据（`sa.dat`、`gta_sa_diff.dat`）；绑定 Retail SA 版本，地址随官方更新漂移 |
| CodeWalker | GTA V 绝大多数资产类型读写往返（RPF TOC 重建/扩容/碎片整理，AES=Rijndael-256-ECB、NG=17 轮表密码）；世界/模型/地图编辑；ModManager 方向 | 仓库**无 LICENSE**（仅 "educational purposes only"）；密钥以 154 KB 混淆块 `magic.dat` 内置而非明文，仍需对自有 exe 做 375 项 SHA1 指纹扫描反推；纯离线，无游戏内组件；Gen9（Enhanced 版）布局靠 `BlockLength_Gen9` 分支追赶 |
| SHVDN | 完整 .NET API 表层、Euphoria 辅助（30,986 行）、native 枚举由 NativeDB 生成（v3 6,460 具名） | 依赖**闭源 ScriptHookV** 才能进游戏；无未知哈希运行时兜底；隔离仍是 `AppDomain`(net48) 而非 ALC；测试仅覆盖数学类型 |
| FiveM | 组件化框架、natives 数据库+代码生成、netsync、多语言运行时 | 关键 loader/反作弊/部分组件不在本仓库；**非白名单代码不得自由复用** |
| librw | RenderWare 机制层一比一（11 个 `PLUGINBASE`、133 个 `PLUGINOFFSET`、`MAKEPLUGINID`/`Vend_*`/`libraryIDPack`）；5 套渲染后端；五平台互转工具；HEAD 已把 PS2 从 SCE SDK 完全解绑（ee-gcc 2.9 + newlib + `ctorgen`） | `rpworld` 至今仍是 stub（`src/rwobjects.h:852`，无 sector tree）；`rpsprite/rpgui/rpu2d/rppatch/rpbsp` 缺失；仅有的 2 条 `static_assert` 被注释掉（`src/skin.cpp:438-439`）→ **不保证内存布局兼容**，只保证磁盘 chunk 兼容 |
| FusionFix | 当代最完整的开源接管闭环：68,497 行 C++（自有约 55,800）（59 个 `export module` 的 `.ixx`）+ 1,681 条去重特征码 + 575 个 hook 安装点（safetyhook/injector 自持）+ RAGE 类布局逐字段誊写（`comvars.ixx` 140 条签名）+ RPF 容器 TOC 解密（`rpfloader.ixx`）+ 着色器参数表按名注册与常数寄存器接管（`shaders.ixx`）+ 1,763 份 fxc 反编译 asm（带 `// Summary:` 意图注释）+ 8 语言 863 行设置 schema 经 `asi-settings-validate.exe` CI 校验 | **hook 基础设施全在子模块里**（`external/{injector,hooking,inireader,FusionFixUtils}` 与着色器子模块在浅克隆中均为空目录，只能从调用点反推语义）；**没有版本表**，跨 build 全靠 223 处 `pattern.empty()` 分支 + 456 条版本注释，exe 一旦重编译即成片失效且无自动化回归（仅设置校验与 `fxc -Verify`）；仅 32 位 x86、IV/EFLC 单一目标；GPL-3.0 之上另挂 CLA 允许 Owner 再许可至专有/商业条款（`contributing.md:11`），并重新分发 239 MB 游戏衍生资产，vendored 的 `source/gxt/license.txt` 正文只有一行上游 URL |
| citizenfx/natives | 6,437 份 Markdown 即 native 数据库本体（44 个 namespace，NETWORK 824 / VEHICLE 751 / PED 612），双哈希（64-bit JOAAT + 32-bit）+ `aliases` 元数据 | 纯文档仓库、无 IDA/Ghidra 产物、无 `status` 字段（置信度靠散文：unknown 108、unused 66）；校验器在仓外（`.ci/setup-builder` 浅克隆 `native-doc-tooling`），CI 只校验**本次改动的文件**（`.ci/run-pr:29-37`），全局一致性无人守（实测同一 64-bit 哈希 `0xCFEB8AF24FC1D0BB` 有两条记录）；JOAAT 不变式仅 4,313/4,881 成立 |
| txAdmin | 服务端运维层完整实现（React+TS+Node，MIT，但 © Take-Two）：菜单/权限/计划任务/discord 桥接/一键 recipe 部署 | 与引擎逆向**零重叠**：不含任何游戏内存布局；它消费 FiveM 的 closed netcode 与 CFX native 表层，本身不含反作弊基线（pure 校验在引擎侧 `PureModeState.h:12-38`） |

五条共同规律，直接可用于第 7 章的路线图判断：

1. **没有任何一个项目做到了"完整"** —— 唯一的完整是"够玩/够用"：OpenRW 停在 57% opcode、SHVDN 停在依赖闭源上游、FiveM 停在闭源件缺席。
2. **知识密度与可移植性正相关，与偏移依赖负相关**：格式/数据库路线（CodeWalker、FiveM）能跨版本存活，偏移路线（MTA）每版本重做。
3. **规模最大者自由最少**：FiveM 代码量第一、许可限制也第一。这决定了"GTA 逆向的工业级成果已经被发行商内部化"。
4. **CI 从不碰真实资产**：所有项目都无法在合法 CI 里加载原版数据，于是"完成度"只能靠人肉验证 —— 这是重写类项目十年不能完全收敛的制度性原因。
5. **同一项目在不同统计口径下会伪装成不同物种**：FusionFix 的 68,497 行 C++ 有 73% 在 `.ixx`（C++20 模块）里，`*.cpp/*.h` 口径下只剩 1,364 行；citizenfx/natives 的"代码量"是 0，知识却全在 6,437 份 Markdown 里；OpenRW 的 12,928 行 `.inl`（opcode 绑定表）若不计入，其总量会凭空少掉五分之一。结论：**任何"按语言/后缀统计衡量逆向深度"的方法，必须先确认非主流后缀集合**（`.inl` / `.ixx` / `.md` 数据库 / codegen 产物 / 反编译 asm），否则会得出方向相反的结论 —— 本报告第一版就真的得出了"FusionFix 只有千余行胶水、重心已搬离地址层"的错误判断（已在本章订正）。
