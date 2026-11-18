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
