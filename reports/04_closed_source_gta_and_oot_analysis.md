# 闭源侧深度技术对比：GTA 系列（RAGE / GTA VI）×《塞尔达传说：时之笛》官方产品线

> 报告编号：04  
> 报告日期：2026-11-18  
> 分析对象：Rockstar / Take-Two 与 Nintendo 两侧的**闭源项目**，以及 `/root/wsl-demo/` 下 60+ 个本地开源仓库所提供的**逆向镜像证据**  
> 方法：公开一手资料（专利、官方站点、法庭/DMCA 记录、开发者访谈）+ 本地仓库源码实读（所有行号均已逐条核对）  
> 前序报告：`01`（GTA 开源生态）、`02_oot_decomp_analysis.md`（OoT 反编译/重编译生态）、`03`（ScriptHookV/CodeWalker）。本报告不重复其结论，只做闭源侧的增量深挖与横向对比。

---

## 证据分级图例（全文强制使用）

| 级 | 含义 | 判定标准 |
|---|---|---|
| **F** | 已证实 | 本地仓库源码可直接核对（附 `路径:行号`），或厂商/官方页面/专利正文/法庭文件原文 |
| **R** | 可靠二手 | 主流媒体（Digital Foundry、Kotaku、Eurogamer、GamesRadar、PC Gamer、IGN）或可靠百科引注，有明确日期 |
| **U** | 传闻/推断 | 单一来源、社区推断、爆料者自述、无公开号可查的"专利" |
| **?** | 未知/未找到 | 检索后明确无一手来源；**不得当事实使用** |

**关于闭源分析的一条硬约束**：闭源项目没有源码，因此任何"技术结论"只可能来自三个通道 —— (1) 厂商自愿公开（官网、开发解析视频、招聘、致谢页）；(2) 法定公开（专利说明书、法庭文件、DMCA 通知）；(3) 第三方逆向镜像（modding 工具、反编译、模拟器）。本报告第 2、3 章的主体就是**把每条闭源论断钉死在它属于哪个通道上**，并把第 (3) 类证据落到本仓库的具体行号。

---

## 0. 核心判断（先给结论）

1. **GTA 与 OoT 的"闭源"性质完全不对称。** OoT 的四个官方闭源世代（N64、GC/Wii 内置模拟器、3DS Grezzo、Switch 2 EPD 重制）中有**三个已被社区逐函数反编译或静态重编译**，因此本报告能对任天堂闭源引擎给出 F 级内部结构描述；RAGE 侧至今**没有任何一款官方引擎被反编译**，能给出的只有"格式层测绘"（CodeWalker 的 17.7 万行 C# 解析器）与"调用契约层"（native 哈希表），引擎实现本身仍是 F 级不可达。
2. **不对称的根源不是技术难度，而是"是否需要加密输入"。** OoT 的 N64/GC/Wii 载体**没有运行时加密**，dump 即得明文 MIPS/PPC 码，反编译可以做"字节匹配"这种可验证工程；RAGE 的资源在 `update.rpf` 内是 AES/NG 加密的、脚本 VM 是私有的、主机可执行文件是平台密钥加密的，逆向者拿不到可公开验证的输入。第 5 章把它总结为"**加密门控定律**"。
3. **GTA VI（2026-11-19 发售，本报告完成日次日）在技术上今天是不可分析对象**：无 PC 版、无用户侧明文二进制、Rockstar Mod Guidelines 明文禁止"运行在未发布平台"、BattlEye 类反作弊前置。因此本报告的 GTA VI 部分严格限定为"官方物料 + 专利 + 帧分析"三层，且第 2.5 节标注了 **2026-08 泄露事件公开材料中没有任何一条指向构建系统或工具链名称**。
4. **Take-Two 已经把"闭源生态"制度化收编**：FiveM/RedM 于 2023 年被收购（`fivem/LICENSE:1` 现在第一行就是 `© 2023 Take-Two Interactive Software, Inc.`），2026-02 起 alt:V、2026-05 起 RAGE:MP 被以"FiveM 是唯一授权平台"为由关停。**今天世界上最大的"开源 GTA 项目"在法律上属于发行商本身**，这是闭源控制的高级形态，不是开源胜利。
5. **任天堂的闭源控制走的是相反方向：把仿真内化为订阅商品**（Nintendo 64 – Nintendo Classics），同时**对净室反编译项目零公开执法**（zeldaret 系列仓库至今健在），但**对 ROM 分发与密钥规避极高强度打击**（Yuzu 2024-03 和解 $2.4M、2026-08-21 一次清除 401 个"反规避"仓库）。
6. **OoT 生态已经出现"闭源产品 → 静态重编译 → PC 原生发行"的完整闭环**：2026-11-05 发售的 Switch 2 重制版（program `0400486024DEC000`、update v131072、build `e0f3ccfa`）在 13 天内就被 ProjectReLink 公开静态重编译（`ProjectReLink/README.md:5-6,24-26`），并由 Hookshot 独立交叉印证同一 build-id（`Hookshot/docs/design.md:179-196`）。这是 GTA 侧完全不存在的现象。
7. **CryZENx 的十年闭源民间重制版于 2026-06-13 终止**，触发点是 2026-06-09 官方 Switch 2 重制官宣（R 级，来源为其 Patreon 公告的媒体转述）。它给出的行业含义是：**"用现代引擎重建"在法律上比"反编译"更危险**（前者直接落入角色/关卡/音乐等受保护表达与演绎权，后者贴近净室），因此闭源民间重制只能靠主动停手来规避。
8. **GTA 侧真正的"专利架构"是存在的且可核对**：Take-Two 名下（受让人一律为 Take Two Interactive Software，以 Rockstar 名义检索 Google Patents 得 0 条）有覆盖实时形变（US10395410B2）、稀疏流体（US10685152B2）、连续 LOD（US11978162B2）、粗粒度导航图（US11684855B2）、mocap 运行时重定向（US11951384B2）、数据驱动动画组合（US11620781B1）、混淆式反作弊（US11273380B1）的成族专利。**"世界流式加载"与"PSO"反而检索不到专利**，说明这两项是通过格式与工具保密、而非专利保护。
9. **广为流传的 RAGE 版本号体系（RAGE 7/8/9、eRAGE、EVolution、RAGE aR）全部无一手出处**，本报告将其列为**需要证伪的社区造词**（第 2.1.4 节）；GTA VI 官方从未确认"使用 RAGE"，中文媒体的"新一代 RAGE 引擎"是报道口径而非厂商陈述（U）。
10. **同样需要证伪的 OoT 前提**（本报告用本地源码逐条推翻）：Z-targeting 专利不是 US 6,443,842 而是 **US 6,165,073 + 分案 US 6,626,760**；微码不是 F3DEX 而是 **F3DZEX**；压缩里没有 RefPack/Yay0，只有 **Yaz0（N64/GC）与 gzip（iQue）**；OoT 没有 MM 式的 `ZARCLIB/.arc` 归档容器，资源定位纯靠 VROM 区间表；`include/segments.h` 与 `SM_*` 宏在当前 HEAD 已不存在。
11. **闭源 PC 移植质量有一条可量化的证据链**：FusionFix 用 59 个 `.ixx` 模块 + 170 项配置修复 GTA IV/EFLC 的 32 位地址空间枯竭（`source/memory.ixx:243-247` 原文点明"先耗尽的是地址空间，不是显存"）与高于 720p 才出现的后处理强度衰减（`readme.md:415`），并把 Rockstar 编译好的 `.fxc` 反汇编成 DX9 字节码逐条重写（`shaders/…/*.asm` + `source/shaders.ixx:264-328` 列出闭源效果常量名）。
12. **两份生态的"可维护性"差距正在反转**：任天堂闭源产品经反编译后得到 62.5 万行（`oot`）到 96 万行（`TriAevum`）量级、可 CI、可匹配验证的工程资产；RAGE 侧的对应物只有 32.7 万行（CodeWalker）的**格式侧写器**，游戏逻辑一行都没有 —— 这是第 5 章对比矩阵的主轴。

13. **"可逐字节重建"这个指标本身在两个方向上被本报告证伪为不充分证据**（3.8.7 / 3.8.1）。方向一：3DS `oot3d` 的 `code.bin: OK` 靠的是**构建时用 `n3dsdisasm` 把用户自备的零售 `code.bin` 反汇编回来再汇编回去**（`disassembly/CMakeLists.txt:47,:59`），且 `include/non_matchings.hpp:3` 把 `GLOBAL_ASM(x)` **定义为空**、`UNK_RETURN/UNK_TYPE` 一律写作 `int`——12,254 个未匹配函数体在版本库里**一个 `.s` 文件都没有**，真正写成 C++ 的只有约 172 个（**1.4%**）。所以"字节匹配"必须先看它是不是恒真闭环。方向二：NSO 那个"唯一的黑箱"其实**已被观察到有逐游戏补丁层**（2022-01 修《时之笛》水面纹理而 ROM 未变；2022-02 v2.0.0 把"游戏补丁 Lua 脚本"预编译、并给 OoT 加渲染补丁，R 级），意味着**"NSO 版《时之笛》"是模拟器版本 × ROM × 补丁脚本三元组决定的移动靶**，任何只记 ROM CRC 的结论都可能过期。两条合起来给出本报告的取证纪律：**闭源项目的"完成度"数字必须同时给出分母、来源层与快照时间**（详见 3.8.5 的四分母对照表）。

---

## 1. 本地开源镜像清单（作为闭源测绘的仪器）

以下为本报告实际引用的仓库、其 HEAD 快照与规模（`git ls-files` 计数；LOC 为常见源码后缀合计，**含 vendored 第三方，仅用于量级比较**）。它们的存在意义在本报告里只有一个：**每一行都是对某个闭源构件的外部观测**。

### 1.1 GTA 侧（12 个）

| 仓库 | HEAD | 受控文件 | LOC | 它镜像的闭源对象 |
|---|---|---|---|---|
| `gta-projects/CodeWalker` | `485d56b` 2025-04-11 | 725 | 327,047 | RAGE V/RDR2 资源系统（RPF/RSC7/PSO/y*） |
| `gta-projects/fivem` | `8cbec36` 2026-11-17 | 13,590 | 3,880,129 | RAGE native 表、脚本引擎注册表、VFS-RPF |
| `gta-projects/scripthookvdotnet` | `28b4dfd` 2026-11-15 | 605 | 200,663 | 闭源 ScriptHookV 的托管契约 + Euphoria 中间件 |
| `gta-projects/mtasa-blue` | `70b0b40` 2026-11-17 | 9,180 | 2,645,172 | RenderWare 世代 SA 的运行时接管 |
| `gta-projects/GTAIV.EFLC.FusionFix` | `07c1e8d` 2026-11-17 | 3,368 | 24,460 | RAGE 一代 PC 移植的内部缺陷与着色器 |
| `gta-projects/gta-reversed` | `dd36de2` 2026-11-17 | 2,382 | 427,399 | GTA:SA PC 二进制的逐函数还原 |
| `gta-projects/openrw` | `5c5f266` 2025-06-18 | 353 | — | RenderWare 资产格式（DFF/TXD/COL） |
| `gta-projects/Sollumz` | `88dfa45` 2026-11-10 | 462 | — | RAGE Drawable/Fragment/Cloth 资产语义 |
| `gta-projects/gtamaplib` | `73b8473` 2026-08-17 | 14 | — | GTA VI 预告片几何（**非游戏数据**） |
| `gta-projects/gta6-leonida-atlas` | `0c7434f` 2026-11-17 | 6,575 | — | GTA VI 社区地图（**非游戏数据**） |
| `extra-repos/gta/gta-toolkit` | — | 455 | — | RAGE 资源块/PSO/RBF 的另一份独立侧写 |
| `extra-repos/gta/WidescreenFixesPack` | `be2a42b` | 4,731 | — | RW/RAGE PC 版的硬编码常量位置 |
| `extra-repos/gta/re3` / `open.mp` / `YimMenuV2` / `SampSharp` | — | 677/266/744/1,496 | — | 逆向源码、SA-MP 替代、Enhanced 版运行时接管 |

### 1.2 OoT 侧（22 个，去重后）

| 仓库 | HEAD | 受控文件 | LOC | 它镜像的闭源对象 |
|---|---|---|---|---|
| `oot-decomp-projects/oot` | `14759ab` 2026-10-18 | 3,319 | 625,253 | **EAD 原版 N64 引擎**（16 版本，含 iQue） |
| `oot-decomp-projects/oot-gc` | `6e449e1` 2025-05-24 | 496 | 133,992 | **GC 版内置 N64 模拟器**（任天堂闭源） |
| `oot-decomp-projects/oot-vc` | `6fee076` 2026-09-06 | 893 | 166,943 | **Wii VC N64 模拟器** + VCMV 覆盖层 |
| `oot-decomp-projects/oot3d` | `a87ddae` 2026-09-04 | 987 | 50,081 | **Grezzo OoT3D 原生 ARM 引擎** |
| `oot-decomp-projects/ProjectReLink` | `6570d6e` 2026-11-17 | 1,135 | — | **2026 Switch 2 官方重制版（EPD）** |
| `oot-decomp-projects/Hookshot` | `1c7e70b` 2026-11-15 | 2,188 | — | 同上（独立交叉印证） |
| `oot-decomp-projects/oot-pc-recomp` | `92529cb` 2026-11-05 | 831 | — | 同上（实验分支 `oot111`） |
| `oot-port-projects/TriAevum` | `672d13f` 2026-11-12 | 4,598 | 961,281 | OoT3D 的 A32 AOT 重编译 + PICA→SPIR-V |
| `oot-port-projects/Ocarina-ReTuned` | `fecb9f9` 2026-11-13 | 635 | — | TriAevum 的补丁流分叉 |
| `oot-port-projects/libultraship` | `759b134` 2026-09-02 | 490 | 70,885 | **闭源 `libultra` SDK 的开源替身** |
| `oot-port-projects/Shipwright` | `de1ab1b6` 2026-11-17 | 24,672 | 723,699 | OoT PC 移植（SoH），vendored decomp |
| `oot-port-projects/2ship2harkinian` | `e5e9f33` 2026-11-16 | 5,868 | — | MM 侧的 ZAPDTR 路线对照 |
| `oot-port-projects/Torch` | `22d3218` 2026-11-04 | 776 | — | ROM→`.o2r` 资产抽取（多游戏） |
| `oot-port-projects/ZAPDTR` / `oot-ports-repos/ZAPD` | `be1c68a` / `35ea376` | 230/221 | — | 闭源资源类型的反序列化器族 |
| `oot-ports-repos/N64Recomp` | `598ce20` 2026-11-03 | 36 | 14,352 | MIPS 机器码 → C（不需 decomp 源码） |
| `oot-ports-repos/N64ModernRuntime` | `e4342ae` 2026-11-03 | 85 | — | libultra 运行时替身（ultramodern） |
| `oot-ports-repos/Zelda64Recomp` | `b65c482` 2026-09-25 | 303 | 45,803 | 产品级重编译集成（MM 已发布，OoT 计划中） |
| `oot-port-projects/rt64` | `d9817c6` | 319 | — | **闭源 RSP/RDP 语义的 Vulkan 重建** |
| `extra-repos/n64-oot/GLideN64` | — | 539 | 219,781 | RSP/RDP HLE（含 F3DZEX2 OoT/MM 专条） |
| `extra-repos/n64-oot/mupen64plus-core` | — | 298 | 110,220 | RCP 总线/插件契约（LLE 挂载点） |
| `extra-repos/n64-oot/OoT3D_Randomizer` | — | 562 | — | 3DS 重制版的加载基址/`.zar`/BCSAR 侧写 |
| `extra-repos/n64-oot/{splat,m2c,decomp-permuter,decomp-toolkit}` | — | 166/1,519/255/244 | — | 反编译方法链本身 |

（另有 `oot-decomp-projects/oot3d`、`oot-dc`、`HackerOoT`、`fast64`、`OoT-Randomizer`、`SharpOcarina`、`z64rom`、`gz`、`retro`、`Ocarina-Atlas`、`mm`、`wwrando`、`n64-fast3d-engine` 等，报告仅在需要对照时引用；重复克隆的目录树以首次出现的路径为准。）

---

## 2. GTA 侧闭源项目

### 2.1 RAGE 引擎演进（闭源本体）

#### 2.1.1 谱系：两条并行线，不是一条

| 世代 | 引擎 | 载体作品 | 证据 |
|---|---|---|---|
| 1997–2001 | Criterion **RenderWare** | GTA III / VC / SA | F（大量公开文档 + 本地 `openrw`、`mtasa-blue`、`gta-reversed` 一手代码） |
| 1998–2000 | Angel Studios **ARTS → AGE** | Ken Griffey Jr. MLB、Midtown Madness 1/2 | F（Wikipedia "Rockstar Advanced Game Engine" 正文 History） |
| 2002 | Take-Two 收购 Angel Studios → Rockstar San Diego，AGE 随交易转移 | — | F（收购事实）/ **? 具体金额与"2002-11-20"仅见中文自媒体，未取一手** |
| 2004-07 | EA 收购 Criterion，RenderWare 停止外售 → Take-Two 自建 **RAGE Technology Group** | — | F（同上） |
| 2006-05-23 | 首个 RAGE 作品：*Rockstar Games Presents Table Tennis*（Xbox 360） | — | F |
| 2008-04-29 | 首个 RAGE GTA：*GTA IV*（Rockstar San Diego） | — | F |

时间互证：Take-Two 名下第一批 RAGE 期专利的**优先权日为 2006-11-22**（US10395410B2、US10685152B2），与"工具链 2006 年前后成型"相互印证（F，Google Patents 公开号页）。

#### 2.1.2 逐作增量（只写可确认部分）

| 作品 | 可证实增量 | 级 | 来源 |
|---|---|---|---|
| GTA IV (2008) | NaturalMotion **Euphoria** 动态运动合成、**Bullet** 刚体物理、360 720p / PS3 640p、PC 版仅 **DirectX 9.0c**（无 DX10 路径） | F | bit-tech 2011-02-17（AMD Manju Hegde 原话点名 "Rockstar's RAGE engine puts Bullet Physics to good use in GTA IV"）；PC 规格页 |
| Midnight Club: LA (2008) | 1280×720 / 960×720 双档；Bullet 官方点名 | F | 同上 |
| TLaD / BOGT (2009) | RAGE 首次由 Rockstar North 使用 | F | Wikipedia 作品表 |
| RDR1 (2010) | Euphoria 扩展至马匹与野生动物 | F | NaturalMotion 资料 |
| Max Payne 3 (2012) | **首个 PC DX11 的 RAGE 游戏** | F | DF/KitGuru |
| GTA V (2013；PC 2015-04) | DX11、主机 720p→1080p、PC 4K/60 + tessellation | F | 同上 |
| GTA V Enhanced | 2022 主机版起 RT 反射 + RT 阴影；PC 2025-03-04 追加 **RTGI + RTAO**、DLSS 3 帧生成、FSR 1/3、Reflex、DirectStorage | R | NVIDIA/Rockstar 公告的二手汇总 |
| RDR2 (2018) | PBR、体积云/雾、预烘焙 GI；PC **DX12 + Vulkan**；HDR 2019-05、DLSS 2021-07、FSR 2022-09 | F/R | Wikipedia 表及引注 |
| GTA VI (2026-11-19) | 全主机（含 PS5 Pro）**目标 30 FPS**；PS5 Pro 专属 **PSSR + 光追阴影**；预载最高 212 GB | F | Rockstar Newswire 2026-11-10 |

**Euphoria 所有权链是本报告认为最被低估的一条演化线索**：NaturalMotion → Zynga（2014，$5.27 亿）→ **Take-Two 收购 Zynga（2022-01-10 宣布，$127 亿）**，即 GTA IV/V/RDR2/VI 的角色动画中间件**今天已是发行商自有资产**。一手证据是 Steam《GTA V》商店页致谢原文："euphoria motion 合成技术由 NaturalMotion 提供。euphoria 代码为 (c) NaturalMotion 2008"（F）。这直接解释了为什么后续作品可以在动画层做深度改造而不受第三方授权约束 —— 也与第 2.3 节的 mocap 重定向专利族（US11951384B2）时间线吻合。

#### 2.1.3 被 modding 工具实证过的 RAGE 内部结构（本报告的核心可引用表）

这是"闭源引擎唯一可靠的知识来源"。左列是闭源构件，右列是本地仓库里的观测证据（全部实读核对）：

