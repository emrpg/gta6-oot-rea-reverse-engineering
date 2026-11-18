# OoT 周边工具与 App 专项（三）：交互式地图 / 追踪器 / 存档编辑 / 音乐工具 / 练习菜单

> 报告编号：08  
> 报告日期：2026-11-18  
> 分析对象：《塞尔达传说：时之笛》(The Legend of Zelda: Ocarina of Time) 的**周边工具与 App 生态**——3D/2D 交互式地图、Randomizer 进度追踪器（Tracker App）、存档编辑器、音乐/曲谱工具、练习菜单与竞速辅助，以及闭源商业伴侣服务  
> 方法：① 本地既有仓库源码实读；② 本轮新增 `git clone --depth 1` 共 **53 个**周边工具仓库到 `/root/wsl-demo/oot-peripherals/`（`du` 合计 2.9 GB）；③ 闭源侧（MapGenie / Zelda Dungeon / EmoTracker / track-oot.net / 官方 Zelda Notes / iOS 商业 App）以官网与一手页面取证。所有 `路径:行号` 均实读核对。  
> 前序报告：`02`（OoT 反编译/重编译生态）、`04`（闭源侧技术对比）、`05`（extra-repos）、`07`（GTA 周边工具）。本报告只写"周边工具"这一层，不复述引擎取证结论。

---

## 证据分级图例

| 级 | 含义 |
|---|---|
| **F** | 本地仓库源码 / 官方站点 / 一手文档，本次直接读取 |
| **R** | 可靠媒体或社区 wiki 的转述，未回到一手 |
| **U** | 二手传闻、自媒体、来源单一 |
| **?** | 存疑、来源冲突或本次无法验证 |
| **未找到** | 本轮取证明确检索过且无结果 —— **不得写成事实** |

---

## 0. 核心判断（先给结论）

1. **OoT 的周边工具生态是"三层依赖"结构，而不是并列的一堆 App**：底层是**数据供给层**（noclip 的 `.zelview0` dump、OoTR 的 spoiler log/auto-tracker context、模拟器的内存总线、存档文件），中层是**语义层**（场景名/物品 ID/逻辑规则），上层才是用户看到的地图与勾选框。判一个 tracker 的寿命，看它绑在哪一层：绑 spoiler log（文本格式，稳定）的活得久，绑内存偏移（版本敏感）的活得辛苦，绑官方 ROM 资产（法律敏感）的直接死。
2. **能看 N64 原版 OoT 全 3D 场景的唯一公开途径是 noclip.website**（F）。它不是"截图地图"：`src/zelview/f3dzex.ts:807-835` 定义了 `F3DZEX_GBI` 枚举、`:857 runDL_F3DZEX()` 用约 26 条 `case` 在 TypeScript 里**直接解释显示列表**（对照 EAD 自带反汇编器 `oot/src/code/ucode_disas.c` 的 92 个 `case G_`、RT64 的 113 条 `GBIInstance`）。这意味着浏览器里跑的是一条**独立实现的 RSP 几何前端**。
3. **noclip 的场景命令 ID 与反编译项目逐项吻合，但它自陈"0x09 未知"**（F）：`src/zelview/tools/extractor.ts:33-51` 的 `HeaderCommands`（Spawns=0x00 … End=0x14）对应 `oot/include/scene.h:403-423`，而 `:42` 的 `// 0x09 is unknown` 说明它不是抄 decomp 的表。场景标识直接用原版名（`ydan_scene`/`Bmori1_scene`/`HAKAdan_scene`/`tokinoma_scene`），可在 `oot/include/tables/scene_table.h:12,13,15,19,77,79,83,97` 一一对上。**两条独立逆向路径在同一个编号表上收敛，是该引擎结构可信度的最强证明。**
4. **noclip 的法律姿态是"代码 MIT + 明写非干净室 + 派生数据自托管"**（F）：`LICENSE:3-5` 原文 "Some implementations of game softwares were reverse-engineered, and **not in a "clean-room" manner**"；仓库内 `src/assets/` 只有字体与图标（5 个文件），但 `DataFetcher.ts:11-14` 显示生产环境数据来自独立的 `PUBLIC_STORAGE_URL`，`zelview/scenes.ts:149` 请求 `ZeldaOcarinaOfTime/spot04_scene.zelview0`。**这与 Ship of Harkinian/Zelda64Recomp 的"用户自备 ROM"是两种不同的风险模型**：noclip 自己托管派生 dump。
5. **2026-11-05 官方 Switch 2 重制版把"周边工具"重写了一遍**：本报告新克隆的 54 个仓库中，**存档编辑器、地图追踪器、随机izer、trainer 四类都出现了"Remake/SW2"专用实现**（`OOT-SW2SE`、`ZeldaOOTRemakeSaveEditor`、`Ocarina-Atlas`、`oot-remake-rando`、`NX2navi-Trainer`），而闭源侧 MapGenie 的 OoT 条目**现在只剩"Switch 2 Remake"**、N64/3DS 的老图被标 "Under Construction"。商业方比开源方切换得更快。
6. **任天堂这次主动下场做伴侣 App**：**ZELDA NOTES**（Nintendo Switch App 内，免费、无需 NSO 订阅）= 时光之线任务日志 + 地图 + 今日统计 + 传闻任务 + **可演奏陶笛/作曲/二维码分享** + 2000+ 问答 + amiibo 每日空投（F，官方 2026-09-09 页面）。这是 OoT 历史上第一次"官方周边 App"，直接吞掉了 MapGenie/Zelda Dungeon 的一部分需求（对照第 04 号报告的判断：任天堂 2026 年的策略是自己下场）。
7. **追踪器生态的 ABI 是随机izer 自己给的**（F）：本地 `oot-decomp-projects/OoT-Randomizer/Notes/auto-tracker-ctx.md:1-5` 定义了 **auto-tracker context**：固定地址 `0x8040_000C`、4 字节版本号（当前 6）、字段表（`0x38 RAINBOW_BRIDGE_CONDITION`、`0x48 SPECIAL_DEAL_COUNTS`…）。写入侧见 `Patches.py:964`、`:1220-1226`，符号来自 `Rom.py:52` 的 `data/generated/symbols.json`。**"让外部工具无需 symbols.json 也能可靠读取"是明文设计目标** —— 这在商业游戏里极其罕见。同一文档 `:27-29` 也暴露了脆弱点：randomizer 6.2.181 改了布局却没改 context 版本。
8. **闭源地图 App 的真实技术形态是 Web 地图引擎**（F）：MapGenie = MapGenie.io / Magic Lamp Technologies Limited / **IGN Entertainment 品牌**，`MapGenie+` **$14.99/年**、自称 276 款游戏地图，页面内是 `.maplibregl-ctrl-*`（**MapLibre GL**），iOS 端开发者署名 Digitech Apps，Android 端 "Map Genie"，免费层带 AdThrive 广告。它是"订阅制攻略数据库"，不是游戏工具。
9. **Zelda Dungeon 没有 OoT 交互式地图**（F，**对任务书前提的更正**）：站内只有 Echoes of Wisdom / Tears of the Kingdom / Breath of the Wild 有 `*-interactive-map`，`/ocarina-…` 与 `/ocarina-of-time-interactive-map/` 均 404。但它的 OoT 攻略页在 **2026-11-17** 已按重制版重构导航（"Nintendo Switch 2 Primary Walkthrough" / "Nintendo 64 / 3DS Primary Walkthrough" / "Master Quest Walkthrough"）。重制版的交互地图位被 Eurogamer / Nintendo Life / **Zelda Central（有 Interactive Map 专页）** 抢走。
10. **"练习菜单"在三个世代是三种东西**（F）：3DS 侧是 `gamestabled/oot3d_practice_menu` 的 **IPS + ARM hook**（注入 `0x100000` 首指令、`svcControlProcessMemory` 重授权 0x5C7000 起 184 KB 新代码页，L1+R1+SELECT 唤出，直读 HID 共享内存 `0x10002000`）；N64/GC 侧是**游戏自带的调试功能**（TCRF 记录各版本 Gameshark 码可开启游戏内 Inventory Editor；GC `gc-eu-mq-dbg` 自带 test scenes 与 `elf_msg`，见第 04 号报告）；2026 重制版则是**官方 Hall of Fallen Foes**（已击败敌人雕像 + 弱点/出没图鉴 + **Boss 无限重战并记录最佳用时**）—— 即"练习模式"被官方吸收为游戏特性。
11. **周边工具的资产来源有三种，法律后果完全不同**（F）：① **托管派生 dump**（noclip）；② **要求用户自备 ROM/存档**（OoTR 官网页脚"do not distribute copyrighted material"、`ootrandomizer.com` 要求 US/JP v1.0；zeldaret/oot 用 MD5 白名单）；③ **自制美术**（Archipelago 的 OoT poptracker datapack `StripesOO7/oot-tracker` 的 `images/` 里每件物品都带 `.xcf` 源文件，如 `1-4 Poh.png` + `1-4 Poh.xcf`，可证非提取）。另有一个 2.0 GB 的反例（`Ecksters/OoT-Interactive-Map`，其中 `maps/` 占 1.2 GB）——见第 5 章。
12. **执法时间点在 2026-11 收紧到了周边工具的上游**：11-05 重制版发售 → 11-06 Lichii 已能 60FPS 运行 → **11-13 任天堂再发 DMCA，下架 Switch 2 模拟器与改机固件代码**（R，新浪财经 2026-11-13），社区同时观察到 `ns2jb` 越狱被用于提取重制版文件；11-17 报道称月初已开使，但法律专家认为对模拟器/重编译工具胜诉"并非板上钉钉"（U）。对照先例：2024-06 GameBanana 数日内收到 60+ DMCA，其中含**王国之泪随机器与自定义地图**（R）。**追踪器/地图类 App 至今没有 Apple 下架记录，iOS 上的商业 tracker 在 11-10/11-12 还照常更新版本号。**

---

## 1. 本地既有仓库

### 1.1 Ocarina-Atlas —— 面向 2026 重制版的实时地图 + 金蜘蛛追踪器

`/root/wsl-demo/extra-repos/n64-oot/Ocarina-Atlas`，HEAD `fae037e6927f0ba47bec3bdae63085579fc44454`（2026-11-17 18:59:12 -0600，"fix(win): fix High-DPI WinForms scaling & clipping in OcarinaAtlas.cs [v2.3.1] (#42)"）。本轮把它上游 `antonioalvromero/Ocarina-Atlas`（★60，TypeScript）重新克隆到 `oot-peripherals/Ocarina-Atlas` 做比对：**两者 commit 完全相同，文件清单相同 —— 本地不是 fork，就是上游本体**（F）。

| 项 | 事实 | 证据 |
|---|---|---|
| 自述 | 面向 OoT Remake（Title ID `0400486024DEC000`）的 "Interactive Hyrule field atlas and dungeon floor tracker"；支持 Lichii/Fritchii/CheezyEmu/Longshot/Project ReLink/Triforce/HeyListen 内存遥测；宣称 **100% (184/184) automatic collectible tracking** | `README.md:5-7` |
| 分发约束 | "仅通过 GitHub Releases 分发，**不含 ROM/密钥/NAND/存档**" | `README.md:15-17` |
| 包元数据 | `name = oot-remake-map`、`version = 2.3.1`、private；运行时依赖只有 5 个（`@tanstack/react-router`、`@tanstack/react-start`、`lucide-react`、`react 19.3.1`、`react-dom`） | `package.json:2-4,16-22` |
| 无 3D 库 | devDeps 里是 `electron 45`/`tailwindcss 4`/`typescript 7`/`vite 8`/`vitest 5`，**没有 three.js/pixi/maplibre** | `package.json:23-35` |
| 许可 | **AGPL-3.0**；README 说明"新写部分 AGPL、上游 `lucasriondel/oot-remake-map` 无 LICENSE、地图素材属 Nintendúo" | `LICENSE:1-2`、`README.md:233-247` |
| 体量 | `src/` 65 个 .ts/.tsx、24,411 行（剔除 18 个测试文件 18,381 行）；另有启动器 15,384 行 | `src/` 统计 |

**它是跨语言的四形态交付**（F，这点在同类工具里独一份）：Web/PWA（`public/manifest.webmanifest:2-16`，`display: standalone`、theme `#13100c`、3 图标；`shortcuts` 字段未找到）+ Windows WinForms 启动器 `launcher/OcarinaAtlas.cs`（C#/.NET Framework 4.x，7,766 行，`AssemblyVersion 2.3.1.0` 见 `:22-28`）+ Linux C 桥 `launcher/linux/longshot_rpc_bridge.c`（2,129 行）+ Electron AppImage（`launcher/linux-appimage/main.cjs`，1,329 行）。`vite.config.ts:78-86` 的插件链里自建了 `lichiiTelemetryPlugin()`（SSE/API 中间件）。

#### 1.1.1 地图渲染：不是 3D，是 1 张 WebP + 58 个手写 SVG

* 世界底图：**单张 6000×4364 WebP（3.4 MB）**，`dataset.map = {image:'/content/maps/hyrule-full.webp', width:6000, height:4364}`（`src/data/oot.json` 的 `map` 键；文件 `public/content/maps/hyrule-full.webp`）。
* 房间/楼层图：**58 个手写 SVG**（含网格 pattern 与出口箭头），合计 4.6 MB，例 `public/content/maps/7e802ffa8684f3cd.svg:1` 的 `viewBox="-28 -35 900 498"`；物品图标 540 个 WebP（31 MB，`public/content/media/`）。
* 渲染方式：`<img>` + `translate3d/scale` 的绝对定位舞台（`src/components/MapViewport.tsx:353,390,393`；滚轮缩放 `:202-205`，拖拽平移 `:324`），坐标投影集中在 `src/lib/coords.ts:11,19,27`（`pinPosition` 输出百分比 CSS），`MIN_ZOOM/MAX_ZOOM` 在 `:40-41`。
* 数据结构：`src/lib/types.ts:6-24` 的 `PlaceMap{id,label,image,aspect}` 与 `Place{kind:'zone'|'dungeon', region, pin, maps[], startMap}`；实例数据 `oot.json` 含 **32 places（20 zone + 12 dungeon）、8 region、184 items（gs 100 / heart 36 / thought 19 / bean 10 / container 8 / fairy 6 / bottle 5）、35 requirements、7 categories**。
* 3D→2D 投影：用 SVG `viewBox` 反解（`getSvgViewBox` `src/lib/lichii-server.ts:1292`），再 `projectToPlaceMap` `:1427`、`projectToWorldMap` `:1473`、按 Y 硬阈值判楼层 `determineDungeonFloor` `:1348`、child/adult 双图 `determineMapId` `:1398`；玩家指针带航向旋转罗盘（`src/components/PlayerPin.tsx:85-88`，`rotate(yaw)`）。

#### 1.1.2 实时通信层：一个 RPC 门面，三类宿主

统一入口是 **JSON-RPC over 持久 TCP `127.0.0.1:6545`**，方法只有三个：`status`、`read_memory`、`search_memory`。

| 宿主类别 | 具体宿主 | 实现 | 关键行号 |
|---|---|---|---|
| 模拟器自带 RPC | Lichii / Fritchii / Eden / CheezyEmu | 直接连 `127.0.0.1:6545`，需在 `%APPDATA%/<emu>/nx2_switches.txt` 写 `EDEN_NX2_DEBUG_SERVER=127.0.0.1:6545` | `src/lib/lichii-server.ts:12-14`（三个常量）、`:20-36`（host/port 环境变量覆盖）、`:1887-1995`（`LichiiRpcClient`，`net.createConnection` 在 `:1967`，队列深度 24 丢最旧、750 ms 连接超时、1 s 重连节流）；README 触发说明 `README.md:36-45,74` |
| PC 静态重编译 | Longshot / Hookshot / Project ReLink / Triforce / HeyListen | **自研桥把"读进程内存"伪装成同一 RPC**：Windows 用 C# `ReadProcessMemory`，Linux 用 C `process_vm_readv` | Win：`launcher/OcarinaAtlas.cs:1209` `internal sealed class LongshotRpcBridge`，注释 `:1206` 明说 "Exposes LichiiRpcClient-compatible status, read_memory, and search_memory"，进程名表 `:1237`，P/Invoke `:1261`（用点 `:1517,1919,2124`），`TcpListener(IPAddress.Loopback, port)` `:1545-1548`；Linux：`launcher/linux/longshot_rpc_bridge.c:30` `GUEST_WINDOW_BASE 0x100000000000ULL`、`:41` 端口 6545、`:301` `process_vm_readv()`（回退 `/proc/<pid>/mem`，`AGENTS.md:47`）、`:1520/:1545/:1605-1617` 三个方法 |
| 自带 tracker API | NX2Saria | 无需桥，`tracker_api = 1` / `NX2SARIA_TRACKER_API=1` | `README.md:35`；heap 下限 `src/lib/lichii-server.ts:2304`（`0x1000000000n` 与 `NX2SARIA_MIN_GUEST_HEAP_PTR 0x100000000n` 的切换在 `:2301,:2309`） |

**内存语义全部动态推导，拒绝硬编码偏移**（这是本项目最有价值的工程约束，`AGENTS.md:57` 明列为"关键不变量"）：

* NSO `MOD0` 定位：`MAIN_MOD0_OFFSET 0x03c4705cn`、以 "MOD0" 字节模式扫描（`src/lib/lichii-server.ts:2293-2295`），`ensureMainBase` `:2383`、`validateMainBaseHeader` `:2347`，重扫节流 1500 ms `:2296`。
* vtable 派生：`deriveDynamicVtables` → `sub36Vt1 = mainBase + 0x04ba1818n`、`sub36Vt2 = mainBase + 0x03ee4f8cn`（`:2353-2356`）。
* `.bss` 单例偏移：`MAIN_SUB6_BSS_OFFSET 0x04e6c7a8n`、`MAIN_SUB24_BSS_OFFSET 0x04e6c938n`、`MAIN_GDM_BSS_OFFSET 0x04e90ce8n`（`:2298-2300`）。
* **玩家坐标**：`Sub36` 头 `x=buf.readFloatLE(0x1c)`、`y=0x20`、`z=0x24`、`dx=0x28`、`dz=0x30`，`yaw = atan2(dx,-dz)`（`:3674-3689`；校验器 `isValidSub36Buffer` `:2715`，同偏移复用于 `:2724-2726`）；场景/年龄从 `Sub[7]+0x34` → `pObj+0x768` 取形如 `Work/Banc/Scene/<X>/<Y>.bcett` 的资源名反解（`:3692-3730`，解析链 `resolveSubPointersFromPObj` `:2965`、`resolvePlayerFromBss` `:2989`、`resolvePlayerViaHeapFallbacks` `:3016`）。
* **收集品**：GameDataMgr `+0x170`（Bool 容器，`GDM_BOOL_ENTRY_STRIDE 0x18n`）与 `+0x230`（Int 容器，stride `0x28n`）、`GDM_STOCK_TOKEN_HASH 0x04735978`、`GDM_COUNT_TAG 8`（`:2320-2326`）；`resolveGameDataMgr` `:2457`、`parseGdmContainerHeader` `:2731`、`classifyGdmCollectibles` `:2796`、`classifyGdmRequirements` `:2854`。
* **金蜘蛛计数**：`parseGdmSkulltulaCount` 校验 vt+tag 后读 `buf.readUInt32LE(0x20)` 并限 0–100（`:2881-2890`）；存档侧同键 `0x04735978`（`:1777-1778`）。
* **主 flag 表**：`MASTER_FLAG_ENTRY_0/1_HASH 0x00006e56f79838e8/ean`、`COLLECTED_BIT 0x100000000n`，32768×6 分块读（`:2329-2334`），解析 `parseMasterFlagTableChunks` `:2468`、`readMasterFlagCollectedIds` `:2659`。
* 场景名 → 地图地点的映射表 `SCENE_TO_PLACE` 覆盖约 **490 条** `Work/Banc/Scene` 名（`:304-908`）。

事件模型是服务端单一 SSE 广播：live 时每 120 ms、非 live 每 750 ms（`:4370-4373`），端点 `/api/lichii/stream`、`/api/sync/state`、`/api/lichii/state`（`:4440,4448,4462`）；缓存 TTL 80 ms `:3861`、存档目录扫描 TTL 1500 ms `:3956`。客户端 `EventSource` + 轮询回退、重连 3 s（`src/lib/useLichiiTracker.ts:109,118`）。

