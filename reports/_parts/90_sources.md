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