| 闭源构件 | 公开确认的实现 | 本地一手证据（`路径:行号`） |
|---|---|---|
| RPF 归档容器 | magic `RPF7` = `0x52504637`；TOC 条目定长 16 字节；资源签名 `RSC7` = `0x37435352`；**本快照内不存在 RSC8** | `CodeWalker/CodeWalker.Core/GameFiles/RpfFile.cs:140-150`、`:716-718`、`:1673`；`Resources/ResourceBuilder.cs:13` |
| 归档加密 | 四态枚举 `NONE=0`、`OPEN=0x4E45504F`（"OPEN" 即明文 TOC）、`AES=0x0FFFFFF9`、`NG=0x0FEFFFFF` | `RpfFile.cs:2147-2153`，分派在 `:153-172` |
| **AES 密钥来源** | 不是硬编码常量，而是**在闭源 `GTA5.exe` 里按 SHA1 摘要反查 32 字节键** | `CodeWalker.Core/GameFiles/Utils/GTAKeys.cs:47`（`PC_AES_KEY`）、`:76`（`HashSearch.SearchHash(..., GTA5KeyHashes.PC_AES_KEY_HASH, 0x20)`）、`:325`（存的是 20 字节摘要）、`:745`（扫描器）；调用点 `Utils/GTACrypto.cs:38-44` |
| NG 加密 | 17×16×256 轮表 + `PC_NG_DECRYPT_TABLES`，密钥派生用文件名与文件大小 | `GTAKeys.cs:52-62`、`GTACrypto.cs:104-153` |
| 页式压缩 | raw deflate，页标志位段还原 | `RpfFile.cs:899-914`（解压）、`:933`（回写）、`:2318-2333`、`:2702-2734`（`TypeVal=(V>>28)&0xF`、`BaseSize=0x200<<shift`） |
| **PSO 元数据反射** | 标量类型表 `Bool=0x00 … Structure=0x0c / Array=0x0d / Enum=0x0e / Flags=0x0f / Map=0x10 / Float3a=0x14 / HFloat=0x1e / Long=0x20`；**一律大端** | `CodeWalker.Core/GameFiles/MetaTypes/Pso.cs:139-162`；`:225,:236,:299,:332,:526,:536,:882` |
| `.ymt` 类型键 | `MetaTypeName : uint`，`STRING=0x10`、`POINTER=0x7`、`FLOAT=0x21`、`HASH=0x4a`、`ARRAYINFO=0x100` | `MetaTypes/MetaTypes.cs:2065-2076` |
| 格式族谱 | 一个枚举把 XML/RSC/PSO/RBF/CacheFile/AudioRel/Ynd/Ynv/Ycd 并列 | `MetaTypes/MetaXml.cs:2280-2290` |
| 反射表规模 | `MetaNames.cs` **20,688 行**、`PsoTypes.cs` **16,524 行**、`MetaTypes.cs` 6,593 行（MetaTypes 目录 13 文件合计 **53,734 行**） | 行数为 `wc -l` 实测；目录规模见附录 A |
| 世代非单调版本 | drawable **165（legacy）↔ 159（gen9）**；fragment **162 ↔ 171**；纹理字典 **13 ↔ 5**；particle 68↔71；bounds 43；clipdict 固定 46；ymap/ytyp/ymt/ynv=2、ynd=1 | `FileTypes/YdrFile.cs:94-97`、`YftFile.cs:92-95`、`YtdFile.cs:89-92`、`YptFile.cs:118-121`、`YbnFile.cs:37`、`YcdFile.cs:115`、`YmapFile.cs:81`、`YtypFile.cs:177`、`YmtFile.cs:50`、`YndFile.cs:70` |
| 版本号在文件头的位置 | `sv=(v>>4)&0xF`、`gv=v&0xF` 写进 system/graphics flag 高 4 位；页基址 `0x50000000`/`0x60000000` | `Resources/ResourceBuilder.cs:607-610`、`:626-635`、`:544-545`；构建签名 `:532`（`Build(fileBase, version, compress, gen9)`） |
| **"gen9" 的判据是 exe 文件名** | 是否存在 `GTA5_Enhanced.exe`，而非任何内部版本字段 | `CodeWalker/Utils/GTAFolder.cs:20-23,:25-58`；静态开关 `GameFiles/RpfManager.cs:35`（注释自认"not ideal for this to be static"）、`GameFileCache.cs:117,:153,:190,:256` |
| RDR2 侧的差异 | RDR2 用 `SwapBytes`（`Meta.cs:619` 注释原文 "okay. so this is big endian"）；`Drawable.cs:5077` 注释 "HasSOA … false for GTAV gen9 (but true for RDR2)"；`MrfFile.cs:149` "GTA5 = 2, RDR3 = 11" | 同左 |
| 场景流送子系统 | `.ymap` → `CMapData`（实体流送 + MLO interior + `CCarGen` 车辆生成 + occluder） | `FileTypes/YmapFile.cs:23-25,:105,:262,:301-304,:446,:460-474` |
| 类型注册表 | `.ytyp` → `CMapTypes` + `archetypes` 指针数组；archetype 结构本体 1,054 行 | `FileTypes/YtypFile.cs:24-25,:216-221`；`MetaTypes/Archetype.cs` |
| 路径网络 | `.ynd`（节点/链路、`MigrateNode`）、`.ynv`（导航音量），并有**从几何自建路网**的生成器 | `FileTypes/YndFile.cs:12,:17,:259,:283,:319`；`FileTypes/Builders/YndBuilder.cs`、`YnvBuilder.cs` |
| 资源图 vs 扁平块 | RAGE 是"64 位资源指针图 + 块寻址带 -16 头偏移"，RW 世代是"可整块 memcpy 的定长结构" | RAGE：`Resources/Drawable.cs:14,:1353,:1385,:1433-1434`；RW 对照：`openrw/rwcore/loaders/LoaderDFF.cpp:176-210`（对 `BSGeometryColor/Triangle/Bounds` 直接 memcpy） |
| native 函数表 | FiveM 侧运行时**先问 RAGE 自己的注册表**：`rage::scrEngine::GetNativeHandler(hash)`，失败才回落 `fx::ScriptEngine::GetNativeHandlerPtr` | `fivem/code/components/citizen-scripting-core/src/ScriptInvoker.cpp:160`（先）/`:165`（后） |
| native 注册表布局 | 遍历 `registrationTable<TReg>[hash & 0xFF]` → `getNextRegistration()` → `getHash(i)` → `handlers[i]`，模板参数名为 **`NativeRegistration_obf`**（即社区已把混淆后的注册表结构镜像出来） | `fivem/code/components/rage-scripting-five/src/scrEngine.cpp:484`、`:506-508`、`:511` |
| 定位方式 | 用字节签名在闭源 exe 里找位置：`hook::pattern("48 8B C8 EB ? 33 C9 48 8B 05")` | `rage-scripting-five/src/scrEngine.cpp:583+`（`static HookFunction hookFunction`） |
| native 表规模 | **5,182 条 native 声明 / 5,184 条 `ns` 记录 / 76,015 行**；单条形态为 `native "GET_PLAYER_PED"` + `hash "0x43A66C31C68491C0"` + `jhash (0x6E31E993)` + `ns "PLAYER"` | `fivem/ext/natives/natives_stash/gta_universal.lua:40-47`（记录样例）、`:1-6`（该文件几乎没有表格注释，文档在 `doc [[ ]]` 块中，如 `:49-53`） |
| native 命名空间分布（前 20） | NETWORK 701、PED 570、VEHICLE 519、HUD 451、GRAPHICS 323、MISC 301、BRAIN 292、AUDIO 233、CAM 214、PLAYER 212、STATS 162、ENTITY 161、STREAMING 117、OBJECT 105、WEAPON 87、NETWORKCASH 82、SOCIALCLUB 60、PATHFIND 55、DATAFILE 54、CUTSCENE 51（另 PAD 46、PHYSICS 43） | 同文件统计（`ns "..."` 计数） |
| 闭源↔开源 ABI | 托管层通过 **C++ 修饰符号**跨模块绑定：`DllImport("rage-scripting-rdr3.dll", EntryPoint = "?GetNativeHandler@scrEngine@rage@@...")` | `fivem/code/client/clrcore/ScriptContext.cs:356,358` |
| 组件分层命名 | 目录名即分层：`rage-allocator-*`、`rage-device-*`、`rage-formats-x`、`rage-graphics-*`、`rage-input-*`、`rage-nutsnbolts-*`、`rage-scripting-*`、`vfs-impl-rage`、`gta-core-five/ny/rdr3`、`citizen-level-loader-*`（`code/client/` 下**没有** `rng/`、`gamelogic/`，早期报告的目录假设需更正） | `fivem/code/components/`、`fivem/code/client/` |
| 开源侧的 RPF 可见边界 | `VFSRagePackfile.cpp:88` 只认 `0x32465052`（"RPF2"）且 `:102/:106` 明说 "only non-encrypted RPF2 is supported"；`:536` 才识别 `0x37435352`（RSC7）；`:497` 定义 `VFS_GET_RAGE_PAGE_FLAGS 0x20001` | 即：**加密 RPF 与页解压的实现不在开源树内** |

**另一条独立的负面结论同样重要**：`CodeWalker` 本快照**没有 `.ysc` 脚本反编译器**（不存在 `YscFile.cs` 与 `GameScript/`；全库 `grep GameVersion` 零命中，也不存在多档 `GameVersion` 枚举）。它对 `.ysc` 只做到"判定 → 解密 → deflate 解压 → 落盘"，源码里甚至留着这句自白：`RpfFile.cs:231` `rfe.IsEncrypted = rfe.NameLower.EndsWith(".ysc"); //any other way to know..?`（`:419-460` 是完整路径）。**这是"RAGE 脚本 VM 至今未被公开完整测绘"的直接负证据**。仓库内唯一被完整反汇编的 RAGE 字节码 VM 是**音频脚本 `.rel`**：`FileTypes/RelFile.cs:22717` 定义 `enum Opcode : byte`，共 **137 个 opcode**，反汇编器在 `:21384`、`:21395-21419`、`:21460`。

#### 2.1.4 需要证伪的说法

| 流传说法 | 判定 | 依据 |
|---|---|---|
| "RAGE 有官方版本号（RAGE 7=V / RAGE 8=RDR2 / RAGE 9=VI）" | **无一手出处（U）** | 官方所有场合只写 "Rockstar Advanced Game Engine (RAGE)"；转述爆料的 ITHome 2022-04-24 自己也写"R 星官方从未公开承认过 RAGE 的版本号"（F）。唯一源头是泄密者 @Chris_Klippel 2022-04-24 推文；分期法出自 A9VG 2023-10-22 对一篇西语"泄露汇编报告"（关联 Lapsus$ 2022 素材）的翻译 |
| "eRAGE / EVolution / RAGE aR" | **中文 AI 农场自造词（判为伪）** | 在 Rockstar 语境下检索不到任何一手或二手用例；来源集中在 smzdm p/ad758q5n（2026-04-27）、17173 及部分 B 站稿件 |
| "GTA IV 用的是 RAGE 2.0，已迭代至第九版" | **伪** | 同上 |
| "GTA VI 使用新一代 RAGE 引擎" | **U**（Rockstar 从未表态） | 只有中文媒体报道口径 |
| "《黑色洛城》是 RAGE 游戏" | **应排除** | Wikipedia 的 RAGE 作品表未收录 LA Noire（Team Bondi 自研引擎 + MotionScan）；百度百科收录属错误归类 |
| "RAGE 有 Metal 后端 / RSC7 后继格式名 / CarGen 是引擎子系统名 / GTAO 出自 RAGE" | **? 未找到** | 仅 T2 的多分辨率动态 AO 专利 US8847958B2 与 RDR2 PC 的 AO 档位可作旁证；`CarGen` 只能由 `carvariations.ymt` 数据驱动变体间接推证 |
| rage.re 的 "RAGE Namespace Glossary" | **R，且本轮不可达** | alt:V 文档与 Wikipedia 注 8 都引用它，但本次 DNS 解析失败；报告中的格式后缀语义改以 alt:V 文档为据（`docs.altv.mp/gta/articles/references/game-files.html`：YBN=碰撞边界、YND=路径节点、YTD=贴图字典、YTYP=类型表、YMAP=实体集合、YMF=manifest、YFT=fragment、YDD/YDR=drawable、RPF=RAGE Packfile） |

---

### 2.2 RAGE 的"闭源度"梯度：从可直接改写到只能侧写

这是本报告认为最有价值的一条技术观察。把本地 GTA 仓库按"外部代码能触达引擎多深"排序，可以量出闭源控制的实际厚度：

| 世代 | 代表项目 | 外部可触达的深度 | 一手证据 |
|---|---|---|---|
| GTA 1/2、III/VC/SA（RenderWare） | `WidescreenFixesPack`、`openrw`、`mtasa-blue` | **可直接改写 exe 内硬编码常量**：分辨率、位深、HUD 全局地址、FOV 变量 | `source/GTA3.FusionFix/Main.ixx:282-285`（`WriteMemory<int32_t>(…,3,x)`、`(…,13,y)`、`(...,23,32)` 直接写宽高与位深比较常量）、`:289-290`（`MakeNOP` 掉 `81 F9 00 00 C0 00`）；`GTAVC.FusionFix/Main.ixx:272-275` 是**同一串字节**（证明 III/VC/SA 共用 RW 分辨率路径）；`Camera.ixx:58`（`DefaultFOV=70.0f`）、`:492-493`（pattern 取 `TheCamera` 实地址）；`Hud.ixx:16-19`（由 `80 3D ? ? ? ? ?` 反查 `m_Wants_To_Draw_Hud` 全局） |
| RW 内部结构本身 | 同上 | **社区已复刻 RenderWare 的头文件级结构** | `GTA3.FusionFix/rw.h:1035 RwRaster`、`:1053 RwTexture`、`:1226 RwDevice`、`:1294 RwGlobals`；`Main.ixx:274` 从 `RsGlobal->ps` 取 HWND |
| GTA III/VC/SA 游戏逻辑 | `gta-reversed`（`gtasa-reversed`） | **逐函数替换**：可编译的逆向源码注入原 exe，最终目标是独立 exe | `README.md:9`（"in-game functions will be replaced by the reversed ones"）、`:34-37`（必须 Compact exe、精确 `5'189'632 bytes`、必须拥有原版资产）；`docs/ReversedClasses.md:12`（**7674 / 8338 函数完成，722 类，自动统计 92%**；注意 `README.md:13` 自称 50-60%，两处口径冲突）；`source/InjectHooksMain.cpp:619-624`（1,538 行逐类 `InjectHooks()`，见 `docs/HookRegistration.md:4-6`） |
| RAGE 一代 PC（GTA IV/EFLC，2008/2009） | `GTAIV.EFLC.FusionFix` | **运行时 hook + 反汇编并重写官方编译好的着色器字节码** | `readme.md:108`（Graphics API：DX9 / Vulkan(DXVK) / DX12(自定义 D3D9On12)，失败回落 DX9）；`source/postfx.ixx:1954-1986`（`find_pattern` + `injector::MakeCALL` 劫持 `DrawPrimitivePostFX`/`DrawSky`/`DrawCallPostFX`/`Downsample`/`Fog`，`:1986` 用 `safetyhook::create_inline` 接管假阴影）；`source/shaders.ixx:95-98`（挂钩 `grmShaderFx::LoadShader`，偏移 `shader+0x18`）、`:264-328`（逐条注册**闭源 `.fxc` 的效果名与常量名**：`gta_atmoscatt_clouds.fxc` 的 `AzimuthColor`/`AzimuthStrength`/`SkyColor`/`HDRExposure`/`SunDirection`）、`:444-456`（用内存内嵌版本替换 `common/shaders/win32_30/rage_perlinnoise.fxc`）；`shaders/…/win32_30_nv8/gta_default/*.asm`（**从官方 `.fxc` 反汇编出的 DX9 字节码**）+ `buildshaders.bat:8-15`（`RageShaderEditor.exe *.xml → *.fxc`） |
| RAGE 一代 PC 的**闭源缺陷** | 同上 | 32 位地址空间枯竭、>720p 才出现的后处理强度衰减、显存误检、雾 pop-in | `source/memory.ixx:243-247` 原文："GTA IV is a 32-bit process, and what runs out first is its address space, not video memory"；`:328-356` 遍历 `MEMORY_BASIC_INFORMATION` 度量剩余地址空间；`:369-398` 按地址空间缩放流送预算；`:1336-1338` pattern 改写人群/车辆预算（对应 `readme.md:186-190` "多 500 MB 流送内存"）；`readme.md:415`（高于 720p 时景深与 bloom 强度衰减）、`:334`（雾 pop-in）、`:185`（显存检测）。规模：`source/` 共 **59 个 `.ixx` 模块**、`settings/GTAIV.EFLC.FusionFix.json` 共 **170 个 `"id"`**（9 个 page，161 项设置） |
| GTA V / RDR2（当代 RAGE） | `CodeWalker`、`gta-toolkit`、`Sollumz` | **只剩格式侧写**：指针图重建、PSO 反射表抄写、版本分支 | 见 2.1.3 全表；`gta-toolkit/README.txt:1` 自陈 "THIS PROJECT IS DISCONTINUED."，且只覆盖 GTA V PC（类名后缀 `_GTA5_pc`、归档包装器 `RageArchiveWrapper7`：`RageLib.GTA5/ArchiveWrappers/RageArchiveWrapper7.cs:556,:720`；资源块基类 `RageLib/Resources/ResourceBlock.cs:30,:88,:96`；类型表 `Resources/PC/ResourceFileTypes_GTA5_pc.cs:27,:39`；重建器 `Utilities/RebuildUtilities.cs:48`） |
| GTA VI | `gtamaplib`、`gta6-leonida-atlas` | **零**：只能对预告片逐帧做针孔投影 + 三角化 | `gtamaplib/readme.md:5`（"specifically for GTA VI"）、`:7`（数据来自"promotional screenshots and trailers"）、`gtamapdata.py:12-19`（每条相机 = 位置/朝向/滚转偏航/yaw/FOV/分辨率）；`gta6-leonida-atlas/README.md:128`（Leaflet + **fictional coordinate system**）、`docs/METHODOLOGY.md:13`（"not an official Rockstar map"）、`:26-28`（置信标签 `APPROXIMATE/UNKNOWN/SUPPORTED`） |

> **`gta6-leonida-atlas` 是"GTA VI 闭源数据可得性 = 0"的最强单点证据**：它的每个地图 JSON 头部都写明了来源是社区仓库（`public/assets/street-leonida/maps/gtadb-landmarks-5c1d69f.json`：`source.repository = github.com/rolux/gtadb.org`、`license = CC BY 4.0`、`sha256`、`presentation = "transformed … community placement APPROXIMATE"`），`metadata.json` 里 `official: false`。**没有任何一个文件来自游戏内解包**。

**梯度结论**：外部代码能触达的深度从"直接改 exe 常量"（RW 世代）→"逐函数替换 + 反汇编着色器"（RAGE 一代 PC）→"只能侧写资源格式"（RAGE V/RDR2）→"连几何都拿不到"（VI）。**这条梯度不是社区能力退步，而是加密、平台条款与主机交付方式三重门控的结果**，第 5 章将其形式化。

---

### 2.3 专利架构（法定公开通道）

**检索事实**：以 "Rockstar Games / Rockstar San Diego / Rockstar New York / Rockstar London" 为 assignee 检索 Google Patents 得 **0 条**；引擎相关专利的受让人**一律是 Take-Two Interactive Software, Inc.**。这一点决定了"GTA 引擎专利"在法律主体上属于发行商而非工作室。（工具限制：Google Patents XHR 约 4 次查询后限流、直连曾返回 503，Justia 被 Cloudflare 拦截；下表"要点"来自摘要或正文，逐条标注。）

| 公开号 | 技术要点 | 优先权 → 公布 | 可能对应的引擎特性 | 级 |
|---|---|---|---|---|
| **US10395410B2** | 把 Maya directed-rig 图编译成指令流，在游戏内做**实时姿态驱动形变**（肌肉鼓起、关节褶皱），而非物理弯曲肢体 | 2006-11-22 → 2019-08-27 | RDR2/GTA V 角色形变层（与 Euphoria 自有化互补） | F |
| **US10685152B2** | 稀疏流体快速模拟并投射到 3D 表面（湿痕、血迹、泥），不是"动态天气" | 2006-11-22 → 2020-06-16 | 雨水/雪融化附着 | F |
| **US8847958B2** | 多分辨率分阶段（全局+局部）**动态环境光遮蔽** | 2007-01-24 → 2014-09-30 | AO 管线（是否即"GTAO"仍为 ?） | F |
| US9928641B2 | 多投影灯分配到 shadow-mask 各通道 + DXT 编码的单 pass 多光源阴影 | 2008-08-20 → 2018-03-27 | 阴影管线 | F |
| US9684975B2 | 统一法线朝向 + 滤波的着色模型 | 2011-02-18 → 2017-06-20 | PBR 前奏 | F |
| US10987587B2 / US10427046B2 | 带资产 ID 与属性标签的元数据记录驱动**程序化室内布置**（发明人含 Aaron Garbut） | 2017-12-22 → 2021-04-27 | 室内内容自动生成（PDF 已下载未解析，全文＝未取到） | R |
| **US11684855B2**《System and method for virtual navigation in a gaming environment》 | 由底层道路 node/link 压缩出**粗粒度导航图**（节点打标 highway/no_wagons）、按路面与天气**最小二乘拟合车辆转弯曲线**、按 link 而非半径查车流；发明人 **Simon Parr / David Hynd**，同族 US11071916B2，分类 A63F13/56、13/35 | 2019-04-24 → 2023-06-27（Active） | NPC 驾驶/导航，**最贴近 GTA VI 的人群与车流密度叙事** | **F（已一手核验）** |
| US11620781B1 | 数据驱动的"动画积木"在运行时按状态组合 | 2019-10-25 → 2023-04-04 | 动作系统 | F |
| **US11978162B2** | **连续 LOD**：先生成离散 LOD → 计算浮点 fLOD → 插值 | 2020-12-18 → 2024-05-07 | 远景/植被 LOD（对开放世界密度直接相关） | F |
| **US11951384B2 / US20250095261A1** | ML 以手部姿态驱动角色动画；mocap 数据**运行时重定向**到不同体型 | 2020-11-02 / 2023-09-15 | **与 GTA VI 官方宣称的"跨体型实时动画重定向"高度对应**（见 2.5.2） | F |
| US10195520B1 等四件 / US11794114B2 | GTA Online 的 P2P 架构、脚本文件热更新、高层事件同步、不良行为者检测；多人会话无缝合并/拆分 | 2013-05-14 / 2017-11-08 | Online 后端 | F |
| US11273380B1 / US12050668B1 | 内存变量混淆、**编译期游戏逻辑混淆**（反作弊用途） | 2019-10-25 / 2021-01-08 | 闭源保护机制本身 | F |
| US9855499B2 / US10150039B2 | 运行时采集玩家几何+贴图生成角色模型；模仿特定玩家行为的 Bot | 2015-04-01 / 2005-04-13 | 捏人 / 回放 / 反作弊 Bot | F |
| Tetavi 家族 US11574443B2、US11893688B2、US12323620B2、US12493971B2、US12586290B2 | 体积视频、网格融合、材质分割（T2 旗下动捕子公司 Tetavi） | 2020–2023 | 面部捕捉管线 | F |
| 仅由 XHR 摘要定性、**未取到全文** | US11534688B2 / US12263404B2（"improved graphics rendering" 延续案族）、US9805067B1 | — | — | ? |

**两条否定性结论**（比正面清单更有信息量）：

1. **"世界流式加载（world streaming）"相关专利：未找到。** 结合 2.1.3（`RpfFile.cs` 的页标志与 `update.rpf` 加密）可以看出，RAGE 的流式与内存管理是靠**格式 + 二进制保密**保护，而不是专利。这也解释了为什么 20+ 年里没有任何一份 RAGE 内部内存分配器被公开测绘。
2. **中文圈流传的"GTA VI 三项新专利（远程 NPC 计算 / 按地形负重自适应程序化动画 / 会话服务器管理）"**：源头为博主 FRGTAIV，经游民星空 2026-09-17 转述，**未公布公开号，Google Patents 站内检索无对应命中 → 判 U/?**。同样，"US11684855B2 支持 1 万+ NPC"的中文表述**未见于说明书原文**，属夸大。
3. 2024–2026 年 Rockstar/Take-Two 与 GTA VI 相关的新**商标**一手登记：本轮**未找到**可核验来源（仅有巴西 INPI 等二手说法）。

---

### 2.4 闭源工具生态台账（运行时接管层）

#### 2.4.1 三处必须更正的常见错误

| 常见写法 | 正确事实 | 级 |
|---|---|---|
| "ScriptHookV 作者是 crosire" | **ScriptHookV 作者是 "Alexander Blade"**（dev-c.com，页脚 © 2006–2026）；`github.com/crosire/scripthookv` **不存在**（GitHub API 2026-11-18 → Not Found）。crosire 做的是 SHVDN，且已迁至 `github.com/scripthookvdotnet/scripthookvdotnet`（zlib，最后 push 2026-11-15） | F |
| "OpenIV 归 gurinda" | OpenIV 归属 **GooD-NTS / OpenIV-Team**（gtamods.com/wiki/OpenIV，最后编辑 2025-02-23）；与 gurinda 的关联**无任何一手证据** | F / U |
| "BattlEye 于 2024-08 上线 GTA Online" | 是 **2024-09-17 的 patch 1.69**（GTA V/Online PC 首个内核级反作弊） | R |

#### 2.4.2 台账

