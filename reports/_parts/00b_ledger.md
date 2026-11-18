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