#### 1.1.3 双通道追踪：内存 + 磁盘存档

除 RPC 外它**也直接读重制版的存档**（只读）：`parseSaveSlot` `src/lib/lichii-server.ts:1713-1860` 解析 `caption.sav`/`progress.sav` 的 TLV 头字段哈希 —— `0xdc941483`(GKS 数)、`0xaccbda17`(yaw)、`0x8b26618e`(xyz 指针)、`0xf74e0e8e`(scene)、`0x0cccc6cd`(subScene)（`:1743-1756`），progress 分段 `INPLACE_SECTIONS={1,3,5,7,21}` `:1486`、`0xa4aee8cd`(yaw)、`0xa3db7114`(master flag 数组 `:1820`)。槽位发现按 Title ID 找 `slot_*`：`addSlotIfValid` `:1527`、`collectSlotsFromTitleDir` `:1535`、`:1547`、5 层布局尝试 `:1561-1588`、`detectEmulatorFromPath` `:1594`、`findLatestSaveSlotDir` `:1683`，并跨 Wine/Steam/Flatpak/Bottles 前缀搜索（`:86,117,69`）。**N64 `.sav`/`.a1`/`.eep` 解析器与 CRC 校验：未找到**（重制版存档格式无 CRC；全仓唯一 checksum 相关是发布包 `.sha256`，`src/lib/release-notes.ts:684`）。

诚实性设计值得单列（F）：字段表 `:909-1268`（Skulltula 100、HeartContainer 8、PieceOfHeart 主 flag 14 + bool 22、Bean 10、Fairy 6、Bottle 4、Thought 22、Requirement 34）之上，还有 `ALL_SAVEBUF_SEC1_BOOL_HASHES` 与完整性下限 `EXPECTED_SAVEBUF_COLLECTIBLE_HASH_COUNT`（`:1213-1221`，门控点在 `:3295,:3442`）—— **字段数不足时宁可不上报，也不误报**；`AGENTS.md:56` 明确禁止"接近即猜测"的收集判定，`:57` 要求 ASLR 安全，`:58` 只读保证。逻辑侧**没有完整 logic 引擎**，只有 35 键前置条件门控（`src/lib/prerequisites.ts:74,117,132`）；**OoTR 种子集成、动态 hint 表、IndexedDB、撤销栈：均未找到**。持久化是 `localStorage` + `oot-map:v1:` 前缀（`src/lib/storage.ts:6`，槽位 `:81-160`，写合并 150 ms `:7`），多设备同步走服务端文件（`src/lib/sync-server.ts:29,139,297`，上限 256 KB）。

#### 1.1.4 语言与平台覆盖面

`src/lib/i18n.ts:28-44` 定义 **15 个 locale**（en/es/fr/pt/zh-CN/zh-TW/de/it/ja/ko/nl/ru/pl/tr/uk），每语言一张扁平文案表（`EN_MESSAGES` `:200-387` 共 342 个 key，其余按 188 行间隔排布到 `:2834`），另有 `CATEGORY_LABELS:3040`、`PLACE_LABELS:3178`、`REGION_LABELS:3691`、`REQUIREMENT_LABELS:3844` 各 15 语言；长文本攻略只有 es/fr/pt 有独立 JSON（`:22-26` + `src/data/i18n/`）。**版本覆盖只有一条**：`README.md:168`、`AGENTS.md:8` 都限定单一重制版构建；所谓"差异"仅是同一构建内的 child/adult 双图与 `age`/`night` 过滤。**OoT3D / GameCube / iQue / N64 版：未找到支持**。它也不提 noclip / Track-OOT / EmoTracker / MapGenie / Zelda Dungeon / OoTR 官方（全部未找到）—— 2026 新生态与 2003-2011 老生态之间**没有交叉引用**，这是本报告第 8 章要讨论的断层。

### 1.2 oot3d_practice_menu —— 闭源 3DS 引擎里的练习菜单注入

`/root/wsl-demo/oot-port-projects/oot3d_practice_menu`，HEAD `e8bdabe13797937a121b55a67479568a48fefdeb`（**2022-01-02** 14:29:05 -0500，"Merge pull request #20 from HylianFreddy/master"，作者 `gamestabled`，浅克隆故完整历史未找到），87 files / 17,614 insertions，C/C++/h 合计 **15,734 行**。仓库**无 LICENSE 文件**，唯一许可文本是被复制进来的 Luma3DS GPLv3 头（`src/draw.c:3-27`、`include/utils.h:2-25`）；作者署名不在 README，而在源码头（`include/menu.h:4-5` "Modified 2020 Gamestabled / 2021 HylianFreddy"、`src/main.c:2` 源自 n3rdswithgame、`src/loader.c:2-3` 源自 RicBent 的 Magikoopa；鸣谢段 `README.md:10-18`）。

* 定位（F）：`README.md:4` 给 speedrunner 与 glitch hunter 的屏上菜单；`:6` 方法通用但地址暂硬编码；`:8` 需带 game patching 修复的 Luma3DS。
* 版本矩阵（F）：US `0004000000033500`（`code.ips`+`exheader.bin`，`README.md:29`）、JP `0004000000033400`（`code_j.ips` 改名，`:30`）、EUR `0004000000033600`（`code_eur.ips`，`:31`），需开机按 Select 开 game patching（`:32`）；Citra 用 `exheader_citra.bin` + `code_citra_{us,jp,eur}.ips`（`:36-40`）。
* 工具链（F）：**devkitARM/GCC，不是 Grezzo 用的 RVCT** —— `Makefile:5-10`（DEVKITARM）、`:61` `-march=armv6k -mtune=mpcore`、`:69` `-std=gnu++11 -fno-rtti`、`:72` `-nostdlib -T $(LINK_SCRIPT)`；版本切换 `:41-57`（`oot.ld`/`oot_j.ld`/`oot_e.ld` + `-D _USA_/_JP_/_EUR_`）、`:87-90` `citra=1 → -DCITRA`、`:164-172` 构建后自动跑 `patch.py`。
* **IPS 产物可量化**（F）：`code.ips`/`code_eur.ips`/`code_j.ips` = **147,926 B**，`code_citra_*.ips` = **147,750 B**，两个 exheader = **2,048 B**。`code.ips` 共 **22 条记录**，覆盖 **0x0 – 0x4EA5E4**（即 vaddr 0x100000–0x5EA5E4），EOF 后 0 尾字节（**无 SHA1 尾标**）。构成：1 条 `@0x0` 的 loader stub + 16 个 hook 桩（4 字节各一，地址与链接段一一对应）+ `.loader` `@0x3c99a8` 692 B + svc 桩 `@0x3c9c5c` 2,180 B + `.text` 三段 65,535+65,535+13,798（因 IPS 单记录 64 KB 上限分片，`patch.py:24-39`）。**区域间差异极小**：code.ips vs code_eur.ips 仅 8 处（各 1–4 字节）、code_j.ips 17 处。实机 vs Citra 记录数与偏移完全相同，差别在 `.loader` 内 3 字节、svc 表内 171 字节与末段 13,622 B —— 根源是 `src/draw.c:226,234` 的 `#ifndef CITRA` 去掉了 `svcFlushProcessDataCache`。
* exheader 差异（F）：magic `CtrApp`，两文件仅 7 字节不同，位于 0x34/0x38/0x3C 三个 u32（实机 `0x12E/0x12E000/0x0`，Citra `0x62/0x61220/0x19B000`）——**追加内存区域配置不同，用于给 0x5C7000 起的新代码页建立映射**。同样的差异模式出现在 `OoT3D_Randomizer/romfs/exheader*.bin`，属同源工具产物。
* 注入机制（F）：`.patch_loader 0x100000` 覆盖 `code.bin` 首指令（`oot.ld:6-7`；IPS 首记录 @0x0）→ `src/patches.s:18-21 loader_patch: b hook_into_loader` → `src/hooks.s:37-44`：`bl loader_main` 后 `bl 0x100028`（原指令）再 `b 0x100004`。内存重授权：`src/loader.c:20-31` `svcControlProcessMemory(handle, NEWCODE_OFFSET, NEWCODE_OFFSET, size, 6, 7)`（RWX），常量 `src/newcodeinfo.h:9-10`（`0x5C7000` / `0x2E000`），syscall 桩 `src/svc_loader.s:17-45`。
* 运行时对象（F）：每帧钩子 `oot.ld:63` → `patches.s:13-16` → `hooks.s:24-35` → `src/main.c:255-257 setGlobalContext()`；`GlobalContext` 字段偏移在 `include/z3D/z3D.h:403-436`（actorCtx `0x208C`、csCtx `0x2298`、roomCtx `0x4C30`、linkAgeOnLoad `0x5C00`、sceneLoadFlag `0x5C2D`、nextEntranceIndex `0x5C32`、mainCamera `0x364`），`PLAYER` 宏 `z3D.h:494`，`gSaveContext = 0x00587958` `z3D.h:482`（三区域共用，无 ifdef）。
* 输入（F）：**不 hook SDK，直读 HID 共享内存** —— `z3D.h:520-521`（`real_hid 0x10002000`）、`src/input.c:8-13`（`*(0x005AEC5C)+0x1C`）；组合键状态机 `src/commands.c:256-306`，默认 **L1+R1+SELECT** 唤出（`:262-268`，含防锁死设计）。
* 功能实现（F，逐项带偏移）：无敌 `src/cheats.c:75-77`（`health=healthCapacity`，字段 `z3D.h:74-75` = `0x42/0x44`）；时间冻结 `:38-40,127-129`（`dayTime 0x0C`，`z3D.h:61`）；帧进/暂停 `main.c:29-41,175-208`；**传送核心 API** `src/entrances.c:38-56 EntranceWarp()`（写 nextEntranceIndex/fadeOutTransition/linkAgeOnLoad/nextCutsceneIndex 后置 `sceneLoadFlag=0x14`）；物品给予**不调 Item_Give**，直写 `gSaveContext.items[]`/ammo/`itemMenuChild`（`src/inventory.c:217-218,239-251,1080-1093`，字段 `z3D.h:91-98`）；装备 `src/equips.c:34-71`（`Player_SetEquipmentData 0x34913C`，`z3D.h:543-549`）；坐标显示 `src/debug.c:73-74`（actor world pos 浮点打印，偏移 `z3Dactor.h:139` = `0x028`）；任意内存观察 `main.c:43-122` + `src/watches.c:38-41,120-132`，非法地址保护用 `svcQueryMemory`（`src/common.c:4-13`，`main.c:51-55`）；NoClip/冻结 Link `src/scene.c:109-124` + `main.c:211-252`（`stateFlags2 |= 0x08000000`）；Actor 堆遍历/删除/生成 `src/debug.c:42-47,215-217,107-141`（`Actor_Spawn 0x3738D0`/JP `0x3733E8`，`z3D.h:533-539`）。**BOSS 立即重置：未找到实现**（最接近是 Warps→Bosses 表 `src/warps.c:56-71`；`bossBattleVictories[9]` 仅在 `z3D.h:146` 声明，全仓无引用）。菜单装配 `src/menus.c:44-58`（`gz3DMenu "Practice Menu"`）。
* **地图/追踪器功能：未找到**（无 romfs、无坐标追踪、无 spoiler）。
* **与 OoT3D_Randomizer 是同祖异枝**（F，重要）：`patch.py:13` 与 `OoT3D_Randomizer/code/patch.py:13` 逐行一致，注入骨架同源（Randomizer `code/oot.ld:3,9-10` 的 `ENTRY(loader_patch)`/`.patch_loader 0x100000`/`nninitRegion=0x100028` ↔ 本项目 `hooks.s:39-44` 把它写死成 `bl 0x100028`；两边 `hooks.s:30/:34` 同为 `b 0x2E25F0`；`loader.c` 版权头逐字相同）。**分岔点**：Randomizer 已改用 `PATCH/HOOK` 宏（`code/src/asm/patches.s:3-11`）与自动符号 `NEWCODE_OFFSET/_LD_TEXT_START`（`code/src/ld_symbols.h:14-17`），本项目仍是硬编码 `newcodeinfo.h:9-10`。**同一个 3DS 补丁项目族里，一边进化成可维护框架，一边停在硬编码快照** —— 这条差异在第 7 章用于解释"周边工具维护性"的分化。
## 2. 交互式地图与 3D 场景查看器

### 2.1 noclip.website —— 一个自研 RSP/RDP 前端，两条 OoT 通道

`oot-peripherals/noclip.website`，上游 `magcius/noclip.website`（作者 Jasper St. Pierre），★4,315，MIT © 2018，HEAD `2cebac2a`（2026-11-14），83 MB，pnpm 工作区 + Rust/wasm32 子模块，运行时依赖只有 6 个（crc-32 / fzstd / gl-matrix / librw / reflect-metadata / tiff）。它在 `src/main.ts` 里注册了 **三个** OoT 相关场景组（F）：

| 注册点 | 模块 | 目标版本 | 体量 |
|---|---|---|---|
| `main.ts:16` + `main.ts:198` | `src/zelview/scenes.js` | **N64 OoT** | `src/zelview/` 共 2,048 行 |
| `main.ts:17` + `main.ts:257` | `src/zelview/scenes_beta.js` | N64 OoT 内测/异常转储 | 同上目录 |
| `main.ts:18` + `main.ts:180` | `src/OcarinaOfTime3D/oot3d_scenes.js` | **3DS OoT3D**（同目录还带 MM3D `:19`、LM3DS `:20`） | 13 个 .ts / **9,831 行** |

**这是"能否在浏览器里看 N64 原版 3D 场景"这一问的唯一肯定答案**：N64 侧不是贴图拼图，而是把 SGI 微码搬进 TypeScript。

#### 2.1.1 N64 通道：`zelview` = 段式地址 + F3DZEX2 解释器

* 容器格式 `ZELVIEW0` **故意保留游戏里的段式（segmented）地址**，而不是像其他游戏那样先把指针重定位成文件偏移：`zelview0.ts:11-15` 的 `VFSEntry{filename, buffer, vStart, vEnd}` 与 `:31 lookupFile(vStart)` 直接以虚拟地址索引。
* 显示列表解释器：`zelview/f3dzex.ts:807-835` 定义 `enum F3DZEX_GBI`（G_VTX=0x01 … G_LINE3D=0x08；几何侧 0xD7/0xD8/0xD9/0xDA/0xDE/0xDF；RDP 侧 0xFF…0xF6），`:857 runDL_F3DZEX()`，`:873 cmd = w0 >>> 24`，`:877-1020` 约 **26 条 `case`**。对照 EAD 反编译自带的离线反汇编器 `oot/src/code/ucode_disas.c`（92 个 `case G_`）与 RT64 的 113 条 `GBIInstance`：**noclip 只实现了一条子集**——它只需要"能画出这一帧"，不需要反汇编整个 ROM。
* 场景目录：`zelview/scenes.ts:16 pathBase = 'ZeldaOcarinaOfTime'`，`:149 fetchData(\`${base}/${id}.zelview0\`)`，共 **110 条 `ZelviewSceneDesc`**（spot04 / ydan / Bmori1 / HAKAdan / ddan / HIDAN / tokinoma / hiral_demo / hakaana_ouke …），场景名与 `oot/include/tables/scene_table.h:12,13,15,19,77,79,83,97` **逐条可对齐**（F）。
* 已知缺口（F，诚实标注）：`zelview/scenes.ts:159` 留 `"TODO: Implement scenes with JFIF backgrounds"` —— **N64 原版的预渲染背景（JFIF 静态背景）尚未支持**，即打开一个森林场景能看到地形网格却看不到背景图。
* 特殊条目：`zelview/scenes_beta.ts:6-8` id=`zelview_beta`，显示名 **"The Legend of Zelda: Ocarina of Time (F-Zero Overdump)"**，挂 37 个纯数字场景号 —— 这是一份把别的 ROM 数据错当 OoT 转储的样本，被当作独立可浏览条目保留。

#### 2.1.2 三个独立实现给出的场景命令表**并不一致**（本报告最重要的交叉验证结果）

同一张 N64 场景头命令表，在三个互不相通的代码库里各写了一遍：

| # | `oot/include/scene.h:403-425`（zeldaret 反编译） | `SceneNavi/ROMHandler/HeaderLoader.cs:92-107`（C#，2014） | `noclip/src/zelview/tools/extractor.ts:33-51`（TS） |
|---|---|---|---|
| 0x00 | `PLAYER_ENTRY_LIST` | `Spawns` | `Spawns` |
| 0x02 | **`UNUSED_2`** | `Unknown0x02` | **`Camera`** ← 分歧 |
| 0x03 | `COLLISION_HEADER` | `Collision` | `Collision` |
| 0x04 | `ROOM_LIST` | `Rooms` | `Rooms` |
| 0x06 | `SPAWN_LIST` | `Entrances` | `EntranceList` |
| 0x08 | `ROOM_BEHAVIOR` | `RoomBehavior` | `SpecialBehavior` |
| 0x09 | **`UNDEFINED_9`** | `Unknown0x09` | 注释 `// 0x09 is unknown` |
| 0x0A | `ROOM_SHAPE` | `MeshHeader` | `Mesh` |
| 0x0B | `OBJECT_LIST` | `Objects` | `Objects` |
| 0x0C | **`LIGHT_LIST`** | `Unknown0x0C` | 注释 `// 0x0C is unused` ← 分歧 |
| 0x0D | `PATH_LIST` | `Waypoints` | `Waypoints` |
| 0x0E | `TRANSITION_ACTOR_LIST` | `Transitions` | `Transitions` |
| 0x0F | `LIGHT_SETTINGS_LIST` | `EnvironmentSettings` | `Environment` |
| 0x14–0x19 | `END / SOUND / ECHO / CUTSCENE / ALTERNATE_HEADER / MISC` | — | 只到 `End = 0x14` |

三点结论（F）：

1. **编号 100% 一致，语义命名反映不同年代**：0x00/0x06 的差异（`Spawns/Entrances` vs `PLAYER_ENTRY_LIST/SPAWN_LIST`）是反编译项目后期的**重命名**，两个工具用的是旧社区命名；因此这不是理解冲突，而是**同一认知在不同时间点留下的化石层**。
2. **0x0C 是分水岭**：反编译已解明为 `LIGHT_LIST`（光源表），而 2014 年的 SceneNavi 仍写 `Unknown0x0C`、noclip 至今写 `// 0x0C is unused`。→ **周边工具普遍落后反编译项目一个认知世代**；它们只需要"够画出来"，不需要把每个表都解出来。
3. **0x02 是真冲突**：反编译标 `UNUSED_2`，noclip 却实现了 `Camera = 0x02`。本次无法判定谁错（noclip 未在 `scenes.ts` 之外给出 0x02 的解析器），**列 `?`**。

#### 2.1.3 3DS 通道：解析 Grezzo 的闭源容器

`OcarinaOfTime3D/zar.ts:2` 直接把 `.zar` 写作 "Zelda ARchive / **Grezzo ARchive**"，`:8-10` 用 magic 区分四作 —— `ZAR\x01` = OoT3D、`GAR\x02` = MM3D、`GAR\x05` = Luigi's Mansion 3DS；`:55` 的内部代号断言 `'queen'`(OoT3D)/`'jenkins'`(MM3D)、`:92` LM3DS 用 `'SYSTEM'/'agora'`。**`.zar` 与 OoT3D 随机器解析的是同一容器**（对照 `OoT3D_Randomizer/source/patch.cpp:660-661`，F）。

| 文件 | 本质 | 关键取证 |
|---|---|---|
| `zar.ts` | 容器包 | magic/版本见上 |
| `LzS.ts:13` | 仅是压缩封装 | `LzS\x01` → LZSS，`:8` 在 0x08 处存解压后长度 |
| `cmb.ts:1144-1163` | **Grezzo 版 CMDL 模型容器**（非碰撞） | magic `'cmb '`；版本字 `0x06`=Ocarina / `0x0A`=Majora / `0x0C`=EverOasis / `0x0F`=LM（`:24-26`）；子块 `skl `:59、`mats`:402、`vatr`:865、`mshs`:912、`prm `:955；`VatrChunk`(`:12-23`) = position/normal/tangent/color/uv0-2/boneIdx/boneWt；材质含完整 **combiner 枚举**(`:114-219`)；纹理解码走 `Common/CTR/pica_texture.js` |
| `zsi.ts:397-401` | 场景头（`:409` 自陈"ZSI 是 Z64 场景头的微改"） | `ZSI\x01`→Ocarina、`ZSI\x09`→Majora；`:101-111` HeaderCommands |
| `cmab.ts:300-343` | 材质动画（UV 滚动/颜色） | `'cmab'`+subversion 0x01(`:306`)，子块 `strt`/`mads`(`:221`)/`txpt`(`:174`) |
| `csab.ts:311-317` | 骨骼动画（Hermite 关键帧） | OoT3D 走 `parseOcarina`：subversion `0x03`(`:215,219`)，MM/LM 为 `0x05`(`:267`)，子块 `anod`(`:184`) |
| `ctxb.ts:13-19` | 纯纹理包 | `'ctxb'`，复用 `cmb.parseTexChunk` |