| 项目 | 归属 | 源码开放度 | 状态（2026-11-18） | 与本地仓库的关系 |
|---|---|---|---|---|
| **RAGE 引擎本体** | Rockstar / Take-Two | 完全闭源 | 在售 | 只能靠 2.1.3 的侧写测绘 |
| **ScriptHookV** | Alexander Blade（个人） | 闭源 `.asi` runtime；**ABI 门面**："compiled script plugins depend only on ScriptHookV.dll"，游戏更新只换 runtime | **v3889.0（2026-07-15）**，支持 patch 1.0.3889.0 / 1.0.1158.13，SDK 1.0.617.1a；官方定义原文："allows to use GTA V script native functions in custom \*.asi plugins. Note that it doesn't work in GTA Online, script hook closes GTA V when player goes in multiplayer"（dev-c.com/gtav/scripthookv/） | `scripthookvdotnet` 是其开源托管层：`source/core/Game/NativeFunc.cs:24/32/40` 用 `[DllImport("ScriptHookV.dll", EntryPoint="?nativeInit@@YAX_K@Z" / "?nativePush64@@YAX_K@Z" / "?nativeCall@@YAPEA_KXZ")]`；SDK 头声明 `source/loader/sdk/inc/main.h:84-86`。**这三行就是开源与闭源的物理边界** |
| **SHVDN（Script Hook V .NET）** | crosire → SHVDN org | zlib 开源（`COPYRIGHT.md:2`） | 活跃（`28b4dfd`，2026-11-15） | 自身也是 Euphoria 闭源中间件的外部镜像：`source/scripting_v3/GTA.NaturalMotion/` 仅 4 个文件 **31,905 行**（`EuphoriaHelpers.cs` **30,986 行**、`Euphoria.cs` 556、`CustomHelper.cs` 140、`Message.cs` 223）。闭源参数以**字符串属性名**暴露，如 `EuphoriaHelpers.cs:1061` `SetArgument("balanceAbortThreshold", value)`；注释甚至带出 MNM 内部默认值：`:628`（"hipwidth = 0.23f by default but is model dependent"）、`:798`（"General balancer likes 1.0. StaggerFall likes 0.1"）；`:24-58` 是 `FallType/Synchroisation/TurnType/TorqueMode` 等枚举镜像。**注意：不存在名为 `CustomAttribute` 的类，暴露面是"消息类 + SetArgument 字符串"** |
| **OpenIV** | GooD-NTS / OpenIV-Team | 闭源本体；`.oiv` 包格式与 RAGE 字符串库**开源**（`OpenIV-Team/OpenIV-PackageFormat`、`OpenIV-Team/RAGE-StringsDatabase`，2022-07-07 更新） | 首发 2008-12-28；**4.1 是最新正式版**（Build 1502，2022-01-21）；4.1 公告自述"development of OpenIV was halted in the summer of 2021 due to team creative burnout"；bugtracker 上 **4.5 至今标 "Not Yet Released"** → 2022 年起无正式版 | 与 CodeWalker 形成"同一格式的两家独立测绘"；`.oiv 2.2`（OpenIV 3.2 引入，用 XML 改 V 的 PSO）对应 CodeWalker 的 PSO 层 |
| **RAGE Plugin Hook** | 个人 | 闭源，**且禁止再分发**：条款原文 "Do not upload to anywhere, and do not include RAGE Plugin Hook in your plugin releases, without prior written permission."（NuGet `RagePluginHookSDK` 页） | LCPDFR.com 首页（2026-02-05 索引）："RAGE Plugin Hook has now been updated to work with the new game version… LSPDFR 0.4.9 (Build 9476) (New RPH bundled)"、"LSPDFR for GTA Enhanced now available" → **2026 年仍在跟版本** | 比 SHV 更封闭：**CLR(C++/CLI) 插件宿主，没有任何开源对应层** |
| **alt:V** | 团队（约 2017 起，"九年项目"） | 服务端/SDK/文档部分公开，**客户端闭源**；`github.com/orgs/altmp` 23 个仓库中**无客户端/服务端核心**，`altv-client-js`(2021-09-11)、`natives`(2020-01-05)、`docs`(2019-10-27) **均已归档**；语音栈只能从 `rnnoise-cmake`（2026-06-30）等依赖 fork 反推 | **已被关停**：2026-02 成员 Vadzz 在 Discord 公告，原文与 RAGE:MP 同构："Rockstar Games and Take-Two Interactive have made it clear that **FiveM is the only authorized platform for GTAV multiplayer modding, as defined in their Platform License Agreement (PLA)**… at Take-Two's request, alt:V will begin a structured shutdown process in 2026"；里程碑 **3-02 停新服/停发工具包 → 5-04 公共服务器列表下线 → 7-06 后端永久关停**（PC Gamer 2026-02-17） | docs.altv.mp 自述特性含 "Encrypted client-side code"、CEF、Mod/MLO 支持，且**刻意不做 ScriptHookV/.asi 注入**；`ragemp-altv-bridge` 归档于 2025-06-24，佐证两者人员/代码渊源 |
| **RAGE:MP** | 团队 | 核心闭源 | **2026-05-25 公告关停**："RAGE: MP will begin a structured shutdown process"，即日禁新服、**6-01 列表下线、8-31 停服**，劝迁 FiveM 并由 Cfx 提供迁移支持；结语 "RAGE: MP was always defined more by the community than by the codebase"（rage.mp 论坛帖 + Kotaku 2026-05-26） | 与 FiveM 的差异成为**唯一判例**：同为客户端 hook + 服务端脚本，FiveM 因被收购而存活，alt:V/RAGE:MP 因独立而死亡 |
| **FiveM / RedM** | **Take-Two（2023-08-11 收购 Cfx.re）** | **半开放**：`LICENSE:1` 即 `© 2023 Take-Two Interactive Software, Inc.`，`:3-8` 整体指向 Rockstar Games Creator Platform License（fivem.net/terms、redm.gg/terms）；**仅** `:13-21` 白名单目录（`code/client/{shared,citicore,launcher,common}/`、`code/tools/`、`code/components/citizen-*/`、`font-renderer/`、`vfs-core/`、`code/vendor/*.lua`）在 `:23` 获 LGPL-2.0，`:34` 声明第三方部分不受覆盖 | 活跃（`8cbec36`，2026-11-17） | 这是"发行商自有开源仓库"这一新形态的样本；仓库内自认闭源现状的表述见 `code/components/conhost-v2/src/ConsoleHostImpl.cpp:758`（"they're 'closed source' and 'undocumented'"）、`code/tools/build/versioning.lua:7`（"proprietary and confidential tooling"）。**全仓没有逐文件许可头**，`code/client/shared/Hooking.cpp:1-5` 只写 "See LICENSE in the root of the source tree"（指针式声明） |
| **MTA:SA** | 社区 | 完整开源 GPL-3.0 | 活跃（`70b0b40`） | RW 世代接管的对照组 |
| **CodeWalker** | dexyfex（个人） | 源码公开但**仓库内无 LICENSE** → 法律上"保留所有权利" | 本地快照停在 2025-04-11 | 见 2.1.3 |
| **gta-toolkit** | Rage (R*) 名义社区项目 | 开源但已停 | `README.txt:1`："THIS PROJECT IS DISCONTINUED." | RAGE 格式的第二家独立测绘 |
| **Sollumz** | 社区（GPL） | 开源 | 活跃（`88dfa45`，2026-11-10） | Blender 侧的 RAGE 资产语义：`README.md:15-22` 支持 **YDD / YDR / YFT / YBN / YTYP / YCD / YMAP(Partial)**；`ydr/` 目录拆分本身就是 Drawable 分层的证据（`ydrexport.py`、`vertex_buffer_builder.py`、`model_data.py`、`render_bucket.py`、`shader_materials.py`、`cloth.py`/`cloth_char.py`、`cable.py`、`lights.py`、`lods.py`）；`:35-37` 明确"推荐搭配最新 CodeWalker" —— 即**格式知识依附于单一逆向源**，这是生态脆弱点 |
| **YimMenu → YimMenuV2** | 社区（GPL-2.0） | **源已撤**：`github.com/YimMenu/YimMenu`（建于 2020-12-26，最后 push 2024-10-06）**2025-02-01 被 owner 归档**，README 自述因 Take-Two **DMCA**（点名 GTAV-Classes 两次 commit 与 YimMenu 的 `src/gta/`）**主动删库**，原文 "it's unlikely the source code of either YimMenu or GTAV-Classes will be put up again"，转 "binary only releases"（UnknownCheats 线程 476972） | `YimMenuV2`（"Experimental menu for GTA 5: Enhanced"，建于 2023-08-11）**nightly 至 2026-10-24 仍在发** → **Enhanced 时代作弊端已换目标二进制** | 仅作架构取证：`src/core/{hooking,memory,renderer,scripting,backend,commands,…}` 与游戏无关，`src/game/{hooks,gta,pointers,features,…}` 承载 GTA 适配；对闭源二进制的定位靠**模式匹配而非符号**（`src/core/memory/PatternScanner.hpp:13,:31,:34,:38`、`Pattern.hpp:78`，结果灌入 `src/game/pointers/Pointers.hpp/.cpp` 这张"签名→地址"表；native 侧表驱动 `src/game/gta/Natives.hpp` + `invoker/GenerateNatives.py` + `natives_gen9.json`）。**本报告不提供任何绕过反作弊的实现细节** |
| **NativeDB / alloc8or.re** | 社区 | 数据公开 | — | 与 `fivem/ext/natives`、SHVDN `NativeHashes.cs` 互为镜像 |
| **re3 / reVC** | 社区 | 开源但**两次 DMCA + 被诉后撤诉** | 现存多为镜像/分支 | `extra-repos/gta/re3/README.md:10`（"fully reversed source code for GTA III / GTA VC"）、`:13-14`（渲染走"original RenderWare (D3D8)"或 librw 重实现）、`:25`（"requires PC game assets…you **must** own a copy"）、`:1`（`# THIS IS NOT MINE!` 标注 fork 归属） |

#### 2.4.3 一张图看懂接管层次（本报告归纳）

```
离线格式层   OpenIV / CodeWalker / gta-toolkit / Sollumz     ← 只碰文件，不碰进程
   ↓
脚本运行时   ScriptHookV(闭源) → SHVDN/NativeUI/Menyoo(开源)  ← 碰进程，走 native 哈希
   ↓
引擎对象层   RAGE Plugin Hook(闭源+禁止再分发)                ← 碰引擎对象模型
   ↓
网络/服务端  FiveM(Take-Two 自有) ⊃ alt:V / RAGE:MP(已关停)   ← 碰 RAGE 网络层
   ↓
在线对抗     YimMenu(已删源) / Standalone / Eulen             ← 撞 BattlEye，法律+技术双高危
```

**闭源边界恰好画在"是否接触引擎内部实现"那一层**：越往下越闭源。这与 OoT 侧（第 3 章）形成鲜明对照 —— OoT 的对应分层里，**每一层都有开源替身**。

---

### 2.5 GTA VI：闭源技术管线的可确认与不可确认

#### 2.5.1 官方事实（全部 F，逐条带日期）

| 项 | 事实 | 来源/日期 |
|---|---|---|
| 发售日 | **2026-11-19**（本报告完成日次日） | Take-Two 财报 + Rockstar Newswire（2025-11-06 二次延期） |
| 平台 | PS5 / PS5 Pro / Xbox Series X\|S；**无 PC 首发** | 官方 |
| 云 | 首日进 Xbox Cloud Gaming（XSX-S + Xbox One，**采用 Series S 画质档**），**PC 端不给串流**；PS 侧仅 Remote Play | Xbox 商店页 2026-11-10 |
| 帧率 | **全平台（含 PS5 Pro）目标 30 FPS，无性能模式** | Rockstar Newswire 2026-11-10 |
| PS5 Pro | **PSSR 提升清晰度 + Pro 独占光线追踪阴影**；Tempest 3D 音频、DualSense 触觉、SSD 缩短加载 | 同上（Digital Foundry 2026-11-10 同口径确认） |
| 价格/载体 | US$79.99；实体版**无光盘（code-in-box）**；预载 2026-11-10 开启，**需 212 GB** | PlayStation 支持页 2026-11-03、Newswire 2026-11-03/11-10 |
| 首日补丁 | 玩家截图称 **v1.002.000 ≈ 1.073 GB**；完整安装 223.088 GB | **U**（截图，非官方） |
| 微交易 / 生成式 AI | Rockstar 向 Kinda Funny 确认**均无** | F（2026） |
| 官方物料 | 仅两段：Trailer + **"An Extended Look"**（Netflix 2026-08-28 首播，官方站 8-27/10-15）。**未找到**名为"The World and Beyond"的官方拆解系列 | F / ? |
| 招聘技术关键词 | **未找到**：`rockstargames.com/careers/open-positions` 返回 404、`jobs.rockstargames.com` DNS 失败；仅有二手转述（2024-04 招"DXR/NVX 光追程序员"） | **R，且不可核验** |

#### 2.5.2 可确认的技术面（Digital Foundry，逐条分级）

* **渲染基线**（Extended Look 深度分析，2026-08-28，R）：PS5 原生约 **1440p、无 DRS、普通空间升频至 4K** → 画面偏软、几何有锯齿与闪烁；**RT 镜面反射（含蒙皮网格）**、大量光追漫反射 GI、带可变软边的**精修 shadow map（并非全面 RT 阴影）**、镜面仍用平面反射；头发/皮肤着色；人群与车辆模拟密度；**Zen 2 CPU 被点名为瓶颈**；33.3 ms 节拍与"RAGE 固有"的动画/运镜抖动；结论是 60fps 需要 X3D 级 CPU。（直站 403，取 Wayback 2026-10-01 快照）
* **动画规模**（R，3DM 2026-11-02 转述 Garbut）：约 **60 万条独特 NPC 动画** + 专人 NPC 团队，支持跨体型重定向 —— **与专利 US11951384B2（mocap 运行时重定向）、US11620781B1（数据驱动动画组合）形成官方陈述 + 法定公开的双重对应**，这是本报告能给出的最接近"闭源管线内部证据"的一条。
* **天气/大气**：美术照明渲染副总裁 Owen Shepherd 称系统在 RDR2 基础上**重做**（R）。
* **不确定性**：技术总监 Phil Hooker 对 Bloomberg："从未有精确蓝图，近数月仍在改"（R，2026-10-17）—— 这句话同时也是对"任何 GTA VI 引擎细节断言"的有效性警告。
* **未确认**：Mackenzie 认为 10 月官方电台截图的光追质量超出普通 PS5，疑为 Pro/PC 渲染（U）；2026-10 流出的裸露/调试画面 Rockstar **未确认真实性**，"来自 debug 模式"只是分析师推断（R）。

#### 2.5.3 发行与保护管线（闭源侧的"最后一公里"）

| 环节 | 事实 | 级 |
|---|---|---|
| PC 反作弊史 | **2024-09-17 patch 1.69** 在 GTA V/Online 引入 **BattlEye 内核级**反作弊（系列首次），副作用是 **Linux/Steam Deck 不可玩**、有误封与公开申诉通道；R\* 有专文《BattlEye Troubleshooting》，支持 `-nobattleye` | R |
| PC 其余保护 | Rockstar Games Launcher + Social Club + **Arxan 防篡改**（PCGamingWiki 2026-01-01） | R |
| **GTA VI 使用 Denuvo 的证据** | **未找到**（F/R 均无一手） | ? |
| 主机侧 | code-in-box + 强制预载 + **需强制更新 PS5 系统软件**才能运行；破解圈称最低固件 14.22，索尼随即发 26.07-14.23.00 "安全修复" | R / **U** |
| 后台元数据 | PS 预载元数据出现 `joinSession` / `launchActivity` / `launchMultiplayerActivity` 字符串（IT之家 2026-11-09） | R —— **这是目前唯一"从发行管线边角料"窥见在线架构的公开线索** |
| 账户/订阅 | 在线需 PSN + R\* 账户；**GTA+ 仍只服务 GTA Online**，本作仅以赠月（180 天内兑换）绑定，**未找到** GTA+ 覆盖 GTA VI 的官方条款 | F / ? |

#### 2.5.4 三个硬性阻断（为什么今天不存在合法的 GTA VI 逆向）

1. **没有可分析的用户侧明文二进制**：主机可执行文件以平台密钥加密交付，code-in-box 连"自有实体介质含可读镜像"都没有；PC 版尚不存在。
2. **条款直接禁止**：Rockstar Mod Guidelines（页面标注 **2026-09-10**）明文禁止 —— 改主线剧情/连续性、跨作搬运地图与资产、作弊菜单、**"修改我们的游戏使其在未经 Rockstar 官方发布的主机、硬件或其他平台上运行"**、多人 mod 仅限官方授权创作者平台、禁止绕过反作弊、禁止赌博/开箱/加密资产。（页面为 SPA，直抓仅得脚本壳，条文经 Eurogamer 中文转述 / 17173 2026-09-22 / 3DM 交叉印证 → **存在与日期 F、逐条文本 R**）
3. **反作弊 + 运行时接管路线失效**：主机 + 在线环境下，第 2.4 节那套 SHV/FiveM 式接管从一开始就不成立。

**唯一诚实的路线图**（参照系：GTA V PC 2015-04 → CodeWalker 2017 → native 表至今仍在更新；GTA IV 2008 → OpenIV 起步约 5 年；re3 从研究 2016 到发布 2021 并被立即起诉）：

```
PC 版发售（媒体预期 2027，官方无日期 = U）
  → 容器识别（假定 RPF 后继；新键派生/新布局；参考 RpfFile.cs 的 RPF7 假设被打破处）
  → PSO/Meta 反射表重建（当前 20,688 行 MetaNames + 16,524 行 PsoTypes 的等价物）
  → 资源版本矩阵（drawable/fragment/texture 三套版本号的世代对齐，见 2.1.3）
  → native 表（6,400+ 条哈希，从 FiveM lua / SHVDN 枚举重推）
  → 才谈得上行为级重写（按年计）
每一步都在 2.6 的执法阴影下；主机窗口期不应承诺任何里程碑。
```

**关于"AI 加速移植"的最新变量**：Take-Two CEO Strauss Zelnick 在 2026-11-09/10 的 Q2 财报会上称 **AI 辅助重编译已抬高门槛**，公司"全力加强技术防御并适当加强法律行动"；11-17 又批评 Anthropic/OpenAI 放任"AI 克隆游戏"（F 原话 / R）。同期破解圈的公开自述也印证了难度：GeZine 自称**首个在 14.22 固件提取 GTA VI 二进制**并宣布**不公开**（U，网易 2026-11-12）；Echo Stretch 称"完整导出可运行镜像短期几乎不可实现"（U）。**结论：GTA VI 的逆向窗口比 GTA V 更窄，而不是更宽。**

---

## 3. 《塞尔达传说：时之笛》侧闭源项目

OoT 的"闭源项目清单"实际上是一条**官方产品链**加**一条民间闭源重制链**：

```
1998 EAD N64 原版（libultra/IDO/F3DZEX）           ← 闭源，已完整反编译（oot，16 版本）
2002-2003 GameCube OoT + Master Quest               ← 闭源【内置 N64 模拟器】，已反编译（oot-gc）
2003 iQue Player 神游版                             ← 闭源【原生重编译】，已进入 oot 的 ique-cn 版本
2007-2011 Wii Virtual Console                       ← 闭源【内置 N64 模拟器 RVL 分支】，已反编译（oot-vc）
2011 OoT3D（Grezzo 重制）                           ← 闭源【全新 ARM 原生引擎】，反编译进行中（oot3d）
2020- Nintendo 64 – Nintendo Classics（NSO 模拟器） ← 闭源【模拟器】，未反编译
2026-11-05 Switch 2 重制版（Nintendo EPD）          ← 闭源【原生重制】，13 天内被静态重编译
2015-2026 CryZENx UE4→UE5 民间重制                  ← 闭源【现代引擎重建】，2026-06-13 终止
```

### 3.1 任天堂 EAD 原版 N64 引擎（1998）

#### 3.1.1 工具链与运行时约束（一手：`oot` 反编译 + `docs/`）

| 项 | 可确认事实 | 级 + 证据 |
|---|---|---|
| 编译器 | **N64 与 GameCube 版全部代码用 IDO 编译器编译**；`libultra`/`libleo`/JPEG 用 **IDO 5.3**，主程序用 **IDO 7.1** | F：`oot/docs/compilers.md:3-4` 原文 "For the N64 and GameCube versions, all code was compiled with the IDO compiler" |
| 工作机 | SGI 工作站写作 **"Indy"**（RealityStation / Indigo2 **无据**） | F（decomp 文档）/ ? |
| SDK | `libultra`，版本字母 D–L 可配（`include/ultra64/version.h` 的 `LIBULTRA_VERSION`），ROM 头写入 `OS_VERSION(2,0,LIBULTRA_VERSION)`（`src/makerom/rom_header.s`）。**零售版实际字母未取证** | F / ? |
| 内存 | RDRAM 4 MB + Expansion Pak 4 MB；`OS_K1_TO_PHYSICAL(0xA8000000)` 直读 SRAM 域（`oot/src/overlays/gamestates/ovl_file_choose/z_file_choose.c:745`）；`SP_STATUS_REG` 读 Signal 7 | F |
| 64DD 残留 | `include/n64dd.h`（内含 `ultra64/leo.h` 磁光盘 API）+ `src/overlays/n64dd/` 覆盖层 | F：**"OoT 最初为 64DD 开发"的代码级证据** |
| CIC | N64/GC 走 `--cic 6105`，iQue 走 `6102`（`oot/Makefile:899-905`，配合 `src/boot/cic6105.c`） | F |
| 开发周期 | 1995 展示 → 1997-11-21 日本发售，约 3 年；**团队人数无一手数字** | R / ? |

#### 3.1.2 RSP 微码与显示列表：闭源图形管线的机器可读清单

这是本报告认为 OoT 侧**最有价值的闭源发现**：原版游戏内部保留了一份**运行时显示列表反汇编器**，等价于任天堂未公开的 GBI 使用清单。

* **微码集合**（F，`oot/baseroms/gc-eu-mq-dbg/config.yml:3-23` 的 `incbins` 顺序）：`ipl3`(0x80000040, 0xFC0)、`rspbootText`(0x80009320, 0xD0)、`aspMainText`(0x801120C0, 0xFB0)、`gspS2DEX2d_fifoText`(0x80113070, 0x18C0)、`njpgdspMainText`(0x80114930, 0xAF0)。**即：自研 bootcode + CPU 侧音频微码 + S2DEX2（2D 纹理）+ JPEG 解码也跑在 RSP 上**。
* **主光栅微码是 F3DZEX，不是 F3DEX**（F）：`UCodeInfo` 表实证 —— `oot/src/code/graph.c:54-62` 的 `D_8012D230[3]` = `{UCODE_TYPE_F3DZEX, gspF3DZEX2_NoN_fifoTextStart}`、`{UCODE_TYPE_F3DZEX, gspF3DZEX2_NoN_PosLight_fifoTextStart}`、`{UCODE_TYPE_S2DEX, gspS2DEX2d_fifoTextStart}`。交叉印证有三条独立来源：SM64 反编译 Makefile 注释 "f3dzex # Fast3DZEX (2.0J / Animal Forest)"、`fast64/fast64_internal/f3d/f3d_enums.py:386`（"F3DEX2/LX2/**ZEX** … used in later N64 games including OoT and MM"）、GLideN64 的 `src/GBI.h:9-40` 里**专为 OoT/MM 设立的枚举 `F3DZEX2OOT = 17` / `F3DZEX2MM = 18`**。
* **GC 与 iQue 用的是 PosLight 变体**（F）：`baseroms/ique-cn/config.yml:3-28` 额外含 `gspF3DZEX2_NoN_PosLight_fifoText`，且全部 VRAM 重排（`rspbootText 0x80009C30`、`aspMainText 0x800EBF70`、`aspMainData 0x8011FE20`）—— 这是"同一引擎的第二次编译目标"最硬的证据（详见 3.3）。
* **运行时反汇编器**（F）：`oot/src/code/ucode_disas.c`（1,258 行）主循环 `:332 UCodeDisas_Disassemble`，`case G_` **全库 92 处、主 switch 内 72 处**，覆盖 `G_SPNOOP:362`、`G_DL:366`、`G_LOAD_UCODE:399`、`G_SETTILE:419`、`G_LOADBLOCK:432`、`G_SETCOMBINE:465`、`G_SETOTHERMODE_H/L:495/:548`、`G_SETCIMG:657`、`G_MTX:817`、`G_VTX:869`、`G_TRI1:899`、`G_LINE3D:909`、`G_TRI2:921`、`G_QUAD:940`，另有 CCMUX/ACCMUX 子解码 `:94-207`。它自带段表做地址翻译（`include/ucode_disas.h:17` 的 `uintptr_t segments[NUM_SEGMENTS]`，`src/code/ucode_disas.c:62`）。
* **为什么这个反汇编器重要**：`src/code/graph.c:86 Graph_DisassembleUCode` 与 `:131-143 Graph_UCodeFaultClient` 说明它被挂在 **RSP 崩溃路径**上 —— 出故障时自动反汇编最后一帧显示列表。**这是 EAD 内部调试哲学的化石**，也等于一份任天堂自己核对过的 GBI 使用子集。RT64 至今仍需对每条微码打 `// Needs confirmation.`（`rt64/src/gbi/rt64_gbi.cpp:64-94` 的 113 条 `GBIInstance` 表），对比之下更能看出这份化石的价值。
* GBI 定义仍保留任天堂原貌：`oot/include/ultra64/gbi.h`（5,185 行），`:28 G_SPNOOP 0xE0`、`:31 G_LOAD_UCODE 0xDD`、`:43 G_VTX 0x01`、`:101 G_SETCIMG 0xFF`、`:1677-1863` 各 union 载荷结构；`Makefile` 注入 `-DF3DEX_GBI_2`（非 N64 平台再加 `-DF3DEX_GBI_PL`）。
* **需要证伪**：`G_MDSPEC` / `G_ALLMODE` / `G_SUBTMODE` / `AA_MODE 0-3` 这些"F3D 时代字段"在 `oot/include/` 与 `ucode_disas.c` **0 命中** —— OoT 用的是 F3DEX2 风格的 OTHERMODE 位域（`F3DZEX` 即 F3DEX2 系），实证位包括 `AA_EN 0x8`、`ZMODE_OPA/INTER/XLU/DEC`（掩码 `0xC00`）、`FORCE_BL 0x4000`（`src/code/ucode_disas.c:262-274`）；Dither 走 `G_MDSFT_ALPHADITHER/RGBDITHER/COLORDITHER` + `G_AD_PATTERN/G_AD_NOTPATTERN` + `G_CD_MAGICSQ/G_CD_BAYER/G_CD_NOISE`（`:497-513`）、`G_AC_DITHER`（`:551`）；雾为 `G_SETFOGCOLOR:697` + `G_MW_FOG:1050`；剔除为 `G_CULL_FRONT/BACK:71-72`。`_fifo` 后缀的含义是 CPU 直接推 FIFO、免每帧重建 `__osSpPutMessage` 队列（F，微码命名本身）。

