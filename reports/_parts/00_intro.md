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
