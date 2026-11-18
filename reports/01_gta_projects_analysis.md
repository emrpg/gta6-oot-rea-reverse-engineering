# GTA 系列核心逆向 / 开源引擎项目 深度分析报告（含 GTA VI 闭源侧）

> **生成时间** 2026-11-18 ｜ **分析对象** 9 个本地浅克隆仓库（`/root/orca-workspaces/gta-repos/`，`/root/wsl-demo/gta-repos/` 同步软链）+ 闭源侧公开证据
> **一手依据** 全部结论出自本地源码逐文件阅读与可复现的 `grep`/`find` 度量；闭源与已下架项目单列并标注核验状态；事实（F）/传闻（R）/未知（U）/无公开依据（?）在表内逐条标记
> **规模** 本地仓库工作区合计约 1.2 GB（含 `.git` 1.7 GB），最大单仓 `citizenfx/fivem` 1,275,212 行一手代码

**这段是给只想看结论的人的 12 句话：**

1. 现存 GTA 逆向生态里，**没有一个项目"完整"**：OpenRW 停在 57% 的 opcode 覆盖、MTA 的网络层根本不在源码树里、FiveM 的关键组件靠 CI 外挂私有 overlay、CodeWalker 连 LICENSE 都没有。
2. RenderWare 时代（GTA III–SA）可重写，RAGE 时代（IV/V）**只剩接管与格式解析**——这条分水岭在代码里是可见的：MTA 用 481 个硬地址，FiveM 用 3,229 处签名扫描。
3. 逆向的"证据形态"至少有六种：格式解析、偏移表、特征码扫描、符号数据库、字节码层重建、库级 API 镜像。本报告给它们各找到了一个代表仓库并逐条量化。
4. librw 与 OpenRW 构成一组罕见的对照：**语义同构、命名重构**（librw，`Rp*` 命中 0 次）对 **格式级镜像**（OpenRW，把 RW 当资产流规格），体量差 4.6 倍。
5. 最著名的 re3/reVC 至今是 GitHub API 上的 **HTTP 451 / `block.reason = dmca`**（2021-09-30 生效）；诉讼 2023-04 和解、条款未公开，因此**没有留下任何 fair-use 先例**。本报告只做文献级分析，未取用任何镜像源码。
6. Take-Two 2017-06 对 OpenIV 的 C&D、2021-07 的模组下架潮、2024 年 GTA Online 部署 BattlEye，与 **2026-09-22 版 Rockstar Mod Guidelines**（禁改剧情、禁跨作素材、禁"让游戏跑在官方未发布的平台/硬件上"）共同构成当前法律边界。
7. RAGE 引擎的公开知识里**已被代码证实**的部分集中在容器与资产格式：RPF7/RSC（NG + AES 双密码体制）、`.ytd/.ydr/.ydd/.ybn/.ymap/.ytyp/.ycd/.ymt/.ynd`、PSO、`.ysc` 字节码 VM、64-bit JOAAT native 哈希（GTA V 约 6,400+）。
8. 所谓"RAGE 9 / eRAGE / EVolution"等版本号**全部是媒体与 AI 农场产物**，Rockstar 从未公布过 RAGE 版本编号。
9. GTA VI（发售前 1 天）官方事实：2026-11-19 发售、仅 PS5/PS5 Pro/Xbox Series X|S、无 PC、$79.99、双主机 30 FPS、PS5 Pro 独占 RT 阴影 + PSSR、约 212 GB 安装、Leonida/Vice City、无微交易、无生成式 AI。
10. **GTA VI 逆向在技术上不存在用户侧入口**：无 PC 版即无用户可得的可执行文件；Mod Guidelines 明令禁止跨平台；主机侧还需绕过签名与 BattlEye。本报告因此把"现在能否做 GTA VI 逆向"判为**否**，并给出条件与替代路径。
11. 若真要立项，最短路径不是重写引擎，而是：格式与哈希层（CodeWalker 那一层，6–12 个月可自用）→ 离线编辑器 → 数据/资产层接管 → 运行时接管（十年量级）。
12. 生态里最大的"开源"项目（FiveM，127 万行）版权归 Take-Two，白名单之外的 `rage-*`/`gta-*` 逆向核心**不得自由复用**——GTA 逆向的工业级成果已经被发行商内部化，这是全报告最重要的结构性结论。

---

## 目录