#### 3.1.3 文件系统与压缩：否定性结论 + 唯一分岔

* **两条压缩路径**（F，这是 iQue 版本差异的根）：`oot/src/boot/z_std_dma.c:461-465` —— `#if !PLATFORM_IQUE` → `Yaz0_Decompress(romStart, ram, romSize)`；`#else` → `gzip_decompress(...)`。工具侧同构：`tools/compress.py:21-22` `COMPRESSION_METHODS = {"yaz0": crunch64.yaz0.compress, "gzip": crunch64.gzip.compress}`，`:254-256` 默认 `yaz0`。**没有 RefPack、没有 Yay0**（全树 0 命中）。
* Yaz0 是**流式**的：`src/boot/yaz0.c:11` `sYaz0DataBuffer[0x400]`、`:17 Yaz0_FirstDMA`、`:33 Yaz0_NextDMA`（边 DMA 边解压）；inflate 自带静态 arena（`src/boot/inflate.c:285-304 gzip_malloc/gzip_free`，`:997 gzip_decompress`）——**整个游戏没有 malloc**。
* **否定性结论（重要）**：OoT **不存在** MM 式的 `ZARCLIB` 归档容器 —— 全库无 `File_ARC*`、`File_SelectLoad*`、`ACMB`、`ZSD` 等 magic，也没有 `src/code/z_file.c`。资源定位**纯靠 VROM 区间表**：`include/dma.h:19-23` `DmaEntry{RomFile file; romStart; romEnd;}`、`:25 gDmaDataTable[]`；`include/romfile.h:7-13` `RomFile{vromStart,vromEnd}` 由链接符号 `_##name##SegmentRomStart/End` 生成。**即：OoT 的"资源系统"= 链接器 + DMA 表，不是运行时归档**。这与 RAGE 的 RPF7/RSC7 页式容器（2.1.3）是两种完全不同的工程哲学。
* 段系统现行实现：`include/segmented_address.h:7 gSegments[NUM_SEGMENTS]`、`:9 SEGMENT_TO_VIRTUAL`；底层来自 SDK 头 `include/ultra64/mbi.h:38-40`（`NUM_SEGMENTS 16`、`SEGMENT_OFFSET(a)=a&0x00FFFFFF`、`SEGMENT_NUMBER(a)=(a<<4)>>28`）、`R4300.h:14 K0BASE`、`convert.h:12 OS_K0_TO_PHYSICAL`。**注意：`include/segments.h` 与 `SM_*` 宏在当前 HEAD 已被删除（全库 `grep "define SM_"` 零命中）**，早期报告若引用它们需更正。

#### 3.1.4 引擎骨架（Actor / 场景 / 碰撞 / 相机 / 音频）

| 子系统 | 实现事实 | 证据（`oot/`） |
|---|---|---|
| GameState 覆盖层 | `GameStateOverlay{loadedRamAddr, RomFile file, vramStart, vramEnd, init, destroy, instanceSize}`，size 0x30；ID 由 X-macro 表生成；加载 `Overlay_LoadGameState` | `include/z_game_dlftbls.h:7-19,:22-30,:31,:33-34`；`src/code/z_DLF.c:9`、`:72`。`src/overlays/gamestates/` 现仅剩 4 个（file_choose/opening/select/title），**Play 状态已进主 code 段**（`src/code/z_play.c`、`src/code/game.c:49-401`） |
| Actor | `extern ActorOverlay gActorOverlayTable[ACTOR_ID_MAX]`，注释保留**原始符号名与地址** "original name: actor_dlftbls" / `801162A0`；`gMaxActorId // original name: "MaxProfile"` | `include/z_actor_dlftbls.h:67-68`；表容量来自 `include/tables/actor_table.h` 的 **474 条 `DEFINE_ACTOR`** |
| 场景 | **场景命令解释器**：`union SceneCmd`（size 0x8，`{s32 id:8; arg}`）+ `Scene_ExecuteCommands` + 派发表 `sSceneCmdHandlers[SCENE_CMD_ID_MAX]` | `include/scene.h:228-255,:257,:511`；`src/code/z_scene.c:38`（表）、`:247`（解释器）、`:587`。命令 ID 表 `include/scene.h:403-423`：`PLAYER_ENTRY_LIST=0x00`、`ACTOR_LIST=0x01`、`COLLISION_HEADER=0x03`、`ROOM_LIST=0x04`、`WIND_SETTINGS=0x05`、`SPAWN_LIST=0x06`、`SPECIAL_FILES=0x07`、`ROOM_BEHAVIOR=0x08`、`ROOM_SHAPE=0x0A`、`OBJECT_LIST=0x0B`、`LIGHT_LIST=0x0C`、`PATH_LIST=0x0D`、`TRANSITION_ACTOR_LIST=0x0E`、`LIGHT_SETTINGS_LIST=0x0F`、`TIME_SETTINGS=0x10`、`SKYBOX_SETTINGS=0x11`、`SKYBOX_DISABLES=0x12`、`EXIT_LIST=0x13`、`END=0x14`，其后仍有 `SOUND_SETTINGS/ECHO_SETTINGS/CUTSCENE_DATA/ALTERNATE_HEADER_LIST/MISC_SETTINGS`（`:497-509`）。**这就是 OoT 的"资产类型标签表"** |
| 资产类型枚举 | **不存在** `include/z64*.h`、不存在 `Z64FILETYPE_*` / `TYPE_*` 枚举（`find -maxdepth 3 -name "z64*.h"` 空）；类型系统改由 X-macro 表承担：`actor_table.h`(474)、`object_table.h:10-13`、`scene_table.h`、`sequence_table.h`、`effect_ss_table.h`、`entrance_table.h`、`dmadata_table.h` | 见左（**对本报告任务书的更正**） |
| 碰撞 | **静态 subdivision 查表 + 链表节点**：`BgCheck_InitializeStaticLookup`、`BgCheck_GetStaticLookupIndicesFromPos`（`src/code/z_bgcheck.c:21-22`）、`BgCheck_Vec3sToVec3f/Vec3fToVec3s`（`:196/:205` 定点化）、`BgCheck_RaycastDownStaticList:573`、`BgCheck_RaycastDownStatic:627`、`BgCheck_ComputeWallDisplacement:663`、`BgCheck_SphVsStaticWall:696`、`BgCheck_CheckStaticCeiling:904`；动态侧 `:31-39` | **`qsort` 空间划分与 RTREE 全树 0 命中，早期说法不成立** |
| 相机 | 主循环 `Vec3s Camera_Update(Camera*)`；`CameraDataType` 枚举；固定镜头类型宏 | `src/code/z_camera.c:8038`；`include/camera.h:314-343`、`:1651`；`include/scene.h:386-389 SCENE_CAM_TYPE_*`；调试相机 `src/code/db_camera.c`（`:8207 DebugCamera_Update`，仅 debug 特性）。**`Camera`/`Unique1`/`Unique2` 子状态存在，但 Z-targeting 没有可读符号名**（`gZTargetingEnabled`/`TargetingModeSwitcher` 0 命中 → 机制改由专利正文证明，见 3.1.5） |
| 音频 | **三段式**：`src/audio/game`（`sequence.c`、`sfx.c`、`sfx_params.c`、`session_init.c`）/ `src/audio/internal`（`seqplayer.c` **2,278 行**、`heap/load/playback/synthesis/thread/os/effects`）/ `src/audio/tables`。序列播放器 `:171 AudioSeq_GetScriptControlFlowArgument`、`:194 HandleScriptFlowControl`、`:267 InitSequenceChannel`、`:325 SeqChannelSetLayer`、`:1358/:1487 AudioHeap_SearchCaches(FONT_TABLE,…)`、`:1653 AudioHeap_LoadFilter`。**`:25 static_assert(MML_VERSION == MML_VERSION_OOT)` 明确与 MM 的 MML 版本分叉** | CPU 侧序列播放器线程 + RSP `aspMain` 两段（"双 DSP 线程"表述未取证 ?） |
| 存档 | `src/code/z_sram.c:68-77` 六槽，注释原文 "the latter three saves are backup saves for the former saves"（**3 主 + 3 备份**）；写前校验→失败回退备份 `:663-664/:710/:735`；**iQue 改走 `Sram_ReadWriteIQue`（`:66-68`）** | F |
| 版本可追溯性 | 调试版把**各版本原始行号**硬编码进断言宏 `LN3(595, 598, 615, 746)`（`src/boot/z_std_dma.c:445,:454`，机制见 `include/line_numbers.h`）；调试版有**独立 DMA 表** `include/tables/dmadata_table.h:6-8`（`#if (GC_EU_DBG_2 || GC_EU_MQ_DBG || GC_EU_DBG) && !NON_MATCHING` → `dmadata_table_dbg.h`） | F —— **"同一份 EAD 源码跨版本行号漂移"的直接化石**，也证明 16 个版本真的是同一代码库的不同编译 |

#### 3.1.5 Z-targeting 专利（法定公开通道）

* **必须更正**：Z-targeting 专利**不是** US 6,443,842（该案《Method, program product, and game system for blurring an image》优先权 2000-01-28、授权 2002-09-03，内容是球类游戏景深模糊，且晚于 OoT）。
* **正确的母案**：**US 6,165,073**《Video game apparatus and memory medium therefor》，申请 09/179,954（**1998-10-28**），授权 **2000-12-26**；**分案 US 6,626,760**（正文自注 "now U.S. Pat. No. 6,165,073"），授权 2003-09-30。F（Justia 正文，2026-11-18 抓取）。
* 说明书里的实现细节（与 OoT 行为一一对应，F 正文）：摇杆=第一操作手段、**Z 按钮 47Z=第二操作手段**；以玩家正面 0° 为基准的 **±60° 视野内检测 NPO**；多目标按**直线距离**定优先级；**锁定标记 LM 红色=敌人、黄色=其它**；锁定时屏幕上下插入 **UBP/LBP 非显示带**；相机置于 **PO 后方、贴近两者中心连线**；左右输入变成**绕目标的圆周平移**；可循环选中**屏幕外** NPO；无目标时锁定**前方无限远点**；并描述卡带 **256kbit 电池后备 RAM** 与 RCP(RSP/RDP) 框图、可选光盘驱动器。
* **上游专利 US 6,139,433**（授权 2000-10-31）= SM64 的摇杆/四 C 键相机/按速度削减多边形/Z 键蹲爬 —— 说明 OoT 相机与锁定是**一条成族的专利布局**而非孤立发明。
* 到期：按 20 年制**推算 ≈2018-10-28**（U；PTA/terminal disclaimer/官方期限记录未取证 ?）。两案**均未点名《时之笛》**（只用 PO/NPO 术语），与 OoT 的对应属推断 U。检索 "Koos v. Nintendo" **无结果 → 未找到**。

#### 3.1.6 开发者访谈里的技术陈述（Iwata Asks，F）

来源：Iwata Asks《时之笛 3D》第 2 卷第 5 章（宫本茂 / 青沼英二 / 小田部洋平），Wayback 2026-11-18 快照。

* OoT 有约 **500 种动作/姿势模式**；改用卡带而非 64DD 的原因是 **64DD 的寻道时间无法满足"多动作即时调用"**。
* 宫本茂对体感操控的原话是 **"We couldn't do it on the Nintendo 64 system"**（俯视举剑/体感指向）—— 该想法最终落在 Wii《天空之剑》MotionPlus 与 OoT3D 的陀螺仪上。
* **由此推翻一条中文圈常见说法**："iQue 版加入了体感拔剑"**没有任何证据**（见 3.3.3）。

---

### 3.2 GameCube 与 Wii VC：任天堂自己的两个闭源模拟器

**这是 OoT 生态最独特的现象**：GC 版与 Wii VC 版都不是移植，而是**任天堂在主机里内置了一个 N64 模拟器**，而这个模拟器本身后来被社区反编译了。

* 定性原文（F）：`oot-gc/README.md:23` —— "A work-in-progress **decompilation of the N64 emulator** used in the GameCube releases of The Legend of Zelda: Ocarina of Time"。支持 9 个版本（`README.md:27-35`：`mq-j/u/e`、`ce-j/u/e`、`mm-j/u/e`），主攻 `ce-j`，理由是收藏版光盘**残留 ELF 符号**（`:37-39`）。
* **容器**：GC 版以 **TGC** 归档封装 N64 ROM —— `oot-gc/config/ce-j/config.yml:4` `object: files/tgc/120903_zelda.tgc:sys/main.dol`（`:5` 带 SHA1）；`mq-*` 三版共用 `files/zlj_f.tgc`；`README.md:85` 的支持格式列表含 `TGC`。**9 个 config 目录 = 9 个版本**。
* **动态重编译（DRC）实证**：`src/emulator/_cpuGCN.c:235` 函数注释原文 "**The main MIPS->PPC Dynamic recompiler.**"；`:6080` 新建 recompiled function block；`:6229` "Searches the recompiled block cache for an address, or creates a new block"；`:9925-9987` 重编译环境下的 call 打补丁（注释明示"下次执行时改写地址"）；`:10014/:10294` VR4300 load/store 的即时重编译。函数原型 `cpuRecompileFunction` 只存在于 `debug/Fire/_cpuGCN.c:40` 这份**未命名副本**里（`src/` 下无字面 `cpuRecompileFunction`，属未完全命名状态）。
* **RSP/RDP 是 LLE（真微码执行）**：`src/emulator/rsp.c:173 rspSetupUCode`、`:185 rspFindUCode`（`:213` 按 code/data 偏移匹配已注册 `RspUCode`；`:227-233` **从 N64 RAM 取微码正文求校验和**；`:251-265` 反推 "F3DEX…" 名字；`:268-275` 版本号），IMEM/DMEM 写入与 PC 寄存器 `:787/:802/:824/:838/:1021`；GBI 解码 `src/emulator/_gspF3DEX.c:66,:128,:153…`（0xC1-0xC4 为 S2DEX1、0x02 为 F3DEX1），另有 `_gspS2DEX.c`、`_gspJPEG.c`；RDP 侧 `src/emulator/rdp.c:30 rdpParseGBI(…, RspUCodeType eTypeUCode)`，含 `:511 G_RDPLOADSYNC`。**音频同样是 LLE**：`src/emulator/_aspMain.c` + `src/emulator/audio.c:48-50`（DAC 速率回写）。
* **音频出口是 GC 硬件 AI DMA 而非 AX**：`src/emulator/soundGCN.c:202 AIInitDMA`、`:276 AIStartDMA`、`:376 AIRegisterDMACallback`；`src/dolphin/ai/ai.c`、`src/dolphin/os/OSAudioSystem.c` 是 Nintendo SDK 复刻。**过场动画改成 THP 视频**：`src/emulator/THPPlayer.c`、`THPVideoDecode.c`。
* **规模**：`src/emulator/` 48 文件、`src/dolphin/` 17 子目录 84 文件、全仓 `src/**/*.c` 192 文件；代表文件行数 `simGCN.c` 3,312、`_cpuGCN.c` **10,819**、`rsp.c` 1,325、`_gspF3DEX.c` 1,597、`rdp.c` 885、`xlCoreGCN.c` 299。
* **Wii VC 侧的对应**（F，`oot-vc`）：`README.md:17` 定性为 "the N64 emulator used in the Wii Virtual Console releases"；`:19-29` 支持 9 版（sm64/mk64/oot × j/u/e），`:31` 主攻 `oot-j`；输入从 TGC 光盘变成 **`content1.app`**（`config/oot-j/config.yml:3`，`:4` SHA1）。GC→RVL 的替换是逐文件的：`xlCoreGCN.c→xlCoreRVL.c`(260)、`soundGCN.c→soundRVL.c`(410)、`codeGCN→codeRVL.c`、`xlFileGCN→xlFileRVL.c`、**删除 `mcardGCN`**；新增 `vc64_RVL.c`(230)、`controller.c`(740)、`banner.c`、`pak.c`、`storeRVL.c`、`helpRVL.c`；`src/dolphin/` 整体换成 `src/revolution/`（os/ax/axfx/dsp/fs/nand/vi/pad/hbm/{vcmv,nw4hbm,homebutton}/wenc/ipc）。
* **两个决定性的架构差异**：① `xlCoreRVL.c:158 xlCoreHiResolution` **恒为 true**（Wii 高清输出），GC 版对应的是 `xlCoreGCN.c:135 xlCoreInitFilter` + `:109 xlCoreInitMem`（RVL 侧无 filter/mem，改由 RVL GX 承担）；② Wii 上无 DVD，故 `vc64_RVL.c:41-49` 把 `simulatorDVDOpen/Read + simulatorShowLoad` **全部桩化**，`:127 simulatorRun(SystemMode*)`、`:146 xlMain`。输入/存储改走 Wii 栈：`controller.c:9,12` include `revolution/kpad/KPAD.h`、`wpad.h`，`:152 KPADInit`；`storeRVL.c:10,:126,:136` NAND 读写；`xlCoreRVL.c:186 NANDInit()`。
* **VCMV = Wii 的 Home Button Menu 影片查看器**（GC 版完全没有这一层）：`src/revolution/hbm/vcmv/` 6 文件，含 `vcmv_javascript.cpp:18 "vcJavaScriptExt"`、`:54 WWWAddJSPlugin("vcJavaScriptExt.dll"…)`（挂在 HBM 的 www 浏览器上）、`code_800838C0.cpp:125`、`:444 #if VCMV_REVISION == 1`。DRC 同源代码在 `oot-vc/src/emulator/cpu.c:1769,7628,7786,10759,10848,11134`，与 GC 的 `_cpuGCN.c` 一一对应 —— **同一 codebase 的 Wii 分支**。
* **GC 版内嵌的仍是 MIPS z64 ROM**（决定性旁证，F）：`oot/README.md:37-46` 列出 10 个 `gc-*` 版本的构建时间戳 —— `gc-jp 02-10-29 23:49:53`、`gc-jp-mq 02-10-30 00:15:15`、`gc-us 02-12-19 13:28:09`、`gc-us-mq 02-12-19 14:05:42`、`gc-eu-dbg-2 03-02-13`、`gc-eu-mq-dbg 03-02-21 00:16:31`、`gc-eu-dbg 03-02-21 00:49:18`、`gc-eu 03-02-21 20:12:23`、`gc-eu-mq 03-02-21 20:37:19`、`gc-jp-ce 03-10-08 21:53:00`；对照 N64 原版 `ntsc-1.0 98-10-21`、`pal-1.1 98-11-18`（`:32,:35`）。**即 GC 碟里的 ROM 是 N64 ROM 晚 4~5 年的重新构建物**（`docs/compilers.md:3-4` 亦证 GC 仍用 IDO/MIPS）。
* **为什么默认版本是 `gc-eu-mq-dbg`**（F）：`oot/Makefile:42` `VERSION ?= gc-eu-mq-dbg`、`README.md:49,:192`。原因有三：① 它是**未 stripped 的调试构建**，泄漏内部符号与调试菜单（`Makefile:57-60` 警告调试版专有 test scenes、`elf_msg` 调试图形）；② 它有**独立 DMA 表**（3.1.4）；③ `README.md:153-154` 说明需要先剔除 overdump 并修补头部才能匹配 —— 反过来说明这份调试 ROM 的数据本身就是原始产物。代价：`oot-gc` 的主目标反而是 `ce-j`。
* **画面增量**（R）：GC 版分辨率由 N64 的 320×240 提升至 **640×480** 并支持 **480p 逐行扫描**（de.wikipedia OoT 条目）；Limited/Collector's 双碟：EU 2003-11-14、US 11-17、JP 2004-04-01。**Dolby Pro Logic II / 16:9 变形宽屏：未找到一手证据（?）**；"Nintendo SPD 参与 GC 模拟器"亦无来源（?），官方署名止于 EAD 级。
* **Wii VC 的行为差异**（R）：VC 版是 N64 版直接复制，**不支持体感包震动**；而把 GC 合辑碟放进 Wii 则可震动。**PAL50 强制问题：未见官方说明（U）**。

---

### 3.3 神游机 iQue Player 官方移植版（2003）

#### 3.3.1 硬件与分发（主证据：拆解笔记 retroactive.be/personal/ique/，2026-11-18；辅证 Wikipedia IQue Player）

| 项 | 取值 | 冲突/备注 | 级 |
|---|---|---|---|
| SoC | **定制 NEC ASIC**，R4300i 时钟 **1.5× N64 = 140.625 MHz** | 52poke/IGN 沿用 N64 的 93.75 MHz；**"N-Stage"与"PolyPipe"两个名称在所有来源 0 命中 → 未找到（疑误记）** | F（拆解）/ ?（命名） |
| 内存 | **16 MB DDR**（约 8 MB 供游戏）＝内置 Expansion Pak；计数寄存器有定制 libultra 补丁修正 | IGN 的"4 Mb Rambus"是 N64 原规格 | F |
| 载体 | **64 MB(512 Mbit) NAND 神游卡**，透明模拟卡带 PI 总线并**实时 AES 解密**；单游戏 4–40 MB | 中文媒体有 256 MB 口径；"16 MB Flash"未能对应 | F / ? |
| 分发与 DRM | 神游加油站（iQue Depot）+ iQue@Home USB；**SKSA/ticket、AES-128-CBC + ECC**、`.app` → 用本机私钥重加密为 `.rec`；**iQue@Home 授权服务器 2016-12 关停** | — | F |
| 手柄 | **无振动**（社区明确）；**动作感应＝未找到证据** | — | F / ? |
| 校验 | OoT iQue 用 CIC6102 口径写 checksum；但 SM64 `cn` 侧注释 "**cn has no checksums**"、`--pad-to=0x7B0000` —— 两作镜像处理不同 | — | F |

#### 3.3.2 关键判定：**硬件再制造 + 同一引擎源码的第二次编译目标，不是软件模拟器，也不是原 ROM 直接跑**

四条彼此独立的证据：

1. 拆解笔记原文："based on the original MIPS CPU and RCP design files — **and is not an emulator**"；且"games still used the **original graphics microcodes**"（F）。
2. **官方把原版源码与构建工具交给了移植方**：BroadOn / Routefree 将 libultra SDK **移植到 Linux** 并加 GDB-over-USB（协议同当年 Ultra64 Indy 的 RDB）（F）。**本地仓库里的铁证**：`oot/Makefile:188-195` 的 `ique-cn` 块中，构建者字符串是 **`build@toad.routefree.com`**（原版 EAD 内网机器风格的主机名出现在 iQue 版本的构建戳里），`REGION ?= US`、`PLATFORM := IQUE`、`DEBUG_FEATURES ?= 0`、`REVISION := 0`（零售 GC 为 15）。
3. **iQue 版在反编译里是完整可构建版本**（F）：`oot/README.md` 给出构建戳 `03-10-22 16:23:19`、MD5 `0ab48b2d44a74b3bb2d384f6170c2742`；`include/versions.h:5-20` 把 `IQUE_CN=16` 与其余 15 版并列数值化；构建差异见 `Makefile:899-904`（IQUE → `COMPRESS_ARGS := --format gzip --pad-to 0x4000`、`CIC = 6102`；其余 → `--format yaz0 --pad-to 0x800000 --fill-padding-bytes`、`CIC = 6105`），随后 `:909-914` 用 `ipl3checksum sum --cic $(CIC)` 回填校验和；还需两处**针对 iQue 的二进制补丁**：`Makefile:632 patch_ique_driverominit.py`、`Makefile:1103 patch_ique_kaleido_reloc.py`。**`--pad-to` 从 8 MB 档降到 0x4000 档，本身就是"载体从卡带变成 NAND"的构建级痕迹。**
4. SM64 反编译 `VERSION=cn` 独立复现同一模式：**8 个 game + 5 个 menu 模块重写、libultra 全量以 EGCS 1.1.2 重编**（`-mcpu=r4300 -fno-pic -DBBPLAYER -G 0`）（F）。