渲染不是固定管线 PICA，而是**自研 GLSL3 转发着色器模拟 PICA combiner**：`render.ts:44 DMPProgram`、`:58 BindingsDefinition`、`:451 OoT3DProgram`，8 个顶点属性（`:49-56`）。场景目录 `oot3d_scenes.ts`（3,725 行 / 193 KB）：`:3591` id=`oot3d`，**109 条 SceneDesc 按 10 个地区分组**，另含 **Master Quest（`:3712` 起，`_dd` 后缀）**；`:316 enum ActorId` **431 个成员**，`:857-1300` 有 **302 处 `actor.actorId ===` 逐 actor 特判**（例 `:1045-1054` 用 `variable>>>12` 反解宝箱尺寸）。**碰撞未还原**：`zsi.ts:103` 定义了 `Collision = 0x03`，但 switch 分支 `:376-388` 里没有对它的处理 —— 与 N64 通道形成鲜明对照（N64 侧 SceneNavi 有 `HeaderCommands/Collision.cs`）。

#### 2.1.4 作为"地图 App"的能力与不自足之处

* **相机状态 = 可分享的地图书签**：`SaveState.ts:8-12`（cameraWorldMatrix + sceneData + fovY），序列化为 `ShareData=<Ascii85>`(`:196,235`)；URL 格式 `#<sceneDescId>;ShareData=…`（`main.ts:599-611` 解析，`:830,843` 用 `history.replaceState` 回写）。内置 **7,679 条** `src/DefaultSaveStates.json`，其中 **oot3d 113 条覆盖 104 个场景**（即绝大多数 3DS 场景都预置了"看点"）。
* **没有 overworld/minimap 拼装**：`src/` 内不存在任何底图拼接或小地图逻辑（**未找到**）；`oot3d_scenes.ts:124-190` 的面板只有 Render Hacks 与环境滑条。→ **noclip 是场景查看器，不是地图 App**，尽管它比多数地图 App 更了解几何。
* **渲染后端由 URL 查询串决定**（F，已核实）：`main.ts:488 if (location.search.includes('webgpu'))` → `main.ts:515 await initializeViewerWebGPU(canvas)`，`viewer.ts:366-371` 实现、`:403-404` 是不支持时的文案。**快捷键 T / 1-9 / . / , / F4 在代码里 0 命中，本报告不采信**（仅在站点帮助文本中出现，列 R）。
* **资产全部外置**：`.env:1 PUBLIC_STORAGE_URL=https://z.noclip.website`；`src/DataFetcher.ts:11-15` 开发态 base=`/data`、生产态取该变量，`:17-23` 断言路径不得以 `data/` 开头；`rsbuild.config.ts:77-92` 仅在 dev 挂载本地 `data/`。仓库 `data/` 只有 `Examples` + `.gitkeep`，`src/assets/` 只有 logo/字体。3DS 路径规则：`oot3d_scenes.ts:838 pathBase='ZeldaOcarinaOfTime3D'`，`:3508-3509` 拼 `…/scene/<id>.zar` 与 `<id>_info.zsi`。
* **用户自带文件即可离线看**：`Scenes_FileDrops.ts:78` 识别拖入的 `.zar/.gar/.gar.lzs`，转交 `OcarinaOfTime3D/scenes.ts:355 createSceneFromZARBuffer`（入口 `main.ts:752-753`）。
* **署名机制缺失**：README:3 说 "See the application for full credits"，但当前代码树里 **没有任何 credits UI 文件**（`find -iname '*credit*'` 为空，`src/ui.ts`/`index.html` 无面板，**未找到**）。OoT 相关署名只存在于源码注释：`oot3d_scenes.ts:3593`、`mm3d_scenes.ts:949`（"Names graciously provided by James Knight… organized by Starschulz"），另有一批 `throw new Error("Starschulz")` 形式的待办标记（如 `oot3d_scenes.ts:1302`）。README:7-13 另有 AI 贡献披露政策。

### 2.2 SceneNavi —— 2014 年的 C# N64 关卡编辑器（本地缺口填补者）

`oot-peripherals/SceneNavi`，上游 `xdanieldzd/SceneNavi`（Daniel Rudein / DigitalZero Domain），MIT © 2014（`LICENSE.md:1-3`），HEAD `d0813a3`（2021-03-12），1.6 MB，**78 个 .cs / 7,166 行**（F）。README:1-6 自述 **"A level editor for The Legend of Zelda: Ocarina of Time and Majora's Mask, written in C#"**，要求 .NET Framework 4 + **OpenTK 1.1 + Nini 1.1.0**（`README.md:11-16`）。

结构上是三类模块的组合（F）：

* `ROMHandler/` —— ROM 直读：`DMATableEntry.cs`、`SceneTableEntryOcarina.cs`/`SceneTableEntryMajora.cs`、`ObjectTableEntry.cs`、`EntranceTableEntry.cs`、`ActorTableEntry.cs`、`HeaderLoader.cs`；
* `HeaderCommands/` —— **每个场景命令一个处理器**：`Actors.cs`、`Collision.cs`、`EnvironmentSettings.cs`、`MeshHeader.cs`、`Objects.cs`、`Rooms.cs`、`SettingsSoundScene.cs`、`SpecialObjects.cs`、`Waypoints.cs`、`Generic.cs` + `IPickableObject`/`IStoreable` 接口（可拾取/可写回）；
* `SimpleF3DEX2/` —— 又一条**独立的 GBI 解释器**：`F3DEX2Interpreter.cs`（862 行），`:222 nextcmd[i] = (General.UcodeCmds)(nextw0[i] >> 24)` 做段内跳转解析，`:309` 按 `w1>>24` 取段号建 `Vertex`；`General.cs:65,67,71` 给出 `POPMTX=0xD8 / MTX=0xDA / DL=0xDE`，与 noclip 的 `F3DZEX_GBI` 编号一致；`CombinerEmulation/` 用 `ArbCombineManager.cs` + `GLSLCombineManager.cs` 两套后端把 **N64 combiner 翻译成 GL 组合器**。

**它同时是"碰撞已还原"的那一个**（`HeaderCommands/Collision.cs`），恰好补上 noclip 3DS 通道的缺口。加上 `TableEditorForm.cs`（表编辑）与 `ActorRendering/`，这是本批次里唯一**可写回**的场景工具 —— 其余地图工具全部只读。

### 2.3 Ecksters/OoT-Interactive-Map —— 2.0 GB 的瓦片式交互地图

`oot-peripherals/OoT-Interactive-Map`，★34，MIT（`LICENSE:1-3`），HEAD `020dab1b`（2025-02-21），**2.0 GB**（F）：

* 技术栈：**Leaflet 1.0.2 + jQuery 3.1.1 的纯前端**（`index.html:288-289`），`L.CRS.Simple`（`js/scripts.js:49`）；**无 three.js、无 3D**。
* 底图：**4 层瓦片金字塔** `maps/{child,adult}{Angled,Top}/{z}/map_tile_{x}_{y}.png`（`scripts.js:20-33`），zoom 8–15，**37,864 个 PNG 被 git 跟踪，`maps/` 占 1.2 GB**。每层附带游戏世界坐标包围盒（`scripts.js:39-43`，Child Angled `[-13000,25000] → [40000,-8000]`）——即**位图瓦片与游戏坐标是标定过的**，因此可以在地图上量算。
* 素材署名（F）：`index.html:247` "Beautiful Map Images by **Peardian** (vgmaps.com)"、`:248` "Data extraction from game ROM provided by **MzxRules**" → **ROM 渲染式截图切片 + ROM 解析表数据**，不是手绘。
* 数据层：`js/mapdata.js` 含 `dataRegions`(37 个区域轮廓)、`dataTable`/`dropTables`(敌人→掉落)、`searchables`、`mapData`（**101 个场景 / 1,870 个 actor 条目**：enemy 952、container 560、nature 358），坐标**双份保存**（归一化 Leaflet 经纬 `childAngledCoords` + 游戏原始 `{x:1708,y:0,z:-471}`）。`generateMapData.html` 是**离线构建脚本**，把 `rawmapdata.js` 的扁平表（`regionsRaw`/`roomsRaw`/`setupsRaw`/`enemies`/`nature`/`containers`，`:4-6`）聚合成嵌套结构并统计 actor 数量（`:44-71`）。
* `data/` 目录是 **ROM 头解析的文本转储**：`scenes/`(101)、`rooms/`(388)、`actors/`(349)，含绝对地址与命令号（`scene0.txt:1` "Scene at 02499000"、`04:`/`0E:` 前缀）。
* `research/`(36 MB) 有 `BombDrops/`（草堆炸弹掉落统计）与 `VersionCompare/`（1.0/1.1/1.2 的 Actors/Scene 转储 Mergely 对比，**还留着 GLideN64 的调试日志 `glidenhq.dbg`**）—— 这是"地图仓库顺手当了逆向工作台账"的实例。
* 缺口：**只有 N64 原版（美版 1.0–1.2 语境）**；3DS 仅出现在**已注释掉的** Glitches 面板（`index.html:218`）；**无 MQ、无追踪器功能**（唯一持久化是 `localStorage['firstTime']`，`scripts.js:573`）。
* **自相矛盾之处值得记录**：`README.md:9` 让用户去 mediafire 下载 `maps.7z`，而仓库里 1.2 GB 瓦片**已全部提交**。→ 该项目的资产再分发姿态是"事实上全量托管"。

### 2.4 其余地图类工具与闭源侧

| 项目 | 形态 | 底图来源 | 关键事实 |
|---|---|---|---|
| **`ootm`**（SiriusGG，Java Swing） | 桌面 App，348 个 .java / **17,068 行**，43 MB，HEAD 2023-08-10 | `res/graphics/maps/**` **138 张 JPG**（1277×864） | **入口随机器专用传送图**：每个地点一个 Java 类硬编码出口（`exitmaps/KokiriForest.java:11-31`）；`OoTMConstants.java:32-40` 署名 "Maps by Peardian / Sirius GG，origin VGMaps.com，thanks Ship of Harkinian, Kevsmatrix"；逻辑对齐 OoTR 规则（README:5）；按 seed 名存 `~/OoTM/<seed>/*.json`（`SaveLoad.java:13`）；**无 spoiler log 自动导入** |
| **`OoT_MapExt`**（CMuncey，C） | 命令行，`mapExt.c` **仅 126 行**（+`bSwap.h` 19 行），2019-10-01 | 输入 = 解密 NTSC v1.0 ROM，硬编码文件名 `ZOOTDEC.z64`（`:40`） | 只做**原始字节切片**：读场景命令 `0x04`（房间表，`:63-76`）导出 `<name>.zscene`(`:55-56`) 与每房间 `<name>-N.zmap`(`:96-98`) + 偏移 txt（`temp.txt`，来源 cloudmodding wiki）；**不解析多边形/纹理**；README 指定配合 **Utility of Time** 使用，并说 `extractChest` 是为 OoT Item Randomizer 列宝箱（`README.md:7`） |
| **`ootr_map_tracker`**（cheuer） | **EmoTracker/poptracker 数据包**（JSON+Lua，非程序），13 MB，132 图，2023-08-10 停更 | 手绘/D-pad 式地牢小图 | `tracker_layout.json:2` **`"map_enabled": false`** —— 地图功能已被官方停用，仅 `all_maps` 遗留段（`:292-316` 引用 `images/map_2.jpg`）；靠 `scripts/logic.lua` 的 flags 对齐 OoTR 术语 |
| **`ootr-tracking-map`**（rafaell22） | **Electron 39**（`package.json` name `ootr-logic-tracker` v3.1.0），ESM JS，5,156 行，2026-07-02 活跃 | 单张 `public/assets/map.png`(465 KB) + `map_rebuild.png`，`main.js:36,49` 一次性 drawImage，点位在 `data/locations.js` 以**像素 x/y 硬编码** | Canvas 手绘箭头 / foolish 标记 / hints；**直接读 OoTR spoiler log JSON**（`trainingMode.js:23-38`：`locations`/`gossip_stones`/`entrances`/`randomized_settings.starting_age`）；**内置 pocketsphinx WASM 语音识别**（`recognizer.js:3` + `data/voiceRecData/`）—— 本批次唯一的语音入口 |
| **`Hamsda/EmoTrackerPacks`** | Lua pack 集，48 MB，581 张 PNG/14 MB | `maps/maps.json:1-13` → `images/map_spawns.png`（child/adult 双图层） | 见 §3.4 |
| **MapGenie / Zelda Dungeon / 官方 Zelda Notes（闭源）** | — | — | 见 §2.5 详版取证 |

**底图血缘图（F）**：`Ecksters/OoT-Interactive-Map` 与 `ootm` **同源**（都以 Peardian / VGMaps.com 的 OoT 底图为基底）；三个 OoTR tracker 与它们无共享底图；**noclip 与上述任何项目都没有底图血缘**（它渲染真实场景几何，从不使用位图底图）。因此"时之笛交互地图"在社区里其实是**两条互不相交的谱系**：一条是位图（VGMaps → Ecksters/ootm/MapGenie），一条是几何（ROM → SceneNavi/noclip）。
### 2.5 闭源地图与伴侣工具（详版取证）

#### 2.5.1 MapGenie —— 订阅制攻略数据库，不是游戏工具

* **归属**：`mapgenie.io` 页脚 © 2026 **Magic Lamp Technologies Limited**，品牌挂 **IGN Entertainment**；OoT 条目为 "Ocarina of Time (Switch 2)" 卡片。
* **技术形态（可从公开页面结构直接证明）**：地图容器类名为 `.maplibregl-ctrl-zoom` / `.maplibregl-ctrl-attribution` 等 → **MapLibre GL**（开源 WebGL 地图引擎）。即"游戏地图 App"的底层其实是**矢量瓦片地图库**，与 GIS 站同源；这也解释了它为什么能提供免费缩放/标记同步而不自研渲染器。
* **商业层**：**MapGenie+ $14.99/年**，权益为"PRO features on all **276** MapGenie maps"（收集品自动清单、跨设备同步、去广告、离线地图等）；免费层带 **AdThrive** 广告。移动端：iOS 开发者署名 **Digitech Apps**，Android 发行方 "Map Genie"。
* **对 OoT 的实际覆盖（关键，F）**：**只有 Switch 2 重制版内容是在线的**；旧版（N64/3DS）Child/Adult 地图仍标注 **"Under Construction"**。→ 商业方在 2026-11 的内容策略是**把资源全部押在新版本**，老图谱未回填。这与 §1.1.4 的 Ocarina-Atlas（同样只支持单一重制版构建）形成同向证据：**闭源与开源在新版本上罕见地一致，共同放弃了 N64/3DS 长尾**。

#### 2.5.2 Zelda Dungeon —— 任务书前提不成立

* 站内确有交互地图产品线，但**仅限** `breath-of-the-wild-interactive-map`、`tears-of-the-kingdom-interactive-map`、`echoes-of-wisdom-interactive-map`；**`/ocarina-of-time-interactive-map/` 返回 404，OoT 无交互地图**（F）。
* 其 OoT 主攻略页 `dateModified = 2026-11-17`（发售后 12 天）已完成重制版改造：导航拆为 **Nintendo Switch 2 Primary Walkthrough / Nintendo 64 · 3DS Primary Walkthrough / Master Quest Walkthrough** 三入口 —— 说明**MQ 在 2026 年仍以独立条目存在**，与 §2.1.3（noclip 3DS 目录里 `Master Quest` 以 `_dd` 后缀自成一组）互相呼应。
* **未见 OoT Randomizer 板块**（F）：随机器内容与该站没有交集，追踪器/随机器生态完全由 `ootrandomizer.com` 自己的 wiki 承载。
* 结论：重制版的"第三方交互地图"位置被 **Eurogamer / Nintendo Life / Zelda Central（有 Interactive Map 专页）** 占据（R），而非历史上最有名的 Zelda Dungeon。

#### 2.5.3 EmoTracker —— 闭源时代的托管 pack 生态如何活到今天

* **域名与所有权变更**：`emotracker.net` 现 **301 重定向到 GitHub `EmoTracker-Community/EmoTracker`**（F）。旧版为 C++/Qt → 中期 C#/**WPF** → 当前克隆已是 **C#/.NET 10 + Avalonia 11.3.3**（跨 Win/macOS/Linux，`EmoTracker/EmoTracker.csproj:8,29-41`），即**正在完成从 Windows-only 到跨平台的移植**。原作者 viktlgb 的去向与迁移动机**未找到**（README/CLAUDE/docs 无一处提及，depth-1 克隆只有 1 个 merge commit）。
* **托管侧**：`EmoTracker-Service`（Lua）负责 pack 仓库托管，客户端 `PackageManager` + `PackageRepository` 从社区仓库列表（`EmoTracker.Data/Packages/PackageManager.cs:245-314`），本地源只有目录与 zip 两种。
* **pack 规范**：`manifest.json` + `locations.json` + `init.lua` / `autotracking.lua` + `repository.json`，安装目录 `Documents\EmoTracker\packs`；同一规范被 poptracker 直接继承（§3.4），字段级一致。
* **OoT 侧的官方 tracker 名录**（OoTR wiki "Trackers"，页面时间 2026-03-18，R）：Hamsda map pack、coavins 自动追踪 fork、Fouton **PugHUD**、JRJathome、Raikaru/atz、Xopar、Darkened5ky/Spleebie（→ 本地 `cheuer/ootr_map_tracker` 的 uid `ootr_map_tracker_spleebie_darkened5ky` 正是这条名录的产物，F）。**即"闭源引擎 + 开源 pack"是该生态的实际运作方式**，本报告把 EmoTracker/poptracker 都算作"引擎"而非"App"。
* **新增能力（2026）**：内置 **MCP server**（62 个 tool，`http://localhost:27125/`，JSON-RPC 2.0，协议 `2024-11-05`，无鉴权）+ NDI / Twitch Chat HUD / 语音（`docs/mcp-server.md:15-19,65,85,126`、`README.md:21-25`）。**追踪器状态成为 AI 客户端可直接读写的对象**，是本轮观察到的最新形态变化。

#### 2.5.4 官方伴侣：2026 年任天堂自己下了场

| 官方特性 | 内容 | 与社区工具的替代关系 |
|---|---|---|
| **ZELDA NOTES**（Nintendo Switch App 内，免费、**无需 NSO 订阅**） | 时光之线任务日志、**地图**、今日记录、传闻任务、**触摸屏虚拟陶笛**（演奏引导 / 自动演奏 / 作曲 / **二维码分享**）、2000+ 问答、amiibo 每日空投 | 直接吞掉 MapGenie 的"任务清单 + 地图 + 收集进度"三件套，以及全部第三方陶笛辅助器（§5.3）的引导需求 |
| **Hall of Fallen Foes（败敌殿堂）** | 已击败敌人雕像 + 弱点/出没图鉴 + **Boss 无限重战并记录最佳用时** | 覆盖"练 Boss"这一最主流的练习需求，使 practice-menu 类工具在重制版上**失去普通玩家市场**（对照 §1.2 的 3DS practice menu） |
| **重制版陶笛的麦克风识别** | 支持 Switch 2 **内置麦克风哼唱识别**（GameSpot 2026-09-08，R） | 新输入面 → `flipparina` 立刻提供"喇叭出声喂麦克风"路径（`README.md:44`，F） |
| **官方遥测** | 1.1.1 为 Zelda Notes 引入 NPLN/leaderboard 库 + 存档内两条滚动日志（**1,024 条事件 + 1,024 条位置，离线也写**，`oot-remake-modding/docs/game-versions.md:39-61`） | 这是重制版**唯一官方数据出口**；正因没有内存 RPC，社区 tracker 宁可轮询 `progress.sav`（§7.3） |
| 未提供 | **无 MQ、无慢放/帧进练习模式、无官方 modding API** | 这三项缺口分别由 noclip（`_dd` 场景组）、社区 practice/trainer、`oot-remake-modding`+KaeGae 填补 |

