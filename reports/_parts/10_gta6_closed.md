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