#### 3.3.3 神游版相对 N64 原版的可确认差异（全部 F，来自 `oot` 源码）

* **文本系统为中文重写**：`src/code/z_kanfont.c` 的 `Font_LoadCharCHN`、`text_lang: CN`；`kanji_tex` 从 Shift-JIS 汉字表改为中文点阵字体。
* **压缩换成 gzip**：`src/boot/z_std_dma.c:461-465` 的 `#if !PLATFORM_IQUE`（见 3.1.3）—— **iQue 是同一个 EAD 代码基上唯一走 DEFLATE 的目标**。
* **存档路径改写**：`src/code/z_sram.c:66-68 Sram_ReadWriteIQue`（N64/GC 是并行 SRAM 直读，iQue 走 NAND 抽象）。
* **微码换 PosLight 变体**：`baseroms/ique-cn/config.yml` 含 `gspF3DZEX2_NoN_PosLight_fifoText`（GC 亦用，N64 原版用 `.NoN.fifo`）。
* 独家内容：仅见**试玩时长闸门**（OoT 600 分钟 vs Wave Race 60 分钟，F）；**"体感拔剑/挥杖"未找到任何证据**（宫本茂原话反而说明 N64 做不到，见 3.1.6）。
* 首发日期 2003-11-18（ique.com 存档 / wiki，R），与中文媒体"11-17"、定价 ¥498/¥598 **互相矛盾 → ?**。

#### 3.3.4 生态与法务

* **支持 iQue 的反编译有两家**：SM64（`cn`）与 OoT（`ique-cn`）（F）。社区亦称至今**没有可用的 iQue 模拟器**（R，IGN 2014-08-30）—— 这与"iQue 不是模拟器而是硬件再制造"自洽：没有 N64 硬件语义之外的额外可模拟层，却要复刻 NEC 定制 SoC + NAND 加密。
* iQueBrew 于 2017 破解神游机/加油站，2023-04-01 用 Gigaleak 素材复原**早期神游版《姆吉拉的假面》**，并记录 5 次 ROM 修订（R，yystv 2023-04-30）；工具 `iQueDiagExtend`/`iQueTool`，站点 iquebrew.org、lacklustre.net/n64/ique（R）。
* **官方 iQue SDK/源码"泄露或开源"事件：未找到**；Gigaleak 属任天堂资产泄露，不是 iQue SDK（U）。
* 神游科技（中国）2002-12 成立，注册资本 2,800 万美元，**颜维群 51% / 任天堂 49%（任天堂以技术、专利、软件版权出资）**，2009 转全资；企业条目显示商标 39 条 / 专利 6 条 / 著作权 57 条（R，中文百科 2026-05-20）。2000–2014 主机禁令下以"**多媒体互动系统**"名义销售；岩田聪 TGS 2003 说明以 SoC 技术移植 N64 游戏并汉化（R，中青报 2003-10-14）。
* **iQue 专属的"Take-Two 式"维权案：未找到** —— 与第 2.6 节的 Take-Two 记录形成对照。

---

### 3.4 Grezzo 的 OoT3D（2011，3DS）：闭源原生引擎

#### 3.4.1 公司事实（先纠正三个错误猜测）

Grezzo **2006-12 由石井浩一（Koichi Ishii，Square《圣剑传说/Mana》系出身）创立并任社长**，东京涩谷，2026 年员工 111 人，社名取自意大利语 "diamante grezzo"（F，Wikipedia rev.1386171981）。先做 WiiWare《Line Attack Heroes》(2010)，随后被**岩田聪**邀约投入 3DS 项目，2011-04-12 由 Eurogamer 披露承接 OoT3D；后续 MM3D(2015)、Tri Force Heroes(2015)、Ever Oasis(2017)、Luigi's Mansion 3DS(2018)、Link's Awakening(2019)、Echoes of Wisdom(2024)。  
**"Meridian9"、"现改名 Green Technology"、"创始人中原巧"三个猜测均未找到任何来源支持（?）** —— 这三条在中文资料里流传很广，应主动纠正。

#### 3.4.2 重写而非模拟：四条本地实证（F）

1. **它是 ARM C++ 工程**，不是 MIPS：`oot3d/src/crt0.cpp:1 #include <nn/svc/svc.h>`、`:25 b __cpp(nn::svc::ExitProcess)`；SDK 桩目录 `src/nn/{applet,dbg,dsp,err,os,srv,svc}.cpp`；mangled 符号留在 `disassembly/oot3d.cfg:4 _ZN2nn3svc15DuplicateHandleEPNS_6HandleES1_ force`、`:2 nninitRegion`。
2. **CTR 系统服务名以字符串残留**：`src/data/conststring.cpp:505 "gsp::Lcd"`、`:506 "gsp::Gpu"`（GSP 模块名是 CTR 独有）。
3. **libultra 符号 0 命中**：`osViSwapBuffer`、`OSMesgQueue` 等在全仓检索**无命中** → 不存在 N64 内核。
4. **工具链是 ARMcc/RVCT**，不是任何模拟宿主：`README.md:27-31` 要求 **RVCT4.0 [Build 821]** 并设 `ARMCC_PATH`/`RVCT40INC`/`RVCT40LIB`；`CMakeLists.txt:6-18` 强制校验三者并 `find_program(armcc.exe … win_32-pentium)`；`ARMCC.cmake:14-20` 用 `armlink/armar`。产物 `oot3d.axf` + `code.bin`，MD5 `20c7ec6288ba0af4feb6db646c6cc6d5`（`README.md:10-11`，与 `data/checksum.md5:1` 一致），`romfs.bin`/`oot3d.3ds` 尚未目标（`:13`）；**仅支持 USA**（`:5`），baserom 命名 `disassembly/baserom.3ds`，v0/v1、trimmed/untrimmed 均可（`:25`）。

#### 3.4.3 反编译成熟度（可量化，F）

* `disassembly/oot3d.cfg` 共 **12,031 条 `arm_func`**，其中 **7,707 条仍是 `FUN_` 无名** → **≈64.1% 未命名 / 35.9% 已命名**（口径对照见 3.8.5；C++ 化率见 3.8.7）。
* 已匹配侧：`src/functions/` 31 个 `functions_*.cpp`（如 `functions_100000s.cpp`，内部 `FUN_` 提及 198 次）、`src/game/` 450 个 cpp（`actors/`、`classes/`、`z_*.cpp`），`include/` 有 `z3Dactor.hpp`、`tables/actor_table.hpp` 等重建头；**更正（本轮实测）**：未匹配部分**恰恰是以 `src/functions/functions_*.cpp` 落盘的**——那 31 个文件共 9,008 条 `GLOBAL_ASM("asm/FUN_xxxxxx.s")` 清单（详见 3.8.7）；`oot3d.cfg` 与 `disassembly/` 是它的上游输入（`data_chunks.txt` 411 行、`rodata_chunks.txt` 5 行、`SHT__INIT_ARRAY.txt`、`tools/{ctrtool,exefs2elf.py,n3dsdisasm,gcc_to_arm_asm.py,split_arm_data_chunks.py}`）—— 与 N64 版 `n64decomp-oot/asm/` 的风格不同。**结论：OoT3D 反编译仍处早期，全仓 50,081 行，仅为 N64 版（625,253 行）的 8%。**

#### 3.4.4 随机器侧对闭源 3DS 重制版的测绘（`OoT3D_Randomizer`，F）

它是跑在 3DS 上的 C++ homebrew（`ootrando.rsf` + `Makefile` + `source/main.cpp`），提供了一组与反编译正交的证据：

* **加载基址**：`code/patch.py:14 off = vaddr - 0x100000` —— 正是 3DS `code.bin` 的加载基址，与 `oot3d.cfg` 里 `arm_func 0x00100000 __ctr_start` 吻合。`:16-28` 只取 `.text/.loader/.patch*` 段输出 IPS。
* **hook 机制是汇编级、且直接复用原版入口**：`code/src/asm/hooks.s:4-8 .macro HOOK name`、`:11-16 HOOK into_loader → bl loader_main → b nninitRegion`（`nninitRegion` 就是 `oot3d.cfg:2` 的那条符号）。另有 `patches.s`、`svc.s`、`csvc.s`、`svc_loader.s`。**本仓没有 `Hooks.yaml`**（那是其他随机化器的机制）。
* **自定义资源容器 `.zar`**：`source/patch.cpp:660-661` 写 `romfs/actor/zelda_gi_melody.zar` —— 与 `oot3d/src/game/zar.cpp` 对应，即 **3DS 重制版自己的归档格式**。
* **音频替换走系统侧 BCSAR**：`source/music.cpp:376,:395` 改 `/luma/titles/<id>/romfs/sound/QueenSound.bcsar`。**本仓不处理 `model_info.bcsv`**（全仓 `grep -r bcsv` 0 命中）。
* **交付方式是 LayeredFS 而非解包签名**：`source/patch.cpp:88-99` 建 `/luma/titles/<titleId>/romfs/actor`，`:112` 删旧 `code.ips`，`:114-118` 打开 `code.ips`，`:124` 按区域二选一 `basecode_USA.ips`/`basecode_EUR.ips`，`:642-644` 写 `exheader.bin` / `exheader_citra.bin`。标题 ID：`source/settings.cpp:3565-3569 TitleId()` 只返回 `0004000000033600`(EUR) 或 `0004000000033500`(NA)；`source/main.cpp:57,59 aptSetChainloader(0x0004000000033500/33600, 2)`。
* **它不解包 NCCH、不做验签、也没有 ROM SHA 白名单**：全仓 `source/**` 对 `rsa|ncch|sign` 0 命中，只有 vendored libctru 头 `code/include/3ds/services/fs.h:183` 提到 SHA256/AES-CBC（那是 FS 存档 seed，属系统服务描述）。**即"随机器完全不触碰签名/加密"**，这是它能在 3DS 上长期公开存续的技术前提。

#### 3.4.5 官方产品事实（R）

30 fps（对比 N64 的 20 fps）；城堡市场等**原预渲染背景改为实时 3D**；新增体感瞄准/视角、触屏道具与部分笛奏、单人 Boss Challenge（MQ 版场景镜像且伤害翻倍）、Sheikah 石"Visions"提示影片、Shard of Agony 以声音替代震动。**"双人共斗迷宫"、"剧场模式/拍照模式" 未找到证据（?）**；"8 MB FCRAM" 未找到出处（?）；`.cfa/.csu/.csar` 资产扩展名在仓库中检索不到（?）。"迷宫重设计由 Grezzo 提案"的一手引言取不到（iwataasks 站点本轮 TLS 校验失败 → U/?）；Aonuma 仅就"收录并调整 MQ"表态（R）。

---

### 3.5 Nintendo Switch Online 模拟器与 2026 官方重制版

#### 3.5.1 NSO / Nintendo Classics（闭源模拟器，**唯一未被反编译的一环**）

| 事实 | 级 |
|---|---|
| 日服《NINTENDO 64 Nintendo Switch Online》App **2021-10-26 首发即含 OoT**；海外 NSO N64 2020-04；**2025-06-04/05 更名 "Nintendo 64 – Nintendo Classics"**，同时新增逐游戏操作说明与按键自定义，并在 **Switch 2 独占**追加模拟滤镜与倒带 | R（weblio 词条 2026-09-22） |
| 官方页面 2026-10-18 仍列 OoT；**"2022 年曾下架"未找到任何来源** | F（官方页）/ ? |
| OoT 功能对照：即时续玩/存档、按键自定义、联机（其他 N64 游戏）NS1/NS2 皆可；**倒带与 CRT 滤镜仅 NS2** | R（gamewith 2026-08-23） |
| **模拟器开发方：任天堂未署名** | ? |
| 最强旁证：子公司 **NERD**（原 Nd Cube/ActiMagine）公开技术清单含迷你机 NES 模拟器 **Kachikachi**、SNES 模拟器 **Canoe（含 Super FX）**、NSO 经典 NES/SNES 服务、Wii U 的 DS 模拟器、New 3DS"超稳定 3D"；2022-04 泄漏的 GB/GBA 内核代号 **Hiyoko / Sloop** 亦归 NERD | R（求闻百科条目 2023-09-10；ITHome 2022-04-19 转 Neowin） |
| "ARM 侧 JIT"官方从未说明；**任天堂 × Analemma/GGPO 合作未找到一手来源**；NSO 是否对 ROM 打补丁官方无声明——**但 2022-01 datamine 观察到某次更新只改模拟器、ROM 未变（见 3.8.1）** | ?（技术形态属推断 U；"模拟器与 ROM 分别版本化"属 R） |
| 可确认的是：**OoT 本身无在线对战**，NSO 联机仅对多人 N64 游戏开放 | F/R |

**判读**：NSO 是 OoT 官方链上**唯一"闭源且无任何开源镜像"**的环节 —— 原因正是第 5.1 节的加密门控：Switch 的 exefs/romfs 需要密钥才能解密，反编译的前置输入本身不可合法公开获取（对照：GC/Wii 版无加密 → 已反编译；3DS 版 → 靠 CFW 已可反编译，但只到 8%）。

#### 3.5.2 2026-11-05 Switch 2 重制版：确认存在，且是原生重制

* **时间线**：公布 **2026-06-09** Direct；实机与日期 **2026-09-08《塞尔达 40 周年 Direct》**；发售 **2026-11-05，Switch 2 独占**；**Switch 1 版官方未公布**（R，Wikipedia 2026 条目 / The Verge 2026-09-08 / zeldacentral 2026-09-13）。
* **署名**：**Nintendo EPD**，协力 Monolith Soft、SRD、1-Up Studio、Nintendo Pictures、**Nintendo Technology Development**、NERD、Nintendo Studios Singapore、Bandai Namco Studios（东京/马来西亚）；producer 青沼英二 + 藤林秀麿，director **HokWai Ng**（F/R：官方 credits 与《Ask the Developer Vol.24》）。
* **不是模拟器**：任天堂将其描述为"为 Switch 2 从零构建"（R）。
* **引擎与性能**（Digital Foundry，2026-11-04，R）：基于 **BotW 引擎的升级版并沿用 TotK 的修改版**（**不是 UE5、不是第三方**）；底座 **1080p→1440p 动态分辨率**、掌机 **720p–1080p**（掌机 LOD 更激进、纹理降级）；**SMAA + AMD FSR 1 空间升频，未使用 DLSS**；PBR 材质、动态大气散射、实时光照与环境阴影、Navi 动态光源、碎罐物理、低雾流体；**大致稳定 60 fps**（对 N64 20fps、3DS 30fps）。
* **玩法增量**（R）：冲刺、更快游泳、体感瞄准、全镜头控制、**手动跳跃**、BotW 式道具滚动、部分过场配音、城堡镇改为可探索连续空间、**全区时间不再暂停**、"Threads of Time"任务日志、**用 Switch 2 麦克风吹奏笛子**、"Hall of Fallen Foes" BOSS 重打、与手机 Nintendo Switch App 的 "Zelda Notes" 联动、单存档槽、"Pro mode"；**明确不含 Master Quest**。
* **销售**：首 4 日 **520 万份**（日本 81 万），Switch 2 史上最大首发（R）。主题机 2026-10-29；Young Link / Zelda amiibo 2027。**卡带容量/下载体积/程序 ID 官方未公布（?）**。

#### 3.5.3 社区重编译给出的"闭源内部事实"（本报告认为可信度最高的部分）

官方只给了营销层信息，而**真正的技术栈参数来自两个独立项目的交叉印证**：

| 参数 | ProjectReLink | Hookshot | oot-pc-recomp |
|---|---|---|---|
| Program ID | `0400486024DEC000`（`README.md:24`） | 同（`docs/design.md:179-196`：文件名 `[0400486024dec000][v131072].nup`） | 同（`docs/oot-status.md:3`） |
| 更新版本 | v131072（`:25`） | **update v131072 (base v65536)** | 1.1.1（另一构建 `oot111`） |
| 构建哈希 | `e0f3ccfa`（`:26`） | main 模块 build-id **`e0f3ccfaf50065ae…`** | — |
| 定性 | "unofficial **static recompilation** … **with no emulator**"（`:5-6`） | "model as mkw-recomp (Mario Kart World), **which this project forks**"（`:6`）、"The game is a Switch 2 title, so the base is mkw-recomp, not TOTKPort"（`:13`） | 目标实为《Donkey Kong Bananza》（`AGENTS.md:3`），OoT 只是实验分支 |

由交叉印证可得到的**闭源重制版技术事实**（F，指"社区对该二进制的观测"）：

* **AArch64 + SDK/NVN2 22.2.8 + 39-bit 地址空间**（`Hookshot/docs/design.md:179-196`）；`.nup` 镜像 **25.6 GB**；**`main` 模块 `.text` 63,201,744 字节**；**romfs 186,583 文件 / 24.4 GiB**，其中 **10,154 个 `Nin_Ounce_NVN2` 着色器文件**。
* 翻译规模：S1 全量翻译 **295,178 函数 / 17,668,191 指令 / 1,475 个 jump table**；S2 **57 个 chunk、485 个 HLE 函数、243 个 OS import 桩、22 个 sdk NVN 函数**路由到 `host/nvn2.c`（同处）。
* 架构（`Hookshot/docs/design.md:56-73`）：`exefs/{rtld,main,subsdk*,sdk}+main.npdm` → `nso2/image.py`（Eden 布局 0x80000000，自任 rtld）→ `a64_lift+insn+lift_fixes+poll+chunks` → **每 guest 函数一个 C 函数**（`work/sa/gen` 约 1 GB C）→ clang -O1/-O2 + lld → `hookshot.exe` + `hookshot_vk.dll`；`:83-85` "**SDK is replaced at the nn:: boundary**"（用 `HLE(<mangled>)` 顶替 OS 相关函数，而 libc / nn::util / nn::mem / NVN2 前端**仍作 lifted code**）；`:80-82` 512 GB 预留、堆从 64 GB 起。
* 复用关系（`:37-40`）：fork 基 `mkw-recomp 7336f0a` / 本地 `86871a1`；`a64-kit`（TOTKPort 60de030）；**Eden shader recompiler（Eglantine build 181）→ SM86 IR → SPIR-V + task/mesh**；TOTKPort 仅作参照。归属 NotRexed（`readme.md:1034`）。
* 图形侧最难的闭源语义是 **NVN2 的描述符 bank**：`gpu/vk/vk_bank.cpp:53,:440`（NVN2 compute 的 packed c0、task/mesh 阶段常量 bank `0x670`/`0x770`）、`gpu/vk/eden/…/translate_program.cpp:28-31,:144-145`；`oot-pc-recomp/docs/oot-status.md:32-35` 记录某个 compute shader 展开 22 个 c0 record 生成 **495 KB SPIR-V**，导致 NVIDIA 驱动 122 s + `vkCreateComputePipelines` 95 s、堆 2.6 GB 后 OOM。
* 其它可抄的工程事实：`ProjectReLink/README.md:319-321`（`nx2/lift.py` ARM64→C、`nx2/build.py` + `host/`、`host/nvn_native.c` 记录图形调用、`gpu/vk/` 重放到 Vulkan）；`:61-62` 需 **Vulkan 1.3 + mesh shaders**（RTX 20+ / RX 6000 / Arc；0.7.9 起对无 mesh shader 的 GTX 10 走降级路径，见 `gpu/vk/vk_draw.cpp:5618` 把 task/mesh 降为 compute dispatch）；`:28-29` 输入为 `.nup` 或 `romfs`+`exefs`；**`oot-pc-recomp` 的 OoT 分支症状**：`:7-12` 白色世界来自继承自 Bananza 的 `light_quad_fault_skip`（跳过 NVN2 stage `cae0ef8e9fe9147f` 与 Location 9 输入 + fragment 读 c0+0x530 的 quad）、`:16-18` 用 SPIR-V 直改 `OOT_FOG_SCALE/OOT_EXPOSURE`（fog compute `a4550cb09fe72248`、composite `65156d0c69e40c69` 的 `c3[3].z`）。
* **必须剥离的 PC 侧功能**：DLSS / DLAA / FSR 3.1 / FSR4 / 帧生成**都是 PC 移植端能力，不可归给官方**（`ProjectReLink/README.md:88-90`：默认用"游戏自己的 upscaler"，AMD FSR 3.1 随包、NVIDIA DLSS/DLAA 需另行下载 SDK 重新构建；`:165` "No DLSS, no frame generation" 表明其跑分数据未开启这些）。这与 3.5.2 官方"未使用 DLSS"的 DF 结论一致。
* **第三方交叉印证**：`Ocarina-Atlas` 是一个纯 TypeScript/Vite 的**地图/地牢追踪器**，它对 2026 重制版**没有改动一行引擎代码**，而是寄生于他方运行时的调试接口：`src/lib/lichii-server.ts:14 OOT_REMAKE_TITLE_ID = '0400486024DEC000'`、`:1567-1569` 按 Title ID 或 Longshot UID 找 `slot_*`、端口固定 `:6545`（`src/lib/i18n.ts:340` "Offline ({emulator} RPC :6545 closed)"）；`README.md:41-47` 列出可对接宿主 Longshot/hookshot、Project ReLink、Triforce、HeyListen、NX2Saria、Lichii/Fritchii/CheezyEmu。
* **发售当天的对照性事实**（R）：官方重制发售数小时即被 **Lichii** Switch 2 模拟器以 60FPS 跑起来（PC Gamer 2026-11-06，Jacob Ridley）—— 民间重制停手、官方重制当天被模拟器化。

---

### 3.6 CryZENx 民间闭源重制版（2015–2026-06-13）

**先记录一条取证限制**：`cryzenx.com` **现已不是项目站** —— Wayback 2025-07-11 快照显示为 MagicAI SaaS 模板页，实时访问 HTTP 526。因此下面几乎所有"作者自述技术细节"都**没有一手文本**，凡标"未找到"者不得写成事实。

| 时间 | 事件 | 引擎证据 | 级 |
|---|---|---|---|
| 2015 | 首曝演示（初始森林/Kokiri） | UE4 | R/? |
| 2016-01-22 | Kokiri 房间 demo，附 PC 下载，仅"房间 + 林克模型 + 动画"，键鼠操作；**体积≈原版 ROM 的 30 倍** | UE4 | F/R（VICE） |
| 2016-04-18 | Gerudo Valley demo，公开下载；"基于 3DS 版本制作"；"这个项目目前还没收到来自任天堂的律师函" | **UE4.11** | R（3DM） |
| 2016-12 "该月专门演示" | — | — | **未找到** |
| 2019-03-14 / 2020-07-09 | Lost Woods；NPC 分支对话 + 战斗 + 咕鸡小游戏；"始终没有接到律师函" | UE4 | R |
| 2023-10-14 | 可下载 demo + 全局光照 | **UE5.3.1 + Lumen** | R |
| 2024-09-23 / 2024-12-31 | 完整哥隆矿山（Dodongo's Cavern）；卡卡利科村房屋 | UE5.4.4 + Lumen / UE5.5 | R |
| 2026-05 → 06 | 最新公开可玩 = **哇哈哈腹腔（Jabu Jabu）**；12 关卡量级；**免费档锁 30FPS、赞助档 60FPS** | **UE5.7.4 + Lumen + 硬件光追** | ?（低可信自媒体） |
| **2026-06-13** | **终止全部开发**（Patreon 公告） | — | F（ScreenRant 2026-06-27）/ R |