#### 2.5.5 OoTR 站点侧的公共事实（闭源托管但开源客户端）

* 版本：稳定 **v9.1** / dev **v9.1.40**；累计 **Generated Seeds 2,177,298**；Discord ~50,000 成员；公开 `/api/docs`（可程序化生成随机器）与 `/ai-disclosure` 页（F）。
* **页脚明确声明不分发版权材料、要求用户自备 US/JP v1.0 ROM**（F）—— 与 noclip 的"自托管派生 dump"（§2.1.4）构成两种对立风险模型，也与 zeldaret/oot 的 MD5 白名单同源。
* 其余托管 tracker：The Last Tracker（thelastpr.org，官方后继）、Spoilizer（spoilizer.com，作者 MisoSoup）、tootr.mracsys.com、Hashfrog、tracker.brakke.dev、Barinade、Riptide、**Tracker of Time**（Selene-T：BizHawk 2.7+/ModLoader 3.01/m64p/RetroArch/RMG/**SoH 3.0.0–3.0.1**，同时兼容 AP 与 OoTR）（R）。
* **唯一的纯商业移动 tracker**：iOS **"OoT Tracker: Ocarina Hints"**（MyHighTechLife / Lawrence Harris），免费 + **¥15 买断**解锁；1.2（2026-11-10）更新说明直接写 "Updates for the Ocarina of Time Remake"，1.2.2（2026-11-12）—— **重制版发售后 5 天完成适配**，比本批次多数开源仓库还快（R，App Store 页面）。
## 3. Randomizer 进度追踪器（Tracker App）

### 3.1 三种输入范式（本报告的分类骨架）

| 范式 | 定义 | 成员（本次克隆） | 与游戏版本的耦合方式 |
|---|---|---|---|
| **(a) 离线解析** | 吃随机器产出的 spoiler log / output.json / settings string | Track-OOT、tootr、OoTMM-Spoiler-To-Emotracker、HoodTracker（直接 import OoTR 源码） | 靠 JSON 字段名与 logic 表；文本契约，最稳 |
| **(b) 运行时读取** | 读游戏/模拟器内存或存档文件 | hyrule-tracker、fenhl/oottracker、ootr3d-tracker（Citra）、poptracker/EmoTracker 引擎（USB2SNES/UAT/AP）、Ocarina-Atlas、oot3d-tracker（只读存档） | 靠地址/偏移或 OoTR 官方 ABI，最脆 |
| **(c) 纯人工打卡** | 无任何自动输入，勾选即状态 | Ivan、iRandoTracker、deains-oot-tracker、TestRunnerSRL/oot-tracker、OoT-randomizer-tracker、NaviTrack、Hamsda/Jupiterfire pack | 几乎不耦合（只耦合美术与地点名） |

范式 (c) 有其**自觉的理论依据**（F）：`deains-oot-tracker/README.md` 原文 —— "Most map trackers for OoT include item logic to show you where you can go next. **I don't like this, as I feel it takes away from an important aspect of the game (making routing decisions)**." 这是"追踪器是否应当替玩家做决策"的明确立场声明，而非功能缺失的借口。

### 3.2 Track-OOT —— 生态第一，且**不是 Electron 桌面 App**

`oot-peripherals/Track-OOT`，上游 `GameStateTracker/Track-OOT`（nightly 分支，GitHub 镜像），HEAD `7692801`（2026-11-16），89 MB，★见镜像（原仓在 **Bitbucket**：官网 track-oot.net 的三个外链为 patreon.com/zidargs、discord.gg/wgFVtuv、**bitbucket.org/zidargs/track-oot**，F）。`package.json:2-6` name `track-oot` / "Tracker for OOT-Randomizer" / author **Denis Weiß (ZidArgs)** / MIT（`LICENSE:3` 版权 2026 同名）。

**形态更正（F）**：它是 **纯 JavaScript ES Modules 的 PWA**，不是 Electron 应用 —— `src/` 内 205 个 `.js`、**0 个 `.ts`**；`src/index.js:39-44` 动态注入 `app.html`+`app.js`；`src/manifest.json:4 start_url:"."`；`src/sw.js` 做离线缓存。Electron 只服务于**内置编辑器**（`editor/main.js:1-3,17-33` 是全仓唯一的 `BrowserWindow`）。`electron ^28.1.0` 位于 **devDependencies**（`package.json:32`）。构建走 gulp（`gulpfile.js:22-25` src/logic/dev/prod，`:60-70` `buildWorker` 打包 logic worker），依赖 8 个 git 包（`:16-23`：`@emcjs/core 1.6.1`、`@emcjs/logic 0.3.0`、`gametrackerjs 0.13.1`、`archipelagojs 1.2.0`）。**`.AppImage`/`Setup.json` 未找到** —— README.MD:11-27 只给 `yarn build` / `yarn serve -prod`。

| 能力 | 实现与证据 |
|---|---|
| spoiler 解析 | `src/script/util/spoilerparser/SpoilerParser.js:81-85` 读 `:version`；`:45-49` **只认 `"… Release"` 与 `"… f.LUM"`**，否则弹 "Unknown Spoiler log version"；`:95` 按 `settings.world_count` + multiworld 选 `World N`；`:100-142` 解析 settings / randomized_settings / locations / `:woth_locations` / `:barren_regions` / trials / dungeons / entrances；`parseEntrances.js:12` 读 `{region, from}`；`parseStartingInventory.js:14-16`（新版 dict）与 `:52-54`（旧版 list）双兼容 |
| **自带 logic 求解器** | **不复用随机器输出**：`logic/*.min.json` 8 个文件 ≈**27 MB**，运行时按规则集切换（`src/script/util/logic/LogicExtractor.js:16-27`）；`src/database/rulesets.json:2-6` `logic_rules: glitchless`；自定义逻辑 `src/script/util/logic/CustomLogicUpdater.js` |
| 面板 | `src/database/layouts.json` 定义 19 套布局（items、locationlist、map、race、compact、flat…，`:7 itemgrid / :14 dungeon-status / :28 worldlist / :32 worldmap`）；`src/script/ui/panel/` 有 itemgrid / dungeonstate / worldmap / worldlist / locationlist / shoplist / songlist / multiplayer / **aptextclient** / ctxmenu |
| 金蜘蛛/钥匙/年龄 | `src/database/items.json:286 max:100`、`:287 mark:50`、`:288 counting:true`、`:289-296` 快捷档位 0/10/…/100；钥匙 `src/script/state/item/KeyState.js:16-27`（MQ/vanilla 切换时 `activeListChange` 动态改 max）；地牢态 `DefaultDungeonState.js`+`DungeonStateManager.js`；child/adult 走 `src/database/world.json` 的 spawns/config |
| 存档迁移 | **40 个** `src/script/savestateConverter/StateConverter0..39.js` + `src/script/StateRecovery.js`；`NOTES.MD:5` `localStorage.getItem("savestate")`；`src/script/util/registerStorages.js:9-25` 注册 dungeonRewards/shopItems/gossipstone/songNotes；IndexedDB 走 `LogicExtractor.js:1 IDBStorage` |
| undo/redo | 由 `gametrackerjs` 提供（`src/CHANGELOG.MD:1732` 加入，`:1499` **联机时禁用**） |
| 第二输入 | Archipelago：`src/script/util/archipelago/ArchipelagoController.js:113-170`（`slot_data:true`） |
| 资产 | 内置 **372 个游戏图像 / 27 MB**（`src/images/items/` 158 张），地图 `src/images/maps/{area 25, dungeon 12, grotto 19, interior 32, bossarea 9}` + `hyrule.webp`；i18n `src/i18n/`（en/de）；代码量 **20,882 行** |

**版本耦合的真实机制**（F，关键）：**代码里没有任何自动版本校验**。`src/CHANGELOG.MD:7` `VERSION 3.1.6`、`:11` "change ootr logic to v9.1"（`_latest_changes_dev.md:3` 同）—— 对齐 OoTR 9.1 的动作是**人工替换 logic JSON**。`src/version.json:3-5` 由 CI 注入（本地为空）。这就是范式 (a) 的代价结构：格式契约稳，但 **logic 语义要靠人追**。

### 3.3 hyrule-tracker —— 2026 年新范式：读重制版的进程内存 + `progress.sav`

`oot-peripherals/hyrule-tracker`，★53，MIT（`package.json:3,6,8` v1.4.2 / João Victor Prado），HEAD `1fb2777`（2026-11-17），12 MB，TS + React 19 + Vite 7 + **Electron 45**（`package.json:27`），`electron-builder` 出 Win NSIS + **Linux AppImage**（`:62-80`），入口 `dist-electron/main.cjs`（`:41`），**13,161 行**。

| 项 | 事实 | 证据 |
|---|---|---|
| "live save" 的真实含义 | **只读附加到正在运行的游戏进程内存**（不是模拟器 RPC、不是插件） | `src/main/live.ts:1-4` 注释 |
| 权限最小化 | Windows 只请求 `PROCESS_VM_READ \| PROCESS_QUERY_LIMITED_INFORMATION`；Linux 走 `/proc/pid/mem` + ptrace scope；FFI 用 `koffi` | `src/main/procmem-win.ts:7-8`、`procmem-linux.ts:14`、`package.json:37` |
| 进程白名单 | `Triforce.exe / hookshot.exe / Longshot.exe / nx2navi.exe / relink-game.exe / ocarina-reborn*.exe` + 模拟器 `fritchii.exe / lichii.exe` | `src/gamedata/saves.ts:83-84`、`:77` |
| 地址常量 | 表基址 `0x1010_99f4_f418 / 0x1010_7eea_f418 / 0x1010_8bda_f418`；堆区间 `0x1010_0000_0000–0x1011_c000_0000`（guest `0x10_0000_0000` + 64 GB 偏移）；兜底 64 MiB 重叠扫窗 | `live.ts:27`、`:29`、`:30-32` |
| 轮询节奏 | 值轮询 `TICK_MS=500`（2 Hz）、重找进程 `FIND_EVERY_MS=3000` | `live.ts:21-22` |
| 并行存档路线 | 读 Switch 2 `progress.sav`（`slot_*` 目录；模拟器 NAND 用 title id `0400486024DEC000`），映射表 `data/save-mapping.json` **v2：322 条 entries + 1 counter + 4 derived** | `saves.ts:25`、`:55` |
| 与 Ocarina-Atlas 的分工 | **零注入、零写、无 socket**（`docs/live-memory.md:3-7`），失败即回落存档文件；Atlas 需要游戏侧 RPC 桥 | 同上 |
| 版本矩阵 | **仅 Switch 2 重制版**：已验证 Hookshot v2.1.0 / Longshot v1.0.0 / NX2navi 0.12.0(Triforce 0.4.0) / ReLink v0.7.7 / Fritchii v5.8.4；`README.md:95` 明确**不支持 N64/3DS/GC 原版存档**；随机器**不支持**（`:180` 只致谢其 logic 数据） | `README.md:91`、`docs/live-memory.md:13,29-30` |
| 资产规避 | **不入库任何游戏图像**：物品/参考图由构建期脚本从 Zelda Wiki 抓（`scripts/fetch-item-images.mjs:133,154`，`src/renderer/item-images.json` 记录来源 URL）；GK/心片点位从**用户自己的 RomFS** 离线生成（`scripts/extract-placements.ts:1-6`、`scripts/map-placements.ts:1-9`，GK 用 64-bit flag id 与 `id^2` 双置位） | 同上 |

`README.md:95` 的"不支持 N64 原版"与资产规避策略，使 hyrule-tracker 成为**本报告判断 5/11 的直接证据**：2026 年最有活力的新追踪器选择了一个"法律上最干净、技术上最不稳"的位置。

### 3.4 追踪器谱系全表

| 仓库 | 技术栈 / 入口 | 范式 | 地图 | 游戏资产 | 许可 | HEAD | 代码量 | 评价 |
|---|---|---|---|---|---|---|---|---|
| **Track-OOT** | JS/PWA + gulp（Electron 仅编辑器） | a (+AP) | ✅ 97 张分区图 | ✅ 372/27 MB | MIT | 2026-11-16 | 20,882 | 功能最全；logic 对齐 OoTR **9.1** |
| **hyrule-tracker** | TS+React19+Electron45+koffi | **b** | ✅ `data/world-markers.json` 60 区域 | ❌ 构建期抓图 | MIT | 2026-11-17 | 13,161 | 2026 新形态，仅重制版 |
| tootr (mracsys v1.0.14) | TS+React+Vite+MUI | a | ❌（区域页） | ✅ 207 png + 35 `.kra` 源 | MIT(`LICENSE:3`)+`LICENSE-3RD-PARTY` | 2026-01-24 `4b64d93` | 12,705 | **ER 最强**；显式版本门 `OotSettingAssetMap.ts:833,844,972,1028` 按 `8.2.14/8.2.37/8.3.0/8.2.50` 分支，`:850 graph.get_game_versions(true)` |
| **poptracker** (black-sliver v0.36.0) | **C++20 + SDL2/SDL2_ttf/SDL2_image + Lua**（非 Qt，`meson.build:265-304`） | b/UAT/AP **引擎** | ✅ 由 pack 提供 | ❌（本体不含） | GPL-3.0 | 2026-11-11 `6052adc` | 37,703 | 通用生态核心；包为目录或 **.zip**（`src/packmanager/packmanager.cpp:260,303`），拖入即装（`README.md:13-16`）；JSON schema 在 `schema/packs/*.json` |
| EmoTracker (Community) | C#/**.NET 10 + Avalonia 11.3.3**（`EmoTracker/EmoTracker.csproj:8,29-41`），Win/macOS/Linux | b 引擎 | ✅ pack | ❌ | MIT(`LICENSE:3` 2026 EmoTracker Community) | 2026-10-16 | — | 旧版为 C++/Qt；**新增内置 MCP server**（见 §7.4）；仓库内搜 "ocarina" 零命中 → **不含 OoT pack** |
| EmoTrackerPacks (Hamsda v3.9.0.0) | JSON+Lua pack | c | ✅ `map_spawns.png` child/adult 双层 | ✅ 581 png/14 MB | **未找到 LICENSE** | 2023-05-18 | Lua 20,422 | **access_rules 1,521 条**（dungeons 512/overworld 248/ER 225/exits 152/dungeons_entrance 384）；`scripts/init.lua:1-8` 加载链、`logic_helpers.lua:1-25` 自定义 `can_time_travel/has_age`；7 个 `var_*` 变体目录 |
| OoTMMR_tracker_pack (v29.0.1) | JSON+Lua pack | c | ✅ oot + termina 双世界图 | ✅ 1,251 png/18 MB | 未找到 | 2026-09-06 | Lua 24,079 | access_rules 1,532 条；**`README.md:1` 自declare已弃用**，官方后继 = The Last Tracker 网页版 |
| **Ivan** (L-P) | **Go 1.23 + Ebitengine 2.8.7** | c | ❌（`assets/items.png` 图集，粉丝绘制，`assets/LICENSE.md`） | ✅ | MIT 变体(`LICENSE:1`) | 2025-04-21 `cc875fc` | 2,252 | **竞速党向**：全键盘驱动 + hint 手输 W/B/S/A（`README.md:58-72`、`tracker/hints.go:13-45`）；**唯一自带 undo/redo 栈的人工 tracker**（`tracker/undo.go:9-19`，`-`/`+` 键）；状态存 `ivan.state.json`（`tracker/tracker.go:251`）；带 `inputviewer/` 输入可视化 |
| oottracker (fenhl) | Rust workspace，v0.7.4 | b | ❌ | ✅ 466 png（致谢 Xopar/Maplestar） | **未找到 LICENSE** | 2023-10-20 `b08a60e` | 15,667 | BizHawk Lua / Project64 JS / **RetroArch `READ_CORE_MEMORY`**（`crate/oottracker/src/net.rs:316,318,333`）；抓取区间表 `ram.rs:52-60`（`RANGES`）：`save::ADDR:53`、`0x1c84b4` 按键、`0x1c8545` 场景、`0x1ca1c8` switch flags、`0x1d8870/0x1d887e` **文本框（用来直接读 hint 文字）**、`0x1d8dd4` pause_ctxt；多人/OBS 同步 `net.rs:143 wss://oottracker.fenhl.net/websocket`；**logic 靠构建期下载 `Roman971/OoT-Randomizer/archive/Dev-R.zip` 由 proc-macro 生成静态表**（`crate/ootr-static-derive/src/lib.rs:141,146`）；全仓 `spoiler` 0 命中 |
| ootr3d-tracker (vgfreak95) | Rust / Win32 | b | ❌ | ❌ | 未找到 | 2022-05-15 `f5a6b4b` | 小 | **hyrule-tracker 的 2022 前身**：直接找 `citra-qt.exe` 宿主进程（`src/main.rs:40`），用 `bit_shift(0x0587A0E)` / `move_ptr` 走指针链（`main.rs:54,56,62`）；**基址靠用户手写文件** `C:/Users/VGFreak/citra/address.txt`（`src/citra_memory.rs:4-13`，路径含作者用户名） |
| NaviTrack (lkarvec) | Next.js 12 + React 18 + Redux | c | ❌（仅 5 个 JS 图标组件） | ❌ | CC0-1.0 | 2025-04-07 `56586e4` | 1,562 | 自称 "very alpha state"；spoiler 解析**停留在 roadmap**（`README.md:36-37,41`，代码 0 命中）；含 glitch 百科 `views/glitches/`；Netlify 部署 |
| iRandoTracker (dukevantreta) | **SwiftUI / macOS**（非 iOS：`project.pbxproj:325-329` `MACOSX_DEPLOYMENT_TARGET=15.0`、`SDKROOT=macosx`） | c | ❌ | ✅ 888 KB **重制版**物品图 | MIT(`LICENSE:3` ©2025 André Vants) | 2025-03-28 | 529 | SwiftData 本地持久化（`iRandoTrackerApp.swift:4`）；已知 bug：瓶子互相覆盖（`README.md:5`） |
| HoodTracker | Python 3.8 + PySide2 | a（源码级复用） | 部分（区域探索图） | ✅ 81 png | MIT | 2022-12-16 `ccba251` | 3,166 | **唯一直接 `from World import World` 把随机器当求解器**（`HoodTracker.py:10-21`，OoTR 以 git submodule 挂载，本地目录为空=未拉取）；输入是 **settings string + output.txt**（`README.md:16`）；耦合成本显式：submodule 必须与串同版本；支持 ER/MQ/钥匙/空房间/试炼 |
| OoT-randomizer-tracker (fgil90) | 原生 JS | c | ❌ | ✅ 112 png | GPL-3.0 | 2022-05-17 | 430 | 极简物品面板，左键升/右键降循环；无 logic/spoiler/地图 |
| OoTMM-Spoiler-To-Emotracker (KO0IAID) | C#/.NET WPF | **a→c 桥** | ❌ | ❌ | MIT | 2025-11-08 `2722e31` | 6,349 | **不是 tracker，是转换器**：读 OoTMM spoiler log（`MainWindow.xaml.cs:277-294`）→ 导出 `Tracker - *.json` 供 Juptierfire/wbsch pack 预填；字段模型 `SpoilerLog/Controller/Spoiler.cs` 2,933 行（`WorldFlags:202`，WotH/Foolish/Specific hints `:683,733,784`） |
| deains-oot-tracker | 静态 Web（`src/js` + build.js/Makefile/autoprefixer） | c | ✅ 手绘区域图 | — | MIT | 2025-02-16 | — | 明确**反 logic 立场**（见 §3.1 引文）；部署 GitHub Pages |
| TestRunnerSRL/oot-tracker | 原生 JS（`index.html`+`script`+`style.css`） | c | — | ✅ images | MIT | 2020-05-19 `4df1149` | — | SRL（SpeedRunsLive）平台自带；`README.md:3` "Logic updated to **4.0**"、`:4` "Does not support any sanity options" —— 停在 OoTR 4.0 时代，是**平台内嵌 tracker 的化石** |
| oot3d-tracker (LofiGhoul) | Python + **GTK4/libadwaita** | b（只读存档） | ❌ | ❌ | **无 LICENSE** | 2026-11-13 | 5 个 .py | 100% 完成度追踪：心片 36（按区域+逐条攻略）、金蜘蛛 100（**含 MQ 地牢**）、10/20/30/40/50 token 奖励、心容器 8、3 石+6 徽+感应之石+Gerudo 卡、歌 12、物品 19、装备、升级、瓶 4（`README.md`）；`oot3d_save.py:30-37` `MAGIC_OFF=0x30` 扫 `ZELDAZ`，**只读**（`:3-5`） |
| oot-tracker (StripesOO7) | **poptracker datapack**（Archipelago-OoT） | b/c | ✅ | 129 张含 **.xcf 源文件**（自绘证据） | MIT(2024) | 2026-05-23 `81376de` | 96 MB | `manifest.json` name "Ocarina of Time Archipelago"、`min_poptracker_version 0.29.0`、flags `["ap"]`；`versions.json` 带 sha256+changelog；`datapacke_url.txt` 两个 AP datapackage 端点（`http://192.168.69.21:7777/datapackage`、`https://ap.stripesoo7.org/datapackage`）；`locations/*.jsonc` 每地牢一份 |

**引擎侧要点**（poptracker vs EmoTracker，F）：两者共用同一 pack 规范 —— `manifest.json`+`items/`+`locations/`+`layouts/`+`maps/`+`scripts/*.lua`（`poptracker/examples/uat-example/manifest.json:1-16` 含 `variants.*.flags`），`EmoTrackerPacks/ootrando_overworldmap_hamsda/manifest.json:1-30` 的 `package_uid/variants` 字段与之**完全一致**；`.pt`/`.apt` 专有扩展名 **未找到**。自动追踪接口三类：①内存（`ScriptHost:AddMemoryWatch`、`AutoTracker:ReadU8`，`doc/AUTOTRACKING.md:5-8,34-45`，走 USB2SNES 协议/Lua connector）②变量走 UAT 协议（`:10`，`src/uat/`）③Archipelago（`src/ap/archipelago.h:13,45` 把 `Archipelago` 注册为 Lua 类，连接由 pack 的 Lua 完成，**不是内置 APWebClient**）。EmoTracker 侧则多了 NDI / Twitch Chat HUD / 语音（`README.md:16-23`），**README 未提 Archipelago**；两引擎之间**互不声明兼容分工**（EmoTracker 全库无 "poptracker" 字样，**未找到**）。

### 3.5 自动追踪的 ABI —— 随机器自己写的接口文档

OoTR 是这一整个生态里唯一**明文规定外部读取契约**的项目（F）：`oot-decomp-projects/OoT-Randomizer/Notes/auto-tracker-ctx.md`（60 行）定义 auto-tracker context 位于固定地址 **`0x8040_000C`**，头 4 字节是版本号（当前 **6**），其后 `CFG_DUNGEON_INFO_ENABLE@0x04`、`SPECIAL_DEAL_COUNTS@0x48`（8 家商店）、`RAINBOW_BRIDGE_CONDITION@0x38`（0/1/2/3/4/5/6）、`LACS_CONDITION@0x3C`、`CFG_DUNGEON_REWARD_AREAS@0x50`（0xCF，9×0x16 串）。写入侧 `Patches.py:964`、`:1220-1226`；符号来自 `Rom.py:52/56` ← `ASM/build.py:153-156`；存档位域见 `SaveContext.py:231,137,765`。

同一文档 `:27-29` 也记录了 ABI 的破口：**randomizer 6.2.181 改了 context 布局却没有递增版本号**，只能用 ROM header `0x35-0x37` 判别，而对 fork 无效。**"有版本字段但没人强制它变"与"完全没有版本字段"（2026 重制版）之间，生态表现差别极大**：前者 tracker 能自愈，后者只能人工重建符号表（见 §7）。

### 3.6 闭源/托管与竞赛规则的边界

* **The Last Tracker**（thelastpr.org）：OoTR wiki "Trackers" 页（2026-03-18）列为 OoTMMR pack 的官方后继，属**网页托管**形态（R）。
* **iOS 商业 App "OoT Tracker: Ocarina Hints"**（MyHighTechLife / Lawrence Harris）：免费 + **¥15 买断**，1.2 版（2026-11-10）"Updates for the Ocarina of Time Remake"，1.2.2（2026-11-12）—— 重制发售 **5 天内**即跟进（R，App Store 页面）。
* 其余：Spoilizer（spoilizer.com，作者 MisoSoup）、tootr.mracsys.com、Hashfrog、tracker.brakke.dev、**Tracker of Time**（Selene-T：支持 BizHawk 2.7+/ModLoader 3.01/m64p/RetroArch/RMG/**SoH 3.0.0–3.0.1**，同时兼容 AP 与 OoTR，R）、Barinade、Riptide。
* **竞赛侧的反向约束**（F/R）：OoTR wiki 与 racetime/SRL 规则**禁用 auto tracker 与 logic tracker**（自动追踪与逻辑提示在计时赛中被视为不公平优势）。这条规则解释了为什么 (c) 类"纯人工打卡 + 键盘驱动 + undo"（Ivan、deains）在功能上"落后"却持续存在 —— 它们是**合规形态**，不是没做完。
* **官方推荐链**（F）：`OoT3D_Randomizer/README.md:55` 由随机器自身推荐 **track-oot + Hamsda pack**，即追踪器生态获得随机器项目的背书，这是该生态长寿的制度原因。

---

## 4. 存档编辑器

### 4.1 N64 原版存档布局：C# 与 TypeScript 两份实现互相印证

| 结构 | 位置 | 证据 |
|---|---|---|
| 文件级魔数 `"ELDA"`(0x41444C45) → 据此判定小端并整体字节翻转 | `OoTSaveEditor/OoTSaveFile.cs:26-30,38-51` | F |
| **6 槽 = 3 主 + 3 备**，步长 5200(0x1450)，基址 32(0x20) | `OoTSaveFile.cs:32-35` | F |
| 头：0x00 sound option、0x01 Z 目标模式、0x07 `'Z'`、0x08 ELDA、0x0C GameVersion | `OoTSaveHeader.cs:27-34`（写回 `:45-52`） | F |
| 槽魔数 `"ZELDAZ"` @0x1C/0x20 | `OoTSingleSave.cs:2585-2586`；TS `oot-save-edit/models/saveslot.ts:334-337,410-421` | F |
| 入口 0x02 / 年龄 0x04(child=1) / 死亡数 0x22 / 姓名 0x24-0x2B / 64DD 标志 0x2C | `OoTSingleSave.cs:2583-2590` | F |
| 心：max 0x2E、cur 0x30（**1/16 心单位**）；卢比 0x34 | `OoTSingleSave.cs:2591-2593` | F |
| B 槽 0x68、C 槽 0x69-0x6E、`items[24]` 0x74、`ammo[16]` 0x8C | `OoTSingleSave.cs:2602,2614,2618,2621` | F |
| equipment 主 0x9C / 次 0xA0 / quest_flags 0xA4 + 位表 | `OoTSingleSave.cs:2606-2608`、`:2437-2466`、`:2473-2482`、`:2508-2524`（箭袋/炸弹袋/钱包/鳞片层级） | F |
| **金蜘蛛 token 总数 = u16 @0x00D0**；双人防御心 @0x00CF | `oot-save-edit/models/saveslot.ts:885-899`（`Tokens.GoldSkulltulaToken=0x0080_0000` 在 `:242`） | F |
| 每区域骷髅头计数（3DS 版是 **22 字节位域 0xEB4-0xEC9**） | `Oot3DS-Save-Editor-For-Azahar/oot3d-save-editor.html:178-185` | F |
| **checksum @0x1352**；entranceIndex 影子 @0x1378 | `OoTSingleSave.cs:2624,2637,2664-2665`；TS `:1072-1078,1080-1086` | F |
| 物品 ID 表（`GoldSkulltula = 0x71`） | `OoTSingleSave.cs:7-130`（`:121`） | F |
| 场景永久标志 101×0x1C、otherFlags @0x0EF8(60 B) | `saveslot.ts:906-912,1063-1070` | F |
| **heap 区 / `DEMO` 标志** | **8 个仓库无一解析 0x1450 之后的 heap 或 DEMO 区 —— 未找到** | — |

### 4.2 格式覆盖矩阵（谁支持什么、缺什么）

| 格式 | 实现者（行号） | 缺口 |
|---|---|---|
| N64 `.sra`（32 KB = 0x20 + 6×0x1450） | `OoTSaveEditForm.cs:227` 仅 `*.sra`；`savefile.ts:84-95` acceptedSize 0x8000 | C# 加载时**不做任何合法性判断**；TS 无 `.a1` |
| `.srm`（含 EEPROM 0x800 / MEMPAC 4×0x8000 / SRAM 0x8000 / FLASHRAM 0x20000） | `savefile.ts:185-206`；`fileutil.ts:10-65`（**按大小 + 扩展名双检**，不符即报错 `:105-108`）、`:128-129` 抽 SRAM | 无 `.eep/.a1/.gbx` 分支；**`.a1` 未找到** |
| **GC memory card `.bin`**（GCI 目录 + 块 ECC + shift2 编码） | **8 仓库全无**（仅 `OoT3D-Editor/README.md:44` 把 `save0X.bin` 当 3DS Checkpoint 裸 SaveContext 读） | **无 ECC / 无块目录 → GC 卡实际不可用；未找到** |
| 3DS `save00.bin` / 解密容器 | Lua `index.lua:5,380-392,529-531`（`/3ds/Checkpoint/saves`，标题 0x00335/0x00336）；HTML `oot3d-save-editor.html:253-265`（自扫 `"ZELDAZ"`，`start=i-0x30`，"解密槽或整容器皆可" `:145`）；`oot3d_save.py:30-37`（`MAGIC_OFF=0x30`，只读） | **无 AES/签名处理** → 必须用户自行解密或用 Checkpoint 明文 |
| **Switch 2 重制版** `slot_NN/{progress,caption,option,account}.sav` + `manifest.json`，头 `magic 0x01020304 / version 0x002D80EB / dataOff` | `OOT-SW2SE/editor.html:453-483`（headerEnd 0x175D8/0x2A0 在 `:487-488`）；`Links-Toolbox/nxsave.py:1-14,32`；路径矩阵 `Program.cs:390-465`、`linux/oot_sw2se.py:344-360`、`README.md:133-142`；格式文档 `HeyListen/engine/docs/saves.md:11-56` | 除 manifest 外**无签名/加密**；**iQue 专用工具未找到**（C#/TS 的字节序翻转是最接近的旁证） |
| TLV 哈希 `0xdc941483` | **只在追踪器侧出现**：`Ocarina-Atlas/src/lib/lichii-server.ts:1738-1746`（caption.sav → skulltulaCount） | OOT-SW2SE / Links-Toolbox **均无**该哈希（它们用 progress.sav 的 `0x04735978`，`editor.html:406`）→ **同一格式，两个工具的字段哈希集合互不重叠** |

### 4.3 校验和处理：两处真实分歧

| 仓库 | 校验处理 | 证据 |
|---|---|---|
| OoTSaveEditor (C#) | 加载**不验**；写出时重算 Σu16 × 0x9A9（ushort **自然溢出 2^16**）→ 写 0x1352，并镜像入口到 0x1378 | `OoTSingleSave.cs:2627-2633,2637,2664-2665` |
| oot-save-edit (TS) | **加载即双重验签**（header pattern+padding、slot `"ZELDAZ"`+checksum），但求和用 `sum % 0xFFFF`（**与 C# 不同模数**） | `saveslot.ts:1088-1118`、`saveheader.ts:28-41,118-136`、`savefile.ts:153-178` |
| OoT3D-Editor (3DS Lua) | **10 种 CRC-16 变体爆破检测**后重算，校验区后置 4 字节清零，写前 `.orig` 备份、写后复验 | `index.lua:52-91,98-120,403-426` |
| Oot3DS-Save-Editor-For-Azahar (HTML) | 固定 **CRC-16/ARC** over 0x14DC（自 `yellows8/smealum oot3d_savetool.c`），自身 2 字节先清零 | `oot3d-save-editor.html:171-172,223-251,266-278` |
| OOT-SW2SE | 档内**无校验**；改后**重新解析自检** + `manifest.json` 逐文件 SHA-256 校验并重算（CRC32 只用于 ZIP） | `editor.html:787,802-822,824-832`；结构不变量 `:490-500` |
| Links-Toolbox | **无校验概念**，原位覆写不改长度（`nxsave.py:10`），`save(backup_dir)` 时间戳备份 `:234-241`；显式警告游戏回写 `:340-341` | 同上 |
| Cloowney / oot3d-tracker | 前者**未找到代码**（只有 README）；后者只读 | `oot3d_save.py:3-5` |

1. **N64 校验和模数分歧**：C# 用 2^16 自然回绕、TS 用 `% 0xFFFF`（`OoTSingleSave.cs:2627-2633` vs `saveslot.ts:1088-1096`）。同一份档两个工具可能给出**不同**的校验值 —— 这类差异在"能否被游戏接受"上是有风险的，且**没有任何一方记录自己做过对拍**。
2. **8 个编辑器无一实现"主槽 ↔ 备份槽同步"**：N64 存档 3 主 + 3 备（`OoTSaveFile.cs:32-35`），游戏在损坏时会回落到备份槽，因此**只改主槽的编辑会被旧备份覆盖**。最接近的规避是 Links-Toolbox 的 "Save to…"：把编辑后的槽写入**所有找到的存档位置与任意选定槽**（`README.md:77`）。

### 4.4 2026 重制版编辑器的技术形态

**`Traiver/OOT-SW2SE`（★32，MIT，HEAD `ae88f36` 2026-11-18）**——"字段字典 + 多宿主壳"：单文件 `editor.html`(361 KB) 内嵌 JSON 目录 `:301`，**fields 11,926 条 / 具名 1,960 / inventory 104 / registrations 105 / 21 种类型**。三形态：Windows = WinForms + WebView2（`Program.cs:12-13,153-169`，页面从程序集资源加载）；macOS = Swift（`macos/OOTSW2SE.swift`）；**Linux 不是纯前端** —— Python 起 `127.0.0.1` 私有服务器、页面 `fetch` 调主机动作（`linux/oot_sw2se.py:15,1049,1152`）；只有 `web/` 是纯浏览器版（`web/browser-host.js:5-6,283-290` 用 File System Access / `webkitdirectory`，无上传；Dockerfile 仅 static-web-server）。**不扫盘**：用户手选文件夹（`Program.cs:226-228`）后按 layout 递归探测（`ProfileFolder/Collect` `:418-465`；Longshot/ReLink 走 `%USERPROFILE%\Games\ReLink`，Lichii 走 `%APPDATA%\lichii\nand\user\save\0000000000000000\<profile>\0400486024DEC000\`，titleID `0400486024DEC000`、端口固定 userid `5a3c9e1f0b7d24680000000000000001`，`README.md:138-142`）；槽识别 `slot_00…slot_05` 正则 `editor.html:892-906`，按 `caption.sav` 存时自动开最新槽（`:520-529`）；字段覆盖钱包/上限 `:380-384,433`、8 组容量 `:385-419`、5 瓶位 `:420-431`、心/魔法带 1/4 与 1/10 刻度 `:435-440`、配音语言 `:446-451`、剧情 flag 开关 `:780-786`、65 项目标清单（`README.md:22`）。

**`Cloowney/ZeldaOOTRemakeSaveEditor`（★17，2026-11-15）** —— **克隆内无任何源码**，只有 `README.md` + `docs/images/*`（F）。自述：.NET 单文件 EXE（含运行时、WebView2 播视频，`:31`）、Win x64、选文件夹或直开文件（`:28,39`）、备份到 `%LOCALAPPDATA%\ZeldaOOTRemakeSaveEditor\Backups`（`:103`）、自动更新（`:109`）、**明确"仅支持本项目研究的复刻版格式，不支持 N64 原版 ROM 存档"**（`:113`）、**攻略内容运行时抓 IGN**（`:122`）。→ 校验和与字段偏移在库内不可考（未找到），本报告只能记为 U/R 级；"运行时抓第三方攻略正文"另计法律风险。

**`losava/Links-Toolbox`（★7，MIT，HEAD `36ab599` 2026-11-17）** —— Python 3.14/Tkinter，一个工具集合同时做 mod loader 与存档编辑器，同一 `nxsave.py` 被两边共用（`save_editor.pyw:23,241-249`、`mod_loader.pyw:23`）。mod 开关的实现极朴素而有效：**用文件夹名表达状态** —— 开 = `NN-<名>`、关 = `_/<名>`，字典序末位胜出（`mod_loader.pyw:1-11`）；每次 Save 生成 Triforce 需要的 `~Size Table`（`:8-9`、`README.md:62`）；Switch 的 `.ips/.pchtxt/subsdk` 码被判定"这些预编译端口永不执行"，只取 romfs 部分（`mod_loader.pyw:63`、`README.md:60`）。真正的"代码 mod"走 Longshot 源码路径：把 `longshot/host/*.c`、`nx2/*.py` 覆入每个已装工具链并触发**整包重编译**，先校验所有 hook 锚点存在否则不动（`code_mods.py:1-14`、`README.md:124-130`），安装记录在 `%LOCALAPPDATA%\HyruleHUDSuite\installed\<family>`（`code_mods.py:11-12,29-30`）。**与追踪器无关系**：不读 RAM、无 RPC，与 Ocarina-Atlas 的 6545 live RPC + 存档回退路线正交（`Ocarina-Atlas/README.md:91,143`）。

**法律姿态（8 仓库共同点，F）**：**均未内置任天堂 ROM/模型/音频等原始资产**；已提交二进制只有自制图标/截图（`OOT-SW2SE/editor.ico`、`Oot3DS…/screenshots/*`、`Links-Toolbox/docs/*.png`）与自编译 `.3dsx/.smdh`（`OoT3D-Editor/sd/3ds/OoT3DEditor/`）。`oot-save-edit/test-data/new-game.sra` 是 32 KB 合成空白档（头 `98 09 10 21 "ZELDA"`，对应 `saveheader.ts:28-41`）。"用户自备"条款：`Links-Toolbox/README.md:203`（字段名不入仓，从用户 `game/romfs` 抽取存 `names-cache-*.json`，`nxsave.py:12-13,863,1036`）、`OOT-SW2SE/README.md:146`（只读你打开的文件）、`web/README.md:9`、`HeyListen/engine/docs/saves.md:8-9`。风险项：`oot3d-tracker` 与 `ZeldaOOTRemakeSaveEditor` **无 LICENSE（默认保留所有权利）**；`Links-Toolbox` 的 release zip 内含 6 个改动画文件的 mod（`README.md:36-49`），仓库本身不含该目录。

---

## 5. 音乐 / 曲谱 / 陶笛工具

### 5.1 seq64：N64 音序器的事实标准

`sauraen/seq64` ★165，C++，**GPL-3**（`LICENSE.txt:1`、`README.md:11`），Copyright 2014-2023 Sauraen（`CMakeLists.txt:5`、`CommandLine.cpp:31`），HEAD `47500ab`（2025-10-26「mute and long notes commands optional for RTYI」），1.1 MB。

* **框架是 JUCE，不是 Qt**：`CMakeLists.txt:25 project(SEQ64 VERSION 2.4.0)`、`:27-29` 强制 JUCE 子模块（`.gitmodules:2-3` → juce-framework/JUCE）、`:117 juce_add_gui_app`、`:125-132` 链接 juce_gui_basics；控制台版只链 juce_core/events/audio_basics/data_structures（`:91-95`）。`:36-55` Linux 下用 **`xset q` 探测 X** 来决定 GUI 还是 CLI —— 正是 2026 年 issue #42 的成因。
* 模块：`Source/CMakeLists.txt:2-22` 分 shared（`CommandLine`/`Common`/`SeqFile`）/console（`MainConsole.cpp`）/GUI（`MainGUI.cpp`、`SeqEditor`、`SeqABIEditor`、`TextListBox`）；**`Source/SeqFile.cpp` 6,257 行是转换引擎本体**。
* 读写格式（`CommandLine.cpp:68-70,80-98`）：MIDI `.mid/.midi/.rmi`；MML 汇编 `.mus/.mml/.asm/.s`；二进制 `.com/.aseq/.seq/.m64/.bin/.dat`。**V2 明确不含 bank 编辑**（`README.md:36` 指向 AudiobankToC），`:2256`、`:2587` 的 `TODO banks / TODO bank integration` 是 V1 遗留注释代码。
* **游戏清单不是枚举而是数据**：`abi/*.xml` 文件名即支持列表（`SeqFile.cpp:157-172 getAvailABIs()` 扫目录、`:173-186 loadABI()`）；CLI 支持前缀/包含匹配（`CommandLine.cpp:268-279`，`--game=zelda` → `4 - Zelda`）；GUI 填充 `SeqEditor.cpp:608-611`。OoT ABI = **`abi/4 - Zelda.xml`（599 行）**，另有社区扩展版 `abi/4b - Zelda community only.xml`（590 行）。多作覆盖见 `README.md:50-62`（SM64/SM64 EU/SF64/MK64/OoT 完整；**MM 复用 OoT ABI** 但 SFX 少几条命令 `:61`；Yoshi's Story `:60`）；3DS `bcseq` **不支持**（issue #27）。ABI 注释里保留反编译痕迹：`abi/4 - Zelda.xml:411-415` `loadbank → func_800E4EEC(...)`、`:170` 猜测 FIR 与 "ADPCM codebooks" 有关。
* 本体**不做采样编解码**；"NX64" 在所有相关仓库中**未找到**。
* 维护状态（GitHub API 实取）：#42（2026-08-13 提，2026-08-21 sauraen 关闭）、#39（2025-11-03 open，循环问题）、#38（2025-07-20 open）、#37（2025-04-26 PR 合并，RoadrunnerWMC）、**#34 open：Windows/Linux 的 `chorus` 差异会破坏 OoT 自定义音乐**。

### 5.2 OoT 音序可编辑的完整链条（isghj5 系补齐了 seq64 不管的那半）

```
WAV ──z64audio.exe──▶ .zsound + SEQ64 XML ──┐
                                             ├─▶ .zbank/.bankmeta ──▶ ROM
既有 bank ◀──z64-bank-converter──────────────┘
MIDI ──seq64──▶ .mus / .com(.seq) ──▶ 序列指针表写入
```

* `z64-sample-creation-tools/WAV to ZSOUND.py:39-40`（默认 **32000 Hz**、root note 60）、`:75-77`（产出 `*.vadpcm.bin`/`*.book.bin`/`*.loopbook.bin`）、`:82`（`NAME_TEMPADDR.zsound`）；`z64audio.toml:12`；README:2,26 指明依赖第三方 z64tools/z64audio。
* `z64-bank-converter` 输入必须是 `.zbank` + `.bankmeta` **成对**（`README.md:46-47`；主脚本 `Zelda64 Bank Converter.py:273-279`，`-g {oot,mm}`）。**无 magic，纯定长结构解析**：bankmeta `'>6B1H'`（medium/cache/table_id×2/num_instruments/num_drums/num_effects，`utils/audiobank/Audiobank.py:54,68,78`）；Sample 0x10 字节 `'>4I'`（`structs/Sample.py:60,101-106`），位域 `codec>>28&7`、`medium>>26&3`、`relocated>>24`、低 24 位 size（`:108-113`），并**断言只允许 ADPCM/SMALL_ADPCM + RAM + 未重定位**（`:115-119`）；Loopbook `'>2I 1i 1I'` + 16×s16 尾（`structs/Loopbook.py:56,63-73`）；Codebook order/npredictors + 16×s16（`structs/Codebook.py:60,70-77`）；常量表 `utils/Enums.py:48-62`（ADPCM=0、SMALL_ADPCM=3…）。
* 输出的 XML **就是 SEQ64 V1 bank schema**：struct 名 `ALADPCMBook/ALADPCMPredictor/ALADPCMLoop/ALADPCMTail`（`Codebook.py:88-105`、`Loopbook.py:103-109`），模板见 `README.md:69-196`（`<abfstructs align="16">` 在 `:74`）。
* **为何离不开反编译工程**（F，本报告的核心结构性观察）：采样以 **ROM 绝对地址为键**（`utils/SampleNames.py:5-26`，格式 `type:NAME:KEY:AUDIOTABLE`，例 `0x0FAD40: 'Ocarina:G080:AT01'`），并按 audiotable 号加基址：OoT `0x3FA9E0/0x4006B0/0x41D760/0x427D30/0x4377E0`（`utils/Helpers.py:36-51`）、MM `0x538CC0`（`:53-58`）。同一批地址在 OoTR 里同样硬编码：`Music.py:22-24`（DMA：audioseq=4/audiobank=3/audiotable=5）、`Music.py:242`（要求符号 `CFG_AUDIOBANK_TABLE_EXTENDED_ADDR`）、`Music.py:259`（序列指针表 `0xB89AE0+i*0x10`）、`OcarinaSongs.py:23-26`（播放表 `0xB781DC`/长度 0xA0、激活表 `0xB78E5C`/9 字节）。→ **音乐工具链实际上是反编译项目的下游消费者**，这一点在同类报告里很少被写明。
* `matthewcpp/sbc`（Go，MIT 2021，137 行，`main.go` 无 go.mod）：`README.md:2` 明写是 **SGI IRIX N64 Audio Tools 的 Sequence Bank Compiler 的重写**。输出 `.sbk`（`main.go:52`）：头 = uint16 magic **21297** + uint16 条目数（`:104-109`），条目 (offset,size) 各 uint32（`:112-115`），8 字节对齐、空隙填 0xFF（`:61-66,120-123`）、正文尾 0 填充（`:135`）。链条位置见 `README.md:11-15`：MIDI→(midicvt/musbc)→序列→**sbc→.sbk**→运行时 `alSeqFileNew`/`nuAuSeqPlayerSeqSet`。它解释了 seq64 V2 为何只覆盖 "MIDI↔.mus↔.com/.seq"（`README.md:20,36`）而把库合并留给官方/现代 decomp 构建。
* `Bitlytic/SM64MidiCompanion`（C# WinForms，v0.1，**未找到 LICENSE**，2022-08-02）：**它不自己转，而是调 seq64**——`Form1.cs:24` 执行 `seq64_console.exe --abi=sm64`，输入 `.mid` + decomp `sound_bank` JSON（`:110-111`），输出 `.m64`。`Bitlytic/LmmsToM64`：**本地不存在该仓库（未找到）**。
* **OoTR 自身的音乐能力**（对照：周边工具改数据，随机器改运行时）：`Music.py:26+ bgm_sequence_ids`、`:125 SequenceData`、`:132 process_sequences`（`:190-195` 分派 `.ootrs/.zseq/.mmrs`）、`:206 shuffle_music`、`:241 rebuild_sequences`、`:673 rebuild_pointers_table`、`:684 randomize_music`、`:872/879` disable/restore；`OcarinaSongs.py:28-34` 歌曲激活编码、`:390 generate_song_list`、`:446-469 patch_songs`（`:452 0xB78AA0` 青蛙特殊项）。`.ootrs` 容器 = `.seq` + `.meta`（含 `ZSOUND:` 元数据），见 `data/Music/README.md:2-23`，`:25-30` 明确"序列为 seq64 格式，曲目列表引自 seq64"。OoTR wiki Readme（2026-01-01）指定自定义音乐流程 = MIDI + **seq64** 产 AudioSeq 曲目 → 放 `data/Music` → 打包 `.ootrs`，并警告混音处于 "**legal grey area**"（R）。

### 5.3 陶笛类工具：六种实现，建模的全是"游戏按键"而非真实陶笛

| 仓库 | 输入 | 指法/键位建模 | 输出 | 面向重制版？ |
|---|---|---|---|---|
| **vaexenc/ocarina**（TS/React，MIT 2026，5.7 MB，2026-06-24） | 键盘 5 键（**无 MIDI 导入**） | 仅 A+C 四向（`src/data/note-map.ts:10-16`、`song-data.ts:1`）；bend ±100/±200 音分、vibrato 6 Hz/35 音分（`use-pitch-modifiers.ts:8-17`）；detune 复现 D-F-A-B-D（`instrument-data.ts:31-36`） | 浏览器演奏 + 曲谱卡片；**23 首**按 OoT/MM 分组（`song-data.ts:167-182`）；speedrun 模式（`modes.ts:5`） | **否**（`mic`/`remake` grep 0 命中） |
| **ckarnell/ocarina-songbook**（Python，28 MB，**未找到 LICENSE**，2026-10-16） | 音名 + 拍数（`ocarina build --letters/--beats`，`cli.py:105`）或从 Songsterr/Hooktheory 抓取（`:144`） | 5 键 MIDI **62/65/69/71/74** + 修饰 R/Z=±1、摇杆=±2，音域 B3–F5，`SPELLINGS` 全表 `pitch.py:22-51`；自动挑"最少修饰"移调（`build.py:13-14`） | `songs/*.json`、WAV 预览（`cli.py:125`）、**pynput 直接打到 Ship of Harkinian/模拟器**（`keyboard_play.py:1-2`） | 否（面向 vanilla，`pitch.py:4`）；含 **OoT 音频派生**：`oot_soundfont.py:8-27` 读 `oot.o2r` 的 SoH AudioSample V2（codec 0=VADPCM 9 字节帧 / 3=small VADPCM），引用 decomp `tools/audio/sampleconv`、`tools/audio/extraction/tuning.py` |
| **indigojoha/OoTROcarinaMapper**（单页 JS，**无 LICENSE**，2026-11-11） | 16 列音阶网格点选，或粘贴 16 槽 JSON（`script.js:266-297`） | **按钮而非孔**：`neutrals {L:74,R:77,A:81,Y:83,X:86}`（`script.js:1`），每键 ±1(十字键)/±2(摇杆)（`:3-11`），音名 C5–E6、`highestMidi=88`（`:13-20,28`）；按 complexity 排序、优先中性指法（`:64-71,77-88`） | 网页"按钮五线谱" + `Y+1 or X` 文本（`:100`） | **是**：`index.html:16` "controller inputs for Ocarina of Time on Nintendo Switch 2" |
| **AlexanderCato/zelda-ocarina-button-rockstar**（Python + 提示词，MIT 2026，2026-11-10） | 音名串/歌名交给 AI（`README.md:50`） | **双布局**：`ocarina_check.py:2-3` `NORMAL{L,R,A,Y,X}` / `CLASSIC{R,B,A,Y,X}`，同为 74/77/81/83/86；`:6-12` 修饰物理键（Classic 的半音在 L/ZL）；`:13` 音域 **B4–F6(71-89)**；`:16-21` 12 首歌的按钮串（防误触发）；`:39-58,85-101` 代价最优化指法，`:156-183` 穷举移调，`:184-194` 推荐布局 | 两布局的按键文本 + 难度/调性摘要（自带 `self_test`） | **是**，且记录了重制版实测行为：`SKILL.md:202-212`（约 6 秒换气、bend 会产生连音滑音、`:205` **歌曲识别只认"未修饰"按键**）、`:198-200` 稻草人 8 音曲 |
| **elpendor/flipparina**（**Flipper Zero** `.fap`，C，MIT 2026，2026-11-17） | 内置曲表 或 SD 卡 `.mid` | 三种输出（`ocarina.h:15-20`）：N64(NSO) 手柄、**Switch 2 手柄**、**喇叭出声喂主机麦克风**（`README.md:44`、`docs/catalog.md:3,9`）；基准音高 `ocarina.c:12 base_pitch{62,65,69,71,74}`，指法搜索 `:24-38`（mod 0/±1 + `BEND_SEMITONES=2`，`config.h:26`），±12/24/36 八度折叠 `:61-66`，自动最佳移调 `:143`，12-TET 频率 `:124-125`，喇叭直放 `player.c:67`（音域 36–96，`config.h:20-21`） | USB HID 输入 / 喇叭音频 | **是**。曲库 `songs.c:13-26`（OoT 12 首）、`:28-40`（MM），编码为 `A/v/></^ + 时长` 串，`:12` 注明"时长取自 decomp 的 `sOcarinaSongNotes`"；另解析 SD 卡 MIDI：`flipparina.c:200`、`midi.c:205 (MThd)/:223 (MTrk)/:174 (CC1→颤音)` |
| **samuelyuan/OcarinaPlayer**（Go + Ebiten，MIT 2022，22 MB，2025-11-19） | 键盘（`game.go:43-48` 方向键/X→U/D/L/R/A） | **硬编码 5 键序列** | 桌面演奏 + **预渲染 MP3** 播放（`game.go:78+ songDatabase`、`audio/songs/*.mp3` 18 首含 MM、`audio/ocarina/{A,B,D,D2,F}.wav` 单音）；**不解析 ROM/MIDI** | 否；素材是游戏音频派生（**含任天堂实音**） |

**共同结构性缺陷（F）**：六者**没有一个建模真实陶笛的孔位组合或超吹（`-O`/`-<`）**，全部以游戏提供的 5 个"音按钮" + ±1/±2 弯音为准 —— 它们是**游戏内乐器**的辅助器，不是陶笛软件。真实陶笛指法只以 bend 音分形式间接出现（`ocarina`/`flipparina`）。另注意**两个世代的中性音高集合相差恰好一个八度**：N64 侧 `{62,65,69,71,74}` = D4 F4 A4 B4 D5（`ocarina-songbook/ocarina_songbook/pitch.py:26` 的 `BUTTON_PITCH`、`flipparina/ocarina.c:12` 的 `base_pitch`），Switch 2 侧 `{74,77,81,83,86}` = D5 F5 A5 B5 D6（`OoTROcarinaMapper/script.js:1` 的 `neutrals`、`zelda-ocarina-button-rockstar/…/ocarina_check.py:2-3`），同时**按键从 A+C 四向换成 L/R/A/Y/X**、音域从 B3–F5 抬到 B4–F6（`ocarina_check.py:13` `LOW, HIGH = 71, 89`）—— 音高集合与键位是**成组变化**的，任何跨世代的曲谱文件都不能直接复用。

**官方那一侧**（F）：2026 重制版的 ZELDA NOTES 内置**触摸屏虚拟陶笛**（演奏引导 + 自动演奏 + 作曲 + 二维码分享），且重制版笛子支持 Switch 2 **内置麦克风哼唱识别**（GameSpot 2026-09-08，R）。`flipparina` 的"喇叭喂麦克风"输出（`README.md:44`）正是对这一新识别路径的适配 —— **官方增加一种输入方式，社区工具当天就多一种攻击面**。

---

## 6. 练习菜单与竞速辅助

### 6.1 3DS：oot3d_practice_menu（本地仓库，详见 §1.2）

功能全表（F）：无敌、时间冻结、帧进/暂停、场景传送、年龄切换、物品直写、装备、NoClip、Actor 生成/删除、任意内存观察。**不含地图、不含追踪、不含 Boss 立即重置**（最接近的是 Warps→Bosses 表 `src/warps.c:56-71`；`bossBattleVictories[9]` 在 `z3D.h:146` 声明但全仓无引用）。唤出组合键 **L1+R1+SELECT**（`src/commands.c:262-268`）。

### 6.2 N64/GC：**"练习菜单"其实藏在游戏自己身上**

TCRF 记录各版本（含 **CN iQue 版**）存在可开启游戏内 **Inventory Editor** 的 Gameshark 码（R）；GC `gc-eu-mq-dbg` 构建自带 test scenes 与 `elf_msg`（见第 04 号报告，F）。→ 三个世代里"练习工具"的来源分别是：**注入（3DS）/ 调试开关（N64·GC）/ 官方特性（2026）**。

### 6.3 竞速辅助工具的三种合规等级

| 等级 | 工具形态 | 竞赛侧态度 |
|---|---|---|
| 允许 | 纯人工 tracker（Ivan、deains）、外部计时器、input viewer（Ivan `inputviewer/`） | 正常 |
| **禁止** | **auto tracker 与 logic tracker** | racetime/SRL 明文禁用（F/R）；Track-OOT 为此提供 `race` 布局并在联机时**禁用 undo**（`src/CHANGELOG.MD:1499`） |
| 游戏外作弊 | trainer（NX2navi-Trainer，见 §7.3）、Gameshark 码、内存观察 | 本报告只描述实现层级，不评价使用 |

### 6.4 2026：官方把练习模式吸收进游戏

**Hall of Fallen Foes（败敌殿堂）**：已击败敌人的雕像 + 弱点/出没图鉴 + **Boss 无限重战并记录最佳用时**（F，官方页面）。**ZELDA NOTES** 侧另有"今日记录"统计。**没有**慢放/帧进练习模式，**没有** MQ。→ 官方选择覆盖"重复打 Boss 练招"这一最主流的练习需求，把 NoClip/时间冻结/内存观察这类需求留给社区（`slopped-up-sloperina` 的 speedometer 与 `W_X/W_Y/W_Z` 坐标观察，见 `oot-remake-modding/mods/slopped-up-sloperina/src/watches.c:766`，F）。
## 7. 2026 重制版伴侣工具栈：五层结构与最脆弱的一层

### 7.1 分层

| 层 | 成员 | 依据 |
|---|---|---|
| **L4 移植/输入** | HookLink（nxnative→Metal）、harkinianpad（SoH + iOS）、NX2navi 触屏/手柄、KaeGae 的 Npad/键鼠包装 | `HookLink/README.md:13`；`harkinianpad/README.md:20-23`；`oot-remake-modding/docs/structures.md:79-105` |
| **L3 应用** | Trainer（NX2navi-Trainer）、Randomizer（oot-remake-rando）、联机（Squadala）、Tracker（Track-OOT 之外的 hyrule-tracker、Ocarina-Atlas、rando 自带 overlay）、练习菜单（slopped-up-sloperina）、存档编辑（OOT-SW2SE、Links-Toolbox、Cloowney）、地图（Ocarina-Atlas、MapGenie） | 见各章 |
| **L2 地址语义（基座）** | `oot-remake-modding` 的 `.ld`/`.toml`/docs；KaeGae `kae_sym`/`kae_field`（`api/include/kaegae.h:92-97`）；GameData MurmurHash3 字段名；指令签名扫描 | `symbols/e0f3ccfa.toml:1-12`；`NX2navi_Trainer.py:122` |
| **L1 内存读写** | HeyListen `page_view`（自进程）、NX2navi `plat_mem_read`、Longshot/Hookshot `HosCoopApi.read/write/cas32/call`、pymem `ReadProcessMemory`、hyrule-tracker `process_vm_readv`、（外部 6545 JSON-RPC 伪装层） | `hl_stream.c:833-841`；`debug_tools.c:1-8`；`hos_coop_api.h:38-44` |
| **L0 宿主** | Lichii/Fritchii/Eden 模拟；HeyListen/NX2navi/Longshot/Hookshot 静态重编译；官方只有 Zelda Notes 的存档滚动日志 | `oot-remake-modding/docs/community.md:8-14`；`docs/game-versions.md:51-59` |

### 7.2 L2 基座：`oot-remake-modding` 用三份同源文件充当"官方符号表"

`Kabosune/oot-remake-modding`（★26，LICENSE 有，HEAD `4967c1d` 2026-11-17，24 MB）。**格式不是一份 JSON，而是三份必须互相同步的载体**（F）：

| 载体 | 路径 | 作用 |
|---|---|---|
| 文档表 | `docs/address-map.md:1-128` | 人读，**每条带 "Source" 列（Hookshot / Disasm / Untested）** —— 罕见的自证据分级 |
| 链接脚本宏 | `symbols/e0f3ccfa.ld:1-60` | `VT_ACTOR`/`CCT_CHANGE_POSITION` 等常量，IPS mod 直接链接 |
| 机器可读符号包 | `symbols/e0f3ccfa.toml:1-12,440-520` | KaeGae Core 用，`[function]/[global]/[vtable]/[field]/[hookpoint]`，mods **用名字问** |
| 一致性测试 | `tests/test_kaegae_symbols.py` | toml 与 .ld 必须吻合、两包条目数必须相等（`README.md:107-109`） |

版本标注：Title ID `0400486024DEC000`；**1.1.1 = v131072 = build `E0F3CCFA…`**、1.0.0 = v65536 = `33FA6C2C…`（`README.md:11-15`；`symbols/e0f3ccfa.toml:10-11`；`e0f3ccfa.ld:1`）。模块基址 `0x80020000`，与 Hookshot 对齐以便 `f_XXXXXXXX` 同名互通（`README.md:116-117`、`docs/address-map.md:3`）；Core 从游戏内自识别 build（`[detect] nnmain/mod0/size`，`symbols/e0f3ccfa.toml:14-17`）。

关键语义（F）：

* **玩家坐标**：物理 actor `+0x31c` 是 **f32[3]**、`+0x328` 是 3×4 旋转；组件链 `[[actor+0x270]+0x320]` → `player = movement-0x1d50`；字段名 `Actor.position`（`docs/structures.md:35-54`、`symbols/e0f3ccfa.toml:465-467`）。**单位"看起来是米"**（原文带保留："Units look like metres"，用 Hookshot 自由镜头 8 units/s 与飞行 4 units/s 反推，`docs/structures.md:3-4`）；旋转表 `+0x328` 为 `f32[3][4]`、行距 0xc，文档给出的航向是 **`yaw = atan2(+0x330, +0x348)`**（矩阵行，`:44`）—— 与 Ocarina-Atlas 用速度差分的 `atan2(dx,-dz)`（`lichii-server.ts:3674-3689`）**不是同一种算法**，两条实现各自成立但不可互换。actor 上的速度 `+0x388..+0x390` **是只读输出**，且"Link 的位置被复制到多处、多数副本每帧被游戏重写，写它们没有持久效果"，文档给出的正解是调物理控制器的 **`ChangePosition`**（`f_806d2530`，vtable 槽 `+0xe8`，控制器链 `[[[[[actor+0x270]+80]+32]+16]+8]`）（`docs/sloperina-findings.md:60-62`、`docs/address-map.md:29`）。**92 个场景 / 465 个出生点**带坐标与弧度旋转的清单在 `docs/warps.md:14`。**世界时间与天气不能靠写字段获得**：读取要走 WorldMgr 的"只读面" `[wm+0x578]` 的虚表槽 `+0x30`（世界时间秒，游戏内部 mod 86400 换算时分）与 `+0xF0`（天气 0–4）；写入是**请求式**——setter 先锁 `wm+0x490` 的 mutex，把目标写进 `wm+0xDE8` 起的两个 0x11A0 字节块之一（由 `wm+0x3128` 选块），**WorldMgr 在下一次 update 才生效**（`docs/world.md:12-13`）。WorldMgr 本体 0x40D8 字节，指针变量 `0x4E7EAB0`（1.0.0 为 `0x4E0E668`，由 `tools/port_pack.py` 移植，40 处引用中 37 处一致，`docs/world.md:11`）。存档侧的时间是不具名 GameData 字段 `0xEF10B80F`/`0x3F8D9D86`/`0x46920F1B`，**仅在 load 时起作用**（`docs/world.md:14`）。→ 这一条是"重制版周边工具不能沿用 N64 式改内存直觉"的最具体例证。
* **GlobalContext 的对应物**是一组 GOT 单例槽：`GameDataMgr 0x84ea42a8`（GOT `0x84c9e180`）、`EventMgr 0x84eb4840`、`PouchMgr 0x84e98e28`、相机管理器 `[[0x84ca6bc0]]`（`docs/address-map.md:91-100`）；场景 `0x84eb7648 → [[this]+0x1f0]`、帧计数 `0x84ea4ba0`、WorldMgr `0x4E7EAB0`（`docs/sloperina-findings.md:43-52`、`docs/world.md:11-14`）。
* **SaveContext 的对应物 = GameData**：字段值 = **MurmurHash3_32(名字)**；Int/Float 对象 0x28 字节，值在 `+0x1c` **且副本在 `+0x20`（两处都必须写）**；Bool 0x18，值 `+0x16`、副本 `+0x17`；`progress.sav` 从 0x20 起就是 (hash, value) 对，**无校验和**；约 **15,000** 个字段的清单在 `GameDataList.Product.110.byml.zs`（`docs/sloperina-findings.md:95-108`）。
* 年龄：`f_804d6db0 IsChildLink`，快路径 `[[actor+0x270]+0x358]+0x1c5d`（`docs/address-map.md:43`）。
* patch 工具：`tools/nxpatch.py` 用 clang 汇编 → ld.lld 在 code cave 按 `.ld` 链接 → **每个 hook 点比对原指令字**（`--verify-main`）→ 输出 IPS32（偏移 +0x100 计入 NSO 头）与 zip（`tools/nxpatch.py:1-16,24-26,59-75,231-248`；`docs/writing-a-patch.md:1-20`）；连 36 字节的 `.text` padding 都能装真代码（`split_into_slots`，`:90`）。空间与 hook 点由 `registry/e0f3ccfa.toml:1-12` + `docs/memory-map.md:14-83` 的全局认领表管理，`kaegae.py check` 查冲突。

**一个必须写进报告的陷阱（F）**：重制版 GameData 字段值恰好在 `+0x1c`、副本在 `+0x20`，而 N64/3DS 的 actor 头部 `+0x1c/0x20/0x24` 是 x/y/z。**数字巧合，语义完全不同** —— 重制版跑 TotK 的 ModuleSystem（`docs/engine.md:3-4`），actor 坐标是 `+0x31c` 的三个 f32，与 N64 的 s16/u16 定点量既不同偏移也不同类型。NX2navi-Trainer 正是按 `stride 40 / value_offset 0x1c` 索引 GameData（`NX2navi_Trainer.py:32-35`）。**没有任何仓库声明两者可互换，也未找到映射表** → 把 2011 年的 N64 直觉搬到 2026 年重制版会静默读到存档字段而非坐标。

### 7.3 宿主暴露的接口：三家没有一个提供 RPC

| 宿主 | 对外接口（F） |
|---|---|
| **HeyListen**（Sacrappy，★134，HEAD `e0b06cc` 2026-11-18，74 MB） | **无 RPC / 无 debug server（未找到）**。有：① HTTP 遥测客户端 `POST /telemetry/crash`、`/telemetry/perf`，地址来自 `nx2/addresses.ini` 的 `telemetry=`（**出厂为空**），仅 https，例外 `127.0.0.1/localhost/[::1]` 需 `HL_TELEMETRY_TEST_HTTP=1`（`docs/performance-statistics.md:160,170-173`；`engine/host/hos_telemetry.h:10,16-19,43`）；② Discord IPC（JSON 帧，`engine/host/discord_rpc.c:15`）；③ `ReadProcessMemory` **只读自己进程**做页观测（`engine/host/hl_stream.c:833-841`）。mod：romfs 文件 mod 可用；exefs `.ips/.pchtxt` 在**构建期**进 lift（重建约 16 min）；**subsdk 模块被明确拒**——翻译后的代码没有可改写的指令（`docs/USER_GUIDE.md:152-193`，关键 `:167-169,181-184`） |
| **NX2navi**（ZuanCrisp，32 MB，HEAD 2026-11-15） | **无对外 RPC**；有游戏内 debug 菜单（**F9**）：读走 `plat_mem_read`，写排队到游戏线程；支持 Atmosphere cheat 文件（`<build id>.txt`）并在**自写的 dmnt cheat VM**（opcode 0–A）里逐帧执行（`runtime/debug_tools.c:1-20`；`runtime/cheats.c:1-17`；`tools/cheats.py:96-141`）。**前提：guest 地址 == host 地址（<2^39）**（`docs/architecture.md:25`）—— 这正是外部工具能用绝对地址的条件；subsdk 不读（`docs/launcher.md:397`） |
| **Squadala / oot-remake-multiplayer**（daurosk，★222，GPL-3，22 MB，HEAD 2026-11-17） | 靠 Longshot/Hookshot 的 **C 回调结构体**（非共享内存、非 RPC）：`HosCoopApi{main_base, build_id, read, write, cas32, call, log, pad_buttons, key_down, scratch}`，由 `coop_bridge.c` 加载 `hookshot_coop.dll` 后交付（`hookshot/src/hos_coop_api.h:31-59`）。网络 = **ENet over UDP，默认 27510**，6 通道，1100/576 数据报，服务器限速 60/240 msg/s，**X25519 + XChaCha20-Poly1305 全流量加密**，UPnP/PCP/STUN/relay 打洞，主机存档为准（`docs/hookshot.md:44-57,102`；`docs/protocol.md:1,23,29-40`；`server/options.hpp:17`）；早期模拟器路线是直读 guest 内存（`docs/lichii.md:33,36,38`） |
| **官方** | **官方 modding API 不存在**（`oot-remake-modding/README.md:141` 自称 unofficial fan project）。唯一官方出口 = 1.1.1 为 **Zelda Notes** 引入的 NPLN/leaderboard 库 + 存档里两条滚动日志（**1,024 条事件 + 1,024 条位置，离线也写**）（`docs/game-versions.md:39-61`）。→ **这解释了为什么 rando 的 tracker 宁愿每 1.5 s 轮询 `progress.sav`** |

**NX2navi-Trainer（Miragui，424 KB，2026-11-14）作为"新一代 trainer"的样本**：硬编码 `0x1BDEEE134/138`；save table `stride=40, value_offset=0x1c`；**34 项 cheat 按 `save_hash`（MurmurHash3）+ `index_offset` 索引**（`NX2navi_Trainer.py:20-21,32-35,37-121`）；健康/魔力走**指令签名** `8055ab31 {main+0x4b63438}`、`82b176 973ad612 4933fa3a`（`:122,131`）；功能为无限生命/自定义心、无限魔力、无限卢比与自定义、箭/炸弹/炸弹虫/种子/木棒/坚果/豆子/泡沫种子、黄金甲虫数、全回复、999 卢比，预设存 `nx2navi_preset.json`（`README.md:17-25`、`:222-244`）；**不注入** —— 独立 Python 进程 + `pymem`（VirtualQueryEx 全区枚举 + Read/WriteProcessMemory）+ `keyboard` 全局热键 + Tkinter 半透明置顶 HUD 吸附 `ocarina.exe` 窗口（`requirements.txt:1-2`、`:213-215,246-267,526-527,552-565`）；写时**值与副本双写**（`:371-375`）。**无坐标显示**（README 未列；坐标类需求由 `oot-remake-modding` 的 speedometer 与 sloperina `W_X/W_Y/W_Z` 满足，`mods/slopped-up-sloperina/src/watches.c:766`）。

**与 AR/GameShark 的六项代际差异**（F，本章结论）：① 键不再是绝对地址而是**字段名的 MurmurHash3**；② **签名扫描 + 值域校验**取代固定码；③ "值 + 副本"双写，因为游戏每帧重写副本；④ **写内存被判定不可靠**，官方结论是"调游戏的 ChangePosition/函数"（`docs/sloperina-findings.md:60-62`、`docs/world.md:12-14`）；⑤ 代码级改动从"改内存的 AR 码"变成 KaeGae 的**指令级 hook + 构建期锚点校验**（`docs/writing-a-patch.md:1-20`）；⑥ trainer 自己承认布局会变，**保留手工数值扫描兜底**（`README.md:27`）。

### 7.4 重制版随机器与移动端

`sgurgurich/oot-remake-rando`（MIT，1.4 MB，HEAD `2033b4c` 2026-11-13）：**不改 exe、不改存档，只改 romfs** —— 抽 BYML/gyml（actor 放置、`GameDataList`、RSDB、RSTB）改写后产出 romfs mod 文件夹/zip；可选产出**整份 patched `.nup`**：按 NUP v1 追加改后文件、更新 `131072` 包内该条目的 size/offset/SHA-256、重写 superblock 与 TOC 哈希，**旧字节永不写**（`README.md:100-113`；`tools/nup_patch.py:114,248`）。选项：五类位置（chest 默认；scrub/skulltula/pot/freestanding 标 experimental）、Small Keys 仅 vanilla（**缺房间级钥匙逻辑**）、Boss Keys vanilla|own_dungeon、Maps&Compasses vanilla|own_dungeon|anywhere、Logic/No logic、Open Kokiri Forest/Deku/Door of Time/Zora's Fountain（**以 GameData 起始值实现，只对新档生效**）、keep_event_chests、skip_intro、skippable_cutscenes（`randomizer/settings.py:13-57`、`README.md:186-215`）。**歌曲/NPC 礼物/Boss 奖励做不到**（由 EventFlow 脚本发放，`README.md:165-170`）。spoiler 是 **纯文本** `<name>-spoiler.txt`（`randomize.py:57,128,155,230-231`），**可被 EmoTracker/poptracker 导入的结构化导出：未找到**；生态闭环靠**自带 overlay tracker**：每 1.5 s 轮询 Lichii 的 `progress.sav`，按 `IsGet.<actor>` 判定、`PouchData.*.StockNum`/`PlayerStatus.MaxLife` 计数，图标从用户 dump 的 BNTX/ASTC 解码，数据表 `data/tracker.json` 由 `tools/tracker_data.py` 生成（`README.md:240-266`）。仓库内 `tools/check_no_game_files.py` **在 CI 层面强制不含游戏文件**（`README.md:26-30`）。

移动端两条相反路线（F）：

| | **HookLink**（dsi-hub，★138，Swift，MIT，684 KB，2026-11-17） | **harkinianpad**（chrissotraidis，★436，Python，**无 LICENSE → All rights reserved**，3.6 MB，2026-11-13） |
|---|---|---|
| 路线 | Mac 上的 Swift 构建器，驱动 **nxnative** 把 Switch 2 版**翻译成 Metal/原生**，README:13 "**No emulator is involved**" | **Ship of Harkinian/Shipwright + libultraship** 打成 iOS 原生 app，Metal 渲染 + 横向触控（README:20-23；pin 见 `sources.lock.json:3-28`，v0.2.3/build 10） |
| iOS JIT 限制 | 靠**装机前 AOT 改写**规避：6 步构建含 "unpack, rewrite, split shaders"（`README.md:54`）；"JIT" 一词在 README/CHANGELOG **未找到** | **有专章论证**：发布二进制无任何运行期 CPU 代码生成（搜 `PROT_EXEC/MAP_JIT/mprotect/asmjit` 零命中），逻辑是 AOT 反编译 C，shader 用 `MTLDevice::newLibrary`（合法），`Overlay_Load` 被硬 `return 0`，LUS 的 TinyCC mod 编译器在 `ENABLE_SCRIPTING=OFF` 之后（`docs/ios-feasibility-and-implementation-plan.md:26`；`docs/INSTALL_IPA.md:65` "No jailbreak or JIT"） |
| 资产/用户输入 | 不含游戏/密钥/成品 app（`README.md:11-12`）；用户自备 `.nup`（`0400486024DEC000`, update v131072）+ 开发者模式 + 侧载（`:66-72`） | 不含 ROM 或 ROM 衍生可玩归档（`RIGHTS_AND_LICENSES.md:24-27`）；用户自备受支持 ROM 经 Files 拷入 `HarkinianPad`（`README.md:95,146-149`）；声明 Shipwright 顶层无 license，按社区 modding 模型分发（`:1-30`） |

**EmoTracker 社区分支的意外发现**（F）：`EmoTracker-Community/EmoTracker`（C#/.NET 10 + Avalonia 11.3.3，4 项目方案 + SourceGenerators/Smoke/**MockRandomizer**）内置 **MCP server** —— Streamable HTTP + JSON-RPC 2.0（协议版本 `2024-11-05`），`http://localhost:27125/`（**无鉴权**，`EMOTRACKER_MCP_PORT` 可改），**62 个 tool**（`docs/mcp-server.md:15-19,65,85,126`）。即**追踪器主动把自己的状态暴露给 AI 客户端**。迁移动机**未找到**：README/CLAUDE/docs 无 "viktlgb"、无 fork 说明，仅剩 `LICENSE:3` "Copyright (c) 2026 EmoTracker Community"、wiki 链接指向本 org、以及 depth-1 克隆只有 1 个 merge commit 的不可查历史。

---

## 8. 数据出处与法律姿态对照

| 项目 | 资产来源 | 载体 | 姿态评估 |
|---|---|---|---|
| **noclip.website** | ROM 派生的 `.zelview0` / `.zar` / `.zsi` / `.cmb`，**由项目自己转储并托管** | 独立资产站 `z.noclip.website`（`.env:1`），**不在代码仓库内** | 代码 MIT + `LICENSE:1-9` 公开自陈**"非 clean-room 逆向"**；仓库无游戏资产但**服务端分发派生数据** → 风险转移到托管方 |
| **OoT-Interactive-Map** | Peardian（VGMaps）渲染图 + MzxRules ROM 数据转储 | **1.2 GB 瓦片直接提交进 git**（共 37,864 个 PNG） | MIT，但**事实上全量再分发**；README 还指向 mediafire 的 `maps.7z`（自相矛盾） |
| **ootm** | VGMaps/Peardian JPG + SoH 高分辨率截图 | 138 张 JPG 入仓（43 MB） | 署名链清晰（`OoTMConstants.java:32-40`），仍是第三方渲染物再分发 |
| **Track-OOT** | 提取的物品/地图图 | 372 个图 / **27 MB 入仓** | MIT 覆盖代码，资产未单独授权 |
| **EmoTrackerPacks / OoTMMR pack** | 提取 + 手绘混合 | 581 / 1,251 张 PNG（14–18 MB） | **两仓均无 LICENSE** |
| **OoTMM-Spoiler-To-Emotracker** | — | **113 MB**（本批次最大之一） | MIT；体积主要来自 pack 资源 |
| **StripesOO7/oot-tracker**（AP datapack） | **自制**：129 张图**每件都附 `.xcf` 源文件**（如 `1-4 Poh.png` + `1-4 Poh.xcf`） | 96 MB | **可证自绘**，是"干净资产"的正面样本 |
| **hyrule-tracker** | 构建期从 **Zelda Wiki** 抓图 + 用户自备 RomFS 生成点位 | **不入仓**（`src/renderer/item-images.json` 只记 URL） | 本批次**最保守**的资产策略；MIT |
| **Ocarina-Atlas** | 地图素材署名 **Nintendúo**；不含 ROM/密钥/NAND/存档 | 78 MB 入仓（WebP/SVG） | AGPL-3.0；README:15-17 明确分发约束 |
| **OOT-SW2SE / Links-Toolbox / oot-save-edit / Oot3DS-Azahar / HeyListen** | **零游戏资产**，只读用户提供的文件 | 图标/截图/自编译 `.3dsx` | `Links-Toolbox/README.md:203`（字段名从用户 romfs 抽取，缓存本地不入仓）、`OOT-SW2SE/README.md:146`（只读你打开的文件）、`HeyListen/engine/docs/saves.md:8-9`；**`oot-remake-rando` 用 `tools/check_no_game_files.py` 做成 CI 断言** |
| **zelda-ocarina-button-rockstar / Ivan 的图集** | 粉丝绘制（`Ivan/assets/LICENSE.md` 注明） | 入仓 | 授权链可查 |
| **OcarinaPlayer / vaexenc-ocarina** | **任天堂实音**（`.mp3`/`.wav` 直接入仓；`ocarina` 的 `public/audio/*.ogg` **来源未注明**） | 22 MB / 5.7 MB | 本批次**最直接的音频版权暴露** |
| **ocarina-songbook** | 读用户 `oot.o2r` 建 soundfont | 28 MB，**无 LICENSE** | 派生工具 + 无授权声明 |
| **ZeldaOOTRemakeSaveEditor** | 无资产，但**运行时抓 IGN 攻略正文/视频**（`README.md:122`） | 4.4 MB（无源码） | **无 LICENSE** + 第三方内容抓取 |
| **MapGenie** | 自有绘制 + 订阅数据库 | 闭源 | **IGN Entertainment / Magic Lamp Technologies Ltd** 商营，$14.99/年 |

**平台/执法时间线（周边工具视角）**：Yuzu 2024-03 和解 $2.4M → 2024-05 单次清除 8,535 个镜像 → 2026-02 覆盖 ≥12 款 Switch 模拟器（Citron/Eden/Kenji-NX/MeloNX/Pine/Pomelo/Ryubing/Sudachi）→ 2026-08 401 个 GitHub 仓库因 §1201 下架（Suyu 为主要目标）→ **2026-11-13 任天堂再发 DMCA 打击 Switch 2 模拟器与改机固件**（R，新浪财经），就在重制版可在 PC 跑起数日之后。2024-06 的先例值得记住：**GameBanana 数日内收到 60+ DMCA，其中含王国之泪随机器与自定义地图**（R）—— 即打击对象历史上**已包含"周边工具"**。反向事实：**未发现任何针对 OoT tracker/map App 的 Apple 下架记录**，iOS 商业 tracker 在 11-10/11-12 照常发版。

---

## 9. 结论与预测

1. **"地图 App"是两条互不相交的谱系**（F）：**位图谱系**（VGMaps/Peardian → Ecksters 交互地图、ootm、MapGenie、Zelda Dungeon）与**几何谱系**（ROM → SceneNavi、noclip、OoT_MapExt）。二者不共享底图、不互引、用户群也不重叠。用户口中的"3D 交互式地图"只有第二条能满足，但第二条**没有一个具备 overworld 拼装**（noclip `src/` 无 minimap 逻辑，未找到）。
2. **同一引擎结构被至少三次独立逆向，编号完全收敛，但语义命名暴露年代**（F）：`scene.h` / `HeaderLoader.cs:92-107` / `extractor.ts:33-51` 三表逐条可比，`0x00-0x14` 编号一致；`0x0C` 反编译已解为 `LIGHT_LIST` 而两个工具仍标 unknown/unused，`0x02` 更是硬冲突（`UNUSED_2` vs `Camera`）。**周边工具普遍落后反编译项目一个认知世代，因为它们只需要"够画"。**
3. **能看 N64 原版 3D 场景的公开途径确实存在，但功能不完整**：noclip 的 `zelview` 通道在 TypeScript 里跑 F3DZEX 显示列表（约 26 条 case，vs EAD 反汇编器 92 条、RT64 113 条），**但预渲染背景未支持**（`scenes.ts:159`），3DS 通道的**碰撞块完全未解析**（`zsi.ts:103` 有枚举、`:376-388` 无分支）。真正"完整还原几何+碰撞"的是 2014 年的 SceneNavi（`HeaderCommands/Collision.cs`）与 Utility of Time 生态。
4. **追踪器生态的制度性优势 = 随机器自己写了 ABI 文档**（F）：`Notes/auto-tracker-ctx.md` 给出固定地址 `0x8040_000C` + 版本字段 + 字段表 + 写入侧行号。而 2026 重制版**没有对应物**，社区只能自建 L2（`oot-remake-modding` 的三份同步符号载体 + MurmurHash3 字段名 + 指令签名）。**预测**：重制版周边工具的漂移成本将持续高于 N64 时代；最先死的是硬编码绝对地址的（`NX2navi_Trainer.py:20-21` 写死 `0x1BDEEE134`，且自带"手动扫描"退路），活得最久的是走 `progress.sav` (hash,value) 的（无校验和、字段名哈希与 build 无关，`docs/sloperina-findings.md:95-108`）—— 这也正是 rando 自带 overlay 与 hyrule-tracker 存档路线的选择。
5. **版本键命名不统一是 L2 的额外风险，而非小事**（F）：modding 记 v131072 = "1.1.1"（`oot-remake-modding/README.md:14`），rando 记 v131072 = "2.0 update"（`README.md:4`），HookLink/HeyListen 只写 `update v131072, build e0f3ccfa`。**三套命名之间无对照表（未找到）**。一次官方更新的影响可量化："任何按地址 patch `main` 的东西只在一个 build 上有效"，函数位移 **−2.8 MB 到 +0.4 MB 不等，无统一换算系数**（`docs/game-versions.md:10-14`）；重建成本：1.0.0 符号包靠 `port_pack.py` 指令遮罩投票重建 55 条中的 53 条、余 2 条手工（`:79-99,102-109`）；违反纪律立刻坏：Mouse Grab 0.3.0 自己调 1.1.1 的 `ActorHolder` 地址，在 1.0.0 上崩溃（`:32-34`）；code cave 仅 **3,632 字节**且已被瓜分两处重叠（`docs/memory-map.md:14-25,66-83`）。
6. **重制版让"读内存"这条路线复活并换代**（F）：2022 年的 `ootr3d-tracker` 已经在读 Citra 宿主进程并走指针链（`main.rs:40`、`bit_shift(0x0587A0E)`），但**基址要用户手写进 `C:/Users/VGFreak/citra/address.txt`**（`citra_memory.rs:4-13`）；2026 年的 hyrule-tracker 把同一思路产品化到 6 种原生移植 + 2 种模拟器（`saves.ts:77,83-84`），500 ms 轮询、只请求读权限、失败回落存档。**范式 (b) 的重心已从 N64 模拟器迁到 Switch 2 重制版的 PC 移植**；反面证据同时存在：Rust `fenhl/oottracker`（存量最大的 (b) 派）停更于 2023-10-20，且其 README 记录了赛规对自动追踪的禁令。
7. **闭源商业侧的切换速度超过开源**（F）：MapGenie 的 OoT 条目**只剩 Switch 2 重制版卡片**（N64/3DS 老图标 "Under Construction"），Zelda Dungeon 在 11-17 已把导航拆成 NS2 / N64·3DS / MQ 三入口，官方 **Zelda Notes** 直接免费提供地图 + 陶笛 + 任务日志 + 排行榜，**Hall of Fallen Foes** 免费吸收练习模式。而社区的重制版周边（Ocarina-Atlas 2.3.1、OOT-SW2SE、Links-Toolbox、hyrule-tracker）也在 11-17/11-18 同批发版 —— **2026-11 是 OoT 周边工具 25 年来密度最高的一个月**（本批次 53 仓中 **21 个的 HEAD 提交日期落在 2026-11-10 至 11-18 这九天之内**（实测计数））。
8. **老前提被本次取证推翻两条**：① **Zelda Dungeon 没有 OoT 交互地图**（站内仅 BotW/TotK/EoW 有 `*-interactive-map`，OoT 路径 404）；② **Track-OOT 不是 Electron 桌面 App**，是 ES Module PWA（`src/` 205 个 `.js`、0 个 `.ts`，Electron 仅在 `editor/` 与 devDependencies）。另更正一条任务书表述：`oot-save-editor` 并非某个具体仓库名，实际存在的是 `AdmiralCurtiss/OoTSaveEditor`（2023 停更）、`calmez/oot-save-edit`（2026-03）、`Traiver/OOT-SW2SE`、`Cloowney/ZeldaOOTRemakeSaveEditor`、`fantasmagg/OoT3D-Editor`、`theanimater23/Oot3DS-Save-Editor-For-Azahar` 六个不同世代的实现。
9. **存档编辑层暴露两个工程隐患**（F）：① N64 校验和**模数分歧**（C# 2^16 自然回绕 vs TS `%0xFFFF`，`OoTSingleSave.cs:2627-2633` / `saveslot.ts:1088-1096`），无一方记录对拍；② **8 个编辑器无一做主槽↔备份槽同步**，只改主槽会被旧备份覆盖（唯一规避是 Links-Toolbox 的"写入所有位置"，`README.md:77`）。此外 **GC 记忆卡（GCI/ECC/shift2）、`.a1`、iQue 存档在全部 53 仓中均为"未找到"** —— N64 家族里 GC 与 iQue 两个平台在周边工具层是**真空**。
10. **音乐工具的依赖方向与直觉相反**（F）：`seq64` 本身只做 MIDI↔.mus↔.com（V2 **不做 bank**，`README.md:36`），补齐 bank 的 `isghj5` 两仓**以 ROM 绝对地址为采样主键**（OoT audiotable 基址 `0x3FA9E0/0x4006B0/0x41D760/0x427D30/0x4377E0`，`utils/Helpers.py:36-51`），而这些地址在 OoTR/反编译工程里同样硬编码 → **音乐工具链是反编译项目的下游**，其寿命受制于上游符号表的维护。同时六个陶笛类工具**无一建模真实孔位/超吹**，全部围绕游戏给的 5 个音按钮，且 N64 世代与 Switch 2 世代的中性音高相差整一个八度（D4–D5 vs D5–D6）、键位由 A+C 四向改为 L/R/A/Y/X —— 曲谱资产跨世代不可复用；官方加入麦克风哼唱识别后，`flipparina` 立刻提供"喇叭喂麦克风"路径 —— **官方每增加一种输入，社区就多一种自动化面**。
11. **工具之间几乎没有横向引用**（F）：Ocarina-Atlas 全文不提 noclip / Track-OOT / EmoTracker / MapGenie / OoTR；MapGenie 不提随机器；noclip 不提任何 tracker；EmoTracker 新仓不提 poptracker 也不提 OoT（"ocarina" 零命中）；两个 Lua pack 仓库互为镜像格式却互不声明兼容。**同一个游戏的多套周边工具已经分裂成互不可见的世代** —— 这与第 04 号报告"闭源侧无横向引用"的观察同构，是本报告认为**最值得做的一件事**：一个统一的 OoT 工具索引层（场景 ID / 物品 ID / 字段哈希三张跨世代对照表）目前**不存在（未找到）**。
12. **可维护性的分化在同源项目族里一眼可见**（F）：`oot3d_practice_menu` 与 `OoT3D_Randomizer/code/` 骨架同源（`patch.py:13` 逐字一致、`loader.c` 版权头相同、`nninitRegion=0x100028`），但 Randomizer 已抽象出 `PATCH/HOOK` 宏与自动符号 `NEWCODE_OFFSET/_LD_TEXT_START`（`code/src/asm/patches.s:3-11`、`code/src/ld_symbols.h:14-17`），practice menu 仍硬编码 `newcodeinfo.h:9-10`。**同一代工具里，抽象出符号层的活了 5 年还长出新枝，硬编码的停在 2022-01-02。** 2026 年的 KaeGae/"问名字不问地址"纪律（`oot-remake-modding/README.md`、`docs/game-versions.md:32-34` 的反例）是这条教训的重复上演。

---

## 附录 A：本轮克隆清单（53 仓，`/root/wsl-demo/oot-peripherals/`，`du` 合计 **2.9 GB**，其中 `OoT-Interactive-Map` 一个仓占 2.0 GB）

按最后提交日期倒序（HEAD 短哈希 / 日期 / LICENSE / 体积）：

```
HeyListen              e0b06cc 2026-11-18  GPL-3      74M   Sacrappy/HeyListen（重编译宿主）
OOT-SW2SE              ae88f36 2026-11-18  MIT       1.6M   Traiver/OOT-SW2SE（Switch 2 存档编辑器）
Ocarina-Atlas          fae037e 2026-11-17  AGPL-3     78M   antonioalvromero/Ocarina-Atlas
HookLink               2295235 2026-11-17  MIT       684K   dsi-hub/HookLink（iPhone/AOT）
flipparina             280ebb0 2026-11-17  MIT       484K   elpendor/flipparina（Flipper Zero）
oot-remake-modding     4967c1d 2026-11-17  MIT        24M   Kabosune/oot-remake-modding（L2 基座）
oot-remake-multiplayer 4d4ca8a 2026-11-17  GPL-3      22M   daurosk/oot-remake-multiplayer（Squadala）
Links-Toolbox          36ab599 2026-11-17  MIT       1.6M   losava/Links-Toolbox（mod loader+save ed）
hyrule-tracker         1fb2777 2026-11-17  MIT        12M   jv-prado/hyrule-tracker（范式 b）
Track-OOT              7692801 2026-11-16  MIT        89M   GameStateTracker/Track-OOT（Bitbucket 镜像）
ZeldaOOTRemakeSaveEditor d9a9314 2026-11-15 无 LICENSE 4.4M  Cloowney（仅 README，无源码）
NX2Navi-Zelda-OOT-Port 86ee12e 2026-11-15  32M       ZuanCrisp/NX2Navi-Zelda-OOT-Port
noclip.website         2cebac2 2026-11-14  MIT        83M   magcius/noclip.website
NX2navi-Trainer        2400543 2026-11-14  MIT       424K   Miragui/NX2navi-Trainer
harkinianpad           3a969e9 2026-11-13  无         3.6M  chrissotraidis/harkinianpad（iOS+SoH）
oot3d-tracker          676244f 2026-11-13  无        324K   LofiGhoul/oot3d-tracker（GTK4）
oot-remake-rando       2033b4c 2026-11-13  MIT       1.4M   sgurgurich/oot-remake-rando
OoTROcarinaMapper      878d400 2026-11-11  无        232K   indigojoha/OoTROcarinaMapper
poptracker             6052adc 2026-11-11  GPL-3      22M   black-sliver/poptracker（引擎）
zelda-ocarina-button-rockstar 02bad96 2026-11-10 MIT  448K  AlexanderCato
OoT3D-Editor           966287f 2026-11-10  GPL-3     3.8M   fantasmagg/OoT3D-Editor（3DS 本机 Lua）
ocarina-songbook       2996904 2026-10-16  无         28M   ckarnell/ocarina-songbook
EmoTracker             7fa259c 2026-10-16  MIT        23M   EmoTracker-Community/EmoTracker（含 MCP）
Oot3DS-Save-Editor-For-Azahar ed23757 2026-10-12 MIT  2.6M  theanimater23
OoTMMR_tracker_pack    22eff4f 2026-09-06  无         36M   jupiter0fire（README 自declare弃用）
ootr-tracking-map      e6071e3 2026-07-02  无        9.0M   rafaell22（Electron + pocketsphinx 语音）
ocarina                47660e4 2026-06-24  MIT       5.7M   vaexenc/ocarina（React）
oot-tracker-lua        81376de 2026-05-23  MIT        96M   StripesOO7/oot-tracker（AP datapack，含 .xcf）
oot-save-edit          c986a1e 2026-03-19  CC0-1.0   2.7M   calmez/oot-save-edit（TS/Deno）
tootr                  4b64d93 2026-01-24  MIT        19M   mracsys/tootr（ER tracker）
z64-bank-converter     8b71c30 2026-01-14  GPL-3     488K   isghj5（archived）
OcarinaPlayer          0bf2361 2025-11-19  MIT        22M   samuelyuan（Go/Ebiten，含实音）
OoTMM-Spoiler-To-Emotracker 2722e31 2025-11-08 MIT   113M   KO0IAID（spoiler→pack 转换器）
seq64                  47500ab 2025-10-26  GPL-3     1.1M   sauraen/seq64
z64-sample-creation-tools 65b122a 2025-07-24 GPL-3   4.3M   isghj5（archived）
Ivan                   cc875fc 2025-04-21  MIT(变体)  31M   L-P/Ivan（Go/Ebitengine，undo+input viewer）
NaviTrack              56586e4 2025-04-07  CC0-1.0   912K   lkarvec（Next.js，alpha）
iRandoTracker          2cc4e6c 2025-03-28  MIT       1.5M   dukevantreta（macOS SwiftUI）
OoT-Interactive-Map    020dab1b 2025-02-21 MIT       2.0G   Ecksters（Leaflet + 1.2 GB 瓦片）
deains-oot-tracker     e71ae69 2025-02-16  MIT       1.2M   deains/oot-tracker（反 logic 立场）
OoTSaveEditor          25ccfd5 2023-11-17  MIT       472K   AdmiralCurtiss（C#/.NET）
oottracker             b08a60e 2023-10-20  无        6.1M   fenhl/oottracker（Rust，自动追踪）
ootr_map_tracker       a1aebe8 2023-08-10  无         13M   cheuer（pack，map_enabled=false）
ootm                   bc45c2e 2023-08-10  Unlicense  43M   SiriusGG/ootm（Java 传送图）
EmoTrackerPacks        279e8c5 2023-05-18  无         48M   Hamsda（OoTR map pack）
HoodTracker            ccba251 2022-12-16  MIT       1.8M   hoodedpaladin（import OoTR 作求解器）
SM64MidiCompanion      1e63313 2022-08-02  无        488K   Bitlytic（调 seq64_console）
OoT-randomizer-tracker 64a5cd9 2022-05-17  GPL-3     1.6M   fgil90（极简）
ootr3d-tracker         f5a6b4b 2022-05-15  无        480K   vgfreak95（Rust，读 Citra 进程）
SceneNavi              d0813a3 2021-03-12  MIT       1.6M   xdanieldzd/SceneNavi（**N64 关卡编辑器**）
sbc                    f06cee2 2021-03-08  MIT       212K   matthewcpp（Go 重写 SGI sbc）
oot-tracker            4df1149 2020-05-19  MIT       1.9M   TestRunnerSRL（logic 停在 4.0）
OoT_MapExt             5687c7f 2019-10-01  无        216K   CMuncey（126 行 C，切 .zscene/.zmap）
```

**许可分布**（实测 53 仓，按 LICENSE 正文判定）：有 LICENSE 文件 40 仓 —— **MIT 28**（其中 `Ivan/LICENSE` 是缺标题行的 MIT 全文）**/ GPL 8 / AGPL-3 1 / CC0 2 / Unlicense 1（`ootm`，"free and unencumbered…public domain"）**；**无 LICENSE 13 仓**（ZeldaOOTRemakeSaveEditor、harkinianpad、oot3d-tracker、OoTROcarinaMapper、ocarina-songbook、OoTMMR_tracker_pack、ootr-tracking-map、oottracker、ootr_map_tracker、SM64MidiCompanion、ootr3d-tracker、EmoTrackerPacks、OoT_MapExt）—— 按默认规则即"保留所有权利"，**周边工具生态近四分之一处于无授权状态**，这本身是个结构性风险。