- [1. 研究范围、方法与数据边界](#1-研究范围方法与数据边界)
  - [1.1 这份报告回答什么问题](#11-这份报告回答什么问题)
  - [1.2 数据来源与可复现性](#12-数据来源与可复现性)
  - [1.3 一条必须写在前面的安全警告：这个生态的"仿冒仓库投毒"](#13-一条必须写在前面的安全警告这个生态的仿冒仓库投毒)
  - [1.4 本研究遵守的边界](#14-本研究遵守的边界)
  - [1.5 规模速览（详见 6.2）](#15-规模速览详见-62)
- [2. 全景台账：GTA 逆向/开源引擎生态（39 项，逐条标注核验状态）](#2-全景台账gta-逆向开源引擎生态39-项逐条标注核验状态)
  - [2.1 引擎级重写与反编译（路线 A/B）](#21-引擎级重写与反编译路线-ab)
  - [2.2 RAGE 时代的运行时接管与脚本层（路线 C/E）](#22-rage-时代的运行时接管与脚本层路线-ce)
  - [2.3 资产/格式逆向与工具链（路线 D）](#23-资产格式逆向与工具链路线-d)
  - [2.4 明确排除项（F：取证判定为仿冒/投毒，禁止下载）](#24-明确排除项f取证判定为仿冒投毒禁止下载)
  - [2.5 台账的两条硬结论](#25-台账的两条硬结论)
- [3.1 OpenRW（rwengine/openrw）](#31-openrwrwengineopenrw)
  - [项目定位](#项目定位)
  - [元数据表](#元数据表)
  - [目录与模块清单（含行数统计）](#目录与模块清单含行数统计)
  - [构建系统与依赖图](#构建系统与依赖图)
  - [逆向机制核心：RenderWare 类布局镜像（带代码证据）](#逆向机制核心renderware-类布局镜像带代码证据)
  - [game 层与 engine 层分工](#game-层与-engine-层分工)
  - [关键子系统逐个拆解](#关键子系统逐个拆解)
  - [三作共用代码与 profile 机制](#三作共用代码与-profile-机制)
  - [测试与工程质量](#测试与工程质量)
  - [成熟度与缺口表](#成熟度与缺口表)
  - [资产依赖](#资产依赖)
  - [逆向来源标记](#逆向来源标记)
  - [对本报告其他项目的启示](#对本报告其他项目的启示)
- [3.2 re3 / reVC（GTAmodding/re3）—— 被 DMCA 屏蔽的"最著名重写"](#32-re3--revcgtamoddingre3-被-dmca-屏蔽的最著名重写)
  - [3.2.1 本章的性质声明（重要）](#321-本章的性质声明重要)
  - [3.2.2 实测确认的技术事实（2026-11-18 本机执行）](#322-实测确认的技术事实2026-11-18-本机执行)
  - [3.2.3 为什么"取一份镜像"是不对的选择](#323-为什么取一份镜像是不对的选择)
  - [3.2.4 本节最有实操价值的发现：仿冒镜像的指纹差别](#324-本节最有实操价值的发现仿冒镜像的指纹差别)
  - [3.2.5 OpenRPA：本项目原计划在列，但**当前不存在可核验的官方源**](#325-openrpa本项目原计划在列但当前不存在可核验的官方源)
  - [3.2.6 一条与本节相关的安全观测（附我的复核结论）](#326-一条与本节相关的安全观测附我的复核结论)
- [3.3 ScriptHookVDotNet（crosire/scripthookvdotnet）](#33-scripthookvdotnetcrosirescripthookvdotnet)
  - [项目定位](#项目定位-1)
  - [元数据表](#元数据表-1)
  - [目录清单与规模](#目录清单与规模)
  - [原生侧与托管侧架构](#原生侧与托管侧架构)
  - [native hash ABI 调用链详解](#native-hash-abi-调用链详解)
  - [RAGE 内部布局泄露点](#rage-内部布局泄露点)
  - [脚本域与生命周期](#脚本域与生命周期)
  - [工程质量](#工程质量)
  - [开源 ↔ 闭源边界](#开源--闭源边界)
  - [RE 来源标记](#re-来源标记)
  - [对逆向 GTA VI 的启示](#对逆向-gta-vi-的启示)
- [3.4 CodeWalker（dexyfex/CodeWalker）](#34-codewalkerdexyfexcodewalker)
  - [项目定位](#项目定位-2)
  - [仓库元数据](#仓库元数据)
  - [解决方案与模块清单（行数）](#解决方案与模块清单行数)
  - [RAGE 资源格式全景](#rage-资源格式全景)
  - [RpfArchive 深剖](#rpfarchive-深剖)
  - [分块标签与哈希机制](#分块标签与哈希机制)
  - [MetaBase 反射式类布局镜像](#metabase-反射式类布局镜像)
  - [序列化往返保真度](#序列化往返保真度)
  - [运行时与离线分界](#运行时与离线分界)
  - [逆向来源标记](#逆向来源标记-1)
  - [成熟度与缺口](#成熟度与缺口)
  - [对 GTA VI 逆向的方法论启示](#对-gta-vi-逆向的方法论启示)
- [3.5 Multi Theft Auto（multitheftauto/mtasa-blue）](#35-multi-theft-automultitheftautomtasa-blue)
  - [项目定位](#项目定位-3)
  - [元数据与授权真相](#元数据与授权真相)
  - [模块清单与规模](#模块清单与规模)
  - [构建系统与依赖](#构建系统与依赖)
  - [注入与 hook 基础设施](#注入与-hook-基础设施)
  - [逆向类布局镜像证据](#逆向类布局镜像证据)
  - [网络与元素复制模型](#网络与元素复制模型)
  - [Lua 绑定层](#lua-绑定层)
  - [反作弊与兼容性](#反作弊与兼容性)
  - [工程质量](#工程质量-1)
  - [GTA 特有的 RE 技术含量评估](#gta-特有的-re-技术含量评估)
  - [对 GTA VI 的启示](#对-gta-vi-的启示)
- [3.6 CitizenFX / FiveM（citizenfx/fivem）](#36-citizenfx--fivemcitizenfxfivem)
  - [项目定位](#项目定位-4)
  - [授权真相与多许可分拆](#授权真相与多许可分拆)
  - [顶层结构与构建编排](#顶层结构与构建编排)
  - [组件框架架构](#组件框架架构)
  - [natives 数据库与绑定生成](#natives-数据库与绑定生成)
  - [rage 脚本 VM 调用 ABI](#rage-脚本-vm-调用-abi)
  - [netsync 复制模型与 RAGE 内部结构泄露](#netsync-复制模型与-rage-内部结构泄露)
  - [语言运行时层](#语言运行时层)
  - [开源边界表](#开源边界表)
  - [vendor 与私有件](#vendor-与私有件)
  - [工程质量](#工程质量-2)
  - [逆向技术含量评级](#逆向技术含量评级)
  - [对 GTA VI 的启示](#对-gta-vi-的启示-1)
- [3.7 librw（aap/librw）—— RenderWare 本体的重实现](#37-librwaaplibrw-renderware-本体的重实现)
  - [3.7.1 目录与模块清单](#371-目录与模块清单)
  - [3.7.2 构建系统、标准与依赖](#372-构建系统标准与依赖)
  - [3.7.3 核心问题：librw 有多像真 RW？](#373-核心问题librw-有多像真-rw)
  - [3.7.4 平台后端三分割](#374-平台后端三分割)
  - [3.7.5 PS2 与「完全用自由工具链构建」](#375-ps2-与完全用自由工具链构建)
  - [3.7.6 完成度与子库成熟度](#376-完成度与子库成熟度)
  - [3.7.7 librw 与 OpenRW 的对照](#377-librw-与-openrw-的对照)
  - [3.7.8 溯源标记汇总](#378-溯源标记汇总)
  - [3.7.9 结论](#379-结论)
- [3.8 GTAIV.EFLC.FusionFix（ThirteenAG）—— RAGE 时代的 PC 移植修复](#38-gtaiveflcfusionfixthirteenag-rage-时代的-pc-移植修复)
  - [项目定位](#项目定位-5)
  - [元数据与授权真相](#元数据与授权真相-1)
  - [源码 vs 二进制：一张诚实的表](#源码-vs-二进制一张诚实的表)
  - [语言、标准与构建系统](#语言标准与构建系统)
  - [RE 产物一：注入机制](#re-产物一注入机制)
  - [RE 产物二：Hook API 与数量](#re-产物二hook-api-与数量)
  - [RE 产物三：签名机制的可迁移性](#re-产物三签名机制的可迁移性)
  - [版本处理：没有版本表](#版本处理没有版本表)
  - [修复内容的三个深入样本](#修复内容的三个深入样本)
  - [`Update postfx.ixx` 到底改了什么](#update-postfxixx-到底改了什么)
  - [RE 产物四：一张跨代的 key 哈希家族表 + 字典反查](#re-产物四一张跨代的-key-哈希家族表--字典反查)
  - [三方方法论对比](#三方方法论对比)
  - [逆向/工程来源标记](#逆向工程来源标记)
  - [评估](#评估)
- [3.9 citizenfx/natives + txAdmin —— 逆向知识的数据库化](#39-citizenfxnatives--txadmin--逆向知识的数据库化)
  - [3.9.1 布局：Markdown 文件即数据库行](#391-布局markdown-文件即数据库行)
  - [3.9.2 数据周围的工具：三段流水线，校验器在仓库之外](#392-数据周围的工具三段流水线校验器在仓库之外)
  - [3.9.3 HEAD 提交：一次 joaat 混用的数据完整性修复](#393-head-提交一次-joaat-混用的数据完整性修复)
  - [3.9.4 txAdmin：FiveM 的运维层（次要）](#394-txadminfivem-的运维层次要)
  - [3.9.5 数据库仓库 ↔ 引擎内消费](#395-数据库仓库--引擎内消费)
  - [3.9.6 这一层证明了什么](#396-这一层证明了什么)
- [4. GTA VI：为什么"现在不存在合法的 GTA VI 逆向项目"](#4-gta-vi为什么现在不存在合法的-gta-vi-逆向项目)
  - [4.1 官方事实（截至 2026-11-18，逐条带日期）](#41-官方事实截至-2026-11-18逐条带日期)
  - [4.2 结论：三个硬性阻断](#42-结论三个硬性阻断)
  - [4.3 关于 Take-Two 的执法史（决定所有 GTA 逆向项目的天花板）](#43-关于-take-two-的执法史决定所有-gta-逆向项目的天花板)
- [5. 闭源侧：RAGE 公开知识图谱与二进制接管生态](#5-闭源侧rage-公开知识图谱与二进制接管生态)
  - [5.1 RAGE 引擎：可从公开材料确认的部分](#51-rage-引擎可从公开材料确认的部分)
  - [5.2 GTA VI 技术面：只有帧分析，没有结构知识](#52-gta-vi-技术面只有帧分析没有结构知识)
  - [5.3 闭源工具生态开放度台账](#53-闭源工具生态开放度台账)
  - [5.4 对"要不要等 GTA VI 能做逆向"的判定](#54-对要不要等-gta-vi-能做逆向的判定)
- [6. 横向对比：五种逆向路线的技术指纹](#6-横向对比五种逆向路线的技术指纹)
  - [6.1 分类框架](#61-分类框架)
  - [6.2 可量化指标（全部本地实测，同一口径）](#62-可量化指标全部本地实测同一口径)
  - [6.3 从指标读出的六种逆向"证据形态"](#63-从指标读出的六种逆向证据形态)
  - [6.4 完成度、失败模式与共同缺口](#64-完成度失败模式与共同缺口)
- [7. 时间线、方法论迁移与"如果真要做 GTA VI"](#7-时间线方法论迁移与如果真要做-gta-vi)
  - [7.1 历史时差基准（这是唯一能预测未来的数据）](#71-历史时差基准这是唯一能预测未来的数据)
  - [7.2 分阶段路线（严格限定：以"未来存在 PC 版 + 只用自有资产"为前提）](#72-分阶段路线严格限定以未来存在-pc-版--只用自有资产为前提)
  - [7.3 六个项目各自最值得迁移的一件事](#73-六个项目各自最值得迁移的一件事)
  - [7.4 结论（五句）](#74-结论五句)
- [8. 来源清单](#8-来源清单)
  - [8.1 一手来源（本地克隆源码，2026-11-18 固定 commit）](#81-一手来源本地克隆源码2026-11-18-固定-commit)
  - [8.2 法律与执法史](#82-法律与执法史)
  - [8.3 GTA VI / RAGE 技术面](#83-gta-vi--rage-技术面)
  - [8.4 项目官方站点](#84-项目官方站点)
  - [8.5 本报告写作过程中的工具与通道说明](#85-本报告写作过程中的工具与通道说明)

---

## 1. 研究范围、方法与数据边界

### 1.1 这份报告回答什么问题

用户的问题表面上是"把 GTA 系列的逆向/开源引擎项目抓下来分析一遍"，但真正有价值的答案必须同时覆盖三条线：

1. **开源线** —— 那些真正把 Rockstar 引擎"重写/反编译"出来的项目（librw、OpenRW，以及被下架的 re3/reVC），以及围绕 RAGE 引擎做格式逆向与运行时接管的项目（CodeWalker、ScriptHookVDotNet、MTA、FiveM、FusionFix、citizenfx/natives）。它们的源码是**一手证据**：能从代码里读出逆向工程做到了什么程度，而不是靠 README 的自我描述。
2. **闭源线** —— RAGE 引擎本体、GTA VI、以及 OpenIV / ScriptHookV / RAGE Plugin Hook / alt:V / RageMP 这类"二进制接管"生态。这条线决定了开源线**能合法走多远**。
3. **GTA VI 线** —— 截至写作时（2026-11-18），GTA VI 尚未发售（定档 2026-11-19，仅 PS5 / Xbox Series X|S）。因此"GTA VI 逆向"在当下不是一个已存在的技术事实，而是一个**路线图问题**：本报告把它当成"用现有开源项目的经验，推断新标题的逆向时间线与切入点"来处理，并严格区分事实、传闻与未知。

### 1.2 数据来源与可复现性

本报告的全部技术论断优先来自**本地克隆的源码**，其次才是公开资料。每个结论都标注 `文件路径:行号` 或公开来源 URL。本地仓库固定在以下 commit（`--depth 1` 浅克隆），复现命令：

```bash
mkdir -p /root/orca-workspaces/gta-repos && cd /root/orca-workspaces/gta-repos
export GIT_TERMINAL_PROMPT=0 GIT_LFS_SKIP_SMUDGE=1
git clone --depth 1 --single-branch https://github.com/rwengine/openrw                     openrw
git clone --depth 1 --single-branch https://github.com/aap/librw                            librw
git clone --depth 1 --single-branch https://github.com/scripthookvdotnet/scripthookvdotnet   scripthookvdotnet
git clone --depth 1 --single-branch https://github.com/dexyfex/CodeWalker                    CodeWalker
git clone --depth 1 --single-branch https://github.com/multitheftauto/mtasa-blue              mtasa-blue
git clone --depth 1 --single-branch https://github.com/citizenfx/fivem                        citizenfx-fivem
git clone --depth 1 --single-branch https://github.com/ThirteenAG/GTAIV.EFLC.FusionFix       GTAIV.EFLC.FusionFix
git clone --depth 1 --single-branch https://github.com/citizenfx/natives                     citizenfx-natives
git clone --depth 1 --single-branch https://github.com/citizenfx/txAdmin                     txAdmin
for d in /root/orca-workspaces/gta-repos/*/; do ln -sfn "$d" "/root/wsl-demo/gta-repos/$(basename $d)"; done
```

被 DMCA 屏蔽而未克隆的规范仓库只有一个：`GTAmodding/re3`（reVC 为其 `miami` 分支）—— 原因与取证见 3.2。

本地 HEAD 快照（`git log -1`）：

| 仓库 | HEAD | 提交日期 | 提交标题 |
|---|---|---|---|
| openrw | `5c5f266b` | 2025-06-18 | REAMDE.md: fix CI badges |
| librw | `18532c2` | 2026-08-26 | build the ps2 platform entirely with free… |
| scripthookvdotnet | `28b4dfde` | 2026-11-15 | Merge PR #1833 from nomakewan/decoratorfix |
| CodeWalker | `485d56be` | 2025-04-11 | ModManager progress |
| mtasa-blue | `70b0b402` | 2026-11-17 | Fix various crash handler failures (…OOM) (#5442) |
| citizenfx-fivem | `8cbec36a` | 2026-11-17 | chore(scrt-js): NodeJS update (again) |
| GTAIV.EFLC.FusionFix | `07c1e8d` | 2026-11-17 | Update postfx.ixx |
| citizenfx-natives | `c00e0ed` | 2026-10-27 | Fix joaat mixup for RemoveCurrentRise (#7) |
| txAdmin | `1db5eb0` | 2026-10-23 | fix(core/webserver): reject compressed re… |

克隆总量：`/root/orca-workspaces/gta-repos` ≈ **1.7 GB**（含 `.git`），工作区约 1.2 GB。

### 1.3 一条必须写在前面的安全警告：这个生态的"仿冒仓库投毒"

本次检索过程中，GitHub 搜索接口返回的"高星 re3 / reVC / GTA2 逆向 / OpenRPA"结果**几乎全是仿冒或占位仓库**，典型特征：

- 名字极其正经（`hottabxp/re3`、`leanndroguedes/re3-19feb21`、`mrxenginner/reVC`、`Carter54git/revcdos`、`btshahraag20/gta2-memory-master`、`knight-oss/gta2-reverse-engineering-lab`），描述却是一行空洞文本（"GTA III, Vice City"）、语言字段与内容不符（C++ 逆向项目被标成 HTML）、标题塞年份关键词做 SEO（"…2026: Open Source Reverse Engineering and Modding Guide"）。
- 直接取证（3.2.4）：仿冒 re3 仓库照抄真 README，但把 Discord 邀请码从 `ERYg58ttcE` 换成 `RFNbjsUMGg`；`knight-oss`/`btshahraag20` 两个"GTA2 逆向实验室"仓库里**只有 README + index.html + 两三个 svg**，下载徽章指向 GitHub Pages 上一个单一 base64 blob，提交时间就是本报告写作当天。

因此本报告的选品原则是：**只克隆能被官方站点、官方 org 或项目自有文档交叉印证的仓库**，宁可少一个项目也不碰仿冒仓库。凡标注"未能验证官方源"的项目（例如 OpenRPA，见 3.2.5）一律不下载、不编译、不运行。

### 1.4 本研究遵守的边界

- 全程**只读**：克隆、检索、统计、阅读源码；不编译、不链接、不执行任何来自这些仓库的代码或产物（包括 `vendor/` 里的预编译二进制）。
- 不获取、不分析、不复现任何**未发售**标题（GTA VI）的二进制、镜像、破解或 DRM 绕过内容；对 GTA VI 的讨论仅限官方物料与业界报道。
- 不绕开任何 DMCA 处置去取被下架源码（re3/reVC，见 3.2.3）。
- 不提供绕过在线模式反作弊、盗取在线货币或破坏多人服务的实现细节；相关项目（FiveM/MTA/alt:V/RageMP）仅作为**引擎结构与文件格式的逆向证据**来分析。

### 1.5 规模速览（详见 6.2）

九个仓库的一手代码规模（剔除 `vendor/contrib/extern/third_party/node_modules` 后的物理行；`citizenfx/natives` 为纯数据仓库故不计行）：

| 项目 | 一手代码 | 文件数 | 主语言 | 单文件极限样本 | 路线 |
|---|---|---|---|---|---|
| citizenfx-fivem | 1,275,212 | 13,425 | C++/JS/Lua/C# | `ext/natives/natives_stash/gta_universal.lua` 76,015 行 | E 多人+接管 |
| mtasa-blue | 614,522 | 9,180 | C++ | `Client/mods/deathmatch/logic/CModelNames.cpp` 29,392 行 | C+E |
| GTAIV.EFLC.FusionFix | **392,909** | 3,362 | 反编译着色器 ASM 315,659 + C++ 68,497（62 个 `.ixx` 模块 50,222；扣 vendored 后自有约 55,800） | `shaders/.../gta_ped_reflectPS2.asm`（HLSL 字节码转储+手工改造）、1,681 条去重特征码 | C 运行时接管 |
| CodeWalker | 327,165 | 725 | C# | `CodeWalker.Core/GameFiles/FileTypes/RelFile.cs` 24,909 行 | D 格式逆向 |
| scripthookvdotnet | 200,663 | 605 | C# | `source/scripting_v3/GTA.NaturalMotion/EuphoriaHelpers.cs` 30,986 行 | C API 表层 |
| librw | 139,843 | 306 | C++ | `src/`（RenderWare 本体）55,597 / `skeleton/` 71,724 / `tools/` 12,092 | A/B 引擎重写 |
| txAdmin | 57,519 | 729 | TypeScript | FXServer 运维面板 | E 运维层 |
| openrw | 62,959 | 351 | C++17 | `rwengine/src/engine/SaveGame.cpp` 1,376 行（含 12,928 行 `.inl`） | A 引擎重写 |
| citizenfx-natives | — | 6,449 | 数据（44 个命名空间目录 / 6,437 份 Markdown） | 每条 native 一个记录 | D 数据库 |

两个立刻可见的结构性事实：**代码量最大的项目不是重写者，而是接管者**（FiveM）；而**真正的引擎级重写只有数万行量级**（librw 的 RenderWare 本体 `src/` 55,597 行、OpenRW 全部一手代码 62,959 行）—— 这两个数字本身就是第 6、7 章"逆向一个新作要多久"的核心线索。

## 2. 全景台账：GTA 逆向/开源引擎生态（39 项，逐条标注核验状态）

状态图例：**C** = 已克隆并做源码级分析 · **O** = 官方确认开源但未克隆 · **X** = 官方确认闭源（仅二进制分发）· **D** = 已被 DMCA/诉讼下架 · **U** = 无法核验官方源（不采信）· **F** = 取证判定为仿冒/投毒（禁止下载）

### 2.1 引擎级重写与反编译（路线 A/B）

| # | 项目 | 目标 | 地址 | 许可 | 语言 | 状态 |
|---|---|---|---|---|---|---|
| 1 | **OpenRW** | GTA III 全引擎重写 | `rwengine/openrw` | GPL-3.0 | C++17 | **C** 3.1 |
| 2 | **librw** | RenderWare 3 库本体重实现 | `aap/librw` | MIT | C++ | **C** 3.7 |
| 3 | **re3** | GTA III 二进制等价反编译 | `GTAmodding/re3`（main） | 无 OSI 许可 | C++ | **D/X** 3.2（HTTP 451 `dmca`，2021-09-30） |
| 4 | **reVC** | GTA VC 同上 | `GTAmodding/re3` 的 `miami` 分支 | 同上 | C++ | **D/X** 3.2 |
| 5 | **OpenRPA** | GTA:SA 引擎重建 + 多人 | 曾 `Velostream/OpenRPA` | — | C++ | **U** 3.2.5（404、账号 0 公开仓库、Wayback 零抓取、`open-r.pw` NXDOMAIN） |
| 6 | re3 下游移植端（RPi/3DS/WiiU/Switch） | 移植 | `Jai-JAP/RPi-GTA-re`、`GNU-Pattor-Team/re3-3DS`、`JimDude7404/re3-wiiu` 等 | 混合 | C++ | **O** 依赖 D 件，不作独立分析 |
| 7 | **reVCDOS / revcdos** | VC → x86 DOS + 浏览器 WASM 移植 | `Lolendor/reVCDOS`、`Carter54git/revcdos`（dos.zone/revcdos） | 未标 | Python/asm | **O** 派生物，非 reVC 官方源 |
| 8 | `openrw-rs` | GTA III 相机/武器/实体数学的 Rust 端口 | `bhubbard/openrw-rs` | 未标 | Rust | **O** 1★，片段级 |
| 9 | `openrw.js` | RenderWare 的 TypeScript 实现 | `screeny05/openrw.js` | 未标 | TS | **O** |
| 10 | `dff2gltf` | DFF→glTF 转换器（基于 OpenRW） | `davidar/dff2gltf` | — | C++ | **O** |
| 11 | `gta-maxout` | VC 极限改造研究 | `Swoorup/gta-maxout` | 未标 | C++ | **O** 32★，低活跃 |
| 12 | **OpenGTA / fifengine** | GTA 1/2 重实现（历史项目） | `madebr/OpenGTA`（原 `ogta.fifengine.de` 镜像）、`mkmkme/OpenGTA` | 混合 | C++ | **O** 上游站点已死 |
| 13 | `opengta2` | GTA 2 重实现（MonoGame） | `ikkentim/opengta2` | MIT | C# | **O** 28★ |

### 2.2 RAGE 时代的运行时接管与脚本层（路线 C/E）

| # | 项目 | 目标 | 地址 | 许可 | 语言 | 状态 |
|---|---|---|---|---|---|---|
| 14 | **MTA:SA** | GTA:SA 注入式多人框架 | `multitheftauto/mtasa-blue` | GPLv3（`Shared/publicsdk/LICENSE` 另加非商用限制） | C++23 | **C** 3.5；网络层二进制不在源内 |
| 15 | **FiveM / CitizenFX** | GTA V/RDR2 服务端+客户端框架 | `citizenfx/fivem` | **Take-Two Creator Platform + LGPL-2.0 白名单** | C++/JS/Lua/C# | **C** 3.6 |
| 16 | **ScriptHookVDotNet** | GTA V 的 .NET 托管脚本层 | `scripthookvdotnet/scripthookvdotnet` | zlib 类三条件 | C# | **C** 3.3 |
| 17 | **ScriptHookV** | GTA V 原生脚本接口 | `dev-c.com`（gta5-mods 分发页） | 闭源 `.asi` | — | **X**；GitHub 上 `crosire/scripthookv` **不存在** |
| 18 | **GTAIV.EFLC.FusionFix** | GTA IV/EFLC PC 移植修复 | `ThirteenAG/GTAIV.EFLC.FusionFix` | GPL-3.0 | C++/ASM/HLSL | **C** 3.8 |
| 19 | GTAIV.EFLC.FusionShaders | IV 着色器重做 | `Parallellines0451/GTAIV.EFLC.FusionShaders` | GPL-3.0 | HLSL | **O** 与 18 配套 |
| 20 | GTA IV C++ Script Hook（aru 等） | IV 原生脚本接口 | gtaforums topic 390582（历史上曾 `[SRC]` 发布） | 历史开源 | C++/ASM | **O** 论坛反爬，本次未取得源 |
| 21 | `GTAIVMissionScripts` | IV 任务脚本反编译产物 | `0x52617665/GTAIVMissionScripts` | Unlicense | — | **O** 数据件，脚本 VM 逆向的直接证据 |
| 22 | **txAdmin** | FXServer 运维/权限/备份面板 | `citizenfx/txAdmin` | MIT | TypeScript | **C** 3.9 |
| 23 | alt:V | GTA V 多人平台 | `altmp` org（`altv-issues`、`altv-example-resources` MIT、`altv-client-js` **无许可**、`alt-voice-old` GPL-2.0） | 混合 | — | **O/X** 客户端与服务端核心闭源；站点 `docs.altv.mp` |
| 24 | **RageMP** | GTA V 多人平台 | `ragemp.com` 已停放（域名出售页）；活站 `rage.mp`/`wiki.rage.mp`（CF 盾） | 闭源 | — | **X** 无可克隆物；曾被 Take-Two 发停止函 |
| 25 | RedM | RDR2 多人平台 | 与 15 同仓库（`redm.gg/terms`） | 同 15 | — | **C**（并入 3.6） |
| 26 | nativewrappers | RDR2/V native 包装生成 | `nativewrappers/nativewrappers` | MIT | TS | **O** 隶属关系未证实，谨慎引用 |

### 2.3 资产/格式逆向与工具链（路线 D）

| # | 项目 | 目标 | 地址 | 许可 | 状态 |
|---|---|---|---|---|---|
| 27 | **CodeWalker** | GTA V RAGE 资源全格式读写 + 世界/地图编辑 | `dexyfex/CodeWalker` | **仓库内无 LICENSE**（仅 "educational purposes only"） | **C** 3.4 |
| 28 | **OpenIV** | GTA V/IV/RDR2 归档与模型编辑 | `openiv.com`（作者 GooD-NTS；GitHub 仅 `GooD-NTS/OpenIV-Languages`） | 闭源 | **X**；`openiv.mp` **不解析**；2017-06 停止函事件主角 |
| 29 | **RAGE Plugin Hook** | GTA V 插件框架 | `gta5-mods.com/tools/rage-plugin-hook`；`ragepluginhook.net` 根路径 404 | 闭源 | **X** 官方站实质已下线 |
| 30 | NativeDB / alloc8or | GTA V/RDR2 native 哈希与签名数据库 | `alloc8or.re/gta5/nativedb/`、`alloc8or.re/rdr3/nativedb/` | 公开数据 | **O** 与 31 同源生态 |
| 31 | **citizenfx/natives** | native 定义数据库 + 多语言 codegen | `citizenfx/natives`（44 个命名空间目录，6,437 份 Markdown） | 见仓库 | **C** 3.9 |
| 32 | `rage-engine-formats` | RPF7/YDR/YTD 的 Rust 解析器（2026-10，含 Gen9/Enhanced 布局版本表） | `lib.rs/crates/rage-engine-formats` | — | **O** 与 CodeWalker 交叉核对，证明格式逆向仍在进行 |
| 33 | `rage_script` | IDA 对 RAGE/GTA5 脚本的加载器/模块 | `balika011/rage_script` | — | **O** |
| 34 | `BlenDR` | Blender 的 RAGE 格式导入插件 | `spicybung/BlenDR` | — | **O** WIP |
| 35 | `renderware3-flirt` | RenderWare 3.x 的 IDA FLIRT 签名集（逆向元工具） | `ifarbod/renderware3-flirt` | — | **O** 方法论价值高 |
| 36 | IDA native renamer / rdr_natives | RDR2 逆向辅助 | `Sna1lLord/RDR2-IDA-Native-Renamer`（Apache-2.0）、`Sarbatore/rdr_natives`（GPL-3.0） | 明确 | **O** |

### 2.4 明确排除项（F：取证判定为仿冒/投毒，禁止下载）

| 仓库 | 判定依据 |
|---|---|
| `knight-oss/gta2-reverse-engineering-lab`、`btshahraag20/gta2-memory-master`、`izawartka/gta2-trainer-cpp` | 仓库仅 `README.md`+`index.html`+两三个 `.svg`；README 自称 C++/Python/Lua、"coverage 93%"；Download 徽章指向 GitHub Pages 上单一 base64 blob；提交日期 2026-11-18 |
| `yuaahu87-ai/rogue-outlaw-armory` | "RDR2 Ultimate Native Trainer 2026" 同构诱饵 |
| `hottabxp/re3`、`leanndroguedes/re3-19feb21`、`mrxenginner/re3`、`mrxenginner/reVC`、`Jai-JAP/re-GTA` | 未改动地照抄 re3 README，但 Discord 邀请码与真实血统不同（真 `ERYg58ttcE` / 仿 `RFNbjsUMGg`）；血统不可验证 |
| `Dryxio/dryxio`、`xsanstomper/GroveTheft`（"浏览器里跑的 WebGPU RAGE 端口"）、`panah-ryan/GTAIV-Protections-Xbox`、`Apex-master/GTA-crossmap-natives`、`SebastienCozeDev/rage-data-api`、`DarkSlim/GTARESearch`、`ThirtyFoxMCx00E1/wagu`、`0z0sk0/pySAMPAPI` | 主张与体量不符或无法核验，**不纳入分析** |

### 2.5 台账的两条硬结论

1. **真正"活的"开源 GTA 逆向重心已经从"重写引擎"移到"接管 + 数据库"**：39 项里 12 项是引擎级重写/反编译（1–13），其中只有 OpenRW、librw 至今活跃，其余要么被下架（re3/reVC）、要么无法核验（OpenRPA）、要么只是片段移植（7–11）。而接管/数据层（14–26、30–31）几乎全部在近 12 个月内有过提交。
2. **闭源侧的边界正在收缩，但不是因为逆向变强**：ScriptHookV、OpenIV、RAGE Plugin Hook、RageMP 四个闭源件中，RageMP 域名已停放、RPH 官网实质下线、OpenIV 只剩语言包仓库；与此同时发行商把最强的一方（Cfx.re）变成了自己的开源仓库。**"民间闭源强工具"这一层正在被"发行商自有 source-available"取代** —— 这是第 4、5 章全部法律材料的落点。

## 3.1 OpenRW（rwengine/openrw）

### 项目定位

OpenRW（"Open ReWrite"）是 GTA III 可执行文件的跨平台开源重写，GPL-3.0-only。它的技术路线与 re3 / reVGS（GTA-Mods 系）根本不同：**不做二进制等价（binary-identical）改写，也不注入原 exe**，而是把 RenderWare 的*资产文件格式*（DFF / TXD / IMG / COL / IFP）与 GTA 的*数据/脚本/存档格式*（IDE / IPL / GXT / SCM / .b save）作为公开规格重新实现，再在其上搭一层游戏逻辑。渲染后端替换为 OpenGL 3.3 core，物理替换为 Bullet，音频替换为 OpenAL + FFmpeg，窗口/输入替换为 SDL2。

需要先纠正本报告任务书中的一处前提：本仓库当前 HEAD（`5c5f266`）的目录结构是 `rwcore/ rwengine/ rwgame/ rwviewer/ rwtools/ tests/`，**不存在** `source/`、`engine/`、`game/` 三层；也没有 "game profile" 抽象层，README.md 首段自我描述为"a cross-platform, open source re-implementation of Rockstar Games' Grand Theft Auto III"，对 Vice City / San Andreas 未作任何承诺（全仓库对 VC/SA 的引用只有注释与少量枚举残留）。因此下面所有关于"三作共用"的结论都是**否定性**的。

### 元数据表

| 项 | 值 | 证据 |
|---|---|---|
| 上游 | rwengine/openrw | `git remote` |
| 本地 HEAD | `5c5f266`，日期 2025-06-18；本克隆为 squash/浅克隆，仅 1 个 commit，作者信息不可用于统计 | `git log` |
| 许可 | GPL-3.0（`COPYING` 35 KB）+ README 第三方许可清单 | `README.md` |
| 语言标准 | C++17（`CXX_STANDARD 17`，`CXX_EXTENSIONS OFF`）；C11 | `cmake_configure.cmake:198-204` |
| CMake 最低版本 | `3.8...3.28` | `CMakeLists.txt:1` |
| 文件总数 | 353（`git ls-files \| wc -l`）；工作区 3.3 MB（不含 `.git`） | — |
| 手写 C++ 规模 | 约 53.9k 行（`.cpp` 27,819 / `.hpp` 13,124 / `.inl` 12,928） | `find` + `wc -l` |
| 生成代码 | `rwcore/gl/gl_core_3_3.{c,h}` 共 8,478 行（glad 风格无 `dlopen` 加载器），已从覆盖率中排除 | `rwcore/CMakeLists.txt:70-74` |
| 目标产物 | 静态库 `rwcore`→`rwengine`；库 `librwgame` + 可执行 `rwgame`；可选 `rwviewer`（Qt5）、`rwfontmap`（Qt5+FreeType）、`rwtests` | 各 `CMakeLists.txt` |
| 依赖 | Boost（program_options/system/test）、Bullet、GLM、OpenAL、SDL2、FFmpeg（avcodec/avformat/avutil/swresample）、Freetype（仅 tools）、Qt5（仅 viewer/tools）；子模块 imgui、microprofile | `CMakeLists.txt:31-52`、`conanfile.py:31-45` |
| CI | GitHub Actions（Arch Linux / Fedora / Ubuntu 三容器）、AppVeyor（MSVC 2017 + Conan）、Travis（macOS，仅 badge）、CDash（`my.cdash.org/index.php?project=OpenRW`）、codecov | `.github/workflows/build-and-test.yml`、`.appveyor.yml`、`CTestConfig.cmake` |

### 目录与模块清单（含行数统计）

统计口径：该目录**本层**文件（不含子目录）的 `.cpp/.c` 与 `.hpp/.h` 行数。

| 路径 | 文件 | .cpp 行 | 头文件行 | 合计 | 职责 |
|---|---|---|---|---|---|
| `rwcore/rw` | 5 | 16 | 167 | 183 | RW 基础宏：`RW_CHECK`/`RW_ASSERT`/`RW_UNIMPLEMENTED`、`bit_cast`、前向声明 |
| `rwcore/data` | 2 | 152 | 287 | 439 | `Clump` / `Atomic` / `Geometry` / `ModelFrame` / `GeometryVertex` |
| `rwcore/loaders` | 9 | 968 | 604 | 1,572 | **RenderWare 格式层**：`LoaderDFF`、`LoaderTXD`、`LoaderIMG`、`LoaderSDT`、`RWBStream` |
| `rwcore/gl` | 8 | 5,786 | 2,914 | 8,700 | `GeometryBuffer`（VBO+语义化属性）、`DrawBuffer`（IBO）、`TextureData`；含 8,478 行生成 loader |
| `rwcore/fonts` | 8 | 412 | 510 | 922 | `FontMap`/`FontMapGta3`（GSChar↔Unicode）、`GameTexts`、`Unicode` |
| `rwcore/platform` | 5 | 161 | 147 | 308 | `FileIndex`（大小写归一 + IMG 归档统一寻址）、`FileHandle`、`RWWindows` |
| `rwengine/src/engine` | 17 | 4,612 | 1,769 | 6,381 | `GameData`、`GameWorld`、`GameState`、`SaveGame`、`Animator`、`Garage`、`Payphone`、`ScreenText` |
| `rwengine/src/script/modules` | 3 | 923 | 10 | **13,861** | `GTA3Module.cpp`（904 个 `bind()`）+ `GTA3ModuleImpl.inl`（12,928 行 opcode 实现） |
| `rwengine/src/objects` | 16 | 2,962 | 1,379 | 4,341 | `GameObject`/`VehicleObject`/`CharacterObject`/`InstanceObject`/`PickupObject`/`ProjectileObject`/`CutsceneObject` |
| `rwengine/src/render` | 20 | 2,413 | 1,424 | 3,837 | `Renderer` 抽象 + `OpenGLRenderer`、`GameRenderer`、`ObjectRenderer`、`WaterRenderer`、`MapRenderer`、`TextRenderer`、`DebugDraw`、`VisualFX`、`ViewFrustum` |
| `rwengine/src/ai` | 12 | 1,722 | 576 | 2,298 | `AIGraph`/`AIGraphNode`、`CharacterController`、`PlayerController`、`DefaultAIController`、`TrafficDirector` |
| `rwengine/src/data` | 19 | 804 | 1,341 | 2,145 | `ModelData`、`CollisionModel`、`PedData`、`VehicleInfo`、`AnimGroup`、`ZoneData`、`Weather`、`Chase`、`PathData` |
| `rwengine/src/audio` | 14 | 1,394 | 388 | 1,782 | `SoundManager`/`Sound`/`SoundSource`（FFmpeg→OpenAL）、`SoundBuffer(Streamed)`、`SfxParameters` |
| `rwengine/src/script` | 10 | 613 | 1,116 | 1,729 | `ScriptMachine`（VM）、`SCMFile`、`ScriptModule`、`ScriptTypes`、`ScriptFunctions` |
| `rwengine/src/loaders` | 16 | 1,089 | 300 | 1,389 | `LoaderIDE`/`LoaderIPL`/`LoaderCOL`/`LoaderGXT`/`LoaderIFP`/`LoaderCutsceneDAT`/`GenericDATLoader`/`WeatherLoader` |
| `rwengine/src/dynamics` | 5 | 207 | 140 | 347 | `CollisionInstance`（Bullet body 装配）、`HitTest`、`RaycastCallbacks` |
| `rwengine/src/items` | 2 | 83 | 64 | 147 | `Weapon` |
| `rwengine/src/core` | 4 | 45 | 94 | 139 | `Logger`、`Profiler`（microprofile 包装） |
| `rwgame` | 24 | 2,437 | 1,507 | 3,944 | `main.cpp`、`RWGame`、`GameBase`、`GameWindow`(SDL)、`GameInput`、`RWConfig(.inc)`、`HUDDrawer`、`MenuSystem`、`RWImGui`、`State*` |
| `rwgame/states` | 12 | 1,379 | 244 | 1,623 | `LoadingState`、`IngameState`、`PauseState`、`MenuState`、`DebugState`、`BenchmarkState` |
| `rwviewer`（+models/views/widgets） | 35 | 1,667 | 747 | 2,414 | Qt5 资产浏览器（DFF/TXD/IMG/GXT/动画） |
| `rwtools/rwfont` | 1 | 272 | 0 | 272 | FreeType→字体图集离线生成器 |
| `tests` | 43 | 3,439 | 135 | 3,574 | Boost.Test 单元测试 |

关键观察：**单一文件 `GTA3ModuleImpl.inl`（12,928 行）就占全项目手写代码的 24%**，是脚本层重写的绝对主体；而 `rwcore` 去掉生成代码后只剩约 220 行手写 GL 包装 —— 说明 RenderWare 镜像的工作量集中在*格式解析*与*逻辑*，不在图形后端。

### 构建系统与依赖图

```
rw_interface (INTERFACE, openrw::interface)   ← cmake_configure.cmake：所有警告/宏/平台定义
rw_checks     (INTERFACE, openrw::checks)     ← sanitizer 专用，与警告分离
   └─ rwcore  ← Boost::boost, glm::glm, OpenGL::GL(private)
       └─ rwengine  ← bullet::bullet, ffmpeg::ffmpeg, OpenAL::OpenAL, [microprofile]
           └─ librwgame (STATIC)  ← SDL2::SDL2, Boost::program_options, imgui::sdl_gl3
               ├─ rwgame (executable, main.cpp only)
               └─ rwtests (executable)
       └─ rwfontmap (rwtools) ← Freetype, Qt5::Gui
   └─ rwviewer ← rwengine, Qt5::OpenGL/Widgets
```

设计要点：

1. **依赖来源双轨制**。`USE_CONAN` 为真时走 `conan_basic_setup(TARGETS NO_OUTPUT_DIRS)`；否则走 `find_package`。两条路径通过 `cmake/modules/WrapTargets.cmake` 归一化为同一套 namespaced target（`rwdep_wrap_conan_targets()` 把 `CONAN_PKG::bullet3` 包装成 `bullet::bullet`），因此业务 `CMakeLists.txt` 完全不感知依赖来源。注意 `conanfile.py` 中 `stdcpp = 14` 与实际 `CXX_STANDARD 17` 已不同步（陈旧声明，构建时由 CMake 覆盖）。
2. **统一选项注入函数** `openrw_target_apply_options(TARGET x CORE COVERAGE COVERAGE_EXCEPT ...)`（`cmake_configure.cmake:182-245`）：一次性完成 C++ 标准设定、IWYU 挂接、clang-tidy 挂接、覆盖率注册、install/PDB 安装。所有目标必须经此函数，这是"约定即架构"的做法。
3. **平台探测即编译期硬失败**：`cmake_configure.cmake:82-96` 对 Linux/Darwin/FreeBSD/NetBSD/OpenBSD/Windows 逐一 `RW_LINUX`/`RW_OSX`/… 定义，`else()` 直接 `FATAL_ERROR`，无 CMake 平台抽象层。
4. **Git SHA 内嵌**：`include(GetGitRevisionDescription)` + `configure_file(rwgame/GitSHA1.cpp.in ...)`（`rwgame/CMakeLists.txt:1`）。
5. **CTest 驱动 CI**：所有平台 job 都只跑一句 `ctest -VV -S cmake/ctest/script_ci.ctest`，由 `configure_{linux,windows,darwin}.ctest` 决定生成器/选项，`build.ctest` 复用，实现"CI 逻辑放 CMake 而非 YAML"。
6. **无代码生成的 CMake 集成**：`scripts/generate_scripts_functions.py` 与 `rwtools/rwfont` 都是**人工离线执行**，构建期不跑（Python 2 + lxml 脚本，已在 `generate_scripts_functions.py:1` 标注 `#!/usr/bin/env python2`）。因此 `GTA3Module.cpp`/`.inl` 与 `FontMapGta3.cpp` 是"提交进仓库的生成物"。
7. `RW_FAILED_CHECK_ACTION`（IGNORE/ABORT/BREAKPOINT）与 `ENABLE_SCRIPT_DEBUG`、`ENABLE_PROFILING`、`TEST_DATA`、`SEPARATE_TEST_SUITES` 等构成完整的调试开关矩阵（`cmake_options.cmake`）。

### 逆向机制核心：RenderWare 类布局镜像（带代码证据）

OpenRW 的镜像发生在**三个不同层面**，必须区分开：

**层面 A —— RenderWare 二进制流（BS）chunk 树：这是主战场。**
`rwcore/loaders/RWBinaryStream.hpp:89-114` 用 `namespace RW` 复刻了 chunk type ID 常量表：

```cpp
namespace RW {
enum {
    SID_Struct = 0x0001,  SID_String = 0x0002,  SID_Extension = 0x0003,
    SID_Texture = 0x0006, SID_Material = 0x0007, SID_MaterialList = 0x0008,
    SID_FrameList = 0x000E, SID_Geometry = 0x000F, SID_Clump = 0x0010,
    SID_Atomic = 0x0014, SID_TextureNative = 0x0015, SID_TextureDictionary = 0x0016,
    SID_GeometryList = 0x001A,
    SID_HAnimPLG = 0x011E,
    SID_BinMeshPLG = 0x50E,
    SID_NodeName = 0x0253F2FE
};
```

紧随其后的注释直接给出来源（`RWBinaryStream.hpp:82-87`）：

```
 * @file rwbinarystream.h
 * Deprecated RenderWare binary stream interface.
 * ...
 * Many thanks to
 * http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file
```

这是全仓库**唯一一处**明确点名的逆向来源（详见"逆向来源标记"节）。`RWBStream`（`RWBinaryStream.hpp:22-79`）实现 `{id,size,version}` 三元组游标与 `getInnerStream()` 递归下降；`RW::BinaryStreamSection`（同文件 `289-356`）则是另一套基于 `reinterpret_cast` 的 section 读取器。**两者并存且分工不同**（DFF 用前者、TXD 用后者，见 `LoaderDFF.cpp:460-484` 与 `LoaderTXD.cpp:42,179`），属可识别的技术债。

**层面 B —— 偏移保真的 POD 结构（仅用于文件解析，不用于内存对象）。**
`RW::BSGeometry`、`RW::BSGeometryTriangle`、`RW::BSTextureNative`、`RW::BSMaterial` 等（`RWBinaryStream.hpp:144-279`）是逐字段对齐磁盘布局的 POD；`RW::BSGeometryTriangle` 的四个 `uint16_t`（`first/second/attrib/third`）尤其说明问题，作者还留下情绪化注释：`uint16_t attrib;  // Who designed this nonsense.`（同文件 `180`）。DFF 侧最典型的是 `rwcore/loaders/LoaderDFF.cpp:38-51`：

```cpp
// These structs are used to interpret raw bytes from the stream.
/// @todo worry about endianness.
struct RWBSFrame {
    BSTMatrix rotation;   // glm::mat3
    BSTVector3 position;  // glm::vec3
    int32_t index;
    uint32_t matrixflags;  // Not used
};
```

此处有一条**隐蔽但关键**的构建期依赖：全项目**没有**任何 `#pragma pack` / `__attribute__((packed))` / `alignas`（已用 grep 确认为零命中），`RWBSFrame` 之所以恰好 68 字节、可被 `headerPtr += sizeof(RWBSFrame)` 直接推进，全靠 `cmake_configure.cmake:75` 的 `GLM_FORCE_PURE` 全局宏强制 GLM 使用纯 C++ 实现（`mat3` 存为 3 个 `vec3`，无 SIMD 补齐）。**「推断」：这是一个未写成文档、一旦移除即静默破坏所有 DFF 解析的隐式不变量**，比 `#pragma pack` 更脆弱。

**层面 C —— 运行时对象模型是干净重写，不是内存镜像。**
`rwcore/data/Clump.hpp` 中的 `Clump`/`Atomic`/`Geometry`/`ModelFrame` 使用 `std::shared_ptr`（`rwcore/rw/forward.hpp:62-66` 定义 `ClumpPtr` 等别名）、`std::vector`、`std::string`、`GLuint EBO`，与 RenderWare 的 `RpClump`/`RpAtomic`/`RpGeometry` + `RwLink` + memNode 池毫无对应关系。**任务书要求查找的 `rpBaseClass`、`rpIntObj`、`GetT<T>()` downcast、`Rp*` C API shim、plugin registration、`RW::` 核心 chunk 之外的 vtable 镜像，在 OpenRW 中一律不存在**：
- `grep -rn "rpBaseClass\|rpIntObj\|GetT\b"` → 0 命中；
- `grep -rni "plugin"` → 0 命中（仅 `SID_HAnimPLG`/`SID_BinMeshPLG` 两个 chunk ID 名字里残留 "PLG"，且 HAnim plugin 未被解析）；
- 唯一的 `static_assert` 只有 1 处，见下节；
- 无自定义分配器：`grep "operator new\|allocator\|RwMalloc\|MemoryPool"` → 0 命中，全部依赖 `std::unique_ptr`/`std::shared_ptr`/`std::vector`。

**唯一真正的 sizeof 断言在存档层**：`rwengine/src/engine/SaveGame.cpp:549-551`

```cpp
    static_assert(sizeof(BasicState) == 0xBC,
                  "BasicState is not the right size");
    READ_VALUE(state.basic)
```

`BasicState` 定义在 `rwengine/src/engine/GameState.hpp:41-76`，是**全项目最纯粹的偏移镜像**：成员顺序、`_align0.._align5` 填充、以及 `uint8_t dateTime[24]{0};  // Unused` 全部为对齐 0xBC 而存在。同文件 `PlayerInfo`（78-95）带 `uint8_t unknown5[70]{0}`，`SaveGame.cpp:39-116` 的 `Block0RunningScript`/`Block0ScriptData`/`StructPed` 更是把 `unknown[0x100]`、`unknown1[640]`、`unknown3[348]` 这类"逆向未归类区段"原样写进类型系统：

```cpp
struct StructPed {
    uint8_t unknown0_[52];
    glm::vec3 position{};
    uint8_t unknown1[640];
    float health;
    float armour;
    uint8_t unknown2[148];
    std::array<StructWeaponSlot, kNrOfWeapons> weapons;
    uint8_t unknown3[348];
};
```

读档侧配 `CHECK_SIG(expected)` 与 `BLOCK_HEADER(sizevar)` 宏做游标跳转（`SaveGame.cpp:519-534`），共 22 个块（`grep -c BLOCK_HEADER` = 22）。

**结论**：OpenRW 的逆向机制是**"格式级镜像 + 逻辑级清洁重写"**，而非 re3 式的"内存布局/函数地址镜像"。它的可验证性来自"能否正确解析官方数据文件并跑通官方任务脚本"，而非"某函数是否在原 exe 的某地址"。

### game 层与 engine 层分工

| 层 | 目录 | 抽象层级 | 与 GTA 的关系 |
|---|---|---|---|
| RW/图形基元 | `rwcore/` | 最底层，无游戏语义 | 只认识 DFF/TXD/IMG/SDT 字节；唯一含 GTA 特例的是 `fonts/FontMapGta3.cpp` 与 `types.hpp` 里的 `WATER_*`/`WORLD_GRID_*` 常量 |
| 引擎+玩法 | `rwengine/` | 领域对象与系统 | 认识 `VehicleObject`、`AIGraph`、`SCMThread`，但**不含窗口/GL 上下文** |
| 前端/外壳 | `rwgame/` | 平台与状态机 | 唯一链接 SDL2/imgui 的层；持有 `main()` |

依赖严格单向：`rwgame → rwengine → rwcore`。`rwengine/CMakeLists.txt:156-163` 明确不链 SDL，`rwgame/CMakeLists.txt:66-72` 才引入 `SDL2::SDL2` 与 `Boost::program_options`。

入口：`rwgame/main.cpp:12-33` —— 先建 `Logger`/`StdOutReceiver`，再 `RWArgumentParser::parseArguments`（`SDL_MAIN_HANDLED` + `SDL_SetMainReady()` 避开 `SDLmain`），然后 `RWGame game(logger, argLayerOpt); game.run();`。`main` 只 catch `std::runtime_error` 并用 `SDL_ShowSimpleMessageBox` 呈现（`main.cpp:34-52`），注释解释了为何不吞 `bad_alloc`。

主循环 `RWGame::run()`（`rwgame/RWGame.cpp:415-457`）：`while (stateManager.currentState() && running)` → `tickWorld(deltaTime, accumulatedTime)`（固定步长累加器，`RWGame.cpp:459-478`）→ `SDL_PollEvent` 分发（489）→ `stateManager.draw`。`RWGame::tick`（529-592）依次推进 chase、游戏内时钟（`state.basic.gameMinute/gameHour`）、脚本计时器 `timerClockRate`、`tickObjects(dt)`（遍历 `world->allObjects` 调 `object->tick(dt)`）、`vm->execute(dt)`。

状态机：`State`（`rwgame/State.hpp:20-67`）纯虚 `enter/exit/tick/draw/handleEvent/getCamera/shouldWorldUpdate`，`StateManager` 管理栈；6 个实现见 `rwgame/states/`。

### 关键子系统逐个拆解

**渲染管线**（`rwengine/src/render/`，3,837 行）。抽象接口 `Renderer` 定义在 `OpenGLRenderer.hpp:73`（无独立 `Renderer.hpp`），内含 `DrawParameters`（`86-110`：count/start/`Textures`/`BlendMode`/`DepthMode`/colour/ambient/diffuse/visibility）与 `RenderInstruction`（118+）。三段式：
1. `ObjectRenderer`（`ObjectRenderer.cpp`）把 `GameObject` 展开为 `RenderList`，用 `RenderKey createKey(float normalizedDepth, Textures&)` 做 3-pass 排序键打包：`(uint32_t(0x7FFFFF * normalizedDepth) << 8 | uint8_t(0xFF & textures[0]))`（`ObjectRenderer.cpp:38-41`）—— 深度占高位、纹理 ID 占低位，实现"按深度排序 + 同纹理合并"。LOD 距离常量 `kMagicLODDistance = 330.f`、`kVehicleLODDistance = 70.f`、`kVehicleDrawDistance = 280.f`（34-36）。
2. `GameRenderer`（`GameRenderer.hpp:29-`）持有 `std::unique_ptr<Renderer> renderer = std::make_unique<OpenGLRenderer>()`，编排 world/sky/water/particle/post 五套 shader（`worldProg`、`skyProg`、`particleProg`、`ssRectProg`、`postProg`）。
3. `OpenGLRenderer` 实现 `pushDebugGroup/popDebugGroup` + `ProfileInfo` 计数（draws/primitives/buffers/textures/uploads，`OpenGLRenderer.hpp:226-376`）。

Shader 全部以 R"()" 字面量内联于 `GameShaders.hpp`（`namespace GameShaders`，GLSL `#version 330`，`layout(std140) uniform SceneData` 含 projection/view/ambient/dynamic/fogColor/campos/fogStart/fogEnd），**运行时不读磁盘 shader 文件**。GL 顶点布局通过"结构体自带 `static const AttributeList vertex_attributes()`"这一约定实现零反射：`GeometryVertex`（`Clump.hpp:114-137`）声明 position/normal/texcoord/colour 的 stride+offset，并在注释里写出目标偏移 `/* 0 */ /* 24 */ /* 48 */ /* 64 */`；`VertexP2`/`VertexP3`（`OpenGLRenderer.hpp:28-57`）同理。`GeometryBuffer::uploadVertices` 靠 `attributes = T::vertex_attributes();` 提取（`GeometryBuffer.hpp:68-73`）。

剔除：`ViewFrustum`（`ViewFrustum.hpp:11-`）为 6 个 `ViewPlane`；`rwcore/rw/types.hpp:26-29` 的 `WORLD_GRID_SIZE 4000 / WORLD_CELL_SIZE 100 / WORLD_GRID_CELLS` 同时服务于 AI 图与剔除网格。水面 `WaterRenderer` 用数据纹理（`setDataTexture(GLuint fbBinding, GLuint dataTexture)`）实现投影网格 HQ 水面，参数来自 `types.hpp:12-20`。

**碰撞/物理**（`rwengine/src/dynamics/`，仅 347 行 + 使用点）。`CollisionInstance::createPhysicsBody(GameObject*, CollisionModel*, DynamicObjectData*, VehicleHandlingInfo*)` 把 `CollisionModel`（`data/CollisionModel.hpp:14-51`：`Sphere/Box/Triangle` + 每面 `Surface{material,flag,brightness,light}`）转成 `btCompoundShape` + `btTriangleIndexVertexArray`，并挂 `GameObjectMotionState : btMotionState` 回写 `GameObject::position/rotation`。`HitTest`（`HitTest.hpp:17-35`）只暴露 `sphereTest` / `boxTest`。车辆用 Bullet 内置 `btRaycastVehicle` + `btVehicleRaycaster`（`VehicleObject.hpp:66-67`），参数来自 `data/handling.cfg`。角色用 `btKinematicCharacterController` + `btPairCachingGhostObject` + `btCapsuleShapeZ`（`CharacterObject.hpp:20-22`）。浮力是自写的解析式，直接用 `types.hpp` 常量：`WATER_BUOYANCY_K * x + -WATER_BUOYANCY_C * ...`（`VehicleObject.cpp:810-811`、`InstanceObject.cpp:131-132`）。世界侧 `GameWorld::dynamicsWorld`（`GameWorld.hpp:329`）+ 静态回调 `PhysicsTickCallback(btDynamicsWorld*, ...)`（343），`GameWorld.cpp:711-766` 是带 microprofile 探针的 `physicsTick`。

**AI 与路径**（`rwengine/src/ai/`，2,298 行）。`ai::AIGraphNode`（`AIGraphNode.hpp:13-33`）字段 `type/position/size/leftLanes/rightLanes/external/flags/nextIndex/disabled/connections` 是 IPL PATH 段的直接映射；`enum { None = 0, CrossesRoad = 1  /// No documentation for other flags yet, but this is mentioned. }` 是一条诚实的逆向缺口记录。`AIGraph::createPathNodes(...)` / `gatherExternalNodesNear(center, radius, nodes, type)`；`TrafficDirector`（`TrafficDirector.hpp:17-39`）做 `setDensity(NodeType, float)` / `setPopulationLimits(maxPeds, maxCars)` / `populateNearby`；生成半径常量 `kMaxTrafficSpawnRadius = 100.f`、`kMaxTrafficCleanupRadius = *1.25f`（`GameWorld.cpp:54-55`），并用视锥避开"玩家眼前刷车"。寻路最近节点的距离函数是逆向结论的实现（`ScriptFunctions.hpp:263-269`）："This is how the original game calculates distance, weighted manhattan-distance where the vertical distance has to be 3x as close"，即 `|dx| + |dy| + |dz|*3`。

**脚本 VM / opcode**（`rwengine/src/script/`，1,729 + 13,861 行）。**不是 CLEO，也不是 Lua**；是原版 SCM 字节码 VM。
- 线程模型 `SCMThread`（`ScriptMachine.hpp:79-106`）：`char name[17]`、`baseAddress`/`programCounter`、`conditionCount/Result/Mask/AND`、`wakeCounter`、`locals`（`SCM_THREAD_LOCAL_SIZE 256` × `SCM_VARIABLE_SIZE 4`，注释直言 "Changing this will break saves"）、`std::array<pc_t, SCM_STACK_DEPTH 4> calls`。
- 操作数类型直接抄磁盘编码（`ScriptTypes.hpp:178-186`）：`EndOfArgList=0x00, TInt32=0x01, TGlobal=0x02, TLocal=0x03, TInt8=0x04, TInt16=0x05, TFloat16=0x06, TString=0x09`。
- 取指循环 `ScriptMachine::executeThread`（`ScriptMachine.cpp:15-186`）：`auto opcode = file.read<SCMOpcode>(pc)`；条件取反位 `#define SCM_NEGATE_CONDITIONAL_MASK 0x8000`（`ScriptMachine.hpp:20`）；`TFloat16` 解码为 `file.read<std::int16_t>(pc) / 16.f`；`TString` 固定 8 字节。未注册 opcode 抛 `IllegalInstruction(opcode, pc, t.name)`。线程计时器直接写死在局部槽 16/17（`ScriptMachine.cpp:179-183`）：`p.globalPtr = (t.locals.data() + 16 * sizeof(SCMByte) * 4); *p.globalInteger += msPassed;`。硬编码 `opcode != 0x00D6`（"if and"）以维护条件链，并带 `@todo add conditional flag to opcodes`。
- **绑定层是模板元编程的亮点**：`script_bind::arg_traits<T>::arg_size`（`ScriptModule.hpp:12-35`）声明 `ScriptVec2/Vec3/RGB/RGBA` 在字节流里占 2/3/3/4 个槽；递归 `call_unpacked<depth, script_arg, Tret, Targs...>`（72-96）在编译期展开参数游标；`binder<bool, Targs...>` 特化（111-119）自动把返回值写入 `args.getThread()->conditionResult` —— 于是"条件型 opcode"与"动作型 opcode"共用一个 `bind(id, argc, fn)`（`ScriptModule.hpp:163-171`）。
- **生成器**：`scripts/generate_scripts_functions.py`（Python 2 + lxml）读 `SCM.ini`（人类可读签名）+ `SCM.xml`（参数类型），按 `function_template`（29-38 行）与 `binding_template`（40 行）产出 `binding.cpp` + `functions.cpp`，`type_names` 字典（42+）把 `ENTITY/CAR → ScriptVehicle`、`TYPE/FLOAT → ScriptFloat` 映射过去。产物落进仓库：`GTA3ModuleImpl.inl` 每个函数头都是模板展开的注释块（如 `@brief wait %1d% ms` / `opcode 0001` / `@arg time Time (ms)`），`GTA3Module.cpp:19-` 则是 904 行 `bind(0x0000, 0, opcode_0000);`。`.inl` 被 `#include` 进 `.cpp`（`GTA3Module.cpp:14`），从而共享同一 TU 的符号可见性 —— 这是刻意的"生成物内联"手法而非疏忽。
- 桩体只有一行 `RW_UNIMPLEMENTED_OPCODE(0x0003);`（`.inl:64`），而 `RW_UNIMPLEMENTED_OPCODE` 在 `GTA3Module.cpp:7-12` 里被定义为**仅 debug 构建**下打印消息的空宏 —— 生产构建中未实现 opcode 完全静默 no-op。

**音频**（`rwengine/src/audio/`，1,782 行）。`SoundManager` 构造时打开 `audio/sfx.SDT` + `audio/sfx.RAW`（`SoundManager.cpp:44-51`），`LoaderSDT` 解析 SDT 索引并在内存里即时拼 WAV 头（`LoaderSDT.cpp:63-80`：`header->chunkSize = sizeof(WaveHeader) - 8 + assetInfo.size;`）。解码统一交给 FFmpeg（`SoundSource.cpp:7-9,34,85-101,119,157`，含 `avformat_open_input(&formatContext, "SDT", ...)` 这种自定义 IO 名的技巧），送 OpenAL buffer；`SoundBuffer` 与 `SoundBufferStreamed`（按 `kSizeOfChunk` 分块 `alBufferData`）二分。`SfxParameters`（`SfxParameters.cpp`）是 `audio/sfx.dat` 的每音效参数。**未见游戏电台/流式音乐轨（UAUDIO 循环）完整实现**（「推断」，基于 `SoundManager.cpp:36` 的 `// @todo reloading, how to check is it wav/mp3?`）。

**存档**（`SaveGame.cpp/.hpp`，1,420 行）。读档 22 块齐全，**写档整体未实现**（`SaveGame.cpp:499-503`）：

```cpp
void SaveGame::writeGame(GameState& state, const std::string& file) {
    RW_UNUSED(state); RW_UNUSED(file);
    RW_UNIMPLEMENTED("Saving the game is not implemented yet.");
}
```

`getAllSaveGameInfo()` 直接扫原游戏的 `GTA3 User Files` 目录取 `*.b`；Windows 下用注册表 `Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders` 的 `Personal` 值定位"我的文档"（`SaveGame.cpp:1325-1345`），POSIX 下退回 `$HOME`。

**文本 / 字体**（`rwcore/fonts/`）。原版把 HUD 文本存成"16 位字体图集索引"而非 ASCII，OpenRW 忠实保留：`using GameStringChar = std::uint16_t; using GameString = std::basic_string<GameStringChar>;`（`GameTexts.hpp:13-19`），三套字体 `FONT_PAGER 0 / FONT_PRICEDOWN 1 / FONT_ARIAL 2`。`LoaderGXT::load` 跳 4 字节 `TKEY`、按 `blocksize/12` 迭代 `{offset,id[8]}` 表项（`LoaderGXT.cpp:11-31`）。`FontMap` + `Unicode` 负责 GSChar↔U+XXXX，`Unicode.hpp:300` 甚至含 `UNICODE_SHIELD = 0x1f6e1`。字体图集不是运行时渲染的，而是 `rwtools/rwfont/rwfontmap.cpp` 用 FreeType + `QImageWriter` 离线烘成贴图（`FT_Init_FreeType`，`m_glyph_width = width / 16`）。

**资产装载 / "流式"**（`rwengine/src/engine/GameData.cpp`、`rwcore/platform/FileIndex.cpp`）。`GameData::load()`（44-87）一次性顺序加载 `models/gta3.img`、`anim/cuts.img`、5 个 TXD 槽（particle/icons/hud/fonts/generic+misc）、`carcols.dat`、`timecyc.dat`、`handling.cfg`、`waterpro.dat`、`weapon.dat`、`pedstats.dat`、`ped.dat`、`ped.ifp`，再按 `data/default.dat` 与 `data/gta3.dat` 逐行解释 `IDE/SPLASH/COLFILE/IPL/TEXDICTION/MODELFILE`（`loadLevelFile`，110-190）。`FileIndex` 建立"小写相对路径 → 磁盘路径 or IMG 内部条目"的统一映射（`FileIndex.hpp:66-101`，`enum IndexedDataType { FILE, ARCHIVE }`），从而同时满足大小写敏感文件系统。
**没有 RenderWare 风格的流式引擎（CStreaming）**：`ModelData.hpp:84-88` 写着 `/// @todo replace with proper streaming implementation` 的 `virtual bool isLoaded()`/`unload()`，而 `GameWorld.cpp:141` 在 `createInstance` 里同步补载：`/// @todo implment streaming properly`。`LoadingState::tick` 只是 `done(); complete();`（`LoadingState.cpp:15-20`），即"假异步、同步阻塞"。

**内存分配**。无 RW 的 `RwMalloc`/memNode 池等价物，全部标准容器 + 智能指针；`GameWorld` 用 5 个 `ObjectPool`（`GameWorld.hpp:282-288`：`allObjects` + `pedestrianPool/instancePool/vehiclePool/pickupPool/cutscenePool`）与 `GameObject::Lifetime` 分层（配合 `destroyObjectQueued`/`destroyQueuedObjects` 的延迟销毁，避免迭代中删除）——**这是唯一近似"池化"的结构，但不涉及内存复用**。

**平台抽象**。`GameWindow`（`rwgame/GameWindow.hpp:12-38`）极薄：`SDL_Window*` + `SDL_GLContext` + `swap()`。输入 `updateGameInputState(GameInputState*, const SDL_Event&)`（`GameInput.hpp:9`）。宽窄字符只在 Windows 路径存在：`rwcore/platform/RWWindows.{hpp,cpp}` 与 `rwengine/src/rw_mingw.hpp`（三行 `#undef IGNORE/near/far`）。配置 `RWConfig.inc` 用 **X-macro** 单点声明同时生成结构体成员、`boost::program_options` 选项、INI 键映射与 help 文本（`RWConfig.hpp:16-80` 连续 4 次 `#include "RWConfig.inc"` 配不同宏定义）。

### 三作共用代码与 profile 机制

**不存在 profile 机制**。证据链：
1. `grep -rni "profile"` 在 `rwengine/rwgame` 的命中全部是性能剖析（`Renderer::ProfileInfo`、`RW_PROFILE_*` 宏、microprofile），无一处指游戏配置档；
2. 只存在一个 opcode 模块 `GTA3Module`（`rwengine/src/script/modules/`），`RWGame.hpp:44` 直接 `GTA3Module opcodes;` 作为成员，构造 `ScriptMachine(file, &opcodes)` —— 无虚接口、无注册表、无按游戏分派；
3. `RWGame.cpp`/`RWGame.hpp` 里没有 `class GTA3Scene`/`ViceCityScene`/`SanAndreasScene` 之类；`rwgame/CMakeLists.txt` 也不产多目标；
4. `GameData::getPlatformString()` 硬编码 `return "PC";`（`GameData.hpp:66-68`），无 mobile/GOTY/10th-anniversary 变体分支；
5. 三作差异只以**注释与枚举残根**存在：`SCMFile.hpp:18` 的 `enum SCMTarget { NoTarget = 0, GTAIII = 0xC6, GTAVC = 0x6D, GTASA = 0x73 }` 是 SCM 头里的游戏目标字节，`SCMFile.cpp:14` 会把它读进 `_target`，**但 `_target` 从未被读取**（grep 确认仅两处赋值/声明）；`LoaderIMG.hpp:27-32` 的 `enum Version { GTAIIIVC, GTASA, GTAIV }` 明确注为 "GTA III and GTA VC archives -- only this one is implemented"；`GameData.cpp:234-235` 认得 `CAR3, ///< Used in GTASA, ... CAR4`；`RWGame.cpp:253-377` 的作弊码注释统一写 `// III / VC: ...`。

因此"三作共用"的正确表述是：**`rwcore` + `rwengine` 的格式解析与通用玩法层是可复用的，但缺少把这些零件按游戏装配起来的抽象层**。真正的多游戏抽象需要（a）`ScriptModule` 派生出 `VCModule`/`SAModule`、（b）`GameData` 文件清单参数化、（c）`SCMFile::_target` 参与分派、（d）`getPlatformString()` 真实化。这四点在 HEAD 均缺失 —— 与上游公开路线图一致。

### 测试与工程质量

- 框架：**Boost.Test**（非 gtest/catch）。`tests/main.cpp:1-2` 是 `#define BOOST_TEST_MODULE openrw` + `unit_test.hpp`。41 个 suite、141 个 `BOOST_*_TEST_CASE`。
- fixture：`tests/test_Globals.hpp:99-141` 定义单例 `Global`，构造时 `SDL_Init(SDL_INIT_VIDEO)` → 建 800×600 窗口 → `GameData(getGamePath())->load()` → `GameWorld` → `GameState` → 把重力设为 0。**因此"单元测试"实际要求 SDL2 视频 + OpenAL + 完整配置**，CI 用 `ALSOFT_DRIVERS: "null"` + `dbus-run-session` + 后台 `pipewire` 来喂平头环境（`.github/workflows/build-and-test.yml:11-14,38`）。
- 数据驱动测试：`DATA_TEST_PREDICATE`（`test_Globals.hpp:32-33`）= `label("data-test") * disabled()`，共 26 处（分布在 22 个文件）。**只有显式 `-DTEST_DATA=ON` 时才通过 `--run_test=@data-test` 跑**（`tests/CMakeLists.txt:98-102`），而 `getGamePath()` 从用户 `openrw.ini` 里取 `game.path`（`test_Globals.cpp:5-11`），CI 上没有正版数据 → **绝大多数格式解析正确性（DFF/TXD/GXT/SDT/IDE 实际字节）在 CI 里不被覆盖**，CI 覆盖的是合成对象逻辑。
- `SEPARATE_TEST_SUITES` 可把每个 suite 注册成独立 CTest（`tests/CMakeLists.txt:77-96`，`TIMEOUT 300`）；CI 里设 `SEPARATE_TEST_SUITES FALSE`。
- 静态分析：IWYU（含 2.7 KB 定制映射 `openrw_iwyu.imp`，为 GLM/Boost/Bullet/libav 的传递包含归一化）与 clang-tidy（`CHECK_CLANGTIDY`/`CHECK_CLANGTIDY_FIX`，`FORMAT_STYLE "file"`）均可选挂载；UB/ASan/LSan/TSan 通过 `rw_checks` 目标按需注入。
- 风格：`.clang-format` = Google 基线 + 4 空格 + `ColumnLimit 80` + `BreakConstructorInitializersBeforeComma`；`.editorconfig` 无；`scripts/verify-commit` 提交信息检查。文档 Doxygen（`Doxyfile`，`INPUT=.`、`EXCLUDE=tests`）。
- 警告强度很高：`-Wall -Wextra -Wdouble-promotion -Wpedantic -Wmissing-braces -Wold-style-cast`（`cmake_configure.cmake:8-16`）。**「推断」`-Wold-style-cast` 与解析器里大量 `reinterpret_cast`/`static_cast` 并存，靠 `RW_CHECK` 与严格审查而非编译期保证布局安全。**
- 覆盖率：`TEST_COVERAGE` + `cmake/modules/CodeCoverage.cmake` + llvm-cov wrapper，直传 codecov（`.codecov.yml` 关掉 `require_ci_to_pass`），生成物 `gl_core_3_3.*` 被 `COVERAGE_EXCEPT` 排除。
- 脚手架 vs 逻辑比例：`rwengine/src/data`（804 cpp : 1,341 hdr）、`script`（613 : 1,116）、`engine`（4,612 : 1,769）、`objects`（2,962 : 1,379）—— 数据/类型定义层头文件偏重（正常），而真正的"空壳布局镜像"极少；换言之 OpenRW 头文件多为**接口与 POD 数据定义**，不像 re3 那样是"为了对齐偏移而写的死结构体墙"。

### 成熟度与缺口表

| 子系统 | 状态 | 证据 / 缺口 |
|---|---|---|
| DFF/Clump 解析 | **可用**，含 BinMesh PLG、MatList、NodeName、法线重建 | `LoaderDFF.cpp:386-419,452-510`；缺 HAnim 插件（`SID_HAnimPLG` 定义但 0 引用）、缺 morph target、未处理端序（`LoaderDFF.cpp:39` `@todo worry about endianness.`） |
| TXD/DXT/调色板纹理 | **可用** | `LoaderTXD.cpp:16-96`（含 4 张纯色 debug 纹理兜底） |
| IMG 归档 | **部分**：仅 III/VC 版 | `LoaderIMG.hpp:27-32`；线程不安全（`LoaderIMG.hpp:47-49`） |
| IDE / IPL / COL / GXT / IFP / waterpro | **可用**（GTA III 方言） | `rwengine/src/loaders/*`；`CollisionModel.hpp:22` `@todo give shapes surface data` |
| 渲染 | **可用**：GL3.3、雾、alpha/additive 双队列、水面 HQ/LQ、minimap、文字、letterbox、splash | `GameRenderer.cpp`、`MapRenderer.cpp`、`WaterRenderer.*`；无阴影、无法线映射、材质属性硬编码为默认（`OpenGLRenderer.hpp:108` `// Default state -- should be moved to materials`） |
| 物理 | **可用**：车辆 `btRaycastVehicle`、角色胶囊、自写浮力、crane | `VehicleObject.cpp:810`、`CharacterObject.cpp`；碰撞面 `material/flag` 未反馈伤害差异 |
| 脚本 VM | **结构完整**，opcode **约 57% 实现**：904 个 handler 中 389 个含 `RW_UNIMPLEMENTED_OPCODE` | `grep -c` 于 `GTA3Module.cpp`(bind=904) 与 `.inl`(389)；Release 下桩体**静默无输出**（`GTA3Module.cpp:7-12`） |
| AI / 交通 | **基础可用**：图寻路、`TrafficDirector` 密度/上限、车道 `leftLanes/rightLanes` | 缺警车战术、缺 ped 反应/群组行为细化 |
| 音频 | **可用**：SFX(SDT/RAW+FFmpeg)、流式 buffer | 电台/音乐轨不完整（`SoundManager.cpp:36` `@todo reloading`） |
| 存档 | **只读不写** | `SaveGame.cpp:499-503` `RW_UNIMPLEMENTED("Saving the game is not implemented yet.")`；`SaveGame.cpp:1262-1263` `// TODO restore garage data`；`:1289` `@todo determine where the remainingSpawns should be` |
| 流式加载 | **未实现** | `ModelData.hpp:84`、`GameWorld.cpp:141`、`GameData.hpp:42` 三处 `@todo`；`LoadingState.cpp:15-20` 假异步 |
| 多游戏（VC/SA） | **未实现** | 见"三作共用"节 |
| 移动端资产变体 | **未实现** | `GameData.hpp:66-68` `return "PC";` |
| RenderWare 内存模型镜像 | **主动不做**（设计选择） | 0 处 `rpBaseClass`/plugin/自定义分配器；仅 1 处 `static_assert` |
| 整体 TODO 密度 | 245 处 `@todo|TODO|FIXME|XXX`；`RW_UNIMPLEMENTED` 家族共 404 处（其中 389 在 opcode 表） | grep 统计 |

结论：以"能跑通 GTA III 主线"衡量，静态资产管线与玩法核心已达**可玩级**（README 有开场动画/驾驶/城景截图佐证），最大缺口是**写档、流式加载、约 43% 的 opcode**，其次是 VC/SA 抽象层完全缺席。

### 资产依赖

OpenRW 必须外挂正版 GTA III PC 数据目录，运行期零内嵌资产（`assets/` 目录只有 `logo_box.svg` 一个构建产物图标）。

- 入口：`--gamedata PATH`（`RWConfig.inc` 的 `RWCONFIGARG(std::string, gamedataPath, ..., "game.path", CONFIG, "gamedata", "PATH", "Path of gamedata")`）→ `GameData(Logger*, const std::filesystem::path&)`；配置文件本身按 XDG/macOS `~/Library/Preferences`/Windows `%APPDATA%` 定位（`RWConfig.cpp:177-208`）。
- 目录合法性只检查"是目录且无 error_code"（`GameData.cpp:768-775`），**不做版本/校验和判定**，也不检测 Steam/GOG/Rockstar 安装器路径 —— 装错版本只在后续 `logger->error("Data", "Failed to open game file " + path)` 暴露（`GameData.cpp:354-356`）。
- 具体读取清单见"关键子系统 → 资产装载"节。`FileIndex::indexTree` 先递归建盘上索引，之后 `indexArchive` 把 IMG 内成员并入同一命名空间（`FileIndex.hpp:28-60`），使 `index.openFile("landstal.dff")` 与 `findFilePath("audio/sfx.SDT")` 同构 —— 这是它对"大小写敏感文件系统 + 归档内文件名"两个问题的一次性解法（`normalizeFilePath`）。
- 存档目录复用原游戏的 `GTA3 User Files/*.b`；Windows 经注册表解析"我的文档"（`SaveGame.cpp:1325-1364`）。
- 因 `getPlatformString()` 恒为 `"PC"`，移动版（`.opcodes`/`.aif`/DDS 纹理/`mobile` 布局）**不受支持**。

### 逆向来源标记

OpenRW 不用地址/符号做标注，而用**文献引用 + 常量自证 + 未归类区段命名**三种方式：

| 类型 | 位置 | 内容 |
|---|---|---|
| 显式致谢 wiki | `rwcore/loaders/RWBinaryStream.hpp:85-86` | `Many thanks to http://www.gtamodding.com/index.php?title=RenderWare_binary_stream_file` |
| 显式 wiki  opcode 页 | `rwengine/src/render/MapRenderer.cpp:199-200` | `// Colours from http://www.gtamodding.com/wiki/0165 (colors not specific to that opcode!)` |
| 显式 wiki 引用 | `rwengine/src/engine/GameWorld.cpp:988` | `// According to http://www.gtamodding.com/wiki/014C the spawn limit doesn't work.`（随后 `#if 0` 掉原逻辑） |
| 显式论坛文档 | `rwengine/src/engine/SaveGame.cpp:1263` | `// http://gtaforums.com/topic/758692-gta-iii-save-file-documentation/` |
| 行为复刻声明 | `rwengine/src/script/ScriptFunctions.hpp:263-265` | `// This is how the original game calculates distance, weighted manhattan-distance where the vertical distance has to be 3x as close` |
| 数值取自原版 | `rwengine/src/engine/Garage.cpp:120-121`、`Garage.hpp:17,56-57`、`PlayerController.cpp:182`（`// Original game uses 4000.f radius`）、`RWGame.cpp:562`（`// Original game uses milliseconds`） | `// back to non swing just like in original game / Values are from original game`、`// Garage types are from original game` |
| 魔数（chunk ID） | `RWBinaryStream.hpp:113`、`LoaderDFF.cpp:19-35` | `SID_NodeName = 0x0253F2FE`（RW 的 Atomic/NodeName 插件 ID）、`CHUNK_NODENAME` |
| 魔数（文件签名） | `LoaderCOL.cpp:13` | `constexpr uint32_t kCollMagic = 0x4C4C4F43;`（即 `"COLL"`） |
| 魔数（版本比较） | `LoaderDFF.cpp:179` | `if (geomStream.getChunkVersion() < 0x1003FFFF) { headerPtr += sizeof(RW::BSGeometryColor); }` —— RW 3.3→3.5 的几何头差异 |
| 硬编码文件偏移 | `GameData.cpp:323-327` | `ifstr.seekg(0x03C4);` 之后 `read(&visibleWater, 64*64)`、`read(&realWater, 128*128)`，`waterHeights` 固定 48 float |
| 硬编码块大小 | `SaveGame.cpp:571` | `if (scriptDataBlockSize != 0x03C8) { ... }` |
| 尺寸断言 | `SaveGame.cpp:549` | `static_assert(sizeof(BasicState) == 0xBC, ...)` |
| IFP 四字符码 | `LoaderIFP.cpp:65-120`、`LoaderIFP.hpp:39` | `ANPK`/`NAME`/`DGAN`/`CPAN`/`ANIM`/`KFRM` + `"KR00"/"KRT0"/"KRTS"` → `enum Data { R00, RT0, RTS }` |
| 脚本编码常量 | `ScriptMachine.hpp:20-28`、`ScriptTypes.hpp:178-186`、`ScriptMachine.cpp:157,179-183` | `0x8000`/`0xFF`/`SCM_THREAD_LOCAL_SIZE 256`/`SCM_VARIABLE_SIZE 4`（"Changing this will break saves"）、`0x00D6` |
| 存档枚举目标字节 | `SCMFile.hpp:18` | `GTAIII = 0xC6, GTAVC = 0x6D, GTASA = 0x73` |
| 逆向诚实标注 | `AIGraphNode.hpp:16-17`、`SaveGame.cpp:59,81,86,92` | `/// No documentation for other flags yet, but this is mentioned.`；`unknown[0x100]` / `unknown1[640]` |

**未出现**：任何 `0x00[0-9A-F]{6}` 形态的 exe 函数地址、`// ida`、`ghidra`、`decompile`、原二进制符号名（`grep -rni "IDA Pro\|ghidra\|decompil"` → 0 命中）。这与"不注入原 exe、不要求原版二进制"的许可定位一致（GPL 洁净室边界，`CONTRIBUTING.md` 只指向 wiki）。

### 对本报告其他项目的启示

1. **两条逆向路线可直接对照**：OpenRW = *格式驱动 / 数据可验证 / 无需原始二进制*；re3 系 = *内存布局驱动 / 偏移可验证 / 需要原始 exe*。前者产出可自由分发的 GPL 引擎，后者产出必须搭配原程序的二进制补丁。`rwcore/loaders/RWBinaryStream.hpp` 与 re3 的 `Rp*` 头文件对同一 RW 概念的处理差异，是报告里最好的"方法论对照"素材。
2. **`GLM_FORCE_PURE` 的隐式布局依赖**（`cmake_configure.cmake:75` ↔ `LoaderDFF.cpp:45-51`）值得作为反面教材：布局不变量应写成 `static_assert(sizeof(RWBSFrame) == 68)`。OpenRW 全项目只有 1 个 `static_assert`，说明"格式镜像"缺一道自动化护栏，而 re3/ReClass 类项目通常大量使用断言。
3. **用生成器 + 外部规格（INI+XML）批量铺 opcode handler** 是可移植的打法：`scripts/generate_scripts_functions.py` + `GTA3ModuleImpl.inl`（12,928 行）+ `.cpp` 侧 904 条 `bind()` 的组合，让"规格更新"与"逐条填充实现"解耦；配合 C++14 模板元 `script_bind::call_unpacked` 自动展开变长参数、并按返回类型 `bool` 特化写入条件结果，是同类脚本 VM（CLEO、FreeRoam、pylama 系）里最干净的样板之一。
4. **`FileIndex` 的双源统一寻址**（磁盘 or IMG 归档，小写归一化）是任何重制项目都会撞上的第一个非趣味性问题，OpenRW 的解法（单一 `unordered_map<string, IndexedData>` + `IndexedDataType{FILE,ARCHIVE}`）简洁且值得抄。
5. **诚实的缺口标记值得制度化**：`RW_UNIMPLEMENTED` / `RW_UNIMPLEMENTED_OPCODE` 与 `unknown[NNN]` 命名法把"逆向未完成"编码进类型系统与构建期日志。但需注意其反模式：**Release 下 `RW_UNIMPLEMENTED_OPCODE` 展开为空**（`GTA3Module.cpp:7-12`），静默 no-op 会让 mod 脚本或任务在发行构建里"无声失败"，这是所有重制项目都应避免的默认值。
6. **CI 依赖数据文件时的分层测试策略**：`DATA_TEST_PREDICATE = label("data-test") * disabled()` + `TEST_DATA` 选项，让"无资产可跑的单元测试"与"需正版资产的格式测试"在同一个 Boost.Test 二进制里共存，可被任何"必须依赖版权数据"的重制项目直接借鉴（搭配 CDash 与 `ALSOFT_DRIVERS=null`/`dbus-run-session` 的无头技巧）。

## 3.2 re3 / reVC（GTAmodding/re3）—— 被 DMCA 屏蔽的"最著名重写"

### 3.2.1 本章的性质声明（重要）

**本报告没有 re3/reVC 的源码，也不声称对其内部结构做过源码级分析。** 该仓库在 GitHub 上是不可获取状态，我主动放弃了一切镜像/种子获取路径，理由见 3.2.3 与 4.3。因此 3.2 是全篇唯一的**文献级**章节，其结论边界如下：可以断言"它存在过、目标是什么、法律状态如何、周边依赖是什么"；**不可以**断言"它的某个模块有多少行、用了什么断言手法"。凡本节未给出可核验来源的句子，一律不写。

### 3.2.2 实测确认的技术事实（2026-11-18 本机执行）

```text
$ curl -sS https://api.github.com/repos/GTAmodding/re3          → HTTP 451
{ "message": "Repository access blocked",
  "block": { "reason": "dmca",
             "created_at": "2021-09-30T17:33:39Z",
             "html_url": ".../github/dmca/.../2021-09-29-take-two-legal-action.md" } }

$ git ls-remote https://github.com/GTAmodding/re3 HEAD         → 失败（要求凭据／不可读）
```

| 事实 | 值 | 证据 |
|---|---|---|
| 规范仓库 | `github.com/GTAmodding/re3`；**reVC 是同一仓库的 `miami` 分支** | 核验 agent 的 API/org 记录 + 长期引用一致性 |
| 当前状态 | **HTTP 451，`block.reason = "dmca"`，屏蔽时间 2021-09-30 17:33:39Z** | 本机 curl 实测（上面原文） |
| 屏蔽依据 | GitHub DMCA 索引中的 Take-Two legal action 通知（API 返回的 `html_url`） | 同上；注意该 URL 的 raw 变体本次返回 404，仅作为官方索引引用 |
| 无 OSI 许可证 | AUR 打包记为 `LicenseRef-None` | AUR `re3-git` / `revc-git` |
| 上游依赖 | RenderWare 重实现由同作者的 **librw** 承担（MIT，活跃至 2026-08-26，843★）—— 本报告已克隆并分析（3.7） | `github.com/aap/librw` |
| 技术定位 | 目标为**二进制等价反编译**：逐函数重写 GTA III/VC 的 Retail 代码，以换取跨平台移植（RPi、Switch、WiiU、3DS、DOS/WASM 等端口全部建立在此之上） | 公开报道（GamingOnLinux 2021-02；Eurogamer 2021）+ 大量下游移植端的存在性 |
| 时间线 | 研究起点约 2016 → 2018 春改为完整引擎逆向 → 2021-02 公开发布 → 同日启动 DMCA 流程 | 见 4.3 执法时间表 |

### 3.2.3 为什么"取一份镜像"是不对的选择

核验过程中确实发现 archive.org 上有一份 2021-09-06 的 git bundle 快照，并且 Arch Linux 的 `re3-git`/`revc-git` 包正是固定从该快照构建（sha256 钉死）。技术上可用，但：

- 该仓库是被**权利人主张后**由 GitHub 以 451 屏蔽的，且屏蔽发生在反通知恢复之后的**第二轮**处置（4.3 节有完整时序）。主动绕开屏蔽去取归档副本，属于规避版权处置，我不会做，也不建议本项目后续这么做。
- 阅读它的**外围材料**完全合法且信息量足够：librw（同作者的 RW 重实现，MIT）、OpenRW（另一条独立重实现路线，GPL-3.0）、DMCA 通知本身、媒体报道、下游移植端的公开讨论。本报告的分析目标——"一个 RenderWare 时代的引擎被逆向到什么程度、付出了什么代价"——由 librw + OpenRW 两个开源件即可达成，不需要 DMCA 件。

### 3.2.4 本节最有实操价值的发现：仿冒镜像的指纹差别

GitHub 上现存多个自称 re3/reVC 的"镜像"（`hottabxp/re3`、`mrxenginner/re3`、`mrxenginner/reVC`、`Jai-JAP/re-GTA`、`leanndroguedes/re3-19feb21` 等，星数 36–514）。核验 agent 做了一项关键取证：这些仓库的 README 是**未改动的 re3 README 副本**，但里面 advertised 的 Discord 邀请码与真实血统的邀请码**不一致**（真：`ERYg58ttcE`；仿冒批次：`RFNbjsUMGg`）。

这是一条可以直接复用的判别规则：**仿冒仓库会照抄 README 正文，但常常替换社交/下载入口**。任何人后续要碰 re3 血统的东西，先比对邀请码/域名，而不是比对文件名或星数。同理，`knight-oss/gta2-reverse-engineering-lab` 与 `btshahraag20/gta2-memory-master` 被取证为"仓库里只有 `README.md`/`index.html`/两三个 `.svg`，README 自称 C++/Python/Lua、覆盖率 93%，而 Download 徽章指向一个 GitHub Pages 上的**单一 base64 不透明 blob**"，提交时间 2026-11-18（当天）——教科书级投递诱饵。

### 3.2.5 OpenRPA：本项目原计划在列，但**当前不存在可核验的官方源**

任务书里我原本要收的第六个开源项目是 OpenRPA（GTA:SA 引擎重建 + 多人层，署名 Velostream）。实测结论必须如实写：

| 检查 | 结果 |
|---|---|
| `github.com/Velostream/OpenRPA` | **404** |
| `Velostream` 账号 | 存在（user id 53464857），**public repos = 0** |
| GitHub 检索 `"OpenRPA" gta` / `"OpenRPA" "San Andreas"` | **0 结果** |
| Wayback CDX（该仓库及 `open-r.pw`） | **零抓取记录**；`open-r.pw` 经 DoH 解析为 NXDOMAIN |
| SourceForge / GitLab / Codeberg | 无同名项目 |
| `open-rpa/openrpa` | 同名但完全无关（企业 .NET RPA 工具） |

**判定：本报告不收录、不克隆任何自称 OpenRPA 的东西**；SA 侧的引擎重写分析改由已核实的 OpenRW（3.1）与 librw（3.7）承担，两者共同覆盖了同一条技术路线。这条"看起来很有名但今天拿不到源"的记录本身就是第 4 章执法史的一个注脚。

### 3.2.6 一条与本节相关的安全观测（附我的复核结论）

核验 agent 在其会话中报告本机对 `github.com` 的解析出现过 `172.182.252.133`（Microsoft 段）而非 GitHub 自有段，并因此改用 `curl --resolve` + raw.githubusercontent + Wayback 交叉验证。**我在同一环境复核时未复现**：

```text
getent hosts github.com → 140.82.114.4        （GitHub 自有段）
Google DoH              → 140.82.112.4
```

结论：不排除该 WSL 环境存在**间歇性透明 DNS/代理介入**（agent 的绕行做法在这种情况下是正确习惯），但"当前被污染"这一点我**不予断言**。本报告全部 URL 类事实都以 3.2.2 那样的可复现命令为准。另需注意：本环境的 WebSearch 通道返回大量 AI 农场/SEO 结果，Bing/Mojeek/DuckDuckGo 对自动化请求有反爬拦截 —— 这也是 1.3 节"不采信搜索排名"的根因之一。

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

## 3.4 CodeWalker（dexyfex/CodeWalker）

### 项目定位
CodeWalker 是 dexyfex 用 C# 编写的 GTA V / RAGE 引擎数据离线解析与重打包套件，同时是一个基于 SharpDX + Direct3D11 的世界/模型查看器与地图编辑器。它在本报告语境中的价值不在渲染，而在于它把 RAGE 的容器格式（RPF/RSC）、类型元信息（META/PSO/RBF）与资源类布局（drawable/bounds/clip/nav）完整地"可执行化"了：既能解码，也能重新编码成游戏可加载的文件。`README.md:1-8` 自述 scope 为 "viewing the contents of GTAV RPF archives"，并要求 DX11/SM4.0、Windows x64、.NET 4.5+。

### 仓库元数据
- 浅克隆：`.git/shallow` 存在，`git rev-list --count HEAD` = **1**，无 tag（`git describe` 失败），HEAD = `485d56be`（2025-04-11，作者 dexy，"ModManager progress"），remote `https://github.com/dexyfex/CodeWalker` master。
- **无 LICENSE 文件**。仅有 `Notice.txt`（内嵌 Neodymium 2015 / Cameron Berry 2017 的 MIT 片段、hamish-milne FbxWriter 的 GPL 声明）与 `Readme_Src.txt`："This source code is released for educational purposes only."（推断：整体无 OSI 许可证，代码版权实际保留）。
- 源码 vs 附带二进制：克隆内无任何 DLL/EXE/ASI；随源码提交的二进制只有 `Shaders/*.cso` 90 个（由 `CodeWalker.Shaders/CodeWalker.Shaders.vcxproj:79` 的 `ObjectFileOutput` 编译 HLSL 产生）、`CodeWalker.Core/Resources/magic.dat`（154 KB 加密密钥块，`CodeWalker.Core.csproj:17` 作为 `<Resource>` 嵌入）、图标与 92 个 `.resx`。发布包中的 DLL 不在源码树（推断）。

### 解决方案与模块清单（行数）
`CodeWalker.sln` 含 10 个项目：`CodeWalker.Shaders`(vcxproj/HLSL)、`CodeWalker.Core`(netstandard2.0)、`CodeWalker`(net48 WinExe x64)、`CodeWalker.WinForms`、`CodeWalker.ErrorReport`、`CodeWalker.Peds`、`CodeWalker.RPFExplorer`、`CodeWalker.Vehicles`、`CodeWalker.ModManager`、`CodeWalker.Gen9Converter`。**仓库中不存在 `Common.Libraries.props` / `Version.props`**，也不存在 `CodeWalker.Cmd` / `CodeWalkerDoc` 项目（题设假设需修正）。NuGet 依赖：`SharpDX 4.2.0` 全家桶（D3D11/Direct2D1/D3DCompiler/XAudio2/XInput/Mathematics）、`DockPanelSuite.ThemeVS2015 3.0.6`、`FCTB 2.16.24`；**无 OpenTK、无 System.Buffers**。C# 总量 392 文件 / 327,047 行（其中 `.Designer.cs` 44,858 行）。

| 模块 | 文件 | 行 | 职责 |
|---|---|---|---|
| `CodeWalker.Core/GameFiles`（根） | 4 | 9,499 | `RpfFile.cs` 2,882 / `RpfManager.cs` / `GameFileCache.cs` 5,984 / `GameFile.cs` |
| `GameFiles/FileTypes` | 41 | 53,231 | 每种 `.y*` 扩展名的入口解析器 |
| `GameFiles/MetaTypes` | 13 | 53,734 | META/PSO/RBF + `MetaNames.cs` 哈希名字表 |
| `GameFiles/Resources` | 21 | 41,600 | RSC 资源类布局（Drawable/Bounds/Clip/Frag/Nav/Texture…） |
| `GameFiles/Utils` | 5 | 5,452 | `GTACrypto` `GTAKeys` `Jenk` `Data` `DDSIO` |
| `CodeWalker.Core/World`+`Utils` | 28 | 13,800 | 世界模型（Space/Timecycle/Water/Vehicle）与 BVH/FBX |
| `CodeWalker`（查看器/UI） | 223 | 137,028 | WorldForm/ExploreForm/Project 编辑器/渲染 |

### RAGE 资源格式全景
| 格式 | 解析器 | 载体 | 解码 | 回写 |
|---|---|---|---|---|
| `.rpf` | `GameFiles/RpfFile.cs` | 容器 | 完整 | 完整（含重建 TOC、扩档、碎片整理） |
| `.ytd/.gtxd` | `FileTypes/YtdFile.cs`+`Resources/Texture.cs` | RSC | 完整（BC7/DXGI、Gen9 `ShaderResourceViewG9`） | 有 `Save()` |
| `.ydr/.ydd/.yft` | `Ydr/Ydd/YftFile.cs`+`Drawable.cs` 7,190 行、`Frag.cs` | RSC | 完整（ShaderGroup/VertexDeclaration/IndexBuffer/Skeleton） | 有 |
| `.ybn` | `YbnFile.cs`+`Bounds.cs` 5,488 | RSC | 9 种 `BoundsType` 全含 BVH（`Bounds.cs:115,2508`） | 有编辑器面板 |
| `.ymap` | `YmapFile.cs` 3,666 | META | 完整 | 有，但 2 类数据丢失（见下） |
| `.ymt` | `YmtFile.cs` | META/PSO/RBF 三态嗅探（`YmtFile.cs:84-120`） | 场景区域完整 | 部分 |
| `.ynd` | `YndFile.cs` 1,913 | RSC `NodeDictionary` | 完整 | 有 `Save()` |
| `.ynv` | `YnvFile.cs` 1,406 + `Resources/Nav.cs` | RSC | poly/point/portal/edge | `Builders/YnvBuilder.cs` 可**生成** |
| `.ycd` | `YcdFile.cs`+`Clip.cs` 4,466 | RSC v46 | 完整 | 有 |
| `.ymt/.ytyp/.cut` | `MetaTypes/*` | META | 完整 | 有 |
| `.rel`（音频） | `RelFile.cs` **24,909** | 自有容器 | 含脚本 VM 反汇编 | 8 处 TODO |
| `.awc` | `AwcFile.cs` 3,169 | 二进制 | 解码+PCM 合成 | 有 |
| `.fxc` | `FxcFile.cs` 1,896 + `CodeWalker/GameFiles/FxcParser.cs` | DXBC | 含反编译文本 | 否 |
| `.mrf`（move network） | `MrfFile.cs` 5,613 | `MoVE` | 部分（`NotImplementedException` ×5） | 否 |
| `.ypdb`（AI pose db） | `YpdbFile.cs` 411 | 二进制 | `PoseMatcherMatchSample`/`WeightSet` | 否 |
| `gta5_cache_y.dat` | `CacheDatFile.cs` 1,074 | 文本+结构 | LOD 层级/interior/bounds store | — |
无独立 `ShaderGroup.cs`/`PhysicsSystem.cs`/`Gxt2.cs`/`PagedFile.cs`：ShaderGroup 在 `Drawable.cs:14`，物理在 `Frag.cs`，GXT2 为 `Gxt2File.cs`，分页语义由 `RpfResourceFileEntry` + `ResourcePagesInfo` 承担。

### RpfArchive 深剖
头部 16 B：`RPF7` 版本、EntryCount、NamesLength、Encryption 四类（`RpfFile.cs:140-143`；枚举 `RpfFile.cs:2147-2152`：`OPEN=0x4E45504F`、`AES=0x0FFFFFF9`、`NG=0x0FEFFFFF`）。TOC 与名字表整体用密钥解密后，逐条 16 B 表项按第二个 uint 判别类型：`0x7fffff00`=目录、bit31=0=二进制文件、否则资源文件（`RpfFile.cs:192-208`）；名字通过 `NameOffset` 从 names blob 取 NUL 串（`RpfFile.cs:216-217`），再由 `EntriesIndex/Count` 栈式遍历生成路径树（`RpfFile.cs:239-268`）。资源表项为 24-bit `FileSize` + 24-bit `FileOffset`（**扇区=512 B**，`StartPos + FileOffset*512`）+ `SystemFlags/GraphicsFlags`；当 `FileSize==0xFFFFFF` 时真实长度藏在 RSC 头 16 字节的第 `7,14,5,2` 字节里，源码原话 `//just....why`（`RpfFile.cs:2650-2660`、`1683-1690`）。页大小被压缩进 32-bit flags：低 4 位 `BaseShift` 给 `BaseSize=0x200<<shift`，其后 1/2/4/6/7 bit 五档页计数（上限 `0x7F,0x3F,0xF,3,1`）与 4 个 tail 位，高 4 位是版本（`RpfFile.cs:2732-2760`）。
加密：AES 路径是 **Rijndael-256/128-ECB 无填充**（`GTACrypto.cs:47-68`）；NG 路径是 17 轮查表密码，密钥按 `(jenk(name) + length + 61) % 0x65` 从 101 组扩展密钥中选取（`GTACrypto.cs:96-101`）。**仓库不含密钥本身**：`GTAKeys.cs:22-24` 明确 "This file contains only SHA1 hashes of all interesting values. There is NO key."，运行时以 1 MB 并行块、8 字节对齐在 `GTA5.exe` 中扫 SHA1 匹配来定位 AES key、101 组 NG 密钥、17×16×256 解密表与 `PC_LUT`（`GTAKeys.cs:71-152`，`HashSearch` 在 `740-780`，共 375 个 SHA1 字面量）；加密表则由解密表反解：`RandomGauss.Solve()`（GF(2) 线性求逆）与 `LookUpTableGenerator.BuildLUTs2()`。结果被 deflate + AES + 以 `JenkHash(PC_AES_KEY)` 为种子的 4 路伪随机流加性混淆成 `magic.dat`（`GTAKeys.cs:228-244` / `264-313`）。写档同样完备：`WriteHeader`（`958-1003`）、`GrowArchive`/`RelocateFile`/`FindHole`/`Defragment`/`SetEncryptionType`。

### 分块标签与哈希机制
GTA V 的 RSC 资源不用 chunk fourcc 表，而是**虚地址 + 类型标签**：`ResourceDataReader` 用 `SYSTEM_BASE=0x50000000`、`GRAPHICS_BASE=0x60000000` 两段流做统一寻址（`ResourceData.cs:45-46,127-167`），`blockPool` 按偏移去重（`:172-216`），多态块经 `IResourceXXSystemBlock.GetType()` 以枚举/`VFT` 分派（`Bounds.cs:494-517`）。每块首字段是 `VFT`——原样保存的 C++ 虚表指针：`ShaderGroup.VFT = 1080113136`（小端即 `"0ET@"`，`Drawable.cs:22`），`Texture.cs:1342-1370` 甚至列出实采的 exe 虚表 RVA 集合 `0x1406b77d8 / 0x140695e58 / …`，`Clip.cs:3060`、`Frag.cs:3980` 同类。名字侧才是"known-hash 表"：`MetaTypes/MetaNames.cs` 的 `enum MetaName : uint` 含 **20,239** 条 `名字 = jenkins哈希`，其生成器 `MetaNamesUtil.GetNameEnumCode()`（`:28-48`）与 `MetaTypes.GetTypesString()`（`MetaTypes.cs:116-200`）把运行时从游戏 typeinfo 读到的键反查为源码；`JenkIndex`（`Jenk.cs:199-262`）在解析过程中自学习（`RpfManager.cs:353+` 扫描全部 RPF 名字、`YmapFile.cs:114` "just shove them in there"）。

### MetaBase 反射式类布局镜像
本版本已无 `MetaBase.cs`（题设需修正）：改为 **显式 struct + 手写 typeinfo 表** 的双轨镜像。游戏 META 文件自带 `MetaStructureInfo/MetaEnumInfo`（`Meta.cs:231-307`、字段类型码 `MetaStructureEntryDataType`：`Hash=0x4A, Array=0x52, Structure=0x05, Float_XYZ=0x33 …`，`Meta.cs:331-355`），`MetaTypes.EnsureMetaTypes()` 把每张 `.ymt` 的 typeinfo 汇入全局 `StructDict/EnumDict` 并做一致性比对（`MetaTypes.cs:30-70`）。C# 侧结构体注释直接写着 `//128 bytes, Key:1825799514`（`CEntityDef`，`MetaTypes.cs:2766`），配合 `[TC(typeof(EXP))]` 仅用于 PropertyGrid 展示，序列化靠 `Marshal.StructureToPtr/GCHandle` 零拷贝（`MetaTypes.cs:1437-1520`）；`MetaBuilder.GetMeta()` 再把 `GetStructureInfo(MetaName)` 里的**手写**布局表（`MetaTypes.cs:297-…`，含偏移、类型、引用键）写回文件的 StructureInfos 段，使游戏能解释这些块。`MetaWrapper` 抽象类（`MetaTypes.cs:2085-2095`）定义 `Load(Meta, MetaPOINTER)` / `Save(MetaBuilder)`，即 36 个 `MC*` 包装类的双向映射入口。

### 序列化往返保真度
解码/编码严格成对：每个块 `Read()` 逐字段读、`Write()` 先把指针与计数回算再写（如 `ShaderGroup` 的 `ShadersCount1/2`、`ShaderGroupBlocksSize`，`Drawable.cs:57-127`）。未识别字节一律保留为 `Unknown_XXh` 原值（Core 中 **822** 处）并原样写回，属"保真优先"策略。写出时 `ResourceBuilder.Build()` 对每个块断言 `pos_after-pos_before == BlockLength`（Gen9 用 `BlockLength_Gen9`，20 处，如 `Texture.cs:887-888` 144↔128），否则抛 "error in system length"（`ResourceBuilder.cs:558-585`），并按页装箱算法（`:331-430`）重算 `System/GraphicsFlags` 后拼 `RSC7` 头。META 侧条目强制 16 B 对齐以免指针带子偏移（`MetaBuilder.cs:47-63`）。缺口有自我披露：`YmapFile.cs:685-686` `LogSaveWarning("containerLods were not saved. (TODO!)")`；`RpfFile.cs:1708` `//TODO: binary encryption`。

### 运行时与离线分界
全部为离线进程：`Program.cs:20-50` 解析 `menu/explorer/project/vehicles/peds` 参数选择窗体；`CodeWalker/Rendering/DirectX/DXManager.cs` + `ShaderManager.cs` 加载 `Shaders/*.cso`；世界流送用 `GameFileCache`（`ConcurrentQueue` 请求队列、`Cache` LRU 512 MB/10 s，`Utils/Cache.cs:13-14`）+ `gta5_cache_y.dat` 的 LOD 树（`GameFileCache.cs:1251-1270`）；编辑面板含 ymap entity/cargen/grass/lodlight、ynd、ynv(poly/point/portal)、ybn、ytyp(MLO room/portal/entityset)、scenario ymt、train track、audio zones。仓库内**无任何注入式 .asi/C++ 游戏内组件**，`ExploreForm.cs:2269` 反而提示"Loading by the game will require a mod loader such as OpenRPF.asi or OpenIV.asi"。`CodeWalker.ModManager`(1,825 行) 只做 mods 文件夹安装；`Gen9Converter` 调 `Tools/ConvertAssetsForm` 做 PC→Enhanced 资源转换（`ShadersGen9Conversion.xml` 为手建参数名映射表）。

### 逆向来源标记
`Notice.txt`（Neodymium/Cameron Berry/hamish-milne）；`RelFile.cs:16-20` "adapted from CamxxCore's RageAudioTool"；`Todo.txt` 引用 gtaforums timecycle 帖与 RageAudioTool 仓库；`DDSIO.cs:2`（Microsoft/D3DX 派生）；`MenyooXml.cs:32-33` 引用 me2ymap/Guad MapEditor；`GTAFolder.cs:151` 读 OpenIV 注册键；`Bounds.cs:761,836`、`GameFileCache.cs:609` 等大量 "hack/TODO" 自述；虚表 RVA 常量（`Texture.cs:1342`）与 `(MetaName)3844724227` 这类未命名哈希（`MetaTypes.cs:351`）是最直接的"来自二进制"证据。

### 成熟度与缺口
| 级别 | 项 |
|---|---|
| 完整（读+写+UI 编辑） | RPF/RSC 容器、AES+NG 双向、ytd/ydr/ydd/yft/ybn/ycd/yld/ymap/ytyp/ynd/ynv(+ynv 生成器)/ymt 场景区 |
| 读优先、写弱或缺失 | rel(音频 VM)、mrf、awc 合成、fxc 反编译、ypdb、gxt2、heightmap/watermap |
| 明确未做 | 非 META 的 ymap/ped 变体（`YmapFile.cs:239`）、containerLods、二进制条目加密、NavMesh 编辑器之外的 collision 编辑（`Todo.txt` 列表） |
量化：Core 内 TODO 124 处、`Unknown_` 字段 822 个、`NotImplementedException` 10 处（集中在 Mrf/ResourceBaseTypes）。

### 对 GTA VI 逆向的方法论启示
1）"哈希表即格式"：把 20,239 条 jenkins 名表与 375 条 SHA1 密钥指纹作为可再生资产维护，密钥不进仓库、由目标 exe 自举提取——同样适用于 Gen9/Next 的密钥轮换。2）类型元信息优先：GTA V 的 META 自带结构描述，因此先做"运行时自描述→自动生成 C# 布局"的工具链（`GetTypesString()`），比手猜字段可靠。3）保真写回是验证手段：以 `BlockLength` 断言 + 16 B 对齐 + 页装箱重算，能立刻暴露布局错误；`magic.dat`/`.cso` 的"源码+少量二进制"分层分发也值得沿用。4）虚表 RVA 集合可作为跨版本识别锚点（推断）。

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

## 3.7 librw（aap/librw）—— RenderWare 本体的重实现

本节分析 `/root/orca-workspaces/gta-repos/librw`（远端 `https://github.com/aap/librw`，浅克隆仅 1 个提交 `18532c2`，日期 2026-08-26，作者 `aap <aap@papnet.eu>`，`LICENSE` 为 MIT/(c) 2014 aap）。librw 的目标写在 `README.md:4-5`：「a re-implementation of RenderWare graphics, or a good part of it anyway」——它不是游戏引擎，而是 **RenderWare 3 库 API 本身**的重写，是 re3/reVC 那条线的图形依赖底座。`ARCHITECTURE.MD:3-4` 作者自陈「Some of these design decision were taken over from original RW, some are my own. I only take partial responsibility.」，这句话是评估「镜像保真度」的钥匙。

### 3.7.1 目录与模块清单

仓库无 `librw/`、`gc/`、`examples/`、`tests/` 目录；核心全在 `src/`，共 306 个受版本管理文件，`src/` 含 64 个 `.cpp` + 49 个 `.h`。

| 模块 | 行数 | 内容 |
|---|---:|---|
| `src/*.{cpp,h}`（平台无关核心） | 15,410 | `base.cpp` 1156、`geometry.cpp` 1053、`image.cpp` 1324、`clump.cpp` 669、`matfx.cpp` 641、`texture.cpp` 596、`engine.cpp` 573、`raster.cpp` 559、`camera.cpp` 524、`uvanim.cpp` 502、`skin.cpp` 486、`frame.cpp` 470、`tristrip.cpp` 682、`hanim.cpp` 414、`userdata.cpp` 364、`geoplg.cpp` 339、`anim.cpp` 293、`world.cpp` 238、`plg.cpp` 246、`light.cpp` 163、`charset.cpp` 244、`prim.cpp` 70、`render.cpp` 96、`pipeline.cpp` 198 |
| `src/gl/`（不含 glad 4,688） | 6,795 | `gl3device.cpp` 2415、`gl3raster.cpp` 1045、`wdgl.cpp` 875、`gl3skin.cpp` 366、`gl3shader.cpp` 363 |
| `src/d3d/` | 13,061 | 其中 `shaders/*.h` 为生成的 VS 汇编 4,181；手写：`d3ddevice.cpp` 2022、`d3d.cpp` 1087、`xbox.cpp` 1005、`d3d9.cpp` 853、`d3d8.cpp` 678 |
| `src/ps2/` | 7,005 | `ps2raster.cpp` 2238、`ps2.cpp` 1586、`ps2device.cpp` 1252、`ps2immed.cpp` 345、`ps2skin.cpp` 334；另有 `vu1/*.vu,*.dsm` 微码 1,654 行 |
| `skeleton/`（不含 imgui 69,408） | 2,315 | `win.cpp`/`glfw.cpp`/`sdl2.cpp`/`sdl3.cpp`/`ps2.cpp` |
| `tools/` | 12,091 | `playground` 3399、`ps2test` 1343、`im3d` 1324、`im2d` 1156、`demoreel` 1018、`dumprwtree` 145 |
| 第三方 vendored | 82,451 | glad 4,688 + lodepng 8,355 + imgui/ImGuizmo 69,408 |

一手代码（剔除 vendored 与生成的着色器汇编）约 **4.2 万行**（`src/` 手写 38,090 + skeleton 2,315）。

### 3.7.2 构建系统、标准与依赖

两套并行构建：`CMakeLists.txt`（主推，CI 全量使用）与 `premake5.lua` + 仓库内自带的 `premake5.exe`（PS2 路径唯一可用）。CMake 侧 `src/CMakeLists.txt:140-148` 强制 `CXX_STANDARD 11` / `C_STANDARD 11`、`EXTENSIONS OFF`。平台矩阵由 `CMakeLists.txt:16-31` 决定：Windows = `NULL/GL3/D3D9`，Nintendo Switch = `NULL/GL3`（`cmake/nx/NXFunctions.cmake`），`PS2` = 仅 `PS2`（`cmake/ps2/`），其余 = `NULL/GL3`。CI（`.github/workflows/build.yml`）覆盖 win/macOS/Ubuntu 的 D3D9+GL3(GLFW/SDL2/SDL3)+NULL、`ps2dev/ps2dev:latest` 容器与 `devkitpro/devkita64`。依赖极少：OpenGL(≥2.1/ES 2.0)、d3d9+xinput、GLFW/SDL2/SDL3、vendored lodepng/glad/imgui；PS2 走 `ps2dev/ps2sdk`（`cmake/ps2/cmaketoolchain/toolchain_ps2_ee.cmake:16-50`，`mips64r5900el-ps2-elf-g++` + `dvp-as`，并自定义了一门 `DSM` 语言：`enable_language(DSM)`，`CMakeLists.txt:54`）。GameCube 在 `src/rwbase.h:560` 只是平台枚举里的一行注释占位，无实现。

### 3.7.3 核心问题：librw 有多像真 RW？

**关键反直觉发现：librw 完全不使用 `Rw*`/`Rp*` C 符号命名，也不使用 `naRw*` 枚举名。** 全仓 `Rp[A-Z]*` 唯一符号数 = **0**；`Rw[A-Z]*` 只有 2 个唯一名（`RwStateCache`、`RwRasterStateCache`，即各后端的状态缓存结构体，见 `src/ps2/rwps2impl.h:37`、`src/d3d/d3ddevice.cpp:61,72`、`src/gl/gl3device.cpp:146,156`），是全仓仅存的前缀残留。`naRw*`、`rpBaseData`、`rpBaseClass`、`GetT()`、`StreamCallback`、`RhDevice*`、`RpEngineRegisterNamespace`、`RpPLUGINS*`、`RWPLUGINENTRY` 在 librw 中**一个都不存在**。

它的策略是**语义同构、命名重构**：`namespace rw` 下的 C++ `struct` + 成员函数，逐条对应 RW 的 `Rp*Get/*Set/*StreamRead`。规模可用几个可测量指标刻画：168 处 `struct/class` 声明（含前向声明）、公共头文件中 609 条函数声明、434 条 `.cpp` 里的 `Class::func` 定义、191 个十六进制枚举常量（这些常量的**数值**必须与 RW 一致）。

真正被一比一复刻的是 RW 的**内部机制**而非符号名：

- **`RwObject` 头布局**：`src/rwobjects.h:5-10` 的 `struct Object { uint8 type; uint8 subType; uint8 flags; uint8 privateFlags; void *parent; }` 与 RW 的 `RwObject` 字段顺序/宽度完全相同。
- **插件偏移表**：`src/rwplg.h:64-82` 的 `PLUGINBASE` 宏为每个可插插件的对象注入 `static PluginList s_plglist` 与 `registerPlugin/registerPluginStream/setStreamRightsCallback/setStreamAlwaysCallback/getPluginOffset`——这就是 RW `_rpXXXRegister` + `RpEngineRegisterNamespace` 的等价物；共 **11 个对象**嵌入了它（`rwobjects.h:31 Frame, 225 Raster, 367 Texture, 430 Material, 515 Geometry, 610 Atomic, 672 Light, 734 Camera, 805 Clump, 855 World, 881 TexDictionary`）。`src/rwplg.h:3-4` 的 `PLUGINOFFSET(type, base, offset) ((type*)((char*)(base)+(offset)))` 在代码中被用 **133 次**，与 RW 尾部附加数据的做法一致。类型 ID 也照抄：`Geometry` `enum { ID = 8 }`（`rwobjects.h:516`）、`Atomic` `ID = 1`（`:612`）、`Clump` `ID = 2`（`:806`）、`Light` `ID = 3`、`World` `ID = 7`。
- **chunk ID 数值**：`src/rwbase.h:579-637` 用 `MAKEPLUGINID(v,id) (((v&0xFFFFFF)<<8)|(id&0xFF))` 重建 `Vend_core/CriterionTk/CriterionInt/CriterionWorld`，并列出 `ID_NAOBJECT/STRUCT/STRING/EXTENSION/CAMERA/…/UVANIMDICT` 与 `ID_MESH/NATIVEDATA/VERTEXFMT`；厂商号 10/11 是 librw 自定义（用于原生 raster）。`CoreModuleID`（`:648-668`）复刻了 RW 的 module 插件号（`ID_FRAMEMODULE`、`ID_RASTERMODULE`…）。
- **library version 编码**：`src/rwbase.h:690-725` 的 `libraryIDPack/UnpackVersion/UnpackBuild` 是 RW 把 `3.4.0` 打包进 chunk header 版本字段的位运算逆向，上方还留了 `// 0x1C000000  3.7` 的注释表；`src/rwbase.h:726-731` 的 `ChunkHeaderInfo{type,length,version,build}` 即 `RwChunkHeader`。
- **`RwStream`**：`src/rwbase.h:486-546` 改用 C++ 虚基类 `Stream/StreamMemory/StreamFile`，取代 RW 的函数指针 vtable，但 `read8/write32/seek/tell` 语义与 `RwStreamMacros` 一一对应。
- **`RwEngineInstance`**：`src/rwengine.h:158-206` 的 `struct Engine` + 全局 `Engine *engine`（`src/engine.cpp:34`）；`Engine::open()` 上方注释直接写「This is where RW allocates the engine and e.g. opens d3d」（`engine.cpp:244`）。`Engine::init()` 里 `Frame/Image/Raster/Texture::registerModule()` 后紧接 `for(i<NUM_PLATFORMS) new(&Driver::s_plglist[i]) PluginList(sizeof(Driver))` 与六个 `registerPlatformPlugins()`（`engine.cpp:212-235`），是 RW `RpEngineInit` 阶段的忠实重演。
- **`RhDevice*` 的对应物**：`src/rwengine.h:48` `Device`（渲染设备函数表：`beginUpdate/endUpdate/setRenderState/im2D*/im3D*/system`）与 `:74` `Driver`（平台相关但可移植：`rasterCreate/rasterLock/imageFindRasterFormat/rasterFromImage/rasterToImage`）。`ARCHITECTURE.MD:86-88` 明说「The second category does not exist in original RW」——`Driver` 层是 librw 为「任意平台读写任意平台文件」新增的抽象。`DeviceReq` 枚举（`rwengine.h:4-36`）复刻了 RW 那个奇怪的单一 `RpEngineSystem(req,arg,n)` 入口，作者旁注「uhhhm..... why are these not actual functions?」。
- **`RwRenderState`**：`src/rwrender.h:4-41` 顺序与 RW 完全一致（`TEXTURERASTER…STENCILFUNCTIONWRITEMASK`），只额外加了 `GSALPHATEST/GSALPHATESTREF` 用于 GS 语义模拟。
- **atomic→geometry→clump 层级**：`Atomic` 持 `Geometry*`+`Clump*`+`ObjPipeline*`+`renderCB`（`rwobjects.h:623-633`），`Clump` 用三条 `LinkList atomics/lights/cameras`（`:808-810`），`Frame` 有 `matrix`/`ltm` 双矩阵与 `HIERARCHYSYNC*` 私有标志（`:34-46`）——与 RW 的 frame/clump/atomic/geometry + LTM 脏标记模型一致。

**偏移/尺寸断言：全仓 `static_assert` 仅 2 条，且都被注释掉**（`src/skin.cpp:438-439`：`//static_assert(sizeof(Skin::RLEcount) == 2, "RLEcount size");`）。没有 `RWOFFSET` 式宏，没有 `leftover`/`padding` 字段（`leftover` 出现 0 次）。原因是设计目标不同：librw **不追求内存布局兼容真 RW**，只追求**磁盘 chunk 布局兼容**，因此布局校验靠手工逐字段偏移读写完成，最典型是 `src/d3d/d3d9.cpp:172-192`：

```c
header->serialNumber = *(uint32*)p; p += 4;
header->numMeshes    = *(uint32*)p; p += 4;
header->indexBuffer  = nil;         p += 4;
p += 16*2;  // skip vertex streams, they're repeated with the vertex buffers
header->useOffsets   = *(bool32*)p; p += 4;
```

这段「按 4 步进、显式跳过未知字段」的写法，本身就是反出的字节偏移量的化石记录。对内存布局真正敏感的地方（`struct InstanceDataHeader : rw::InstanceDataHeader` 在 `d3d/rwd3d9.h:39`、`ps2/rwps2.h:77`；`Texture` 的 `char name[32]; char mask[32];` `rwobjects.h:371-372`）靠字段顺序自然对齐而非断言。唯一一处显式命名的 pad 字段是 `src/ps2/rwps2.h:23` `Im2DVertex` 里的 `float32 u, v, q, PAD;`（GS 顶点要求 128 位对齐）。另有 31 处 `padding` 命中全在 PNG/BMP 注释里，与 RW 无关。

### 3.7.4 平台后端三分割

同一套 API 由 5 个后端分别实现，`Engine::open` 通过编译期 `#ifdef` 挑一个 `Device`（`engine.cpp:266-275`），但**所有平台的 `Driver` 同时在场**（`engine.cpp:280-296`），这正是 librw 能跨平台转换 DFF/TXD 的根因。

| 后端 | 目录/行数 | Driver 注册 | 状态 |
|---|---|---|---|
| OpenGL 3 | `src/gl/` 6,795 | `gl3.cpp:48 registerPlatformPlugins()` → `PLATFORM_GL3` | 最完整；有 GLSL 源（`shaders/` 714 行）+ skin/matfx 管线 |
| D3D9/D3D8/Xbox | `src/d3d/` 13,061 | `d3d.cpp`/`d3d9.cpp`/`d3d8.cpp`/`xbox.cpp` | 读写原生数据完备；`xbox.cpp:202` 注「xbox can't render」 |
| PS2 | `src/ps2/` 7,005 + 1,654 微码 | `ps2.cpp:56` | 「working as a test only」（`README.md:17`） |
| WarDrum GL | `src/gl/wdgl.cpp` 875 | `wdgl.cpp:327`「wdgl can't render」 | 仅数据侧 |
| NULL | `src/engine.cpp:470-545` | — | 10 条 `assert(0 && "... not implemented")`（`:478,485,492,498,505,511,519,526,533`） |

五套 `InstanceDataHeader`/`InstanceData`/`AttribDesc` 与五套 `readNativeData/writeNativeData/getSizeNativeData`（如 `ps2.cpp:84,129`、`d3d8.cpp:104`、`xbox.cpp:133`）意味着作者必须**分别反出 PS2 VIF 包、D3D8/D3D9 FVF+顶点流、Xbox 顶点格式、GL 原生扩展**的私有二进制布局。这本身就是「RW 内部抽象层被恢复了多少」的直接度量：文件读写侧（`Driver`）与渲染侧（`Device`）被彻底解耦，而真 RW 是把两者焊在同一个 `RwDevice` 里的。

### 3.7.5 PS2 与「完全用自由工具链构建」

HEAD 提交正是这条前沿：`build the ps2 platform entirely with freesce`。做法是在 `premake5.lua:41-44` 加 `--freesce` 选项，`:91-116` 把 PS2 平台一分为二：默认走 SCE SDK（`gccprefix 'ee-'`、`$(PS2SDK)/ee/include`），加上 `--freesce` 则 `gccprefix '../tools/freesce/ee-'` + `$(FREESCE)/ee/include`，并注释解释「a premake-time choice because the 2.9 and 3.2 C++ ABIs don't link」。`tools/freesce/ee-g++`、`ee-gcc`、`ee-ar` 是三个 sh 包装器，专门剥掉 premake 生成的 gcc-3 风格依赖开关（`-MD/-MP/-MF`），因为 freesce 的 **ee-gcc 2.9** 驱动会把 `-MF` 的参数当第二个输入文件。

真正的硬核证据在 `tools/demoreel/Makefile:66-125`：`demoreel_cd_freesce.elf` 用 `-nostdlib` 显式列出 `crt0/crti/crtbegin/crtend/crtn` + freesce 自己的 6 个库（`libgraph/libdma/libpc/libpad/libcdvd/libkernl`）+ newlib 的 `-lc -lm` + `app.cmd`，「Nothing comes from /usr/local/sce」。因为 gcc 2.9 在 EE 上只生成 `_GLOBAL_$I` 构造函数而不注册它们，还要跑 `ctorgen` 生成 `ctorreg.s`——Makefile:93-97 的注释写道：「Without it librw's global ctors (**the PluginList statics**) silently never run」。这句话反过来印证了 §3.7.3 的机制：librw 的插件系统是靠 C++ 全局静态对象自注册的。此外 `includedirs` 备注 freesce 头文件自带本 vintage 的 `IOP_IMAGE_FILE`（`IOPRP242.IMG`），与 SCE 头文件同构。CMake 侧的 PS2 路径仍依赖 `ps2dev/ps2sdk` 且要求 `GSKIT`（`toolchain_ps2_ee.cmake:21-35`），freesce 路线目前只在 premake 分支可用。

另一处前沿证据是手写的 **VU1 微码**（`src/ps2/vu1/`：`defaultPipe.dsm`、`cliptri.vu`、`lighting.vu`、`LL/LS/TL/TSclip.vu` 共 1,654 行），以及 `src/ps2/ps2device.cpp:387-416` 那段注释——先写「It would be nice to be able to test against RW's microcode」，再列出 RW 的 VU 常量内存表 `0x3F0 vuSDmat0 … 0x3FF vuSDVUSwitch`，右侧还标出 RW 原始符号名 `vu1DataNearClip`、`xMaxYMax128`、`vu1DataOffset3D`、`skyClipVect1`。这是一份从二进制里抄出来的 RW PS2 内部 ABI。

### 3.7.6 完成度与子库成熟度

没有 `RW_UNIMPLEMENTED` 宏（0 次）；替代形态是 148 条 `TODO`、20 条 "not implemented"、以及 `TODO` 文件（`tristrips / morphing / Pipelines (PDS, Xbox, PC) / bsp / ps2 rendering / ADC conversion`）。

| RW 子库 | librw 对应 | 成熟度 |
|---|---|---|
| rpgeometry / rpatomic / rpclump / rpframe | `geometry.cpp`+`geoplg.cpp`+`clump.cpp`+`frame.cpp` | 完整（含 mesh/tristrip 生成 `tristrip.cpp` 682 行） |
| rpraster / rptexture / rpiimage | `raster.cpp`+`texture.cpp`+`image.cpp`+`{bmp,tga,png}.cpp` | 完整，DFF/TXD 五平台互通 |
| rpcamera / rplight | `camera.cpp` 524 / `light.cpp` 163 | 完整（含 frustum plane、`enumerateLights`） |
| rpskin / rpsplit | `skin.cpp` 486 + 各后端 `*skin.cpp` | 完整，含 split-skin RLE |
| rphanim | `hanim.cpp` 414 | 良（`hanim.cpp:154`「TODO: handle more (all!) cases」） |
| rpanimation | `anim.cpp` 293 + `rwanim.h` | 部分（`anim.cpp:192` 猜指针/padding 尺寸；插值器不全） |
| rpmatfx | `matfx.cpp` 641 + `*matfx.cpp` | 部分（ARCHITECTURE.MD:31「MatFX (only env map so far)」） |
| rpuvanim / rpuserdata / rpanisot / rpcharset | `uvanim.cpp` 502 / `userdata.cpp` 364 / `texture.cpp:565` / `charset.cpp` | 良（`uvanim.cpp:257`「implement this fully」） |
| **rpworld** | `world.cpp` 238 | **stub**：`rwobjects.h:852`「A bit of a stub right now」；无 `RpWorldSector`、无 sector tree，`World::render()` 自认「this is very wrong, we really want world sectors」（`world.cpp:136`）；`world.cpp:181` 局部光排除需要 world sectors |
| rppatch / rpgeommod | 无 | 缺失（`prim.cpp` 70 行仅占位） |
| rpsprite / rpgui / rpu2d | 无 | 缺失（全仓无 sprite GUI 实现） |
| rpbsp / rpeFFECT / rplegacy normal map | 无 | 缺失（`README.md:14`「BSP is not supported at all」） |
| PDS | `ps2/pds.cpp` 203 | 仅注册/查找壳，管线未接 |

管线本身被刻意简化：`ARCHITECTURE.MD:22-28` 说 RW 的 pipeline「severely overengineered」，librw 只保留 `ObjPipeline` 的 `instance/uninstance/render` 三指针（`src/rwpipeline.h:28-32`）而非 RW 的 stage 链。

### 3.7.7 librw 与 OpenRW 的对照

| 维度 | librw | OpenRW |
|---|---|---|
| 许可 | MIT（`LICENSE:1-3`） | GPL-3.0（`COPYING`） |
| 一手 LOC | ≈42,000（`src/` 手写 38,090 + skeleton 2,315） | ≈58,800（`rwcore` 12,124 + `rwengine` 38,399 + `rwgame` 5,567 + `rwtools` 272 + `rwviewer` 2,414）；RW 本体只有 `rwcore/rw`+`data`+`loaders`+`gl` ≈2,300 行手写 |
| 目标 | 替代 RW **库**（可被 re3/reVC 链接） | 替代 GTA III **游戏**，RW 只是它的 loader |
| RW API 命名 | 语义同构、`namespace rw` 重构；`Rp*`=0，`Rw*`=2 | 同样无 `Rw*/Rp*`（`rwcore/` 中唯一符号数 = 0），`openrw::rw::Clump/Geometry/Atomic` |
| 对象插件机制 | **有**：`PLUGINBASE`×11、`PluginList`/`Plugin`、`PLUGINOFFSET`×133、16 个 `register*()` 注册入口 | **无**：`rwcore/` 中 `Pipeline`/`Plugin` 关键字 0 命中 |
| chunk/stream | 复刻 `MAKEPLUGINID`+`Vend_*`+`libraryIDPack`+`ChunkHeaderInfo`，五平台原生数据双向读写 | 只有 `RWBStream::ChunkID`（`rwcore/loaders/RWBinaryStream.hpp:31`）单向读 `LoaderDFF.cpp` |
| 内存布局断言 | 2 个 `static_assert`，全部注释掉（`src/skin.cpp:438`） | 2 个生效的 `static_assert`，且都不是 RW 布局而是自研结构：`rwengine/src/engine/SaveGame.cpp:549 sizeof(BasicState)==0xBC`、`rwengine/src/data/Chase.cpp:39 sizeof(ChaseEntryRecord)==28` |
| 渲染后端 | 5 套（GL3/D3D9/D3D8/Xbox/PS2）+ 微码 | 1 套 OpenGL 3.3（vendored `gl_core_3_3.c` 5,737 行） |
| 测试 | 无 `tests/`，靠 `tools/` 回归 | 有 `tests/` 3,574 行 + CTest/CDash/codecov |
| 上游依赖 | 被 re3/reVC 使用（`README.md:21`） | 自带 `rwgame` 脚本/物理/音频全栈 |

**谁更「引擎级」？** librw 明显更忠实。它复刻了 RW 的**内部结构机制**（Object 头 + 尾部插件偏移表 + 构造/析构/流三段回调 + rights/always 回调 + module 注册顺序 + 每平台 native raster/geometry 私有格式），这些只有在反复对照 RW 二进制/DFF 才能重建；OpenRW 只到「能读通 DFF 到内存」这一层，其 RW 侧 ≈2.3k 行没有管线、没有插件、没有多平台。LOC 换算是反直觉的：OpenRW 总行数是 librw 的 1.4 倍，但增量全在 `rwengine/rwgame`（游戏逻辑、脚本、物理、AI），RW 本体反而薄得多。代价是 librw 无法直接当游戏跑（`World` 是 stub、无 rpsprite/rpgui），而 OpenRW 能通关 GTA III。

### 3.7.8 溯源标记汇总

`src/ps2/ps2raster.cpp:479`「This code was reversed from RW and is rather complicated…」；`:43`「i don't really understand this, **stolen from RW**」；`:801`「RW's code does not seem to quite work with 24 bit rasters」；`:1001`「but even **gta3's 3.1** seems to??」；`src/ps2/ps2raster.cpp:662`「We use a lookup, RW does bit operations」；`src/ps2/rwps2.h:102`「Not really types as in RW but offsets」；`src/rwobjects.h:311`「// from RW」标注 `PRIVATELOCK_READ/WRITE/…` 私有位；`:376`「actually not in RW」标注主动偏离；`src/skin.cpp:140-141`「numUsedBones and numWeights appear in/after **34003** but not in/before **33002**（probably `rw::version >= 0x34000`）」——用 RW 构建号做版本考古；`src/d3d/xbox.cpp:138`、`ps2.cpp:657` 用 `rw::version >= 0x35000`（RW 3.5）门控写入。RW 原始符号名 `vuSDmat0/vu1DataNearClip/xMaxYMax128/skyClipVect1`（`ps2device.cpp:390-415`）。`src/gl/rwwdgl.h:5`「This is not really RW OpenGL! It's specific to **WarDrum's GTA ports**」（WarDrum 即 GTA III/VC 移动移植方）。`tools/dumprwtree/dumprwtree.cpp:11-71` 保留了 RW SDK 版 `dumprwtree` 的完整 chunk 名称表，并注明区段「From 0x0101 through 0x0135」「From 0x0180 through 0x01c1」，其中 Rockstar 私有扩展 `RSchunks`（`0x0253F2F0+`）列出 `2dfx / Breakable / Collision Model / Environment Material / Specular Material / Extra Normals / Pipeline Set / Node Name`，另有 `0xF21E "ZModeler Lock"`——这份表是 GTA MOD 社区 chunk 知识的权威来源。`skeleton/skeleton.h:7`「same as RW skeleton」表明 `sk::` 应用框架也照 RW SDK 的 skeleton 示例而来。未见 IDA/Ghidra 地址注释或反汇编工具产物引用（推断：主要靠 RW SDK 文档 + DFF/TXD 实测往返，而非仅二进制反出）。

### 3.7.9 结论

librw 与 OpenRW 代表了 RenderWare 复现的两条截然不同的技术路线：librw 是**库级重建**，用约 4.2 万行 MIT 代码把 RW 的对象头、插件偏移表、module 注册时序、chunk/version 编码和五个平台的原生数据格式逐机制复原，并额外做出了一件事——用 freesce（自带 ee-gcc 2.9 + newlib）替换 Sony SDK 完成 PS2 全链路自举；OpenRW 是**应用级重建**，只需要 RW 的磁盘格式可读，因此可以用 ~2.3k 行的 loader 加一套 GL 后端撑起整款游戏。前者是引擎考古，后者是产品复活。

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

## 3.9 citizenfx/natives + txAdmin —— 逆向知识的数据库化

本节关注 FiveM 生态中最不"代码化"、却最具复用价值的一层：把 IDA 里做出的结论沉淀成**可 diff、可校验、可 code-gen 的版本化数据集**。

### 3.9.1 布局：Markdown 文件即数据库行

`/root/orca-workspaces/gta-repos/citizenfx-natives`（浅克隆，1 commit，HEAD `c00e0ed`）共 6437 个 `.md`（`find` 计数），按 **44 个大写命名空间目录**平铺：`NETWORK/`(824)、`VEHICLE/`(751)、`PED/`(612)、`HUD/`(509)、`GRAPHICS/`(392)、`MISC/`(323)、`STATS/`(311)、`TASK/`(306)、`MONEY/`(301)、`AUDIO/`(276)、`PLAYER/`(248)、`CAM/`(245)，尾部如 `SAVEMIGRATION/`(3)、`LOCALIZATION/`(3)。**没有** gta5/rdr3/fourm/nyi 之类的 per-title 顶层目录——本仓库只收原版 GTA V native，见 README.md:21-29（CFX 命名空间另存于 FiveM 源码树）。

记录格式由 `.github/CONTRIBUTING.md:17-45` 约定、由生成器 codegen_out_markdown.lua:170-181 定型：YAML front matter（全库只出现两个键：`ns:` 6416 次、`aliases:` 2298 次）＋ `## NAME` ＋ 一个 ```` ```c ```` 块，其首行注释是 `// <64 位 native hash> <32 位 joaat>`，随后是 C 语法签名。两条与 HEAD 提交直接相关的记录逐字如下（WATER/AddExtraCalmingQuad.md:1-8、WATER/RemoveExtraCalmingQuad.md:1-8）：

````
---
ns: WATER
aliases: ["0xFDBF4CDBC07E1706", "_ADD_CURRENT_RISE"]
---
## ADD_EXTRA_CALMING_QUAD
```c
// 0xFDBF4CDBC07E1706 0xA9419B6D
int ADD_EXTRA_CALMING_QUAD(float xLow, float yLow, float xHigh, float yHigh, float height);
```
````
````
---
ns: WATER
aliases: ["0xB1252E3E59A82AAF", "_REMOVE_CURRENT_RISE"]
---
## REMOVE_EXTRA_CALMING_QUAD
```c
// 0xB1252E3E59A82AAF 0x45268B6F
void REMOVE_EXTRA_CALMING_QUAD(int extraQuad);
```
````

参数/返回类型的演进用 `cs_type(旧类型) 新类型` 标注以保持向后兼容（CONTRIBUTING.md:43，实例 VEHICLE/SetTrainSpeed.md:7）。文档正文不是结构化字段：`## Parameters` 5341 条、`## Return value` 2584 条、`## Examples` 292 条（Lua 282 / C# 107 / JS 92）。

| 标题 | 数据位置 | 记录数 |
| --- | --- | --- |
| GTA V (gta5) | citizenfx-natives 命名空间目录 | **6437**（其中 `N_0x*` 十六进制占位名 1003 条，15.6%） |
| CFX 扩展 | `citizenfx-fivem/ext/native-decls/*.md` | 889（`apiset: client` 546 / `server` 186 / `shared` 79，余为多值写法） |
| GTA V 引擎输入（按 build） | `ext/natives/natives_stash/gta_universal.lua` 76015 行、`gta_0193D0AF.lua` 68527 行、`gta_21E43A33.lua` 60120 行 | universal 内 `native "…"` 5180 条，带 `jhash` 4586 条 |
| RDR3 | `ext/natives/natives_stash/rdr3_universal.lua` | **0 字节（占位）** |
| ny / LibertyM | `ext/natives/natives_stash/ny_universal.lua` | **0 字节（占位）** |

### 3.9.2 数据周围的工具：三段流水线，校验器在仓库之外

| 阶段 | 入口 | 关键脚本 | 产物 / 去向 |
| --- | --- | --- | --- |
| PR 校验 | `.github/workflows/check-pr.yml:24-31` | `.ci/run-pr:11-14` 用 `git diff-tree` 取改动的 `.md`，逐个交 `node $BUILDER_ROOT/verify-one.js`（run-pr:37） | 通过/失败列表；失败即 `die` |
| 构建器安装 | `.ci/setup-builder:8-15` | `apt-get install libclang-14-dev` + **shallow clone `citizenfx/native-doc-tooling`** | 解析器与模板全在该外部仓库（FiveM 侧 `.gitmodules` 亦以 submodule 引用） |
| 部署 | `.github/workflows/deploy-repo.yml`（`publish` 分支，:47-48 rsync） | `.ci/run-deploy:18-21`：`node index.js <repo>` → `natives.json`；`build-template.js lua` → `natives.lua`；`build-template.js reference` → `natives.html` | docs.fivem.net/natives 与运行时 Lua 绑定 |
| 兼容层 | `.github/workflows/run-compatgen.yml`（`workflow_dispatch`，:78 `git fetch --shallow-since=<date>`） | `.ci/run-compat` → `compatgen.js --use-history --start-date`（默认 2023-01-01，`.ci/config.sh:4`） | `natives_global_client_compat.lua`：按 build 历史签名生成跨版本 shim |

**这里没有 IDA/Ghidra 工件**：全仓库无 `.i64/.idb/.idc/` 及重命名脚本（`find` 验证）——反汇编现场在别处，本仓库只存"结论 + 可执行 schema"。同时 CI **只做单文件语法校验**，不做全库一致性检查——这可由数据本身证实：64 位 hash 存在一对重复（`NETWORK/NetworkBlockJoinQueueInvites.md` 与 `NETWORK/N_0xcfeb8af24fc1d0bb.md` 同为 `0xCFEB8AF24FC1D0BB`），且 `status/verified/unverified` 一类字段在本库根本不存在，置信度只能靠散文表达（`unknown` 108 次、`unused` 66、`deprecated` 11、`has been removed` 1）。「推断」带 working/removed/unverified 状态的分级实际存在于 NativeDB/文档站侧，而非这份 Markdown 数据库。

### 3.9.3 HEAD 提交：一次 joaat 混用的数据完整性修复

JOAAT 即 Jenkins one-at-a-time，32 位；可直接读到的实现是 `citizenfx-fivem/ext/sdk/resources/sdk-root/shell/src/utils/joaat.ts:1-16`（先 `toLowerCase()`，再 `h+=c; h+=h<<10; h^=h>>>6`，收尾 `<<3 / >>>11 / <<15`），同一算法复制于 `ext/sdk/resources/sdk-game/src/shared.ts:27-43`；引擎 C++ 侧另有 FNV-1a 家族 `code/shared/fnv.h:80`（`0x811C9DC5/0x1000193`），与 joaat 不是一回事。

按该算法独立复算即得：`joaat("add_extra_calming_quad")=0xA9419B6D`、`joaat("remove_extra_calming_quad")=0x45268B6F`——与上面两条记录的第二个 hash **逐位吻合**。也就是说 DB 的不变量是 `jhash == joaat(小写名)`。HEAD 提交 `c00e0ed "Fix joaat mixup for RemoveCurrentRise (#732)"` 把 `0x7DBCEF6F` 改为 `0x45268B6F`，并把 `RemoveCurrentRise.md` 重命名为 `RemoveExtraCalmingQuad.md`、以 `aliases: [..., "_REMOVE_CURRENT_RISE"]` 保留旧社区名。复算显示旧值既不等于 `joaat("REMOVE_CURRENT_RISE")=0x7CDCA3CF`，也不等于库内任何记录的 jhash，「推断」它是主机版某个"把 rise 计数清零"的另一个 native 的 jhash，被误抄到了这条记录上。

为何这是数据完整性 bug 而非笔误：jhash 是"名字→ID"的唯一性契约，codegen 的 JSON 输出直接把它当字段发射（`ext/natives/codegen_out_json.lua:71`、解析入口 `codegen.lua:113-114`），错误值会把兼容层/文档锚点指向一个不存在或不同的入口，等价于一次与别的 native 身份的隐式碰撞。全库量化：4881 条带 jhash 的记录里 **4313 条满足不变量**，568 条不满足——其中 388 条是 `_0x…` 自动名、177 条是社区名（CONTRIBUTING.md:25 明确"社区命名与 hash 不匹配"，属预期），但仍有 **3 条真实命名记录不匹配**：`MISC/CancelOnscreenKeyboard.md`、`PATHFIND/GetPosAlongGpsTypeRoute.md`、`VEHICLE/SetVehicleExclusiveDriver.md`。它们至今未被 CI 拦下，反向印证 3.9.2 的结论；32 位 jhash 全库 **0 个重复值**。

### 3.9.4 txAdmin：FiveM 的运维层（次要）

`/root/orca-workspaces/gta-repos/txAdmin`，MIT 且版权归 **Take-Two**（LICENSE:3），npm workspaces `core/nui/panel/shared`，735 文件 / 572 个 ts·js·lua 源文件。它同时是一个 FXServer resource（fxmanifest.lua:11-12 `fx_version 'cerulean'` / `game 'common'`）和一个独立 Node 进程：`core/boot/getNativeVars.ts:31-38` 直接调用 `GetConvar/GetCurrentResourceName/GetResourceMetadata/GetResourcePath` 取宿主信息——即其启动本身消费 native 表面。管理面：`FxRunner/ProcessManager`（子进程启停）、`FxMonitor/index.ts:17-45`（heart-beat 与 health-check 双看门狗，`fatalLimit` 60 s / 180 s，崩溃/挂起自动重启）、`Database/instance.ts:17`（lowdb JSON，`DATABASE_VERSION = 5` + migrations）、`AdminStore/providers/CitizenFX.ts:6`（`idms.fivem.net` OIDC 登录）、`deployer/recipeEngine`、`whitelist`、`masterActions`、`cfgEditor`。最接近"反作弊/完整性"的是诊断上报：`core/lib/fxserver/scanMonitorFiles.ts:8-13` 对配置/资源文件做 NFKC 归一化后 SHA-1，限额 300 文件·20 MB·10 层深度。任务假设的 `baseline`、`sv_pure`、`gameResStart`、`backups` 子系统在本仓库**均未出现**：真正的 pure 内容校验在引擎侧（`code/client/shared/PureModeState.h:12-38` 解析命令行 `pure_<n>`；`code/components/citizen-server-impl/src/InitConnectMethod.cpp:399` 定义 `sv_pureLevel`；`net/src/NetLibrary.cpp:1716` 随 server info 下发），`getBackup.ts:22` 只是导出 `playersDB.json`。可见 txAdmin 定位为"运维 + 权限 + 可观测性"，不做内容校验。

### 3.9.5 数据库仓库 ↔ 引擎内消费

链接点是三条路径：`ext/natives/codegen_out_markdown.lua:170-181` 证明 citizenfx/natives 的 `.md` 语法本身就是引擎侧 Lua 模板的输出格式（`ns`/`aliases`/`// hash jhash` 一一对应）；`ext/natives/codegen.lua:582-605` 按输入文件名选择标题（`gta_universal`→`inp/natives_global.lua`、`rdr3_universal`、`ny_universal`，`ourGame` 置位）；`ext/natives/natives_stash/gta_universal.lua:40-52` 展示引擎内的 DSL 记录（`native "GET_PLAYER_PED"` / `hash "0x43A66C31C68491C0"` / `jhash (0x6E31E993)` / `arguments{}` / `ns "PLAYER"` / `returns "Ped"` / `doc`），格式说明见 `ext/natives/README.md:17-31`。反向回灌由 `ext/native-doc-gen/build.sh:29,40,44` 完成：`codegen.lua … markdown` 生成 CFX 侧 `.md`，`native-doc-tooling/index.js` 用 libclang 解析回 JSON（「推断」`.md` 的 C 块正是靠 libclang 抽取类型，这也解释了 setup-builder:8 为何装 libclang）。分工因此清晰：**natives 仓库是权威人类可读数据库，ext/natives 是编译器/消费端**。

### 3.9.6 这一层证明了什么

RAGE 脚本 VM 的表面积在此被精确量化：GTA V 6437 条 + CFX 自有 889 条，44 个命名空间，NETWORK 独占 824 条（12.8%），说明线上化是该 VM 的主战场；1003 条仍是 `N_0x…` 占位，即命名覆盖率约 84.4%。跨 build 版本化以文件名为键（`gta_0193D0AF`、`gta_21E43A33`、`gta_universal`）加 compatgen 的历史签名抽取；多标题扩展则已**预留但未填充**——`ext/natives/Makefile:1-24` 与 `codegen_out_cs_v2.lua:2-7` 的 `PRODUCTS = { global=GTA_FIVE/FiveM, rdr3=IS_RDR3/RedM, ny=GTA_NY/LibertyM }` 表明接入新作只需一个 stash 文件与一套 `.md` 目录。逆向知识的最终形态不是脚本，而是带不变量（`jhash == joaat(name)`）、带 CI、带别名兼容层的数据集——这正是 HEAD 提交值得写进方法论的原因。

## 4. GTA VI：为什么"现在不存在合法的 GTA VI 逆向项目"

### 4.1 官方事实（截至 2026-11-18，逐条带日期）

| 项目 | 事实 | 等级 | 来源/日期 |
|---|---|---|---|
| 发售日 | 2026-11-19（本报告完成日次日） | F | Take-Two 财报与 Rockstar Newswire；2025-11-06 二次延期公告 |
| 平台 | PS5 / PS5 Pro / Xbox Series X\|S；**无 PC 版** | F | 官方，2026-10 |
| Xbox One | 仅通过 Xbox Cloud Gaming（2026-11-10 公布），Rockstar FAQ 明确**云游戏不含 PC** | F | Rockstar FAQ 转载，2026-11-11 |
| 价格 | US$79.99；实体版**无光盘**（code-in-box，2026-11-12 起可提前预载） | F | Reuters / BBC，2026-06-24 |
| 帧率 | 两代主机首发均 **30 FPS** | F | Rockstar 确认，2026-08 |
| PS5 Pro | PSSR 超分 + **PS5 Pro 独占光线追踪阴影**；原生 1440p → 4K 上采样；安装体积可达 **212 GB** | F | Digital Foundry（Will Judd），2026-11-10 |
| 引擎 | 官方与媒体均称 **RAGE**，"为本世代从零适配"；**任何"版本号"（RAGE 9 / eRAGE / EVolution）均为社区或 AI 农场造词，无一手出处** | F/U | Rockstar Newswire；rage.re/维基百科 |
| 舞台 | Leonida 州（含 Vice City） | F | 官方预告片 |
| PC 版 | 无官方日期；2026-10 产业消息指向 **2027 Q1** | R | 业界报道，非官方 |
| 微交易 / 生成式 AI | Rockstar 向 Kinda Funny 确认**均无** | F | 2026 |
| 2026-08 泄露 | "CyberLeek"：地图数据 + ≥16 段实机视频（9 天内流出）；Rockstar 8-26 承认被入侵；Take-Two 向 Discord / Microsoft / X Corp 发出传票（S.D.N.Y.）；泄密者收取 ≥$100k 后于 8-27 退款约 $268k | F | 2026-08-26/27 |
| 泄露内容 | **公开材料中没有任何一条指向构建系统、编译器、编辑器或工具链名称** | ? | 同上 |

### 4.2 结论：三个硬性阻断

1. **没有可分析的用户侧二进制**。主机可执行文件以加密形态交付，且 discless 包装意味着连"自有实体介质"都不含可读镜像；PC 版尚未存在 → 通用硬件上根本拿不到目标文件。
2. **平台条款直接禁止**。Rockstar《Mod Guidelines》（页面日期 2026-09-22）明确禁止：改动主线剧情/连续性、跨作搬运资产、作弊菜单，以及 —— 这一条对我们最致命 —— **"让游戏运行在 Rockstar 未正式发布的控制台、硬件设备或其他平台上"**。
3. **反作弊前置**。BattlEye 已在 GTA Online（2024）落地并配套新版封禁政策；GTA VI 只会同等或更强。这意味着任何运行时接管路线（第 5 章的 FiveM/SHV 那一套）在主机+在线环境下从一开始就不成立。

> 本报告不获取、不分析、也不复现任何未发售标题的二进制、镜像、DRM 绕过或在线模式绕过内容。以下 GTA VI 相关内容**只允许**建立在官方物料与业界报道之上。

### 4.3 关于 Take-Two 的执法史（决定所有 GTA 逆向项目的天花板）

这是本报告最重要的非技术章节。社区叙事里"Take-Two 打击 mod"是传闻，但**它有完整的、可核验的法律记录**：

| 日期 | 事件 | 技术含义 |
|---|---|---|
| 2017-06-15 | OpenIV 收到停止函并关闭（PC Gamer） | 文件格式工具也被追诉 |
| 2017-06-23 | Rockstar 声明：焦点是"GTA Online 作弊"，不是单人 modding | 官方划定"可容忍区" |
| 2017-07-03 | OpenIV 复现（Eurogamer），但**跨作内容（自由城地图）永久下架** | 复现≠胜利，边界被实测出来 |
| 2021-02-20 | **re3 / reVC 遭 DMCA 下架**，Take-Two 主张 **EULA 本身禁止逆向工程** | 这是"重写"路线被直接命中的时刻 |
| 2021-06-10 → 06-28 | 开发者提交 counter-notice，代码恢复 | 反通知机制有效但脆弱 |
| 2021-09-03 | Take-Two **起诉** re3/reVC 作者（VGC） | 从 DMCA 升级到诉讼 |
| 2021-10-02 | **二次下架** | 诉讼期间持续施压 |
| 2021-11-17 | modder 以合理使用/转换性使用抗辩（NME、GamesRadar） | 唯一一次成体系的抗辩 |
| 2023-04 | Take-Two **撤诉**，条款未公开 | **因此"逆向属于合理使用"从未成为先例** |
| 2021-07 | 大规模 mod 下架潮（PC Gamer 统计） | 系统性而非偶发 |
| 2016-10、2020-08/09 | GTA Online 账号重置/洗钱清洗（GameSpot） | 注意：坊间常误记为 2021，2021-07 是 mod 下架潮而非清洗 |
| 2019 / 2023 | Take-Two 先后收购 RedM 与 **Cfx.re（FiveM）** | 多人 modding 被"制度化收编"为 Creator Platform |
| CEO Strauss Zelnick | "我们相当灵活……但如果经济利益受威胁，我们会发停止函" | 官方策略陈述 |

三点直接后果写进所有后续判断：

- **第 5 章会看到：`citizenfx/fivem` 的 LICENSE 顶部已经是 "© 2023 Take-Two Interactive"**。今天"最大的开源 GTA 项目"在法律上属于发行商本身。
- **re3/reVC 的真实代码状态**（见 3.2）必须放在"被下架过两次、被起诉过、最终和解"的语境里读，而不是当作普通开源项目。
- **GTA VI 的任何"逆向路线图"在主机窗口期都不该承诺里程碑**；可行的时间锚点是 PC 版发布之后（若 2027 Q1，则格式草案 6–18 个月、行为级重实现按年计）。

## 5. 闭源侧：RAGE 公开知识图谱与二进制接管生态

### 5.1 RAGE 引擎：可从公开材料确认的部分

**谱系（F）**：ARTS → Angel Game Engine（Midtown Madness 2，2000）→ Take-Two 收购 Angel Studios（2002-11）→ Rockstar San Diego；Criterion 被 EA 收购（2004）→ 成立 **RAGE Technology Group**。首个 RAGE 游戏是 *Rockstar Games Presents Table Tennis*（Xbox 360，2006-05-23）；**GTA IV（2008）是首个用上 RAGE 的 GTA**，中间件为 **NaturalMotion Euphoria**（Dynamic Motion Synthesis）+ **Bullet** 物理。
Euphoria 的所有权链现在很有意思：NaturalMotion → Zynga（2014，约 $527M）→ **Take-Two 收购 Zynga（2022，约 $12.7B）**，即 Euphoria 今日是发行商自有资产 —— 这解释了为什么 RDR2/GTA VI 的角色动画层能"无成本"深度改造。

**版本史只能逐作重建**（F）：不存在官方 RAGE 版本号。可确认的增量：RDR2（2019-05）引入 PBR、体积云、预计算 GI、Vulkan、HDR；DLSS（2021-07）、FSR（2022-09）；GTA V PC "Enhanced"（2025-03）加入 RTGI + RTAO。

**被 modding 实证过的内部结构**（这是闭源引擎唯一可靠的"知识来源"，F）：

| 层 | 公开确认的实现 | 由谁/何物证实 |
|---|---|---|
| 容器 | **RPF7 / RSC** 嵌套归档，NG 与 AES 两种加密；条目元数据 + SHA/文件键映射 | OpenIV 文档；本报告 CodeWalker `RpfArchive.cs` |
| 资产类型 | `.ytd .ydr .ydd .ybn .ymap .ytyp .ycd .ymt .ynd .ymf` 等带类型标签的资源 | GTAMods Wiki + CodeWalker 解析器 |
| 元数据 | `.ymt` = **PSO**（Persistent Shared Object）带类型反射的序列化格式 | OpenIV PSO 文档；CodeWalker `PsoTypes.cs`(16,524 行) / `MetaNames.cs`(20,688 行) |
| 脚本 | 专有 VM 执行 `.ysc`（元数据 + 字节码）；GTA IV 前代为 `.sco` | NativeDB / 反编译脚本转储 |
| 原生函数 | 以 **64 位哈希**寻址、按命名空间分组的 native 表（GTA V 约 **6,400+** 条） | alloc8or.re NativeDB、`docs.fivem.net/natives`、本报告 `ext/natives/natives_stash/gta_universal.lua`（76,015 行） |
| 命名空间词汇表 | `rage::` 系列命名空间在 rage.re 有词条整理（维基百科引用；本次未能独立抓取原文） | rage.re（2023-05-18） |
| 布局仍在演进 | 2026-10 有 Rust 解析器 `rage-engine-formats` 与 CodeWalker 交叉核对记录 Enhanced 版布局版本（drawable v159、fragment 171、纹理字典 v5、clip 字典 v46） | lib.rs/crates/rage-engine-formats —— 说明**格式逆向至今仍是公开进行中的工作** |

**明确标注为不可信的命名**（U）：`RAGE::script VMT`、`pyRage`、`Minigame`、`GameFrontend/Essential`、"混合形状系统"作为*官方组件名* —— 未找到一手出处，本报告不作为事实陈述。另：本次未能确认 RAGE Technology Group 的任何 GDC 演讲。

### 5.2 GTA VI 技术面：只有帧分析，没有结构知识

**可确认**（F，Digital Foundry 2026-08-28 / 11-10）：RTGI + RT 反射（PS5 Pro 独占部分）、其余平台为 shadow map / SSR / cubemap 的混合方案、体积天气在 RDR2 基础上重做、**NPC 独有动画 >600,000 条**（RDR2 约 300,000、GTA V 约 55,000）并支持跨体型实时重定向、LA 专属动捕工作室、Zen 2 CPU 余量被点名为瓶颈改善项。

**不可确认**（?）：容器格式、脚本 VM、native 表结构、编辑器与工具链 —— 全部未知。2026-08 泄露**没有**公开任何工具或构建名称。2026-10 流出的裸露/调试画面，Rockstar 未确认真实性，"来自 debug 模式"只是分析师推断（R）。

### 5.3 闭源工具生态开放度台账

| 工具/项目 | 归属 | 源码开放度 | 与本报告开源仓库的关系 |
|---|---|---|---|
| **RAGE 引擎本体** | Rockstar / Take-Two | 完全闭源 | 只能靠 5.1 的"被实证结构"侧面测绘 |
| **ScriptHookV**（crosire） | 个人，二进制 `.asi` 分发 | 闭源；**其托管层 SHVDN 开源** | 3.3 章；开源↔闭源的教科书式切面 |
| **OpenIV** | gurinda/黑 | 闭源 | 2017 停止函事件主角；格式知识部分被 CodeWalker 以开源方式覆盖 |
| **RAGE Plugin Hook** | 个人 | 闭源 | 与 SHV 同类运行时接管 |
| **alt:V** | 团队 | 服务端/文档部分公开，客户端闭源 | 与 FiveM 同一竞争生态 |
| **RageMP** | 团队 → 后续形态 | 已停摆，历史代码非自由 | 多人服务端接管路线的另一支 |
| **FiveM / CitizenFX** | **Take-Two（2023 收购 Cfx.re）** | **半开放**：仅 `code/client/{shared,citicore,launcher,common}`、`code/tools`、`code/components/citizen-*`、`font-renderer`、`vfs-core`、`code/vendor/*.lua` 为 **LGPL-2.0**，其余受 **Rockstar Creator Platform License Agreement**（fivem.net/terms）约束 | 3.6 章：这是"发行商自有开源仓库"这一新形态 |
| **RedM** | Take-Two | 同 FiveM 模式（redm.gg/terms） | RDR2 侧的镜像案例 |
| **MTA:SA** | 社区（GPL-3.0） | 完整开源 | 3.5 章 |
| **CodeWalker** | dexyfex（个人） | 源码公开但**仓库内无 LICENSE 文件** | 3.4 章：法律上是"保留所有权利" |
| **NativeDB / alloc8or.re** | 社区 | 数据公开（native 哈希表） | 与 SHVDN/`ext/natives` 互为镜像 |
| **re3 / reVC** | 社区 | **DMCA 两度下架 + 诉讼后撤诉**，现存多为镜像/分支 | 3.2 章 |

### 5.4 对"要不要等 GTA VI 能做逆向"的判定

把第 4 章的法律边界和第 5 章的技术现实叠起来：

- **今天（2026-11-18）不存在、也不可能存在**任何合法可分发的 GTA VI 逆向产物。宣称"GTA6 逆向已完成/GTA6 源码泄露已解析"的仓库或帖子，几乎必然是 1.3 节那类仿冒投毒。
- **PC 版才是唯一起点**。参照系：GTA V PC（2015-04）→ CodeWalker（2017）→ native 表至今仍在更新；GTA IV（2008）→ OpenIV 起步用了约 5 年；re3 从研究（2016）到改造为完整引擎逆向（2018 春）再到发布（2021-02）跨过 **17 年**时差并且立刻被诉。
- 因此对 GTA VI 唯一诚实的路线图是：**PC 发布 → 容器识别（假定 RPF 后继、新密码/布局）→ PSO/元数据层测绘 → native 表重建 → 才谈得上行为级重写**，格式草案 6–18 个月，重写按年，且每一步都在 4.3 的执法阴影下。

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

## 7. 时间线、方法论迁移与"如果真要做 GTA VI"

### 7.1 历史时差基准（这是唯一能预测未来的数据）

把每个项目"原版 PC 发售"与"对应逆向成果落地"之间的时差量化：

| 目标 | 原版发售 | 逆向成果 | 时差 | 成果形态 |
|---|---|---|---|---|
| GTA III | 2001-10 (PC) | re3 研究起步 | ~15 年 | 研究性反编译（2016 起步、2018 春改为完整引擎逆向、2021-02 发布 → 立刻被诉） |
| GTA III | 2001 | OpenRW 起步 | ~12 年 | 重写；至 2026-11 仍未完成（opcode 覆盖约 57%） |
| GTA: SA | 2005-06 (PC) | MTA 成熟接管 | ~3–5 年 | 运行时接管 + 多人层 |
| GTA IV | 2008-12 (PC) | OpenIV 格式工具 | ~5–6 年 | 资产格式逆向 |
| GTA V | 2015-04 (PC) | CodeWalker | ~2 年 | 格式逆向 + 编辑器（至今仍是主力工具） |
| GTA V | 2015 | FiveM native 体系 | ~2–3 年起步，**10 年后仍在更新** | native 数据库 + 代码生成 |
| RDR2 | 2019-11 (PC) | RedM / natives 表 | ~1–2 年 | 接管 + 数据库 |
| **GTA VI** | **2026-11-19（主机）** | — | **PC 版前无起点**（业界报道 PC 2027 Q1，R） | 待定 |

三条曲线合起来是一个清晰规律：**格式类成果 2–6 年，接管类 2–3 年，完整重写 10 年以上且从未真正收敛**。而 2021 年之后，重写类项目的法律成本已从"下架"升级为"诉讼"。

### 7.2 分阶段路线（严格限定：以"未来存在 PC 版 + 只用自有资产"为前提）

| 阶段 | 目标 | 可直接复用的本项目经验 | 预计历时 | 法律风险 |
|---|---|---|---|---|
| P0 | 只读静态测绘：可执行段布局、资源容器识别 | CodeWalker 的 RPF/PSO 方法论（哈希→名称表 + 类型反射 + 往返断言） | 3–9 月 | 低（不发布产物则不触发分发条款） |
| P1 | 容器与加密：找到归档索引与密钥来源 | CodeWalker 的完整范式：仓库不写明文密钥，而是内置 154 KB 混淆块 `magic.dat` + 对**自有** exe 做 375 项 SHA1 指纹扫描（8 字节对齐、并行）反推出 NG 加密表（`RandomGauss.Solve`） | 6–18 月 | 中（EULA 禁止逆向，但无分发即难被追） |
| P2 | 资产格式：纹理/几何/碰撞/动画/元数据 | CodeWalker `BlockLength` / `BlockLength_Gen9` 式**往返保真断言**；OpenRW 的 chunk-ID 流格式镜像 | 12–30 月 | 中 |
| P3 | 脚本层：VM 字节码 + native 表 | FiveM `ext/natives` 数据库 + 多语言 `codegen_*` 生成器；SHVDN 的编译期版本门控与哈希枚举生成 | 18–48 月 | 中—高（native 表本身即发行商接口面） |
| P4 | 运行时接管做工具/编辑器 | SHVDN 特征码扫描（186 处）替代偏移表；MTA 的多版本伺候成本作为反面教材 | 与 P2/P3 并行 | 高（在线模式禁止；单人工具相对安全） |
| P5 | 引擎重写 | OpenRW：27 年代代码量 50k 行仍只完成 GTA III 的约 57% opcode，且存档写入未落地 | **年为单位，且历史上从未成功过完整商业 3A** | **最高**（re3/reVC 先例：DMCA×2 + 诉讼，2023-04 以未公开条款和解，未建立任何先例） |

判定：**P0–P2 是现实可做的，P3 取决于 PC 版，P5 在商业 3A 规模上历史上从未有过完整成功案例**。OpenRW/re3 处理的是 RenderWare 时代的单线程 C 风格引擎；GTA VI 面对的是带 Euphoria（现为 Take-Two 自有）、RTGI/PSSR、>600k NPC 动画与专有脚本 VM 的 RAGE —— 复杂度至少高一个数量级。

### 7.3 六个项目各自最值得迁移的一件事

| 项目 | 迁移价值 |
|---|---|
| **OpenRW** | "以文件格式为公开规格"的重写入口：先做 DFF/TXD/IMG 解析器与 `rwviewer`，再谈游戏逻辑 —— 且要吸取它把 43% opcode 留成空桩、Release 下静默展开为无的教训 |
| **re3 / reVC** | 逆向工程的**法律生命周期样本**：研究→重写→发布→下架→反通知→恢复→再下架→起诉→和解；技术之外真正决定项目命运的部分 |
| **CodeWalker** | **知识持久化的正确形态**：不写死密钥、把 chunk 标签做成 jenkins 哈希→名称枚举（20,239 项）、用往返长度断言把"我猜对了"变成可执行证明 |
| **ScriptHookVDotNet** | **摆脱上游依赖的演化路径**：从 hook 提供的枚举 API 迁移到自有特征码扫描 + 直接自造 `fwScriptGuid` 句柄 |
| **MTA:SA** | **偏移路线的完整工程形态与其代价**：481 个函数地址宏 + 217 个 `*SAInterface` 布局头 + 107 处 sizeof 断言 + 离线改导入表的注入方式 + `__LOCAL_SIZE` hook 自检；同时它证明"开源"也可以是**只剩壳**——网络层二进制靠 CDN 下载，源码树里只有纯虚 ABI |
| **FiveM/CitizenFX** | **规模化的唯一办法**：把 native/类型知识放进数据库再生成六种语言绑定；同时也是"工业成果被发行商收编"的边界样本 |

### 7.4 结论（五句）

1. 现在这份报告里能被称为"GTA 逆向工程"的成熟开源成果，**只存在于 RenderWare 时代（GTA III/VC/SA）与 RAGE 时代的工具层**，不存在任何完整重写商业 3A 的先例。
2. **GTA VI 目前不是逆向对象，而是法律与平台屏障的描述对象**：主机独占、discless、加密交付、Mod Guidelines 明文禁止跨平台运行、BattlEye 前置、PC 版未定。
3. 五个（含 re3/reVC 为七个）已核实仓库共同给出的最重要技术启示是：**用格式与数据库承载知识的项目活得最久，用偏移与地址表承载知识的项目最脆弱**。
4. 最重要的一条非技术启示：**`citizenfx/fivem` 现在属于 Take-Two**。开源与发行商之间的边界已从"外部对抗"变成"内部收编"，这会决定未来十年所有 GTA modding 的法律形状。
5. 本报告全程只读、未编译、未执行任何第三方代码，且已剔除该生态里大量仿冒投毒仓库；任何后续动作都应在**只针对自有正版资产、只针对单人/离线场景**的前提下进行。

## 8. 来源清单

### 8.1 一手来源（本地克隆源码，2026-11-18 固定 commit）

全部位于 `/root/orca-workspaces/gta-repos/`（`/root/wsl-demo/gta-repos/` 为其软链），报告中所有 `路径:行号` 均可在这些快照上复核：

| 仓库 | commit | 日期 |
|---|---|---|
| `rwengine/openrw` | `5c5f266b` | 2025-06-18 |
| `aap/librw` | `18532c2` | 2026-08-26 |
| `scripthookvdotnet/scripthookvdotnet` | `28b4dfde` | 2026-11-15 |
| `dexyfex/CodeWalker` | `485d56be` | 2025-04-11 |
| `multitheftauto/mtasa-blue` | `70b0b402` | 2026-11-17 |
| `citizenfx/fivem` | `8cbec36a` | 2026-11-17 |
| `ThirteenAG/GTAIV.EFLC.FusionFix` | `07c1e8d` | 2026-11-17 |
| `citizenfx/natives` | `c00e0ed` | 2026-10-27 |
| `citizenfx/txAdmin` | `1db5eb0` | 2026-10-23 |

未克隆的一手核验物：
- GitHub API `https://api.github.com/repos/GTAmodding/re3` → `HTTP 451`，`block.reason="dmca"`，`created_at="2021-09-30T17:33:39Z"`（2026-11-18 本机实测，原文引于 3.2.2）
- `git ls-remote https://github.com/GTAmodding/re3 HEAD` → 失败

### 8.2 法律与执法史

1. GitHub DMCA 索引 · Take-Two legal action（2021-09-29，re3/reVC 分支网络屏蔽；由 451 响应体的 `html_url` 指认） — https://github.com/github/dmca
2. Yin-Poole, W. "GTA 3 and Vice City reverse-engineering fan project hit with DMCA takedown", Eurogamer, 2021-02-20 — https://www.eurogamer.net/articles/2021-02-20-gta-3-and-vice-city-reverse-engineering-fan-project-hit-with-dmca-takedown
3. Yin-Poole, W. "…back online after Take-Two takedown", Eurogamer, 2021-06-28 — https://www.eurogamer.net/articles/2021-06-28-gta-3-and-vice-city-reverse-engineering-fan-project-back-online-after-take-two-takedown
4. Yin-Poole, W. "GTA reverse engineering fan projects pulled offline for a second time…", Eurogamer, 2021-10-02 — https://www.eurogamer.net/articles/2021-10-02-gta-reverse-engineering-fan-projects-pulled-offline-for-a-second-time-after-yet-another-take-two-takedown
5. Scullion, C. "Take-Two is suing the creators of GTA 3 and Vice City reverse engineering projects", VGC, 2021-09-03 — https://www.videogameschronicle.com/news/take-two-is-suing-the-creators-of-a-gta-3-and-vice-city-reverse-engineering-projects/
6. "GTA 'reverse-engineer' modders defend projects as under 'fair use'", NME, 2021-11-17 — https://www.nme.com/news/gta-reverse-engineer-modders-defend-projects-as-under-fair-use-3097321
7. Chalk, A. "Take-Two dismisses lawsuit against Grand Theft Auto modders", PC Gamer, 2023-04 — https://www.pcgamer.com/
8. "GTA 3 and Vice City get reverse engineered with a new game engine", GamingOnLinux, 2021-02 — https://www.gamingonlinux.com/2021/02/gta-iii-and-vice-city-get-reverse-engineered-with-a-new-game-engine/
9. "How a small group of GTA fanatics reverse engineered GTA 3 and Vice City…"，Eurogamer — https://www.eurogamer.net/how-a-small-group-of-gta-fanatics-reverse-engineered-gta-3-and-vice-city-without-so-far-getting-shut-down-by-take-two
10. Livingstone, C. "GTA modding tool OpenIV shuts down due to cease and desist from Take-Two", PC Gamer, 2017-06-15 — https://www.pcgamer.com/uk/gta-modding-tool-openiv-shuts-down-claiming-cease-and-desist-from-take-two/
11. Livingstone, C. "Rockstar is talking with OpenIV developer, issues new statement on mods", PC Gamer, 2017-06-23 — https://www.pcgamer.com/uk/rockstar-is-talking-with-openiv-developer-issues-new-statement-on-mods/
12. "GTA modding tool OpenIV resurfaces following Take-Two's cease and desist", Eurogamer, 2017-07-03 — https://www.eurogamer.net/gta-modding-tool-openiv-resurfaces-following-take-twos-cease-and-desist
13. Macgregor, J. "Take-Two has been issuing takedowns for GTA mods", PC Gamer, 2021-07-18 — https://www.pcgamer.com/uk/take-two-has-been-issuing-takedowns-for-gta-mods/
14. Bailey, D. "Take-Two says it's 'pretty flexible' when it comes to DMCAing GTA mods", PCGamesN, 2021-08-02 — https://www.pcgamesn.com/gta-mods-dmca
15. "GTA 5 unofficial RageMP mod ordered to shut down by Take-Two", Insider Gaming — https://insider-gaming.com/gta-5-unofficial-ragemp-mod-ordered-to-shut-down-by-take-two/
16. Rockstar Games Mod Guidelines（页面日期 2026-09-22） — https://www.rockstargames.com/community-resources/mod-guidelines
17. Rockstar Support · Online Suspension and Ban Policy（2024 修订） — https://support.rockstargames.com/
18. "Rockstar Revamps Ban Policy for GTA Online Players"（BattlEye），GTA Boom, 2024-09-19 — https://www.gtaboom.com/rockstar-revamps-ban-policy-for-gta-online-players-6eaf
19. "GTA Online Cheaters' Accounts Reset For Using Money Exploit", GameSpot, 2020-09-04 — https://www.gamespot.com/articles/gta-online-cheaters-accounts-reset-for-using-money/1100-6481632/
20. FiveM / RedM 条款（Rockstar Games Creator Platform License Agreement） — https://fivem.net/terms 、 https://redm.gg/terms （仓库 LICENSE 直接引用）

### 8.3 GTA VI / RAGE 技术面

21. Grand Theft Auto VI — Wikipedia（retrieved 2026-11-18） — https://en.wikipedia.org/wiki/Grand_Theft_Auto_VI
22. Rockstar Advanced Game Engine — Wikipedia（retrieved 2026-11-18，引 rage.re 词汇表 2023-05-18） — https://en.wikipedia.org/wiki/Rockstar_Advanced_Game_Engine
23. Take-Two Interactive 收购列表 — Wikipedia — https://en.wikipedia.org/wiki/List_of_acquisitions_by_Take-Two_Interactive
24. Judd, W. "GTA 6 PS5 Pro upgrades finally revealed…", Digital Foundry, 2026-11-10 — https://digitalfoundry.net/news/2026/11/gta-6-ps5-pro-upgrades-finally-revealed-frame-rate-upscaling-ray-tracing-confirmed （自动化抓取返回 403，结论按 agent 记录标注）
25. GTA VI 新闻专线（30 FPS / 预载 / PS5 Pro 条目转载） — https://www.rockstargames.com/newswire
26. Grand Theft Auto — Modding and fan recreations — Wikipedia（retrieved 2026-11-18） — https://en.wikipedia.org/wiki/Grand_Theft_Auto
27. GTAMods Wiki · GTA V 文件格式导航 — https://gtamods.com/wiki/Grand_Theft_Auto_V ；C++ Script Hook — https://gtamods.com/wiki/C%2B%2B_Script_Hook ；OpenIV 页 — https://gtamods.com/wiki/OpenIV
28. rage-engine-formats（Rust，RPF7/YDR/YTD，含 Enhanced 布局版本）2026-10-29 — https://lib.rs/crates/rage-engine-formats
29. OpenIV — https://openiv.com/ · Script Hook V / NativeDB / 反编译脚本 — http://www.dev-c.com/gtav/ · alloc8or NativeDB — https://alloc8or.re/gta5/nativedb/ 、 https://alloc8or.re/rdr3/nativedb/
30. FiveM natives reference — https://docs.fivem.net/natives/
31. "Native registration table"（.ysc / VM 内部结构研究），UnKnoWnCheaTs, 2022-03-16 — https://www.unknowncheats.me/forum/grand-theft-auto-v/493135-native-registration-table.html
32. rage.re — RAGE 命名空间词汇表 — https://rage.re （经维基百科引用；本次未独立抓取正文）
33. AUR `re3-git` / `revc-git`（构建源与 `LicenseRef-None` 认定） — https://aur.archlinux.org/packages/re3-git

### 8.4 项目官方站点

34. OpenRW — https://openrw.org/ · GitHub `rwengine/openrw`
35. librw — https://github.com/aap/librw
36. CodeWalker — https://github.com/dexyfex/CodeWalker · https://www.gta5-mods.com/tools/codewalker-gtav-interactive-3d-map
37. ScriptHookVDotNet — https://scripthookvdotnet.github.io · https://github.com/scripthookvdotnet/scripthookvdotnet
38. MTA — https://multitheftauto.com · https://github.com/multitheftauto/mtasa-blue
39. Cfx.re — https://cfx.re · txAdmin https://github.com/citizenfx/txAdmin · natives https://github.com/citizenfx/natives
40. FusionFix — https://github.com/ThirteenAG/GTAIV.EFLC.FusionFix
41. alt:V 文档 — https://docs.altv.mp（`altv.dev` 跳转） · RageMP 维基 — https://wiki.rage.mp（Cloudflare 反爬）

### 8.5 本报告写作过程中的工具与通道说明

- 版本/许可/星数核验：`git ls-remote`、GitHub REST API（`/repos/*`、`/search/repositories`、`/rate_limit`）。
- 规模统计：本地 `find` + 逐行计数，口径见 6.2（剔除 `vendor/contrib/extern/third_party/node_modules/deps/packages`；物理行）。
- 交叉验证：Wayback CDX、archive.org、raw.githubusercontent、Google DoH（`https://dns.google/resolve`）。
- 已知通道异常：本机 WebSearch 返回大量 AI 农场/SEO 结果，Bing/Mojeek/DuckDuckGo 对自动化请求有拦截；核验 agent 曾观测到 `github.com` 解析到非 GitHub 段（`172.182.252.133`），**我在同一环境复核得到的是正常结果**（`getent` → `140.82.114.4`，DoH → `140.82.112.4`），故本报告仅将"间歇性 DNS/透明代理介入"记为**未定观测**，不作为结论（详见 3.2.6）。
- 全程只读：未编译、未执行任何仓库代码；未运行任何 `vendor/` 预编译二进制。