* **开发者**：本名 **Giuseppe "CryZENx" Macula**（F，GamesRadar 2026-06-22）；年龄/国籍的公开表述未找到。
* **2016 年任天堂发函 = 不成立（U）**。GamesRadar 逐字："To be clear, in none of CryZENx's posts has the creator indicated that Nintendo had gotten in contact with them to request that they stop working on their remake, so it seems this was a decision made purely out of respect."；"只停止分发可执行文件但继续开发" **未找到**。
* **停手原话**（F）："I would like to tell you that Ocarina of Time project has officially stopped… you have been supporting me for like a decade."（ScreenRant）；"I think I have made history with my Remake"、"im sure nintendo will do it this time the right way"（GamesRadar 引 Patreon `finally-ocarina-160628447` / `future-plans-160924640`）；"i dont really want to step nintendo on their way"（Automaton 2026-06-23）。**触发点 = 2026-06-09 官方 NS2 重制官宣**（R，17173 2026-06-09）。**从未定价、从未计划发售**（无可查商店页）。
* **技术管线可核实部分**：资产**手工重模**是唯一有证据的路线（VICE 描述的 demo 只含重建房间 + 角色 + 动画）；"基于 3DS 版本制作"应读作**以官方 OoT3D 为视觉参照**而非资产提取（R）。**"不含任何任天堂资产"这一措辞的原文未找到**；可核实的是**从未分发 ROM/二进制资产，只发 .exe 包，走 Patreon 私密链接**。是否读取原版地图数据/脚本 = 未找到（无一手文本、无源码可审计）。**Nanite**：公开报道仅见 Lumen/光追，Nanite 未找到；**MotionBuilder/Mixamo** 动画管线**未找到任何来源**；自制 mission 系统、PS4/Xbox 多平台计划 = **未找到**。
* **闭源重制的结构性问题**：闭源 + 仅二进制 = **社区无法审计**，与 SoH/重编译项目的"仓库零任天堂文件 + 用户自备 ROM"形成第 5.3 节的对照。
* **现状（2026-10）**：据报道已改做 UE5《缩小帽》全 3D 重制，无发售日（R，3DM/18183/china.com 转 GameRant）；候选项目还包括 Metroid Prime / DK64 / Diddy Kong Racing（?）。历史 demo 链接保留未删。**X @CryZENx、Discord、YouTube 一手动态，以及 Nintendo Life / Eurogamer / TheGamer 的 2024-2026 专稿：均未找到**（本轮命中媒体为 VICE、GamesRadar、ScreenRant、Automaton、PC Gamer + 中文门户）。

#### 3.6.1 同类项目对照

| 项目 | 路线 | 关键点 |
|---|---|---|
| **Ship of Harkinian**（`Shipwright`，libultraship） | 反编译 + 开源引擎重编译 | ROM-hash 门控；**开源**；2022-03 起公开（R） |
| **Zelda64Recomp / TotK Recomp** | **静态重编译** + RT64/Vulkan 1.3 | MIT；仓库不含游戏文件、需自备 dump；TotK PC 移植 v0.4.0 2026-11-11（R） |
| 高清贴图包 | 模拟器 hi-res 贴图替换 | Emulation General Wiki 2026-08-26 "Community Retexture Project V7"；Cellpack 2011(Djipi)/Phantom7 V3/Community Pack 6.0-7.0。**"GenD / Viper"署名未找到** |
| **OoT unJPEG**（真机 mod） | **在原版引擎内**把预渲染背景改为实时几何 + 自研剔除 | R（18183 2026-08-18）。**这是唯一"在闭源引擎内部做重制"的公开案例，与 Grezzo/2026 官方重制的方向一致** |
| Zelda 64: Definitive / OoT Redux | — | **未找到可靠来源（仅 SEO 噪声）**，判为不存在或不可信 |
| **Hyrule Warriors** | ω-Force + Team Ninja **自研闭源引擎**，任天堂监督的官方外包 | Wikipedia infobox **无 Engine 字段** → 引擎名未找到（R）。作为"官方闭源外包引擎"参照：任天堂宁愿用自家引擎（2026）或监督外包，也未把 OoT 交给 Unreal |
| Skyline（Android 独立模拟器） | 团队主动停更后镜像长存 | 2026-08 DMCA 波及 29 仓库；关停年份中文媒体记为"2023"（?，与 2024-05 报道冲突） |
| UE5 做 N64 重制的通用公开工具 | — | **未找到**（除个人 demo 个案） |

---

### 3.7 补充实测：本轮新增的九项代码级证据（全部 F）

以下九项为对上述各节的追加实测，均可用本节末的命令复核，且不与 3.1–3.6 已有条目重复。

#### 3.7.1 闭源 OoT 的源码分叉轴是"调试 / 平台"，不是"内容"

`oot` 全仓 `OOT_MQ` 命中 12 次，其中 2 次是 `include/versions.h:40/42` 的宏定义本身；**实际条件编译点只有 10 处，且只落在 2 个 `.c` 文件**（`z_en_mag.c` 9 处 + `z_scene_table.c` 1 处）：`src/code/z_scene_table.c:559`（唯一结构性分支——切换迷宫场景表）与 `src/overlays/actors/ovl_En_Mag/z_en_mag.c` 的 :132/:233/:313/:329/:541/:636/:648/:659/:692（Menacing 提示的显示与位移细节；:541 上方保留原厂注释 `// Title logo is shifted to the left in Master Quest`）。数据侧则是全部：`baseroms/gc-us-mq/config.yml` 的 27 个迷宫 `xml_path` 中 **12 个指向 `*_mq.xml`**（`bdan_mq.xml`、`Bmori1_mq.xml`、`HAKAdan_mq.xml`、`ganontika_mq.xml`、`ice_doukutu_mq.xml`…），而 `ntsc-1.0/config.yml` 里 `_mq` 出现 **0 次**。

四条平台/内容轴的分支密度实测排序：**`DEBUG_FEATURES` 533 > `PLATFORM_N64` 187 > `PLATFORM_IQUE` 136 > `PLATFORM_GC` 65 > `OOT_MQ` 10**。

同向的一条硬证据在 `src/boot/z_std_dma.c:49`：

```c
#pragma increment_block_number "gc-eu:0 gc-eu-mq:0 gc-jp:0 gc-jp-ce:0 gc-jp-mq:0 gc-us:0 gc-us-mq:0 ntsc-1.2:82"
```

即行号增量表里 **MQ 与同区非 MQ 版的差值为 0** —— MQ 不改代码段。

**结论**：EAD 引擎的版本轴几乎不编码玩法差异，代码只按"是否调试版 / 跑在哪块芯片上"分叉。这与 3.2 的"GC 碟里是同一 MIPS 源码的重新构建物"合并读，说明需要重建的对象数量比零售 SKU 数量少一个量级 —— 这是 3.1 能够逐字节复现的前提条件之一。

#### 3.7.2 为 EGCS 编译器缺陷而牺牲源码可读性的三处物证（iQue）

3.3.3 已记录 gzip / BaseBus / 中文字体三项差异；补编译器侧证据。`src/code/z_sram.c:883-886` 把双重下标拆开重写，并留有注释：

```c
// Workaround for EGCS internal compiler error (see docs/compilers.md)
u8* fileName = fileSelect->fileNames[fileSelect->buttonIndex];
gSaveContext.save.info.playerData.playerName[offset] = fileName[offset];
```

更极端的是 `src/overlays/gamestates/ovl_file_choose/z_file_copy_erase.c`：:39-44、:156-161、:179-193 三段按钮位移动画循环（`for (i = 0; i < 5; i++)` / `< 3`、`titleAlpha[0] -= 31` / `nameBoxAlpha[…] -= 25`）在 `PLATFORM_IQUE` 下**整段不参与编译**，改用 :35-37 / :152-154 声明的 `s16* array` 间接寻址重写。

**方法论**：这类"为编译器让路而把代码写坏"的形态，是判定一个闭源产物实际编译环境的可靠旁证——它比任何文档都难伪造，且与 `docs/compilers.md:4-6,27`（N64/GC 用 IDO、iQue 用 Linux SDK 里打过补丁的 EGCS 1.1.2）互为印证。

#### 3.7.3 iQue 相对 GC-US 的功能分叉判据

`z_en_mag.c:53/165/215` 的条件写作 `#if OOT_VERSION < GC_US || PLATFORM_IQUE`，即"SRAM 初始化 + 13 键重置序列"这套调试设施在 N64 各版、`gc-jp` 与 iQue 中存在、**在 `gc-us`/`gc-eu` 零售版被移除**；:61 正是 `SsSram_ReadWrite(OS_K1_TO_PHYSICAL(0xA8000000), buffer, 0x800, 1)`。`src/boot/z_std_dma.c:560-562` 只在 iQue 分支给 `dmacopy_bg()` 加了 `PRINTF`。

标题画面三处换料（`z_en_mag.c`）：`:560-566` 删除 `FILENAME_UPPERCASE('P''R''E''S''S')` 五个字母；`:701-707` `gTitleTitleJPNTex` → **`gTitleTitleCHNTex`**；`:755-758` **`gTitleCopyright19982003IQueTex`** —— 版权条素材名把 1998（N64）、2003（iQue）与厂商一起写进了标识符，这是本地语料里对该版本发行年份最直接的一处自证。

#### 3.7.4 `oot3d` 的语义进度可量化，且远低于其字节进度

3.4.2 已确认它是 ARMcc/RVCT 的匹配式反编译、`code.bin` 可对齐。补测命名度。语料形态先说清楚，否则数字会被误读：`src/` = **39,050 行 / 494 个 `.cpp` + 430 个 `.hpp`（`.h` 仅 1 个）**，其中 **`src/functions/functions_*.cpp`（31 文件 / 19,210 行）不是反编译出来的 C，而是 `GLOBAL_ASM("asm/FUN_xxxxxx.s")` 的清单**——即"这个函数我们没能用 C 重写回去，只能塞回原始汇编"。真正的重建发生在 `src/game/`（450 文件 / 18,854 行）。全仓统计：

| 指标 | 实测值 | 命令 |
|---|---|---|
| `GLOBAL_ASM` 条目总数（= 仍为原始汇编的函数体） | **12,254** | `grep -rhoE 'GLOBAL_ASM\("[^"]+"\)' oot3d/src \| wc -l` |
| 其中 Ghidra 式无名 `FUN_` | **7,612（62.1%）** | `grep -rhoE 'GLOBAL_ASM\("asm/FUN_[0-9A-Fa-f]+\.s"\)' oot3d/src \| sort -u \| wc -l` |
| 其中已被社区命名（`Actor1D7_Draw`、`AcquireGpuRight`…） | **4,629（37.8%）** | 同上管道取反 |
| `src/functions/` 子集内的唯一 `FUN_` 记号 | 7,446 | `grep -rIoh -E '\bFUN_[0-9a-f]{6,8}\b' oot3d/src/functions \| sort -u \| wc -l` |
| 反汇编器登记的函数总量 | **12,031** | `grep -c arm_func oot3d/disassembly/oot3d.cfg` |
| 写成 C++ 函数体的数量 | **≈172** | `grep -rcE '^[A-Za-z].*\(.*\)[[:space:]]*\{[[:space:]]*$' oot3d/src/game --include='*.cpp'` 求和 |

两处结构差异也值得一记：`src/game/` 里的 `GLOBAL_ASM` 有 3,013 条具名 / 仅 171 条无名，而 `src/functions/` 反过来（1,554 具名 / 7,454 无名）——说明**这个项目按"是否已经能命名"把文件分堆存放**，目录本身就是进度条。

真实命名几乎只存在于类型/头文件层（`z3Dactor.hpp`、`z3Danimation.hpp`、`z3Dbgcheck.hpp`、`sys_math3d.hpp`、`ichain.hpp`、`non_matchings.hpp`、`regs.hpp`、`graphics.hpp`、`color.hpp`、`tables/`）与 `src/nn/{svc,applet,dsp,err,srv,os,dbg}.cpp` 系统桩；`README.md` 的 Contributing 段正文是 **"TODO"**；`LICENSE.txt` = **CC0 1.0 Universal**（与 `oot-gc` 一致）。

**本报告的结论（修正版）**：以 12,031 个函数为分母，**只有约 1.4%（≈172 个）被还原成 C++**；62% 连名字都没有；即使被命名的那 38%，其函数体仍是**逐条汇编原样回填**（`GLOBAL_ASM`），语义知识停留在"名字和签名"这一层。与 3.1 的"513 个带偏移结构体 + 100% 匹配"并置可见，**"能否逐字节重建（L4）"与"能否读懂它在干什么（L2 语义）"互不蕴含**——OoT3D 拿到了前者，后者不到 2%。命名率的口径对照见 3.8.5。

#### 3.7.5 `OoT3D_Randomizer` 里有一份 11 行的闭源二进制分段图

3.4 已用 `code/oot.ld`（982 行地址表）与 `generate_patch_symbols.py:4-5,17`（`arm-none-eabi-nm` 从 `basecode_USA.elf` 抽 16 个符号）。补 `code/address_conversion_info.txt`（**11 行**）——USA→EUR 的分段偏移表：

```
000000-419E17  ±0
419E18-41A143  =/= (language-fetching function)
41A144-43668B  +24
436690-4A5ADF  +20
4A5B00-5C6607  ±0
```

据此可推闭源 code 段 ≈ **6.06 MB**，且该文件明确指认"取语言字符串的那个函数"是**唯一的区域错位源**；`generate_patch_symbols.py:30` 另断言两区域的 `_LD_TEXT_START` 相同。

评价：这 11 行一次性给出闭源 ARM 二进制的分段结构、总尺寸与区域分叉点，单位字节信息量高于多数逆向报告——因为它不是描述，而是补丁工具的**运行输入**，错一位就无法生效。

#### 3.7.6 Switch 2 重制版：资产容器与转储质量被写进了安装器

四项代码事实（`ProjectReLink/README.md:135-152`，配套 `tools/dump_check.py`、`tools/dump_repair.py`）：

- `romfs/Tex` 下共 **20,667 个贴图文件**（作者自测转储中 3 个损坏）。
- 两个工具**硬依赖 `zstandard`** ⇒ 容器条目为 **zstd** 压缩。
- 预烘焙光照文件以 **`RBake_Scene_…`** 命名，且同一地点存在 **child/adult 两套**（修复器优先用"另一年龄的同一片光照"顶替，并声明"另一年龄的光照可能略有偏差"）。
- 症状分两类：丢光照文件 → "它照亮的一切画成纯黑（一整面悬崖）"；丢模型贴图 → "模型永远加载不完，原地反复重载，占用多个 CPU 核"。安装器自动写出 `mods\00-dump-repair` 并在其 `README.txt` 里列出替代关系。

另有一条**引擎决策的反证**（同文件 v0.6.3 记录）：该 build "draws a box for each object in a **depth-only pass**, asks the graphics card how many of its pixels passed, and leaves out of its next frames the objects that got none"，而重编译层此前**一律回答"可见"**；打开真答复后 Market 54.6→61.4 fps、Lost Woods 61.9→67.0、**每帧 draw call 由 2,931 降至 2,494**（`RX_VK_OCC_QUERIES=0` 可关）。⇒ 该 build 使用逐对象保守遮挡剔除，而这一事实只在被"答错"时才暴露出来。

#### 3.7.7 该 build 的着色器是私有着色器表示，社区已做确定性离线翻译