## 附录 B：明确的"未找到"清单

| 检索项 | 结论 |
|---|---|
| GC memory card 存档格式（GCI 目录 / 块 ECC / shift2） | 53 仓**全无**实现 → GC 卡不可编辑 |
| N64 `.a1`（ExpansionPak）/ `.eep` / `.gbx` 存档 | **未找到**（`oot-save-edit` 只按大小分派 EEPROM/MEMPAC/SRAM/FLASHRAM） |
| iQue（CN 版）专用存档工具 | **未找到**（C#/TS 的字节序翻转是最接近旁证） |
| N64 存档 heap 区 / `DEMO` 标志解析 | **未找到**（8 编辑器均止步 0x1450） |
| 主槽 ↔ 备份槽同步 | 8 编辑器**无一实现**（仅 Links-Toolbox "Save to…" 间接规避） |
| OoTR 种子 → Ocarina-Atlas 导入 / 动态 hint 表 / IndexedDB / undo 栈 | **未找到**（Atlas 是存档+内存追踪器，不是随机器 tracker） |
| 结构化（可导入 tracker 的）重制版随机器 spoiler | **未找到**（`oot-remake-rando` 只出纯文本 `-spoiler.txt`） |
| N64 坐标 ↔ 重制版 GameData `+0x1c/+0x20` 的映射表 | **未找到**（数字巧合但语义不同，见 §7.2） |
| v131072 / 1.1.1 / "2.0 update" / build e0f3ccfa 的官方命名对照表 | **未找到** |
| noclip 的 credits UI（README:3 声称"see the application"） | **未找到**（无 credits 文件，署名只在源码注释） |
| noclip 的 minimap / overworld 拼装逻辑 | **未找到** |
| `?webgpu` 之外的快捷键（T / 1-9 / `.` / `,` / F4）在代码中的实现 | **未找到**（仅 `main.ts:488` 的 URL 查询串可证） |
| Zelda Dungeon 的 OoT 交互式地图 | **不存在**（404；站内仅 BotW/TotK/EoW 有 `*-interactive-map`） |
| `Bitlytic/LmmsToM64` 本地仓库 | **未找到**（仅存在于 GitHub 检索结果） |
| EmoTracker 迁往 Community org 的动机说明 / 与 poptracker 的分工声明 | **未找到**（README、CLAUDE、docs 均无） |
| HeyListen / NX2navi 的对外 RPC 或 debug server | **未找到**（只有自进程页观测与游戏内 F9 菜单） |
| 官方 modding API（Switch 2 重制版） | **不存在**（`oot-remake-modding/README.md:141` 自称 unofficial fan project） |
| 针对 OoT tracker / map App 的 Apple App Store 下架记录 | **未找到** |
| 跨世代统一的 OoT 工具索引层（场景 ID / 物品 ID / 字段哈希对照） | **不存在** |