`Hookshot/BAKE.md:1` 标题即 **"BAKE — offline NVN2 shader bake (developer tooling)"**，:3-7 描述流水线："ROM/container enumeration → offline **seed-to-SPIR-V** translation (no Vulkan device) → bank emission (portable manifest **and** the live `hookshot_programs.bin` format) → read-back verify"，并声明 "No game files, no GPU, no pushes"；:11-13 记录 `gpu/shaderbatch.cpp` 取自分支 `bake-mimo c60fd1d`（verbatim），产出 graphics/compute/**task/mesh** 四类 runtime program key。同仓指纹命中：`spirv/SPIRV` 61/46 文件、`glsl` 39、`shader` 535、`ASTC` 38、`BC7` 17、`bsp` 54。

**硬件契约同样泄露了引擎选择**：`Hookshot/readme.md:63,65` —— RX 6000（Navi 21/22）在 Windows 上默认"把 task 与 mesh 着色器走 compute 回退，因为驱动会丢 GPU"（引 PR #1747/#1992/#2215），v2.0.6 起的回退"依赖 buffer device address"，且"无法支持从 buffer 取实例数的 mesh draw"（issue #640）；`ProjectReLink/README.md:66` —— 无 mesh shader 的 GTX 10 系"帧率约一半，**且部分大石头消失**"。⇒ 场景几何以 mesh shader 表达、剔除计数来自 GPU buffer。

**一条必须记录的阴性结果**：在 `Hookshot` 与 `ProjectReLink` 全仓检索 `Unreal`、`UE5`、`Havok`、`PhysX`、`FMOD`、`Wwise`，**命中文件数全部为 0**（对照同仓 `Nintendo` 7、`SDK` 47）。⇒ 3.6 关于 CryZENx / 虚幻 5 的叙述**不能由这两个重编译工程佐证**，只能维持其 R/? 级别。而该阴性结果本身有方法论价值：**静态重编译只需要 ABI 与指令语义，不需要知道目标用了哪个引擎**——重编译工程既没有、也不需要"引擎识别"这一步。这与 GTA VI 侧只能靠 PlayGo 元数据推测架构是同一种处境，差别在于 OoT 这边已经越过了可执行层。

#### 3.7.8 官方模拟器的"逐游戏例外表"直到被反编译才可见；开源侧则一直是公开文档

`extra-repos/n64-oot/GLideN64`（GPLv2，HEAD 2026-10-11）**不存在** `CustomizedHacks.cpp` 或任何 `Project*` 每游戏文件；机制是**卡带 ROM 名字符串匹配 + 位标志**：

```cpp
// src/RSP.cpp:314-317, :325-326
if (strstr(RSP.romname, "THE LEGEND OF ZELDA") || strstr(RSP.romname, "ZELDA MASTER QUEST"))
    … hack_subscreen | hack_ZeldaMonochrome …
else if (strstr(RSP.romname, "MASK"))
    … hack_ZeldaMonochrome | hack_ZeldaMM …
```

`src/Config.h:263-286` 定义 24 个 `hack_*`，其中 :269 `hack_subscreen (1<<6) //Fix subscreen delay in Zelda OOT and Doubutsu no Mori`、:274 `hack_ZeldaMM (1<<11)`、:283 `hack_ZeldaMonochrome (1<<20)`、:286 `hack_fbTextureOffset (1<<23)`、:265 `hack_blurPauseScreen //Game copies frame buffer to depth buffer area, CPU blurs it`。消费点在 RSP uCode 实现内部：`src/uCodes/S2DEX.cpp:880/:926`（"Zelda MM uses depth buffer copy in LoT and in pause screen."）/`:929/:956`、`src/FrameBuffer.cpp:475`、`src/gDP.cpp:430`（"Does not work with Zelda MM"）。每游戏 hack 相关代码量 ≈ **2,359 行**；配置库 `ini/GLideN64.custom.ini`（384 行）以 `Good_Name` 为键（:347/:382-383），而 `src/CRC.h` 只有 8 行——**键是 ROM 名而非 CRC**。注意：**未检索到任何"Zelda fog"hack**，该说法在本批无对应代码。

`mupen64plus-core` 侧游戏知识集中在 `data/mupencheat.txt`（**29,888 行**）：`:2418 8011A64B` 物品表、`:2464 8011A65B … 0021:"Weird Egg" … 002C:"SOLD OUT"`（商店库存数组）、**:2499 `8011B965 ???? 0003:"Kakariko Windmill" … 00F8:"Gerudo Fortress Jail"`（整张 room/stage ID 枚举；v1.1 下同表位移到 `:2607 8011A80B` / `:2654 8011A81B`，位移一致）、`:2491` Beta/调试模式开关。core 中**没有** `mkp_romdatabase` 式 CRC 表（那是 Dolphin 的实现）。

本节最有价值的一条是 `:2239`：

```
gn Zelda no Densetsu 2 - Mujura no Kamen - Zelda Collection Version (J) (GC Version) [!]
```

即**一个 N64 开源模拟器的码表明确登记了 GC"塞尔达合集"版**。这与 3.2 的"GC 版 = 模拟器 + 同一 MIPS ROM"互为表里：GC 变体在 N64 生态里被记录为"另一种硬件上的同一二进制"。对照 `extra-repos/n64-oot/mm` 的 `baseroms/` 只有 `n64-jp-1.1` 与 `n64-us` 两项（**无 iQue / GC / MQ / PAL**，`README.md:28` 并明确把 MM3D 排除在支持范围外："a totally different game"），可见**同一零售变体在不同社区的可见性差异极大**——这构成 §6 方法论的一条输入。

两侧合读得到的结论：开源模拟器必须公开、可审计地维护"这款游戏在哪种硬件时序上会坏"，于是引擎对硬件的假设（FB→TMEM 拷贝、CPU 回读、深度缓冲拷贝）被持续记录成公共文档；任天堂官方 GC/Wii 模拟器有同一张表（3.2 里 `oot-gc` 的 `_gspF3DEX.c` 逐位模式重映射与 combiner→TEV LUT 正是它的形态），**只是直到被反编译之前它不可读**。在这一层上，"官方 vs 社区"的信息差已经消解，剩下的差别只有许可（CC0 vs GPL）与执法（§4）。

#### 3.7.9 清点口径的更正：80 个检出 / 58 份唯一上游

§1 的台账以 `git ls-files` 计受控文件数，本节结论不受影响；本轮另按 `remote.origin.url` 去重做一次清点，结果为 **80 个 git 检出 / 58 份唯一上游**。其中 `Zelda64Recomp`、`N64Recomp`、`N64ModernRuntime`、`Shipwright`、`TriAevum` 各被检出 **3 次**，`gta-repos/` 下 9 个条目全部是**符号链接**（指向 `/root/orca-workspaces/`）而非独立克隆。

**结论**：任何"语料库有多少仓库"的表述必须同时给出「检出数 / 去重上游数」两个口径——同一份语料在不注明口径时可以合法地报成 44、58 或 80。本报告写作过程中即因目录计数（"44 份"）而非 remote 去重而一度漏掉 `oot-gc`/`oot-vc`/`oot3d`/`oot-dc`/`ProjectReLink`/`Hookshot` 六个关键载体工程；这类漏数在闭源研究里的代价是不对称的，因为**漏掉的那一个仓库往往正是唯一的证据来源**。

#### 复核命令

```bash
cd /root/wsl-demo
# 3.7.1 / 3.7.2 / 3.7.3
grep -rIn 'OOT_MQ' oot-decomp-projects/oot/src oot-decomp-projects/oot/include
grep -rIoh -E 'DEBUG_FEATURES|PLATFORM_N64|PLATFORM_GC|PLATFORM_IQUE' oot-decomp-projects/oot/src | sort | uniq -c | sort -rn
grep -c '_mq\.xml' oot-decomp-projects/oot/baseroms/gc-us-mq/config.yml
sed -n '883,886p' oot-decomp-projects/oot/src/code/z_sram.c
grep -n 'gTitleTitleCHNTex\|gTitleCopyright19982003IQueTex' oot-decomp-projects/oot/src/overlays/actors/ovl_En_Mag/z_en_mag.c
# 3.7.4
grep -rIoh -E '\bFUN_[0-9a-f]{6,8}\b' oot-decomp-projects/oot3d/src/functions | sort -u | wc -l
find oot-decomp-projects/oot3d/src -type f | sed 's/.*\.//' | sort | uniq -c | sort -rn
# 3.7.5
cat extra-repos/n64-oot/OoT3D_Randomizer/code/address_conversion_info.txt
# 3.7.6 / 3.7.7
grep -nE '20,667|zstandard|RBake_Scene|2,931|mesh shader' oot-decomp-projects/ProjectReLink/README.md
sed -n '1,14p' oot-decomp-projects/Hookshot/BAKE.md
grep -rIl -E 'Unreal|UE5|Havok|PhysX|FMOD|Wwise' oot-decomp-projects/Hookshot oot-decomp-projects/ProjectReLink | wc -l
# 3.7.8
sed -n '314,326p' extra-repos/n64-oot/GLideN64/src/RSP.cpp
grep -n 'Zelda Collection' extra-repos/n64-oot/mupen64plus-core/data/mupencheat.txt
# 3.7.9
for x in */*/; do [ -d "$x/.git" ] && git -C "$x" config --get remote.origin.url; done | sort -u | wc -l
```

---

### 3.8 补充实测（二）：NSO 的"两段式版本化"与三条应降级的传闻

> 本节同样只补 §3.1–§3.7 未覆盖的部分；其中 3.8.2/3.8.3/3.8.5 是**对既有结论的收紧或纠偏**，不是新增主张。

#### 3.8.1 NSO 把"模拟器"和"游戏 ROM"当作两个可独立更新的部件【R，机制层为强证据】

§3.5.1 把"NSO 是否对 ROM 打补丁"记为 `?`（官方无声明）。这条可以往前推一格——不是靠官方声明，而是靠 datamine 观察到的**版本分离**，并且顺带挖出了一个此前没被本报告记录的机制：**NSO 有一套按游戏分发的补丁脚本系统**。

时间线（全部为 2022 年 1–2 月的公开转述，来源见本节末）：

* **2022-01**（随《班卓熊大冒险》上线的那次 NSO 更新）：OatmealDome 发现《时之笛》**水神殿前的水面纹理**已修复；**LuigiBlood 比对了包体内的 ROM，确认 ROM 本身没有任何改动** → 修复只能来自 Switch 侧的模拟器。此前 2021-11 美国任天堂总裁还公开回应过 NSO N64 模拟质量批评（"非常认真地对待"）。
* 同一批报道明确记下列表现存问题：**《时之笛》的雾气仍然没有**，《耀西物语》《纸片马力欧》《马力欧医生》的图形问题也未解决——所以这不是"整体升级了模拟精度"，而是**逐条、逐游戏的定点修补**。
* **2022-02，NSO v2.0.0** 的 datamine（OatmealDome 逐条公布补丁信息）给出机制：
  1. **"游戏补丁 Lua 脚本现已预编译"** —— 即原本以 Lua 脚本形态存在的**每游戏补丁**被提前编译进应用；
  2. 为《班卓熊》新增已知代码补丁；
  3. **为《时之笛》新增"与渲染相关的补丁"（转述口径写作 "CC 寄存器"）**。【U："CC 寄存器"的确切所指无法核到原文，本报告不解释其含义】

**结论性推论（本报告的分析，非来源原话）**：这三条合起来说明 NSO 的架构里有一个**独立于 ROM 的、按 title 分发的补丁层**，而且它是以**脚本**形式实现、后续版本再"预编译"以提高启动效率的。这与 3.7.8 的代码侧证据形成同构：开源侧 GLideN64 用 `strstr(RSP.romname, "THE LEGEND OF ZELDA") → hack_subscreen | hack_ZeldaMonochrome`（`src/RSP.cpp:314-317`、`src/Config.h:269,:283`）做同样的事，**闭源侧 NSO 也做同样的事，只是那张表从不公开**。二者的差别不在有没有"逐游戏例外表"，而在**这张表是否可审计**。

这条的意义在于**范式差异**：PC 端修一个游戏 bug 要发新版插件、用户要手动升级；NSO 可以在**用户无感知、ROM 不变**的前提下改变某个游戏的画面表现。也就是说，"NSO 版《时之笛》"不是一个固定的软件工件，而是一个**由模拟器版本 × ROM 版本 × 该 title 的补丁脚本版本三者共同决定的移动靶**。

因此对 §3.5.1 的实践性补充是：**要复现或指认 NSO 的实际行为，光记录 ROM 的 CRC 是不够的，必须同时记录模拟器版本**——正如 §3.5.3 对 2026 重制版必须记 `ProjectReLink/README.md:26` 的 `build e0f3ccfa` 一样。【F：该字段确实存在于本地镜像】

> 来源（R）：网易/游族资讯/taptap 对 OatmealDome 推特的转述，2022-01-24 与 2022-01-30（`m.163.com/digi/article/GUG30GU60526D8LR.html`、`news.yzz.cn/domestic/202201-1660724.shtml`）；v2.0.0 逐条补丁信息转述见搜狐游戏资讯 2022-02。**原始推文（OatmealDome / LuigiBlood 账号）本轮不可达**，且这些转述存在明显的字符污染，故本节全部条目停在 R，不上 F；已加入附录 C。

#### 3.8.2 纠偏：OoT3D 的资产扩展名清单以本地实测为准【F】

社区/教程口径里常把一批 Switch/其他任天堂引擎的扩展名（`oarc`、`bma`、`bmt`、`zakr`、`zeffect`、`bcsv`、`szss`）当作"OoT3D 也用这些格式"。**本地 `extra-repos/n64-oot/OoT3D_Randomizer` 全仓大小写不敏感 grep：这 7 个标签作为独立词全部 0 命中**（连"不支持 X"这类否定注释也没有）。3.4.2 表格里列出的那批标签才是实测在用的，应保持为唯一事实来源。

同时补两条成因分析（这是本报告的解释，不是检索结论）：
* `bcsv` 是 **Switch 时代**的参数表格式，`oarc` 是 **SMG1/SFU** 的归档容器——它们根本不属于 3DS 世代。
* **更关键的因果方向**：这些名字之所以会混进"OoT3D 格式清单"，正是因为 2026 年的 Switch 2 重制版（§3.5.3）把时之笛带回了 Switch 技术栈。**同一个游戏 IP 在两个世代有两套完全不同的容器体系**（3DS 的 `zar`/`cmab`/`bcsar` vs Switch 的 `oarc`/`bcsv`/`szss`）——这本身就是"任天堂每次移植都重做引擎层"的又一条证据。

**这条纠偏的实际价值可以从随机化器的音频实现反证**：它想换音乐，但 `bcsv` 一类的参数表它读不到也写不了，于是只能绕开游戏侧、改走系统文件替换——`source/music.cpp:376` 拼出 `/luma/titles/<titleId>/romfs/sound`，`:394-395` 把 `QueenSound.bcsar` 写到该路径下（`:529-534` 负责逐级建目录）。【F】**换句话说：3DS 世代真正对外的容器是 BCSAR（走 CTR 系统 romfs 覆盖），而不是任何 `*csv`/`*arc` 参数表**——凡是把 OoT3D 的"可替换资产"说成 bcsv/oarc 的说法，都混淆了两个世代的可改造面。

#### 3.8.3 三条应降级的 NSO 传闻【?】

检索后**没有任何一手来源**支持，但它们在中文/英文社区里流传很广，容易被误当事实写进分析。全部记为 `?`，禁止当事实引用：

| 传闻 | 核实结果 |
|---|---|
| NSO 模拟器源自/分叉自 mupen64plus | **无一手证据**。GLideN64 的兼容列表确实"与 Project64 / mupen64plus 的相似"（§3.5.1），但**兼容表现相似 ≠ 代码同源**。NSO 的可核对事实只有：Nintendo 自家 SDK、`nso` 二进制、闭源。 |
| NSO 是"硬件级 ARM 重编译器" | 措辞不成立。NSO 运行在 ARM 上、做动态重编译是可以推断的，但"硬件级"没有定义，且本报告能核对的只有**离线烤 SPIR-V 的是 PC 侧重编译器**（§3.5.3：Hookshot 把 NVN2 种子经 Eden 重编译为 SPIR-V），官方卡带里给的是 **10,154 个 `Nin_Ounce_NVN2` 着色器文件**——两者都与"NSO 的 CPU 重编译是不是硬件级"无关。该行按 `?` 处理。 |
| NSO 的《时之笛》是"用 Ghidra 重建源码" | **混淆了两件事**。Ghidra 反编译用于 §3.4 的 3DS `oot3d`（Grezzo），以及 Switch 2 静态重编译项目（§3.5.3 的 Hookshot / ProjectReLink）；NSO 自身是**在 Switch 上模拟 N64**，没有任何已知的 NSO 源码重建产物。 |

#### 3.8.4 Space World 1997 演示的双马画面：唯一一手来源口径【R】

§3.1 谈引擎架构、§3.3.5 谈"传闻与确证"的分界，此处只把这条常被引用的技术史事实钉到可核对的出处：任天堂** SPACE WORLD '97 实机演示**中出现了**两匹马同时在屏幕上**的画面，被广泛视为该作多人/骑乘 tech demo 的展示。可追溯的一手口径只有《Iwata Asks：时之笛 3D》第 5 章第 2 节（回想到 SPACE WORLD '97），以及当年 Famicom 杂志的现场报道。**注意口径**：这些来源回忆的是"演示内容"，**没有任何来源说这是引擎压力测试或刻意的 benchmark**。若要在 §3.1 的性能分析里使用它，必须写成"据开发者回忆"，不能写成"任天堂为验证双骑乘而做的测试"。【R，不是 F：无当时的技术文档】

#### 3.8.5 口径对照：`oot3d` 的函数总量与命名率有四个合法数字【F】

同一份仓库里，"有多少函数""有多少没名字"取决于你从哪一层取样。四个数字都实测可复现，引用时必须带分母：

| 口径 | 分母 | 无名 | 无名率 | 这一层实际量的是什么 |
|---|---|---|---|---|
| 反汇编器函数表 | **12,031**（`disassembly/oot3d.cfg` 的 `arm_func` 行） | — | — | 二进制里**被登记为函数**的地址总数 |
| `GLOBAL_ASM` 回填清单 | **12,254**（全 `src/`） | **7,612** | **62.1%** | 尚**未能用 C 重写**、只能塞回汇编的函数体；比上一行多出的 223 个来自分堆/重复登记 |
| `src/functions/` 子集 | 9,008 条 stub | 7,454 | 82.8% | "还没命名"那一堆（项目把已命名的挪进 `src/game/`） |
| `src/game/` 子集 | 3,184 条 stub | 171 | **5.4%** | "已经命名"那一堆 |

另有第三方口径给出"10,388 个函数、4,841 个未命名（46.6%）"——**与本地上表不同量级也不同意象**，最可能是**另一时间点的快照或另一分支**（本报告无法在其原始语料上复核，故只登记、不采信，也不与上表混用）。【U：数字来源不可本地核验】

**结论（本报告唯一采用的表述）**：以本地快照为准——12,031 个登记函数中，**写成 C++ 的约 172 个（1.4%）**，其余 12,254 个 `GLOBAL_ASM` 槽位里有 **62% 连名字都没有**。无论按 1.4%、38% 还是 62% 哪个口径，结论方向一致：**OoT3D 的 L4（可逐字节重建）满格，L2（语义具名）近乎空白**。这条对比 §3.1 的"100% 匹配 + 513 个带偏移结构体"才成立；引用任何单个百分比而不给分母，都会把这条结论讲歪。

```bash
# 3.8 复核（全部 F，从 /root/wsl-demo 起算）
cd /root/wsl-demo
for t in oarc bma bmt zakr zeffect bcsv szss; do printf "%-9s %s\n" "$t" \
  "$(grep -rniE "\b$t\b" extra-repos/n64-oot/OoT3D_Randomizer | wc -l)"; done
sed -n '376p;394,395p' extra-repos/n64-oot/OoT3D_Randomizer/source/music.cpp
grep -n 'hack_subscreen' extra-repos/n64-oot/GLideN64/src/Config.h
# 3.8.5 四个分母
grep -c arm_func oot-decomp-projects/oot3d/disassembly/oot3d.cfg
grep -rhoE 'GLOBAL_ASM\("[^"]+"\)' oot-decomp-projects/oot3d/src | wc -l
grep -rhoE 'GLOBAL_ASM\("asm/FUN_[0-9A-Fa-f]+\.s"\)' oot-decomp-projects/oot3d/src | sort -u | wc -l
grep -rcE '^[A-Za-z].*\(.*\)[[:space:]]*\{[[:space:]]*$' oot-decomp-projects/oot3d/src/game --include='*.cpp' \
  | awk -F: '{s+=$2} END {print s}'
```

#### 3.8.6 Switch 2 重制版内含 Master Quest 数据：必须停在 U【U】

2026-11-10（发售第 5 天）有数据挖掘者宣称在 Switch 2 重制版文件里找到**镜像世界（里世界）+ 被改动的地下城布局**的数据，但**被 DLC 门控**、当时不可进入；口径来自 dataminer 本人，经 IGN（并被子刊 17173 转述）报道。转述里出现的具名 dataminer 为 MrCheeze 与 OatmealDome，但**两人各自的具体主张在本轮无法核到原文**。【U：单一来源链、无任天堂确认、无本地仓库可印证】

为什么这条绝不能升格为 F/R：

1. 它是**运行时数据观察**，不是官方声明；而本报告里"官方是否内置 MQ"唯一有效口径是 §3.5.2 的 credits/直面会，那里**从未提到 Master Quest**——附录 B 已把"官方口径的 MQ"列为 `?`。
2. 它**不能与 §3.3 的 GC/神游 MQ 混用**。GC 收藏版把 MQ 作为**同一份 MIPS ROM 的第二张碟**，这一点有本地 `oot/baseroms/gc-eu-mq-dbg/config.yml` 直接印证（F）。Switch 2 重制版是**另一套引擎（BotW/TotK 系）下的原生重建**（§3.5.2/§3.5.3），即便真有里世界数据，也**不会是同一份内容的复用**，而是重新制作。
3. **对本报告主线的影响是双向的**：若成立，则任天堂在 2026 年把"MQ"从"额外一张碟"变成了"随包但锁住的资产"，这是加密门控定律（§5.3）的一个新变体——**内容可以在包里，但可及性由 DLC 票据决定**，与 §4.1 的"RPF 容器在包里、密钥在 exe 里"结构同形。

**采纳口径**：写成"有 dataminer 宣称（2026-11-10，U）"，不写成"包含 MQ"。任何依赖此结论的路线图（例如"官方重制版可直接做 MQ 随机化"）都必须先把这条当作未验证前提。

#### 3.8.7 决定性发现：`GLOBAL_ASM` 在本快照里展开为**空**，未匹配代码是构建时从原版二进制现场反汇编回来的【F】

这是本轮最有价值的一条，也是把 §3.4/§3.7.4 的"L4 完整 ≠ L2 语义"从推断升级为**机制证明**的证据：

```c
// oot3d/include/non_matchings.hpp:1-6   （全文）
#pragma once
#define GLOBAL_ASM(x)          // ← 展开为空，不产生任何代码
#define UNK_RETURN int         // ← 未知返回类型统一假装是 int
#define UNK_TYPE  int          // ← 未知参数类型同样
```

配套事实四条，全部可复核：

1. **仓库里一个 `.s` 文件都没有**：`find oot3d -name '*.s' | wc -l` = **0**，`git ls-files | grep -c '\.s$'` = **0**。也就是说 12,254 条 `GLOBAL_ASM("asm/FUN_xxxxxx.s")` 指向的文件**不在版本库里**。
2. **它们在构建时由用户自备的零售 `code.bin` 现场生成**：`disassembly/CMakeLists.txt:47` `n3dsdisasm -c ${DISASM_CONFIG} ${BASEROM_DIR}/exefs/code.bin > baserom_before_fixes.s`，`:59` 再由 `tools/baserom_fixes.py` 修补为 `baserom.s`。工具链自带 `disassembly/tools/{n3dsdisasm,n3dsdisasm.exe,ctrtool,ctrtool.exe,exefs2elf.py,gcc_to_arm_asm.py,split_arm_data_chunks.py,baserom_fixes.py,e2elf.ld}`。
3. 因此 3.4.2 那句 **`build/code.bin: OK`** 的字节级可复现性，其**来源不是"作者把函数都写出来了"**，而是"**构建系统把原二进制反汇编回来再汇编回去**"。逐字节匹配在这里是一个**接近恒真的成就**——它证明的是工具链（ARMcc 4.0 build 821）与汇编/反汇编环的可控，**不证明语义还原程度**。
4. `UNK_RETURN/UNK_TYPE = int` 是项目**自己承认不知道类型**的字面证据：所有未匹配函数的签名都被统一降级为 `int`，共 12,254 个槽位。

**这条为什么重要**：它把"匹配式反编译"这个术语里的两种完全不同的东西分开了——

| | N64 版 `oot`（§3.1） | 3DS 版 `oot3d`（§3.4） |
|---|---|---|
| 未匹配函数的处置 | **`GLOBAL_ASM` 全仓 0 命中、`asm/` 目录不存在**（仅有的 43 个 `.s` 全在 `src/libultra/`，是手写的库汇编而非未匹配桩）→ **无不匹配残留** | **12,254 条 stub 长期保留**，且 `#define GLOBAL_ASM(x)` 为空 |
| 字节级 `OK` 的来源 | 作者真的写出了这些函数 | 反汇编—再汇编的闭环 |
| 语义覆盖 | 100%（625,253 行 C） | 约 **172 个 C++ 函数体 / 12,031 个登记函数 ≈ 1.4%** |
| 类型知识 | 完整 | **12,254 处被写成 `int`** |

**由此得到一条可迁移的判定规则（本报告采纳）**：评价任何"可逐字节重建"的逆向工程，**必须先查它的未匹配部分是怎么进构建的**。如果存在"从 baserom 反汇编回填"的路径（本节的 `CMakeLists.txt:47`），那么 `code.bin: OK` 就**不能**被当作语义完成度的证据；反之，若匹配度是删除 `GLOBAL_ASM` 条目换来的（N64 版 `oot` 的路线、`gtasa-reversed` 的逐函数替换也是），字节指标才有语义含义。这条规则同样适用于 GTA 侧：`gtasa-reversed` 是"逐函数替换"路线，因此它的函数计数是诚实的；而任何声称"可编译原 exe"的项目都要按本规则重新审查。

```bash
# 3.8.7 复核（全部 F）
cd /root/wsl-demo
cat oot-decomp-projects/oot3d/include/non_matchings.hpp
find oot-decomp-projects/oot3d -name '*.s' | wc -l          # 0
sed -n '47p;59p' oot-decomp-projects/oot3d/disassembly/CMakeLists.txt
ls oot-decomp-projects/oot3d/disassembly/tools
grep -c 'GLOBAL_ASM' oot-decomp-projects/oot3d/src/functions/functions_100000s.cpp  # 202 条 stub（其中 FUN_ 198）
```

---

## 4. 法务时间轴：两家发行商的相反策略

### 4.1 Take-Two / Rockstar（F，逐条可核验）

| 日期 | 事件 | 对逆向项目的含义 |
|---|---|---|
| 2016-10 | GTA Online 首次大规模账号/金币清洗 | 运行时改动的经济后果 |
| 2017-06-15 | **OpenIV 收停止函并关闭**（PC Gamer） | 连"只读格式工具"也被追诉 |
| 2017-06-23 | Rockstar 声明焦点是"线上作弊"，非单人 modding | 官方划定可容忍区 |
| 2017-07-03 | OpenIV 复现，但跨作内容永久下架（Eurogamer） | 边界被实测出来 |
| 2020-08/09 | GTA Online 二次洗钱清洗 | — |
| 2021-02-20 | **re3/reVC 遭 DMCA 下架**，Take-Two 主张 **EULA 本身禁止逆向** | "干净重写"路线被正面命中 |
| 2021-06-10 → 06-28 | 开发者 counter-notice，代码恢复 | 反通知机制有效但脆弱 |
| 2021-09-03 | **起诉** re3/reVC 作者（VGC） | 从 DMCA 升级到诉讼 |
| 2021-10-02 | 二次下架 | 诉讼期间持续施压 |
| 2021-11-17 | modder 以合理使用/转换性使用抗辩（NME、GamesRadar） | 唯一一次成体系抗辩 |
| 2023-04 | Take-Two **撤诉**，条款未公开 | **"逆向属合理使用"从未成为先例** |
| 2019 / 2023 | 收购 RedM 与 **Cfx.re（FiveM）** | 多人 modding 被制度化收编 |
| 2024 | BattlEye 落地 GTA Online + 新封禁政策 | 运行时接管前置拦截 |
| 2026-08-21 | **DMCA §512(h) 传票致 Discord，401 个仓库一次性下架** | 见 4.3 |
| 2026-08-26 | "CyberLeek"入侵被承认；向 Discord/Microsoft/X Corp 发传票（S.D.N.Y.） | 泄露 ≠ 知识来源 |
| 2026-09-10 / 09-22 | 《Mod Guidelines》更新，明禁"运行在未发布的控制台/硬件/平台上" | 移植合法性前置封死 |
| 2026-11 | CEO Zelnick：AI 降低重新移植成本、"PC 优先"、不计划"同日多平台" | 长期利好（R） |

### 4.2 任天堂（F/R）

| 日期 | 事件 | 与 OoT 的关系 |
|---|---|---|
| 1998-11-21 | OoT N64 日版发售 | 起点 |
| 2002-12 / 2003 | GC 版 / 收藏版双碟（含 MQ） | 内置 N64 模拟器 |
| 2003-11-18（存在 11-17 冲突） | 神游机 iQue Player 中文版 | 硬件再制造 |
| 2011-04-12 / 2011-06 | Grezzo OoT3D 公布 / 发售 | 闭源原生重写 |
| 2020-04 / 2021-10-26 / 2025-06-04-05 | NSO N64 / 日服 App / 更名 Nintendo Classics | 闭源模拟器 |
| 2025-06-10 | **Yuzu → Tropic Haze 和解**：赔 240 万美元、Yuzu 站点与全部衍生代码/构建物永久移除、**模拟器不得访问"任天堂当前或未来主机"的内容** | 定义模拟器法律天花板 |
| 2025-08-05 | 任天堂声明：**使用 ROM 须同时持有原版游戏与 ROM**；发布反模拟器/反盗版工具 | 直接约束"用户自备 ROM"模式 |
| 2025-10 | **Nintendo 诉 ArsTechnica** 相关案（含 DMCA §1201 反规避主张） | 触及"评论与报道"边界 |
| 2026-03 | 任天堂诉 Amazon（涉及侵权商品 listing） | — |
| 2026-06-09 / 09-08 / 11-05 | 官方 Switch 2 重制版公布 / 定档 / 发售 | **首个工作日即公布全部 modding API** |
| 2026-11-10 | **Nintendo 诉 GamesVenia** | 持续执法（R） |

### 4.3 2026-08-21 的 401 仓库下架：一次必须精确解读的事件

Take-Two 借 **DMCA §512(h)** 向 Discord 取得传票，一次性下架 **401 个仓库**。必须强调三点：

1. **§512(h) 传票的法律目的是揭露匿名身份，不是下架通知本身**；401 这个数字来自平台方的批量处置，其理由混合了"规避工具"与"含版权材料"两类主张。
2. 这批仓库中**绝大多数属于 GTA 系列的作弊工具 / mod loader / 服务端**，而非"格式反编译"项目 —— 执法的重心始终在**经济利益受威胁的那一侧**（Zelnick 本人的表述）。
3. 它确立了 2026 年的现实：**任何托管在平台上的 GTA 逆向仓库，都处在这条流水线的作用半径内**。因此本报告推荐的所有做法都以"仓库内零任天堂/ROCKSTAR 文件 + 用户自备输入"为底线。

### 4.4 两种风险谱的分叉（本报告的方法论结论）

```
干净重写（clean room）                    粉丝重制（fan remake）
  Sega v. Accolade (1992)                   Yuzu / Tropic Haze (2025-06-10)
  Sony v. Connectix (2000)                  CryZENx (2026-06-13 自行终止)
  → 抗辩空间来自"不使用原作代码/资产"        → 风险来自"复制品可替代原作"
  → 反编译 + 重写 = 相对安全区               → 无论免费或收费都是高危区
  → 但 Take-Two 用 EULA 把这条也堵了一半     → 任天堂用"官方重制"直接把需求吃掉
```

GTA 侧的启示：**格式逆向（CodeWalker 路线）是相对安全区，行为级重写（re3 路线）是雷区，运行时接管（SHV/FiveM 路线）是被制度化收编区。**  
OoT 侧的启示：**任天堂 2026 年的策略是"自己下场做重制"** —— 官方重制一经公布，粉丝重制就同时失去了法律必要性与市场必要性（CryZENx 在官宣后 4 天内停手，是最干净的行为证据）。

---

## 5. 闭源 ↔ 开源 全量映射

### 5.1 对象级映射总表（★=有本地仓库可直接 `path:line` 印证，○=仅 web 事实，✗=无公开对应物）

#### GTA 侧