## 附录 C：方法与可复核性

* 全部 53 仓为 `git clone --depth 1`，浅历史导致**作者时间线只能取 HEAD 提交日期**，早期演化不可考（已在各表标注）。
* 本次**未编译、未运行**任何第三方仓库代码，全部结论来自源码/文档/资产清单的静态阅读与 `git ls-files`/`du`/`find` 统计；`path:line` 由本次实读给出，可按 `cd /root/wsl-demo/oot-peripherals/<repo>` 复核。
* 闭源侧（MapGenie、Zelda Dungeon、OoTR 站点、The Last Tracker、iOS App Store、EmoTracker wiki、官方 Zelda Notes/GameSpot）取自站点正文与页面结构（如 `.maplibregl-ctrl-*` 类名），标记为 F 或 R；**未使用任何非公开接口，未获取未发售标题二进制，未复现 DRM/在线校验绕过**。
* 作弊/trainer 类（NX2navi-Trainer、Gameshark 码）只描述其**架构层级与依赖面**，不作为使用建议。
* 两处对既有认知的主动更正已在正文标注：Track-OOT 非 Electron（§3.2）、Zelda Dungeon 无 OoT 交互地图（§2.4/§9.8）；一处对上游 agent 结论的推翻（noclip 具备 N64 3D 通道，§2.1/§2.1.2）。