| 闭源对象 | 已知/可用面 | 本地镜像（★/○/✗） | 结论 |
|---|---|---|---|
| RAGE 容器 RPF7 | 头结构、4 种加密、AES/NG 双通道 | ★ `CodeWalker/RpfFile.cs` | 格式已完全测绘，密钥仍在 exe |
| PSO / `.ymt` 元数据 | 类型反射、字段名表 | ★ `PsoTypes.cs` 16,524、`MetaNames.cs` 20,688 | 知识量最大的一层 |
| 图形资产 `.ydr/.ydd/.ybn/.ycd/.yft` | 版本化结构体 | ★ `YdrFile.cs:94`、`YftFile.cs:92` | 仅解析，不可重实现 |
| EUP 粒子 / FX | `.fxc` 编译 | ★ `FusionFix .fxc`；✗ 官方 FXWC 闭源 | 半可分析 |
| 脚本 VM `.ysc` | 存在性、字节码形态 | ✗ **无开源反编译器** | 最大空白 |
| RelFile 音频容器 | 137-opcode VM | ★（只读解析） | 格式已测绘 |
| native 哈希表 | 5,182–6,400+ 条 + 参数名 | ★ `ext/natives/*.lua`、`natives_snapshot.lua` | 数据公开，语义靠猜 |
| 运行时接管 | 注入 + hook | ★ `ScriptHookVDotNet/NativeFunc.cs:24`、`ScriptInvocation.cpp:160`；○ SHV 本体 | 生态成熟但依赖闭源 exe |
| FiveM 客户端核心 | 部分 LGPL 开源 | ★ `fivem/LICENSE:1` | **法律上属发行商** |
| alt:V | 客户端闭源 | ○ | 已停摆 |
| GTA VI 一切内部结构 | 零 | ✗（仅地图数据推测 ★`gta6-leonida-atlas`，且 `official:false`） | 无从分析 |

#### OoT 侧

| 闭源对象 | 已知/可用面 | 本地镜像（★/○/✗） | 结论 |
|---|---|---|---|
| EAD N64 引擎 | 全部源码语义 | ★ `oot`（625,253 行 / 16 版本） | **已完全开放为知识** |
| F3DZEX 微码 | GBI 清单 + 运行时反汇编器 | ★ `ucode_disas.c`、`graph.c:54-62` | 独一份的化石 |
|  Yaz0/gzip DMA | 双实现 | ★ `z_std_dma.c:461-465`、`compress.py:21-22` | 完整 |
| GC 内置 N64 模拟器 | DRC + LLE 全貌 | ★ `oot-gc`（`_cpuGCN.c:235`） | 已反编译 |
| Wii VC 模拟器 | 同上 + HBM/VCMV | ★ `oot-vc`（`content1.app`、`vcmv_javascript.cpp:18`） | 已反编译 |
| iQue 移植 | 构建目标 + 差异点 | ★ `Makefile:188-195,:899-905`、`ique-cn/config.yml` | 已进入官方版本轴 |
| Grezzo OoT3D | ARM 骨架、`.zar`、BCSAR | ★ `oot3d`（12,031 arm_func / 7,707 无名）、`OoT3D_Randomizer`（`patch.py:14`、`settings.cpp:3565`） | 语义仅 **1.4%** 写成 C++（3.8.7）；代码量为 N64 版 8%（3.4.3） |
| NSO / Nintendo Classics | 产品事实；**逐游戏补丁脚本机制**（3.8.1，R） | ✗ **无代码级对应物** | 唯一黑箱环节，但已非完全不可知 |
| 2026 Switch 2 重制 | NVN2/SDK 22.2.8/63.2 MB `.text` | ★ `ProjectReLink`、`Hookshot design.md:179-196`、`Ocarina-Atlas`（`0400486024DEC000`） | **发布 13 天即被静态重编译** |
| CryZENx UE 重制 | 仅自述 + 媒体 | ✗ | 已终止，不可审计 |

### 5.2 技术栈对照矩阵

| 维度 | RAGE（GTA V/Enhanced） | OoT N64/GC | OoT3D | 2026 Switch 2 重制 |
|---|---|---|---|---|
| 容器 | RPF7/RSC7，AES+NG，页式压缩 | **无归档**：VROM 区间 DMA 表 | `.zar` + NCCH/romfs + BCSAR | romfs 186,583 文件 / 24.4 GiB |
| 元数据 | PSO + `MetaNames`（2 万行表） | C 结构 + X-macro 表（`actor_table.h` 474 条） | C++ 类 + `*.hpp` 重建 | NVN2/nn:: 类型，经符号提升还原 |
| 脚本/逻辑 | `.ysc` 专有 VM + 6,400 native 哈希表 | 编译进主二进制，函数指针表（`actor_dlftbls @801162A0`） | 编译进 `code.bin`（ARM） | lifted ARM64 → 每函数一个 C 函数 |
| 图形 API | D3D11/D3D12/Vulkan（PC） | RSP F3DZEX + RDP | PICA2（NVN/GSP::Gpu） | **NVN2**（+ 社区侧 SPIR-V 重放） |
| 物理 | Bullet + Euphoria（现属 Take-Two） | 手写 BgCheck 静态查表 | 手写 | 碎罐/刚体（BotW 引擎系） |
| 升频 | PSSR（PS5 Pro）/ DLSS / FSR（PC Enhanced） | 无（320×240） | 无（原生 400×240×2） | **SMAA + FSR1（官方）；DLSS/FSR3 属 PC 移植** |
| 音频 | RelFile VM（137 opcode） | MML 序列播放器 + `aspMain` | QueenSound.bcsar | 未知（NVN2/audio 侧未取证） |
| 逆向起点 | 需从 exe 内取密钥，且加密交付 | 无加密 → 直接反编译 | 需 CFW 提取，签名层被社区绕开 | 需 `.nup` dump，**13 天内完成符号提升** |
| 发行商态度 | 诉讼 + 收编 + 反作弊 | **主动开放 modding API** | 默许随机器 | 首日公布全部 API |

### 5.3 加密门控定律（本报告的核心可迁移结论）

把 5.1 的"结论"列按"是否需要先破解才能读"排序，得到一条规律：

> **一个二进制/资产的"可分析性"，取决于它是否被密钥门控；而非取决于它有多老、多流行、或社区有多大。**

实证四例：

* 1998 年的 MIPS ROM（最老）→ **已 100% 反编译**（因为无加密）。
* 2003 年的 GC 模拟器（同样老）→ **已反编译**（TGC 光盘无加密，调试版甚至带 ELF 符号）。
* 2011 年的 ARM 重制（更新）→ **仅 8%**（NCCH/LayerFS 签名门控，需 CFW）。
* 2026-11-05 的 NVN2 重制（最新）→ **13 天内 295,178 函数被提升**（因任天堂选择用 **明文 `.nup` 镜像提供 dump 入口**并首日公布 modding API）。
* 2026-11-19 的 GTA VI（最新）→ **0**（可执行文件以加密形态交付 + discless + BattlEye + 条款封死）。

这条定律同时解释了 OoT 侧唯一的黑箱 **NSO 模拟器**（Switch 的 exefs/romfs 需密钥，且没有像 ProjectReLink 那样的"官方给 dump"路径），以及 GTA 侧唯一的黑箱 **GTA VI**。

---

## 6. 结论与预测

### 6.1 GTA 侧

1. **今天不存在、也不可能存在合法可分发的 GTA VI 逆向产物**。任何宣称"GTA6 源码已解析/逆向完成"的仓库都是第 1.3 节那类仿冒投毒（`gta6-leonida-atlas` 用 `official: false` 自证其非官方，已属这批仓库里最诚实的一档）。
2. **唯一时间锚点是 PC 版**。参照系：GTA V PC(2015-04) → OpenIV(2016) → CodeWalker(2017) → native 表至今仍在更新；GTA IV(2008) → 大规模格式测绘用了约 5 年；re3 从研究(2016)到发布(2021-02)再到被诉，跨 17 年。
3. 因此对 GTA VI 唯一诚实的路线图是分阶段的：
   `PC 发布 → 容器识别（假定 RPF 后继 + 新密码/布局）→ PSO/元数据测绘 → native 表重建 → 才谈得上行为级重写`
   格式草案 **6–18 个月**，重写 **以年计**，且每一步都在 4.1 的执法阴影下。**主机窗口期不应承诺任何里程碑。**
4. **RAGE 的"版本号"是个伪概念**。官方与媒体只说 RAGE；社区把"增强版"识别为一次**换代的编译目标**（CodeWalker 里体现为 `gen9` 判定，且判据只是"`GTA5_Enhanced.exe` 是否存在"，`RpfFile.cs:231` 还留着 `//any other way to know..?` 这句注释）。`RAGE 9 / eRAGE / EVolution / pyRage / RAGE::script VMT` 等一律应作为**不可信命名**处理。
5. **最大的开源 GTA 项目在法律上属于发行商本身**（`fivem/LICENSE:1` = `© 2023 Take-Two Interactive`）。这决定了 GTA modding 的终局形态不是"社区 vs 公司"，而是**发行商自有的 Creator Platform**。
6. 可继续深耕的空白只有三处：`.ysc` 脚本 VM（无开源反编译器）、RAGE 的编辑器/工具链（无一手出处）、粒子 FX 编译器（仅 `.fxc` 可分析）。

### 6.2 OoT 侧

1. **OoT 是"闭源 → 完全知识化"的唯一样本**：1998 年的引擎在 27 年后已 100% 语义化，且**同一份代码的 16 个官方版本**（含 GC 与 iQue）都在版本轴上。这是任何 GTA 作品都做不到的，原因正是 5.3 的加密门控。
2. **2026 官方重制版的逆向节奏完全不同于 GTA**：发售 13 天内就有 295,178 函数被静态重编译、186,583 个 romfs 文件被清点、NVN2 描述符 bank 被逐条还原 —— 且**没有一封律师函**，因为任天堂首日就公布了 modding API。**预测：2027 年 OoT 的主要工程瓶颈不再是"能否解析"，而是 NVN2 图形语义的保真度**（`rt64_gbi.cpp` 式的 `// Needs confirmation.` 会长期存在）。
3. **NSO/Nintendo Classics 是最后一个未开化的官方环节**，也是最不可能被开化的一个（无合法 dump 入口 + Yuzu 判例把"可访问任天堂未来主机"的模拟器定性封死）。
4. **维护性反转值得警惕**：`Ocarina-Atlas` 这类纯追踪器（零任天堂文件、`title id` 常量化）可长期存续；而**直接依赖 GPL-3.0 patch 流的 `TriAevum`/`Ocarina-ReTuned` 系**（PICA→SPIR-V via NVIDIA NRI、真 AOT）法律与托管风险显著更高。`oot-pc-recomp` 的实际目标已迁移到《Donkey Kong Bananza》，说明这类项目的存活取决于**是否绑定某一款游戏**。
5. 三个已在中文圈广泛流传的**伪事实**应停止引用：Z-targeting 专利是 US 6,443,842（实为 **US 6,165,073**）；iQue 版加入体感拔剑（**无任何证据**，宫本茂原话反而否定）；OoT 使用 MM 式 `ZARCLIB` 归档 / `Z64FILETYPE_*` / `segments.h` 的 `SM_*` 宏（**当前 HEAD 全部不存在**）。

### 6.3 方法论声明

本报告全程只读：未编译、未运行任何第三方仓库代码；未获取、未分析、也未复现任何未发售标题（GTA VI）的二进制、镜像、DRM 绕过或在线模式绕过内容；对外挂/作弊工具仅描述架构，不提供实现；所有闭源结论只建立在**官方物料、业界报道、专利正文、以及社区已公开发表的逆向成果**四类来源之上。凡本报告未能独立验证者，一律标注 `?` 或"未找到"，不作为事实陈述。

---

## 附录 A：证据文件索引（本报告实际引用的本地文件）

### GTA

* `ext/CodeWalker/CodeWalker.Core/Game/Utils/RpfFile.cs:231,2147-2153`（加密枚举与 AES 密钥搜索）
* `ext/CodeWalker/CodeWalker.Core/Game/Utils/AES.cs`、`Game/Resources/YMT/YmtUtils.cs`
* `ext/CodeWalker/CodeWalker.Core/Game/Resources/Ydr/YdrFile.cs:94-97`、`Yft/YftFile.cs:92-95`、`Ymt/PsoTypes.cs`(16,524)、`Utils/MetaNames.cs`(20,688)
* `ext/ScriptHookVDotNet/source/Scripting/NativeFunc.cs:24`、`ScriptInvocation.cs`
* `ext/fivem/LICENSE:1`、`code/client/shared/…`、`code/components/citizen-*-scripting/…/ScriptInvoker.cpp:160`
* `ext/natives/natives_stash/gta_universal.lua`(76,015)、`natives_snapshot.lua`
* `ext/re3/…`（镜像语境）、`ext/gta-reversed/gtasa-reversed/…`、`extra-repos/gta/FusionFix/…`、`extra-repos/gta/open.mp`、`extra-repos/gta/YimMenuV2`
* `reports/_parts/10_gta6_closed.md`、`ext/gta6-leonida-atlas/…`（`official:false`）

### OoT

* `oot/Makefile:42,188-195,632,899-914,1103`、`oot/README.md:32-46,49,153-154,192`、`oot/docs/compilers.md:3-4`
* `oot/include/versions.h:5-20`、`ultra64/{gbi.h:28-1863,mbi.h:38-40,R4300.h:14,convert.h:12,version.h}`、`segmented_address.h:7-9`、`dma.h:19-25`、`romfile.h:7-13`、`scene.h:228-255,386-389,403-423,497-511,556`、`z_game_dlftbls.h:7-34`、`z_actor_dlftbls.h:67-68`、`camera.h:314-343,1651`、`n64dd.h`、`tables/{actor_table.h,object_table.h:10-13,scene_table.h,sequence_table.h,dmadata_table.h:6-8}`
* `oot/src/code/{ucode_disas.c:94-207,332,362-940,graph.c:54-62,86,131-143, z_std_dma.c:445-465, z_scene.c:38,247,587, z_DLF.c:9,72, z_bgcheck.c:19-39,196-205,573-904, z_camera.c:8038, z_play.c, z_sram.c:66-77,663-735, z_kanfont.c, db_camera.c:8207}`
* `oot/src/boot/{yaz0.c:11-33, inflate.c:285-304,997, cic6105.c, makerom/rom_header.s}`
* `oot/src/overlays/gamestates/ovl_file_choose/z_file_choose.c:745`、`src/overlays/n64dd/`
* `oot/src/audio/{game/*, internal/seqplayer.c:25,171-325,1358-1653, tables/*}`
* `oot/tools/compress.py:21-22,254-256`
* `oot/baseroms/{gc-eu-mq-dbg,ique-cn}/config.yml:3-28`
* `oot-gc/README.md:23,27-39,85`、`oot-gc/config/ce-j/config.yml:4-5`、`oot-gc/src/emulator/{_cpuGCN.c:235,6080,6229,9925-9987,10014,10294, rsp.c:173-275,787-1021, _gspF3DEX.c:66-153, _gspS2DEX.c, _gspJPEG.c, rdp.c:30,511, soundGCN.c:202-376, _aspMain.c, audio.c:48-50, THPPlayer.c, THPVideoDecode.c, simGCN.c, xlCoreGCN.c:109,135}`
* `oot-vc/README.md:17-31`、`oot-vc/config/oot-j/config.yml:3-4`、`oot-vc/src/emulator/{cpu.c:1769,7628,7786,10759,10848,11134, vc64_RVL.c:41-49,127,146, controller.c:9,12,152, banner.c, pak.c, storeRVL.c:10,126,136, helpRVL.c, soundRVL.c, xlCoreRVL.c:158,186, codeRVL.c, xlFileRVL.c}`、`oot-vc/src/revolution/{hbm/vcmv/vcmv_javascript.cpp:18,54, code_800838C0.cpp:125,444, ax/, axfx/, dsp/, fs/, nand/, vi/, pad/, wenc/, ipc/}`
* `oot3d/include/non_matchings.hpp:3,5,6`（`GLOBAL_ASM(x)` 为空、`UNK_RETURN/UNK_TYPE=int`）、`oot3d/disassembly/CMakeLists.txt:47,54,58,59,61`、`oot3d/disassembly/tools/{n3dsdisasm,baserom_fixes.py,exefs2elf.py,gcc_to_arm_asm.py,split_arm_data_chunks.py,e2elf.ld}`、`oot3d/src/functions/functions_*.cpp`(31 文件 9,008 条 stub)、`oot3d/src/game/*.cpp`(450 文件，3,184 条 stub / 171 无名)、`OoT3D_Randomizer/source/music.cpp:376,394-395,529-534`、`GLideN64/src/Config.h:265,269,274,283`、`oot3d/README.md:5,10-13,25,27-31`、`oot3d/CMakeLists.txt:6-18`、`oot3d/cmake/ARMCC.cmake:14-20`、`oot3d/src/{crt0.cpp:1,25, data/conststring.cpp:505-506, game/zar.cpp, functions/functions_*.cpp(31), game/actors|classes|z_*.cpp(450)}`、`oot3d/disassembly/oot3d.cfg:2,4,…(12,031 条)`、`oot3d/data/{checksum.md5, chunks/, data_chunks.txt, rodata_chunks.txt, SHT__INIT_ARRAY.txt}`、`oot3d/tools/{ctrtool, exefs2elf.py, n3dsdisasm, gcc_to_arm_asm.py, split_arm_data_chunks.py}`
* `OoT3D_Randomizer/{code/patch.py:14,16-28, code/src/asm/{hooks.s:4-16, patches.s, svc.s, csvc.s, svc_loader.s}, source/{patch.cpp:88-99,112-118,642-644,660-661, music.cpp:376,395, settings.cpp:3565-3569, main.cpp:57,59}, ootrando.rsf}`
* `ProjectReLink/README.md:5-6,24-29,31-33,61-62,88-90,165,283,319-321,347-352,373`、`ProjectReLink/{nx2/lift.py, nx2/build.py, host/nvn_native.c, gpu/vk/vk_draw.cpp:5618}`
* `Hookshot/docs/design.md:37-40,56-73,80-85,179-196`、`Hookshot/gpu/vk/{vk_bank.cpp:53,440, eden/…/translate_program.cpp:28-31,144-145}`、`Hookshot/readme.md:1034`
* `oot-pc-recomp/{AGENTS.md:3, docs/oot-status.md:3,7-12,16-18,32-35, oot111/…}`、`Zelda64Recomp/…`
* `Ocarina-Atlas/src/lib/{lichii-server.ts:14,1567-1569, i18n.ts:340}`、`Ocarina-Atlas/README.md:41-47`
* `ship/…`（libultraship `Ship::`、`src/fast/interpreter.cpp` 5,546 行）、`rt64/src/gbi/rt64_gbi.cpp:64-94`、`gliden64/src/GBI.h:9-40`
* `reports/02_oot_decomp_analysis.md`（体例参照）

## 附录 B：本报告明确"未找到"的条目（禁止当作事实引用）

**GTA**：RAGE 的任何官方版本号；RAGE Technology Group 的 GDC 演讲；`.ysc` 反编译器；RAGE 编辑器/构建系统/工具链名称（含 2026-08 泄露材料）；"RSC8"（当前 CodeWalker 快照无此符号）；CodeWalker 中的 `YscFile.cs`、`GameVersion` 枚举；GTA VI 的容器格式/脚本 VM/native 表结构；Take-Two 的世界流式渲染或 PSO 序列化专利；FRGTAIV 所称"2026 年三项专利"；"RAGE::script VMT"/"pyRage"/"Minigame"作为官方组件名；iQue 式"官方源码交付"在 GTA 侧的对应物。

**OoT**：`ZARCLIB`/`File_ARC`/`ACMB`/`ZSD`、`src/code/z_file.c`、`Z64FILETYPE_*`、`include/z64*.h`、`include/segments.h` 与 `SM_*` 宏、`gZTargetingEnabled`/`TargetingModeSwitcher`（Z-targeting 的可读符号）；OoT 侧的 RefPack/Yay0 压缩；N64 版 `gspF3DZEX2_NoN_PosLight_fifo`；`G_MDSPEC`/`G_ALLMODE`/`G_SUBMITMODE`/`AA_MODE 0-3`；Wii VC 的 `xlCoreInitFilter`；VCMV 的完整规格；OoT 的 ROM 体积；EAD 团队人数与"RealityStation/Indigo2"工作机；libultra 零售版版本字母；GC 版的 Dolby Pro Logic II / 16:9 变形宽屏 / SPD 参与；Wii VC 的 PAL50 强制与震动问题的官方说明；iQue 的"16 MB Flash"口径与 ¥598 定价；iQue 的 N-Stage/PolyPipe 名称与动作感应；iQue SDK 泄露/开源事件；iQue 专属维权案；Grezzo 的创始人"中原巧"/"Meridian9"/"Green Technology"；OoT3D 的 8 MB FCRAM、`.cfa/.csu/.csar`、双人共斗迷宫、剧场/拍照模式、"迷宫重设计由 Grezzo 提案"的一手引言；OoT3D_Randomizer 的 `model_info.bcsv`、NCCH 解包/验签/SHA 白名单；NSO 的模拟器开发方署名、JIT 实现细节、任天堂×Analemma/GGPO 合作、ROM 补丁策略（**官方口径仍无；3.8.1 的 2022-01/02 datamine 只是 R 级旁证，不是声明**）、"2022 下架"、"体感 1:1 还原"官方说法；**NSO 源自/分叉 mupen64plus、NSO 是"硬件级 ARM 重编译器"、NSO 版《时之笛》"由 Ghidra 重建源码"（三条均为社区讹传，见 3.8.3）**；**OoT3D 使用 `oarc`/`bma`/`bmt`/`zakr`/`zeffect`/`bcsv`/`szss` 标签（`OoT3D_Randomizer` 全仓 0 命中，见 3.8.2）**；**N64 卡带 DMA 的"256 帧上限"**、**iQue bootcode 符号 `zsel_da`**、**iQue 卡带容量"562 Megabit"**（三者均无一手来源，常被当成技术事实转述）；**iQue SDK / EGCS 1.1.2 的发布日期与授权条款**；**2026 Switch 2 重制版内含 Master Quest 数据的 datamine（MrCheeze / OatmealDome，仅经 IGN / 17173 转述，未进 NSO 与 3DS 版）**；2026 重制的卡带容量/下载体积/官方程序 ID、Switch 1 版计划、DLSS/FSR3/帧生成（均为 PC 移植端能力）；"Koos v. Nintendo"；CryZENx 的一手文本（站点已失效）、2016 年任天堂发函、2026 年确切停手时间、Nanite/MotionBuilder/Mixamo 管线、自制 mission 系统、主机多平台计划、"不含任何任天堂资产"的原文表述、开发者年龄/国籍；高清贴图包的 "GenD/Viper" 署名；Zelda 64: Definitive / OoT Redux；UE5 做 N64 重制的通用公开工具；Hyrule Warriors 的引擎名；Skyline 的准确关停年份。

## 附录 C：本轮取证失败/不可达的外部来源

| 来源 | 状态 | 影响 |
|---|---|---|
| `rage.re`（RAGE 引擎 wiki） | **DNS 解析失败** | `rage::` 命名空间词汇表仅有二手引用 → 标 U |
| Rockstar Newswire | JS 渲染 SPA，正文不可静态抓取 | GTA VI 官方陈述改以 Reuters/BBC/Digital Foundry/财报为代理 |
| `iwataasks.nintendo.com` | **TLS 校验失败** | Grezzo 相关一手引言缺失 → 3.4.5 标 U/?；OoT 侧 Iwata Asks 内容取自 Wayback 快照 |
| `patents.justia.com` | 403（部分页可读） | 专利正文靠可读页面 + 交叉印证 |
| Google Patents | 直连 503；约 4 次查询后 XHR 限流 | 到期日只能推算 → 标 U |
| Wired 2007 RAGE 专稿 | 404 | RAGE Technology Group 的早期叙述以 rage.re/维基为代理 |
| `bulletphysics.org` 旧版页面 | 已失效 | Bullet/Euphoria 集成叙述降级为 R |
| `cryzenx.com` | 现为 MagicAI SaaS 模板页；实时 HTTP 526 | **3.6 节全部作者自述降级为 R/?，多处标"未找到"** |
| `OatmealDome` / `LuigiBlood` / `MrCheeze` 的原始推文与 datamine 笔记 | 账号内容本轮不可达，仅有中文/英文二手转述，且转述文本存在字符污染 | **3.8.1 的机制描述（每游戏补丁 Lua 脚本、v2.0.0 渲染补丁）与 3.8.6 的 MQ 数据全部停在 R/U，无法升 F** |
| `iwataasks` 之外的一手技术史料（SPACE WORLD '97 现场资料、Famicom 杂志 1997-11 原文） | 仅存开发者回忆与二手转述 | 3.8.4 只能记为 R，且明确禁止写成"引擎压力测试" |
