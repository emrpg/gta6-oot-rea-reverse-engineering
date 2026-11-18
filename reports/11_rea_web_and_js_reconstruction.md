# 基于 REA 的 GTA 6 与《塞尔达传说：时之笛》Web/JS 应用图谱逆向与源码重构报告

> **工单关联**：`emrpg/gta6-oot-rea-reverse-engineering#3`（`[REA-Web-And-JS]`）
> **分析工具**：`/usr/local/bin/rea`（REA 6.9.0）— `analyze-javascript-application`、`recover-javascript-sources`、`trace-application-feature`、`compare-source-to-bundle`、`compare-static-and-observed-endpoints`、`inspect-web-network-capture`、`import-reference-source`
> **证据归档**：`/root/wsl-demo/rea-reconstructions/web-and-js/evidence/`（26 份：22 JSON + 4 YAML 视图摘要）
> **还原产出**：`/root/wsl-demo/rea-reconstructions/web-and-js/reconstructions/`
> **本报告取代**：同一工单下先前提交的 63 行版本；差异与逐条核验见第九章。

---

## 〇、结论摘要

1. **线上闭源 GTA 6 地图 `map.stateofleonida.net` 与工单给出的 `vimap.saamexe.com` 是同一应用**：`https://vimap.saamexe.com/` 返回 `HTTP/2 308`，`location: https://map.stateofleonida.net/`（`live/vimap_headers.txt`）。因此"对 vimap 逆向"与"对 stateofleonida 逆向"是同一目标，而非两个站点。
2. **`rea analyze-web-bundle` 在本机不可用**（无浏览器/CDP 端点，且未安装浏览器）。采用等效管线：公开静态资源 `GET` → `analyze-javascript-application` → `recover-javascript-sources`（Wakaru 1.14.0）→ 对还原产物二次建图。该替代路径的每一步与差距在 §1.2 声明。
3. `solmain2809_4.js`（2,928,855 B，单行 0 换行）建图得到 **131 个 `endpoint` 节点 / 125 个不同字面量 / 25 个 `storage` 节点 / 30 个 `javascript-module` 节点**，`unknown` 节点为 0。
4. 后端契约被完整枚举：**`auth/api.php` 的 36 个 `action`（23 个 `admin_*` + 13 个公开）**、**9 个 PHP 脚本**、**17 个 `data/*.json` 数据集**、**24 个外部主机**、`media-admin.php` 的 2 个 action。
5. 其中 **5 个 PHP 脚本、3 个 action、9 个 `data/*.json` 只存在于混淆字符串数组中，未形成 REA 端点观测**——它们由字符串池正则（"推断"级）补出，逐条标注见 §3.4。
6. **反混淆语义保真已量化**：原始与还原两份图的 125 个字面量 = **97 个逐字节相同 + 28 个仅形式不同（成员访问/模板字符串改写）+ 0 个未匹配**（`notes/leonida_endpoint_literal_stability.json`）。还原产物由 0 换行变为 **76,501 行**并新增 `source-map`（1,704,728 B）与 `source-module` 节点；原本动态拼接的 `data/maps/${id}.json` 还原后才成为可寻址模板字面量。
7. `gta6-leonida-atlas` 坐标管线以**真实标识符**还原：`CANONICAL_BOUNDS`（west −16000 / east 4000 / south −8000 / north 12000）、`WORLD_METRES_PER_GTADB_UNIT = 2`、`canonicalToWorld`、`worldToGtadb`、`worldToMap`/`mapToWorld`、`WORLD_BOUNDS`/`MAP_BOUNDS`，来源 `src/features/street-leonida/leonida-coordinates.ts`（与本地副本 `local/leonida-source-coords/leonida-coordinates.ts:26-112` 逐字节一致）。
8. **先前版本的 `worldToSvg` / `svgToWorld` / `SVG_BOUNDS_XZ` / `build3dExplorerUrl` 是自造符号**：在 bundle、还原产物与源码中均为 0 命中；且把 `canonicalToWorld`（世界坐标）与 `worldToGtadb`（gtadb.org 单位逆映射）误当作 SVG 正逆变换对。真实等价物与差异见第九章表 9-1。
9. `Ocarina-Atlas` 确为 **Electron + TanStack/Vite Web + C# + C 桥**的复合工程；本次还原其 **49 项 Electron 暴露面**（1 BrowserWindow、1 contextBridge、30 `ipc-channel`、16 `ipc-handler`、1 Service Worker）与 14 个 `ipcMain.handle` 的行号级位置。
10. **`tootr` 不是"WebGL/React 场景与过场查看器"**：证据显示它是《时之笛》随机化器追踪器（`@mracsys/randomizer-graph-tool`、`src/components/Tracker.tsx`、`<title>TOoTR</title>`），图中无 WebGL 证据；`noclip.website` 的 8 个 `oot3d/*` 场景路径经复核为真（bundle 与仓库两侧均命中）。
11. 三次 REA 调用以固定预算/内存约束失败并如实登记（§3.7、§4.4、§6.7），未以绕过手段重试。
12. 生产包 `leonida-coordinates-CXGuqbYF.js`（586 B）与 `public-path-DoqNwZJ4.js`（300 B）**标识符全部最小化**：真实符号只能按结构配对复原，且尾部导出别名交叉（`export{i as a,r as i,l as n,a as o,c as r,e as t}`），按名字检索会错配；生产构建还把 `publicPath` 的默认基路径内联成字面量 `` `/gta6-leonida-atlas/` ``（源码默认值是 `import.meta.env?.BASE_URL ?? '/'`）。逐条对照见 §4.5 与 `reconstructions/17_minified_coordinate_chunk_symbol_map.md`。
13. 全程**未调用任何后端 API、未尝试鉴权与绕过**；`https://gtaweb.eu/` 返回 `403` 即停止（仅记录，见 §1.4）；7 个公开静态资产二次抓取 **7/7 摘要一致**（`reconstructions/12_input_digest_ledger.md`）。

---

## 一、范围、方法与边界声明

### 1.1 REA 调用矩阵

| REA 操作 | 作用对象 | 产出证据 | 状态 |
| :--- | :--- | :--- | :--- |
| `analyze-javascript-application` | 本地源码树 ×6：`Ocarina-Atlas`、`gta6-leonida-atlas`、`npwd`、`txAdmin`、`tootr`、`noclip.website` | `evidence/{Ocarina-Atlas,gta6-leonida-atlas,npwd,txAdmin,tootr,noclip-website}__app.json` | 完成 |
| `analyze-javascript-application` | 线上产物 ×2：`bundles/stateofleonida/`、`bundles/gta6-leonida-atlas/` | `stateofleonida__app.json`、`gta6-leonida-atlas-bundle__app.json` | 完成 |
| `analyze-javascript-application` | 作用域子图 ×2：`local/leonida-source-coords/`、`local/txadmin-router/` | `leonida_source_coords__app.json`（`ev_32825cf5…`）、`txadmin_router_scoped__app.json`（`ev_868fb827…`） | 完成 |
| `recover-javascript-sources` | 线上 Bundle ×4（`solmain2809_4.js`、`index-CHfdltUy.js`、`8.15df99b7.js`、`index-B34Su3Ax.js`） | `evidence/*__recover.json` + `recovered/<name>/modules/` | 完成（`stateofleonida` 首跑因输出目录残留 `rc=1`，重跑成功） |
| `analyze-javascript-application`（二次建图） | 还原产物 ×4 | `{stateofleonida,tootr,noclip-website,gta6-leonida-atlas}_recovered__app.json` | 完成 |
| `trace-application-feature` | 还原后的 atlas 坐标种子节点 | `notes/trace_leonida_coordinates_incoming.json`（`ev_cac3e667…`） | 完成（覆盖 partial） |
| `trace-application-feature` | `Ocarina-Atlas` 分数坐标种子 | — | **失败**：内存约束（§6.7） |
| `import-reference-source` | `gta6-leonida-atlas` 源码参考树 | `notes/leonida_source_graph.json`（HistoricalSourceGraph，167 源文件） | 完成 |
| `compare-source-to-bundle` | 参考源码 ↔ 还原 bundle 图 | `notes/compare_gta6_src_to_bundle.json`（`ev_7abd6455…`） | 完成，结论为合法"未知"（§4.4） |
| `compare-static-and-observed-endpoints` | txAdmin 作用域图 ↔ HAR 观测 | `evidence/endpoint_comparison_txadmin_vs_observed.json` | 完成 |
| `compare-static-and-observed-endpoints` | stateofleonida 图 ↔ HAR 观测 | — | **失败**：投影字节预算（§3.7） |
| `inspect_web_network_capture` | `live/observed_assets.har` | `evidence/observed_assets__har.json`（父 `ev_a5343e0e…`，视图 `ev_4727ec49…`） | 完成 |

### 1.2 `analyze-web-bundle` 不可用与替代路径

本机无可用浏览器与 CDP 端点，`rea analyze-web-bundle` 所依赖的运行时抓取不可执行。处理方式：

- **保留静态可证部分**：直接对公开静态资源做 `GET`（`live/observed.tsv`，13 笔实测事务；`live/observed_assets.har`，15 条目，SHA-256 `6dc35d53…`），将落地文件作为 `analyze-javascript-application` 的 subject。
- **不假装运行时证据**：因此本报告不含"实际请求过的后端接口响应"。所有接口契约均为**字面量/调用点级静态结论**，authority 为 `ast-static-analysis`。
- **差距声明**：无法观测运行时动态注入的 URL、延迟 `import()` 的真实解析结果、以及服务端根据参数返回的分支；这些在第十章列为未知。

### 1.3 证据分级

| 级别 | 含义 | 本报告标识 |
| :--- | :--- | :--- |
| 观察 | REA 证据记录的节点/观测（含 `authority`、`confidence`、`location`） | `[观察]` |
| 推断 | 由字符串池正则、源码 `grep`、包元数据等非 REA 手段推出，或 REA 节点标签与包清单联合解释 | `[推断]` |
| 未知 | REA 显式返回 `unknown` / `unavailable`，或工具预算拒绝产出 | `[未知]` |

关键约束：对 `storage` 节点，当 `properties.name` 为空时，节点 `identity.key`/`label` 只是回显机制名（如 `local-storage`、`call:localStorage.setItem`），**不得**当作字面键名报告。因此 CSV `02` 的 `key_or_dynamic` 列只取 `properties.name`，其余记为 `(dynamic)`。

### 1.4 安全与合规边界

- 仅请求静态资产与页面 HTML（`GET`）。**未调用** `auth/api.php`、`media-admin.php`、`auth/login.php`、`image-proxy.php`、`proxy.php`、`save-changelog.php`、`get-media-files.php` 中任何一个；也未构造任何管理动作请求。
- `https://gtaweb.eu/` → `403`（5,346 B，`text/html`）：**未尝试**任何 UA/头部伪装或重试绕过，仅记录为拒绝。
- 落地一致性核验：对 7 个公开静态产物做二次抓取并比对 SHA-256，**全部逐字节一致**
  （`solmain2809_4.js 0f5ab96c788a…`、`solmain2809_2.css c49fe66e7cd6…`、
  `8.15df99b7.js ef73ed494db8…`、`index-CHfdltUy.js f1e0ea9556b4…`、
  `index-B34Su3Ax.js d81d24b7da29…`、`leonida-coordinates-CXGuqbYF.js c65bc30cb288…`、
  `public-path-DoqNwZJ4.js de888f99d993…`；前缀为 SHA-256 前 12 位，同一 SHA-256 亦记录于
  `evidence/*/identity` 与 `reconstructions/12_input_digest_ledger.md`）。
  这说明服务端内容在抓取窗口内稳定，且本报告分析的就是公开分发的字节。

---

## 二、图谱总表（14 份建图证据）

| # | Subject（REA `subject.local_path`） | 类型 | 解析 JS/TS | 相关文件 | 读取字节 | AST 节点 | 图节点 | endpoint | storage | route 模式 | Findings | 解析失败 |
| :--- | :--- | :--- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 1 | `bundles/stateofleonida` | 线上混淆 Bundle | 1 | 1 | 2,928,855 | 381,100 | 188 | 131 | 25 | 0 | 264 | 0 |
| 2 | `recovered/stateofleonida/modules` | 还原后 | 1 | 2 | 4,934,429 | 379,424 | 190 | 131 | 25 | 0 | 264 | 0 |
| 3 | `bundles/gta6-leonida-atlas` | 线上生产 Bundle | 5 | 6 | 602,622 | 5,665 | 48 | 0 | 0 | 0 | 35 | 0 |
| 4 | `recovered/gta6-leonida-atlas/modules` | 还原后 | 5 | 12 | 2,234,748 | 5,817 | 60 | 0 | 0 | 0 | 35 | 0 |
| 5 | `gta-projects/gta6-leonida-atlas` | 本地源码树 | 159 | 165 | 4,364,557 | 247,381 | 1,290 | 15 | 3 | 0 | 1,735 | 0 |
| 6 | `local/leonida-source-coords` | 作用域子图 | 2 | 2 | 13,487 | 1,625 | 38 | 0 | 0 | 0 | 50 | 0 |
| 7 | `oot-peripherals/Ocarina-Atlas` | 本地源码树 | 72 | 80 | 1,834,691 | 128,283 | 1,216 | 10 | 4 | 4 | 1,675 | 0 |
| 8 | `oot-peripherals/tootr` | 本地源码树 | 57 | 63 | 737,885 | 60,715 | 1,102 | 0 | 30 | 0 | 1,264 | 1 |
| 9 | `recovered/tootr/modules` | 还原后 | 0 | 2 | 14,879,069 | 0 | 5 | 0 | 0 | 0 | 0 | 0 |
| 10 | `oot-peripherals/noclip.website` | 本地源码树 | 933 | 964 | 34,937,823 | 2,647,300 | 3,085 | 1 | 4 | 0 | 10,072 | 4 |
| 11 | `recovered/noclip-website/modules` | 还原后 | 98 | 198 | 22,721,131 | 415,090 | 992 | 2 | 0 | 0 | 1,490 | 0 |
| 12 | `gta-peripherals/tools-and-apps/npwd` | 本地源码树 | 602 | 656 | 1,429,430 | 126,879 | 7,141 | 22 | 12 | 10 | 7,235 | 0 |
| 13 | `gta-repos/txAdmin` | 本地源码树 | 543 | 600 | 4,454,277 | 401,364 | 3,620 | 92 | 0 | 0 | 5,092 | 7 |
| 14 | `local/txadmin-router` | 作用域子图 | 1 | 1 | 6,148 | 702 | 70 | 56 | 0 | 0 | 72 | 0 |

> 注：**#8/#10 的 subject 是本地仓库工作树**（`/root/wsl-demo/oot-peripherals/…`），不是线上抓取；线上抓取的 `tootr`/`noclip.website` Bundle 分别以 #9/#11 的还原输入形式存在（见 §7）。先前 63 行版本把这两行写成"线上"，属主体标注错位。

`stateofleonida` 原始 vs 还原的节点类型差集 `[观察]`：

| 类型 | 原始 | 还原后 |
| :--- | ---: | ---: |
| `endpoint` | 131 | 131 |
| `javascript-module` | 30 | 30 |
| `storage` | 25 | 25 |
| `javascript-asset` | 1 | 1 |
| `artifact` | 1 | 1 |
| `source-map` | 0 | **1** |
| `source-module` | 0 | **1** |
| `unknown` | 0 | 0 |

---

## 三、GTA 6 线上闭源地图 `stateofleonida`（`solmain2809_4.js`）

### 3.1 产物身份与获取路径

| 项 | 值 | 来源 |
| :--- | :--- | :--- |
| 入口跳转 | `https://vimap.saamexe.com/` → `308` → `https://map.stateofleonida.net/`，`server: cloudflare` | `live/vimap_headers.txt` |
| 主 Bundle | `https://map.stateofleonida.net/solmain2809_4.js`，`200`，2,928,855 B，`text/javascript`，耗时 3.976766 s | `live/observed.tsv:2` |
| 样式 | `solmain2809_2.css`，792,112 B | `live/observed.tsv:3` |
| 页面 | `/leonida.html` → `404`（1,044 B）；站点根 HTML 为 19,064 B，与 `bundles/stateofleonida/leonida.html` 逐字节一致 | `live/observed.tsv:1`、`bundles/leonida_root_index.html` |
| REA `root_artifact_sha256` | `f62c7c0d7c384d6ac3f0b13101277f134989e01cdd1e8d35af264602906e779f` | `evidence/stateofleonida__summary.json` |
| 原始文件 SHA-256 | `0f5ab96c…` | `sha256sum bundles/stateofleonida/solmain2809_4.js` |

> 两者不同是**正常**的：`root_artifact_sha256` 是 REA 的规范化产物标识，不等同原始字节摘要。先前版本把 `f62c7c0d…` 当作了文件 SHA-256，属于概念混用。

技术栈 `[推断]`：Leaflet + three.js r128 + `javascript-obfuscator` 字符串数组（大量 `_0x…` 标识符与 `obj['method']()` 形式成员访问）。

### 3.2 `[观察]` REA 静态图核心指标

`evidence/stateofleonida__app.json`（`ev_f5951034…`）：188 节点 / 131 `endpoint` / 125 去重字面量 / 25 `storage` / 30 `javascript-module` / 264 findings / AST 381,100 / `unknown` 节点 0。还原态图（`ev_5a75e525…`）给出**完全相同的 131/125** 端点计数，因此逐端点导出表 `reconstructions/01_stateofleonida_endpoints.csv`（153 行观测、125 个不同字面量）取自**还原态图**——理由只有一个是行号可寻址：原始 Bundle 为 0 换行单行文件，其 `location.start.line` 无意义。列含 `mechanism`、`resolution`、`authority`、`source:line`（`source` = `bundle.js`，行号指向 `recovered/stateofleonida/modules/bundle.js`）。原始图的定位信息并非缺失而是粒度退化：`location.value.source = solmain2809_4.js`、`line` 恒为 `1`，仅靠 `column`（如 `2,319,201–2,319,364`）区分位置；还原后同一类观测变为 `line 2,065–2,067` 这样的可跳转范围，这是还原的实际取证价值。

### 3.3 隐藏路由与 API 数据结构矩阵

字符串池清单：`reconstructions/16_stateofleonida_string_pool.csv`（192 行、191 个字面量），脚本 `notes/string_pool.py`。列 `contained_in_rea_endpoint_observations` 表示该字面量是否被 REA 端点观测覆盖。

**（a）`auth/api.php` 的 36 个 `action`**（`[观察]` 33 个 + `[推断]` 3 个）：

| 组 | 数量 | action |
| :--- | ---: | :--- |
| 公开/玩家 | 13 | `access_config`、`access_verify`、`game_stats`、`gtadb_source_public`、`leaderboard`、`marker_delete`、`marker_list`、`marker_save`、`marker_update`、`notifications`、`notifications_read`、`ranks`、`site_settings_public` |
| 隐藏管理 | 23 | `admin_community_access_save`、`admin_credits`、`admin_credits_save`、`admin_db_export`、`admin_db_integrity`、`admin_db_overview`、`admin_gtadb_save`、`admin_gtadb_scheduled`、`admin_gtadb_status`、`admin_gtadb_sync`、`admin_gtadb_sync_status`、`admin_news`、`admin_news_save`、`admin_ranks`、`admin_seo`、`admin_seo_save`、`admin_site_settings`、`admin_site_settings_save`、`admin_stats`、`admin_user_delete`、`admin_user_details`、`admin_user_set_rank`、`admin_users` |

其中 `access_config`、`access_verify`、`admin_db_export` 三个仅出现在字符串池，未构成 REA 端点观测 `[推断]`。

**（b）9 个 PHP 后端脚本**：

| 脚本 | REA 端点观测 | 判读 |
| :--- | :---: | :--- |
| `auth/api.php` | 是 | 主契约入口，承载 36 action |
| `get-media-files.php` | 是 | 媒体列表（`?type=`） |
| `media-admin.php` | 是 | 媒体管理，2 个 action：`config`、`upload-to-folder` |
| `auth/community-api.php` | **否** | 字符串池命中，调用点被封装 → `[推断]` 存在第二套社区 API |
| `auth/login.php` | **否** | `[推断]` 鉴权入口（未观测到 fetch 调用点） |
| `image-proxy.php` | **否** | `[推断]` 外链图片代理 |
| `proxy.php` | **否** | `[推断]` 通用代理 |
| `save-changelog.php` | **否** | `[推断]` 变更日志写入 |

**（c）17 个 `data/*` 数据集**：`backgrounds`、`content`、`credits`、`database_changelog`、`el_screenshots`、`lang`、`leaks`、`markers`、`news`、`screenshots`、`seo`、`supporters`、`trailer1`、`trailer2`、`trailer3`、`wiki-cache` 共 16 个静态路径，外加**模板化** `data/maps/${id}.json`（`recovered/.../bundle.js:596`，`resolution: partial`）为第 17 个。其中 9 个仅存在于字符串池 `[推断]`。
> 更正：`data/en-emojibase-data.json` **不存在**。真实字面量是 `assets/libs/emoji-picker-element-data/en-emojibase-data.json`（在 `document.baseURI` 解析处命中），前缀与集合归属均与前版本不同。

**（d）24 个外部主机** `[推断]`（字符串池，无一形成 REA 端点观测，因为它们是媒体/链接常量而非 fetch 调用点）：`map.gtadb.org`、`gta.wiki`、`earth.google.com`、`www.rockstargames.com`、`www.take2games.com`、`drevix.de`、`discord.gg`、`gtaforums.com`、`ko-fi.com`、`instagram.com`、`x.com`、`tiktok.com`、`twitch.tv`、`open.spotify.com`、`player.vimeo.com`、`www.youtube.com`、`youtube.com`、`www.youtube-nocookie.com`、`img.youtube.com`、`via.placeholder.com`、`pagead2.googlesyndication.com`、`www.w3.org`、`github.com`、`map.stateofleonida.net`。
未发现任何 `ws://`/`wss://` 字面量；但图中存在 `call:WebSocket` 机制观测 `[观察]`，说明连接 URL 为动态构造 → 端点 `[未知]`。

### 3.4 `[观察]` 存储键面

`reconstructions/02_stateofleonida_storage_keys.csv`：82 行观测 = **44 个字面键行 + 38 个 `(dynamic)` 行**，去重后 **17 个字面键**；机制分布 `local-storage` 64 / `session-storage` 18。字面键：`app_language`、`bgEffects`、`clusterCustomColors`、`dismissedUpdateBanners`、`external_media_session`、`ftm_last_share_ts`、`gtaCountdownEnabled`、`isPreloaderStyle`、`mapSettings`、`mapStudio.sidebarCollapsed`、`markerCustomColor`、`markerMotionEnabled`、`screenshotChallenge_stats`、`selectedBackground`、`selectedBackgroundColor`、`selectedBackgroundFill`、`selectedBackgroundPath`。

### 3.5 反混淆还原与语义保真证明

| 指标 | 原始 Bundle | 还原产物 |
| :--- | ---: | ---: |
| 文件 | `bundles/stateofleonida/solmain2809_4.js` | `recovered/stateofleonida/modules/bundle.js` |
| 字节 | 2,928,855 | 3,229,701 |
| 换行数（可寻址行） | **0**（单行） | **76,501** |
| Source Map | 无 | `bundle.js.map` 1,704,728 B |
| REA 图节点 | 188 | 190（新增 `source-map`、`source-module`） |
| endpoint 节点 / 字面量 | 131 / 125 | 131 / 125 |

**保真结论**：125 个字面量 **97 逐字节相同 + 28 形式改写 + 0 未匹配**（`notes/leonida_endpoint_literal_stability.json`；归一化规则：`x['y']→x.y`、模板字符串→字符串拼接、去引号/`+`/空白）。28 个形式改写全部是 `javascript-obfuscator` 的成员访问与拼接形式差异，例如
`'auth/api.php?action=admin_users&'+_0x1f4e74` → `` `auth/api.php?action=admin_users&${_0x1f4e74}` ``，`AUTH_CONFIG['api']['baseUrl']+'?action=bookmark'` → `` `${AUTH_CONFIG.api.baseUrl}?action=bookmark` ``。混淆变量名（`_0x1877ef` 等）在还原产物中保留，故未发生标识符漂移。
**可寻址性提升**：`'data/maps/'+_0x1877ef+'.json'`（原始，`resolution: partial`）在还原后成为模板字面量 `data/maps/${_0x1877ef}.json`，且字符串池 `data/*.json` 计数由 16 → 17。

工具契约记录：Wakaru 1.14.0 需 `REA_WAKARU_COMMAND=/root/.local/share/rea-tools/node_modules/@wakaru/cli-linux-x64/wakaru`，且必须 `prlimit` 包裹（不支持 timeout 覆写、附加路径 ≤16）；`recover-javascript-sources` 拒绝复用已存在的输出根目录（首跑 `rc=1` 即由残留目录导致，非引擎失败）。

### 3.7 失败登记：`compare-static-and-observed-endpoints`（stateofleonida）

请求 `notes/endpoint_compare_request.json` 以 `stateofleonida__app.json` 为 `application`、HAR 内联记录为 `capture` 时，REA 返回固定投影预算约束：`resource_constraint`，`output_bytes = 100,663,296`，`retryable: false`。按契约未调参重试，仅登记 `[未知]`：该目标的"静态字面量 ↔ 实际观测请求"差集未由 REA 产出（txAdmin 作用域版本成功，见 §5.2）。

---

## 四、GTA 6 Leonida 地图与 3D 街景：`gta6-leonida-atlas`（0.8.2，AGPL）

### 4.1 两侧建图

- **本地源码树**（#5）：159 解析 / 165 相关 / 4,364,557 B / AST 247,381 / 1,290 节点 / 15 `endpoint` / 3 `storage` / 1,735 findings。端点导出 `reconstructions/08_gta6_leonida_atlas_endpoints.csv`（20 行观测、15 个不同字面量），含查询键 `view`、`place`、`page`、`layer`、`preferences`、`x`、`y`、`z`、`t`，以及地标数据源 `https://raw.githubusercontent.com/rolux/gtadb.org/5c1d69ff…/map/data/6/landmarks.json`。
- **线上生产 Bundle**（#3）：5 解析 / 6 相关 / 602,622 B / 48 节点 / **0 endpoint** / 35 findings；6 个相关文件为 `index-B34Su3Ax.js`(579,650 B)、`leonida-coordinates-CXGuqbYF.js`(586 B)、`gtadb-mx_UyCIg.js`(3,150 B)、`leonida-evidence-BTSqoURv.js`(10,152 B)、`jsx-runtime-DyZJNswf.js`(8,784 B)、`public-path-DoqNwZJ4.js`(300 B)；其中 5 个被 REA 解析。

### 4.2 `[观察]` 坐标变换管线（真实标识符）

来源 `local/leonida-source-coords/leonida-coordinates.ts`，与仓库真实文件 `gta-projects/gta6-leonida-atlas/src/features/street-leonida/leonida-coordinates.ts` 经 `diff` 校验逐字节一致（注意目录是 `features/street-leonida`，不是 `features/project`），子图 `evidence/leonida_source_coords__app.json`（`ev_32825cf5…`）。

| 符号 | 位置 | 定义 |
| :--- | :--- | :--- |
| `WORLD_METRES_PER_GTADB_UNIT` | `:26` | `2`（每 gtadb.org 单位 = 2 世界米） |
| `CANONICAL_BOUNDS` | `:28-33` | `west −16000 / east 4000 / south −8000 / north 12000`（跨距 **20,000 × 20,000** canonical 单位） |
| `gtadbToCanonical` | `:35-38` | `{x,y}` 或 `[x,y]` → `{east,north}` |
| `canonicalToWorld` | `:40-45` | `{x: east*2, z: north*-2}` |
| `gtadbToWorld` | `:47-49` | `canonicalToWorld(gtadbToCanonical(p))` |
| `worldToGtadb` | `:51-56` | `{x: x/2, y: z/-2}`（`canonicalToWorld` 的逆） |
| `worldToMap` | `:58-60` | `{x: point.x, y: point.z}`（SVG 地图 Y 与 three.js 世界 Z 同值，见 `:17` 注释） |
| `mapToWorld` | `:62-64` | `{x: point.x, z: point.y}`（`worldToMap` 的逆，**无缩放**） |
| `WORLD_BOUNDS` / `MAP_BOUNDS` | `:92` / `:101` | 由 canonical × 2 得出：`−32000…8000`，`−24000…16000`，跨度 `40,000 × 40,000` 米 |
| `STATE_*_BOUNDS` 别名 | `:110-112` | 三个边界常量的向后兼容别名 |
| `gtadbDistance` / `gtadbDistanceToWorldMetres` / `worldDistance` | `:66` / `:72` / `:79` | 距离换算 |

**与前版本的实质差异**：前版本称"`worldToSvg(e) = {x: e.east*2, z: e.north*-2}`、`svgToWorld(e) = {x: e.x/2, y: e.z/-2}`"。核对后：第一个式子的体是 `canonicalToWorld`（输出 **world** 而非 SVG），第二个式子的体是 `worldToGtadb`（world → gtadb.org 单位）；真正的 SVG/地图↔世界对是 `worldToMap`/`mapToWorld`，且**不含任何 `×2`/`÷2`**。`SVG_BOUNDS_XZ` 与 `build3dExplorerUrl` 在 bundle、还原产物、源码中均 0 命中。还原式结论与"SVG 跨度 40,000×40,000"这一数值本身成立（`40,000 × 40,000`，前版本此处正确；但曾把边界写成 `64,000 × 40,000` 的说法不成立）。

### 4.3 路由与 3D 探索器链接（真实符号）

| 事实 | 位置 | 级别 |
| :--- | :--- | :--- |
| `explorerPath(placeId?, base?)` 构造 `${publicPath('', base)}?${query}`，`query = new URLSearchParams({view:'3d'})`，`if (placeId) query.set('place', placeId)` | `src/features/explorer/public-path.ts:12-16` | `[观察]` |
| `publicPath` / `projectPath`（基路径归一化，`BASE_URL` 感知） | `public-path.ts:2-9` | `[观察]` |
| `resolveRoute(pathname, search)` 判定 `view: 'map' \| 'explorer' \| 'project'`；`placeId = params.get('place') ?? (oldSlug ? \`region:${oldSlug}\` : null)` | `src/app/routes.ts:9-33` | `[观察]` |
| **遗留路由兼容**：路径段 `place/<slug>` 与 `viewpoint/<slug>`（后者剥离 `-regional-entry` 后缀）→ `region:<slug>` | `routes.ts:18-27` | `[观察]`（前版本未记录） |
| 落地页文案链接 `` `<a href={\`${publicPath("")}?view=3d\`}>` `` | `src/features/project/static-page.tsx:54` | `[观察]`（该目录确为 `features/project`，与坐标模块所在 `features/street-leonida` 不同） |
| **生产构建把 `publicPath` 的默认基路径固化为 `` `/gta6-leonida-atlas/` ``**（源码默认值在此处是 `import.meta.env?.BASE_URL ?? '/'`）：`function e(e,t=\`/gta6-leonida-atlas/\`){…}` | `bundles/gta6-leonida-atlas/assets/public-path-DoqNwZJ4.js`（300 B，单行；体内函数 `e`/`t`/`n` ↔ `publicPath`/`projectPath`/`explorerPath`，导出别名 `export{t as n,e as r,n as t}`） | `[观察]`（chunk 原文）＋`[推断]`（符号配对） |
| 地标数据 URL 模板 `` `${base…}/assets/street-leonida/maps/gtadb-landmarks-5c1d69f.json` `` | `reconstructions/08_gta6_leonida_atlas_endpoints.csv` | `[观察]` |

### 4.4 `[观察]` 追踪与源码↔Bundle 对照（含合法失败）

**`trace-application-feature`**（`notes/trace_leonida_coordinates_incoming.json`，`ev_cac3e667…`；种子 `node-id jag_node_3d7a3972…`，方向 `incoming`，源图 `ev_aabf94a7…`）：`matched_seeds 1`、`traced_nodes 4`、`traced_edges 3`、`observed_facts 6`、`inferred_facts 1`、`unknown_facts 0`、`terminal_paths 0`、`native_handoffs 0`；`coverage.status = partial`。追踪到的 4 个节点类型为 `javascript-module ×2`、`javascript-asset`、`artifact`，即"坐标模块 ← 资源/构建产物"的静态依赖链；REA 明确声明该结果只描述图关系，不证明可达性。

**`compare-source-to-bundle`**（`notes/compare_gta6_src_to_bundle.json`，`ev_7abd6455…`）：`retained_source_files 167`、`retained_application_nodes 53`、`candidate_evaluations 0`、`items` 状态全部 `unknown`（167/167）。这是**合法结论而非工具缺陷**：生产构建未发布 source map，且 167 个参考文件多为设计文档/规范（如 `docs/superpowers/specs/2026-09-06-atlas-immersive-design.md`），不产生任何可比信号；REA 打分权重（`exact-source-digest 100`、`source-map-original-path 70`、`current-path-exact 60`、`current-path-suffix 40`、`unicode-equivalent-path 20`、`basename-match 20`、`language-extension 5`，阈值 20）在此场景下无候选可评。`unmapped_current_node_ids = 53` 同时声明反向覆盖缺口。

### 4.5 `[推断]` 生产 chunk 的逐符号反混淆对照

`bundles/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js` 只有 586 B、单行，**所有标识符被最小化**：
对上表 11 个真实符号逐词 `grep -r` 得到 `bundle=0 / recovered=0 / 仅 src 命中`。因此按结构配对：
最小化体内联为 `e`=边界值 `{west:-16e3,east:4e3,south:-8e3,north:12e3}`、`t`=格式判别、`n`=`{x:east*2,z:north*-2}`、
`r`=`n(t(·))`、`i`=`{x:x/2,y:z/-2}`、`a`=`{x,y:z}`、`o`/`s`=两角点、`c`/`l`=两组边界，
尾部导出别名 `export{i as a,r as i,l as n,a as o,c as r,e as t}`。
逐条依据、实际求值（`−32000 / 8000 / −24000 / 16000 / 40000 / 40000`）与别名冲突说明见
**`reconstructions/17_minified_coordinate_chunk_symbol_map.md`**。
`mapToWorld`、距离三件套与 `STATE_*` 别名未出现在该 chunk 中 `[未知]`：属入口未引用/tree-shaking，不能反证源码缺失。

---

## 五、`txAdmin`

### 5.1 `[观察]` 全树 ↔ 作用域子图

全树（#13）3,620 节点 / **92 `endpoint`** / 5,092 findings / 543 解析 / 7 解析失败；`WebServer/router.ts` 单独作用域（#14）仅 6,148 B、702 AST、70 节点、**56 `endpoint`**、72 findings。作用域化是为了绕开全树端点混杂（含 `dist/`、模板与客户端路由字符串）导致的判读噪声。

### 5.2 路由注册表与观测对照

| 指标 | 值 | 来源 |
| :--- | ---: | :--- |
| 源码 `router.<verb>('path')` 注册行 | 59（57 个不同 path） | `reconstructions/06_txadmin_routes.csv` |
| 同时被 REA 作用域图端点节点覆盖 | **58**（`grade = source+graph`） | 同上 |
| 仅源码可见 | **1**：`/insights` | `reconstructions/06_txadmin_routes_unmatched.json` → `regex_rows_without_rea_node` |
| REA 图内、正则表外的 path | **0** | `rea_paths_not_in_regex_table: []` |

代表性隐藏管理路由 `[观察]`：`GET /legacy/adminManager`（`router.ts:30`）、`POST /adminManager/:action`（`router.ts:55`，通配 action 转发）、`POST /adminManager/getModal/:modalType`（`router.ts:54`）。逐条 method/path/`router.ts:line`/命中数见 CSV。

`compare-static-and-observed-endpoints`（`evidence/endpoint_comparison_txadmin_vs_observed.json`）在作用域图上成功执行，`application` 为 `ev_868fb827…`、`capture` 为 HAR 内联记录；由于本地未运行 txAdmin 服务，观测侧仅覆盖 §3.1 抓到的公共资产事务，因此该对照的"观测集"为空集成员居多——报告不据此推断 txAdmin 端点不存在。

---

## 六、《时之笛》复合工程：`Ocarina-Atlas`（v2.3.1）

### 6.1 `[观察]` 图概览

#7：1,216 节点 / 10 `endpoint` / 4 `route` 模式 / 4 `storage` / 1,675 findings / 72 解析 / 128,283 AST。`route` 覆盖声明为 `application_routes: unknown`、`candidate_enumeration: partial`、`source_parsing: complete`，即 REA 明确指出路由集合不保证完整。

### 6.2 Electron 宿主暴露面（`launcher/linux-appimage/`）

导出：`reconstructions/03_ocarina_atlas_electron_surface.csv`（49 行：`browser-window 1`、`context-bridge-api 1`、`ipc-channel 30`、`ipc-handler 16`、`service-worker 1`）。

| 事实 | 位置 | 级别 |
| :--- | :--- | :--- |
| `new BrowserWindow({…, webPreferences:{ preload, contextIsolation: true, nodeIntegration: false }})` | `main.cjs:1147-1161` | `[观察]`（`observe-browser-window-construction`） |
| `contextBridge.exposeInMainWorld('ocarinaAtlas', {…})` | `preload.cjs`（32 行全读） | `[观察]`（`context-bridge-api`） |
| 渲染端调用面 | `getInitialState`→`invoke('launcher:get-state')`；`onState`/`onLog`→`on('launcher:state'/'launcher:log')` | `[观察]` |
| 14 个 `ipcMain.handle` | `main.cjs:1171, 1176, 1181, 1186, 1194, 1202, 1210, 1225, 1230, 1245, 1270, 1280, 1287, 1292` | `[观察]` |
| 通道清单（16 项，含 2 个推送通道） | `launcher:get-state`、`check-updates`、`clear-logs`、`copy-text`、`open-external`、`open-map`、`refresh-lan`、`reset-rpc-defaults`、`set-drawer-open`、`setup-sync`、`start-server`、`stop-server`、`toggle-lan`、`update-settings` + `launcher:log`、`launcher:state` | `[观察]` |

### 6.3 宿主网络面与外部进程

| 事实 | 位置 |
| :--- | :--- |
| `DEFAULT_HOST '0.0.0.0'`、`DEFAULT_PORT 3000`（**默认监听所有网卡**） | `main.cjs:13-19`、`:62-64` |
| `DEFAULT_LICHII_HOST '127.0.0.1'`、`DEFAULT_LICHII_PORT 6545`、`MAX_LOG_LINES 500` | 同上常量区 |
| `OFFICIAL_REPO_URL = https://github.com/antonioalvromero/Ocarina-Atlas`；窗口尺寸 556×480/732 | `main.cjs:13-19, 62-64` |
| 环境变量覆写 `HOST`、`PORT`、`LICHII_PORT`/`FRITCHII_PORT` | `main.cjs:84-87` |
| `startServer()` 以 `ELECTRON_RUN_AS_NODE:'1'`、`NODE_ENV:'production'`、`APP_DIST_DIR` 等派生子进程 | `main.cjs:990-1012` |
| `setupSync()` 将 `EDEN_NX2_DEBUG_SERVER=<host>:<port>` 写入 6 个候选 `{config,data}Home/{lichii,fritchii,cheezyemu}/nx2_switches.txt` | `main.cjs:253-267` |

`[观察]` 端点/存储：`04_ocarina_atlas_endpoints.csv` 11 行（9 个字面量）含 `/api/lichii/state`、`/api/lichii/stream`、`/api/sync/state`（重复 2 次，不同调用点）、`https://api.openai.com/v1/chat/completions`、`` `https://api.github.com/repos/${safeSlug}/releases/generate-notes` ``，以及 `route` 类 `/`、`/$zone`；另有 2 行值为 `endpoint`/`request`，属变量位被记录为字面占位，**不得**当作 URL `[推断→未知]`。`05_ocarina_atlas_storage_keys.csv` 13 行 = 4 字面 / 9 动态。

### 6.4 归一化分数坐标与多楼层投影

`src/lib/coords.ts` 与 `src/lib/floorGrid.ts` `[观察]`：`Fraction ∈ [0,1]` 图像分数坐标系；`fractionToBox` / `boxToFraction` / `fitSize` / `stageBox` / `clampView` / `zoomViewAt` / `pinPosition`；网格常量 `CELL_WIDTH = 1`、`CELL_HEIGHT = 0.75`、`GAP = 0.06`、`LABEL_HEIGHT = 0.08`；缩放约束 `MIN_ZOOM = 1`、`MAX_ZOOM = 10`。还原式：`reconstructions/ocarina-atlas/OcarinaAtlas_StageProjection.ts`；推导细节：`reconstructions/14_coordinate_transforms.md`。

### 6.5 未确认的 Electron 声明核验（`npwd`）

`reconstructions/10_electron_main_node_audit.csv`：`npwd__app.json` 中 `electron-main` 节点的 `declared_path`（如 `dist/hooks.js`）其 `limitations` 为 *"No extension, directory package, or index candidate exists in the inventoried artifact container."*，authority 为 `static-relationship-inference`，位置来自 `packages/*/package.json`。**结论**：npwd 的 `electron-main` 标签是对包 `main` 字段的静态推断，不是打包应用中的真实入口 `[观察]`。

### 6.7 失败登记：`trace-application-feature`（Ocarina-Atlas）

对分数坐标种子执行入向追踪时 REA 返回 `resource_constraint / memory`：`authentication_bytes = 2,004,904,248` > `available_heap_bytes = 1,873,791,640`（`notes/trace_ocarina_fractionToBox.json` 完整错误对象；`reconstructions/15_endpoint_trace_and_compare.md:79` 为登记行）。按契约不重试、不改堆参数，记 `[未知]`：Ocarina-Atlas 的坐标特征依赖闭包未由 REA 产出，§6.4 因此完全基于源码阅读。

---

## 七、《时之笛》Web：`tootr` 与 `noclip.website`

### 7.1 `tootr` 身份澄清

| 前版本表述 | 核验结论 |
| :--- | :--- |
| "《时之笛》WebGL/React 场景与过场（Cutscene）查看器；React + Babel 运行时生态，专注场景与过场动画轻量化浏览器解析" | **无证据支持。** 本地工作树证据：依赖 `@mracsys/randomizer-graph-tool`、组件 `src/components/Tracker.tsx`、页面 `<title>TOoTR</title>` → 实际是**随机化器物品/逻辑图追踪器**。图中 WebGL 相关节点 0 个，Cutscene 解析证据 0 个 `[未知]` |
| "57 解析 / 737,885 B / AST 60,715 / 1,264 findings" | **数字成立**，但主体是 `#8 oot-peripherals/tootr` 本地源码树；节点类型分布 `javascript-module 1005`、`storage 30`、`javascript-asset 57`、`artifact 7`、`electron-renderer 1`、`package 1`、`unknown 1`，`route 0` |
| 线上 `https://tootr.mracsys.com/assets/index-CHfdltUy.js`（5,339,984 B） | 已抓取（`bundles/tootr/`）并作为 Wakaru 输入，但还原图（#9）仅 5 节点、`parsed_javascript_files 0`、AST 0、14,879,069 B 读取 → **还原产物未能形成等价图**，线上包的应用图谱仍为 `[未知]` |

`tootr` 的 30 个 `storage` 观测 `[观察]` 是其作为追踪器持久化种子/设置的自然结果，与场景渲染无关。

### 7.2 `noclip.website`

| 事实 | 级别 |
| :--- | :--- |
| 本地树（#10）：933 解析 / 964 相关 / 34,937,823 B / AST 2,647,300 / 3,085 节点 / 1 `endpoint` / 4 `storage` / 10,072 findings / 4 解析失败 | `[观察]` |
| `https://z.noclip.website` | **`[推断]`（`grep` 直证，非 REA 端点观测）**：生产 chunk `bundles/noclip.website/static/js/8.15df99b7.js` 命中 1 次；`src/` 内 0 命中；仓库侧唯一出现是根目录 `.env:1` 的 `PUBLIC_STORAGE_URL=https://z.noclip.website` → 构建期环境变量注入产物 |
| REA 在该树上的**全部** `endpoint` 节点 | `[观察]`：源码树（#10）只有 1 个，值 `static_arc.crg1`，机制 `call:archiveCache.get`，authority `static-relationship-inference` / confidence `low`，位置 `src/BanjoTooie/scenes.ts:335` —— 即 Banjo-Tooie 存档缓存键，并非站点 URL；还原树（#11）的 2 个端点节点值均为变量 `Z`（`ast-static-analysis`），不携带可报告的字面量 → **noclip 的 URL 面在 REA 图中基本为空**，不能据此声称其端点已被枚举 |
| `oot3d/*` 场景路径 | `[观察]`：`bdan`（bundle 3 次 / 仓库命中）、`bmori1`(2)、`ddan`(3)、`entra_day`(1)、`entra_night`(1)、`entra_ruins`(1)、`ganon`(9)、`daiyousei_izumi`(1)。bundle 侧是**完整字面量**；仓库侧完整字面量只出现在 `src/DefaultSaveStates.json`，场景名成分则定义于 `src/OcarinaOfTime3D/oot3d_scenes.ts`（`bdan` 等，同目录另有 `csab.ts`、`cmab.ts`、`ctxb.ts`、`mm3d_scenes.ts`） → 组合串在构建期内联，`src/` 中不存在 `'oot3d/' + name` 形式字面量 |
| "自研 WebGL2 `GfxDevice` + `Zelda` 渲染管线按需流式加载场景二进制" | **部分成立且命名需修正**：`GfxDevice` 在 `src/` 中被广泛引用（如 `src/viewer.ts`、`src/KingdomHeartsDDD/scenes.ts`），WebGL2 平台层为真实文件 `src/gfx/platform/GfxPlatformWebGL2.ts`（另有 `GfxPlatform.ts`、`src/gfx/passes/`、`src/gfx/shaderc/GfxShaderCompiler.ts`）；OoT3D 侧模块名是 `src/OcarinaOfTime3D`，本工作树中**不存在**顶层 `src/Zelda/` 目录与 `src/renderers/` 内容。"按需流式加载" 属 `[推断]`，REA 图未产出该调用链 |
| 还原图（#11）：98 解析 / 198 相关 / 22,721,131 B / AST 415,090 / 992 节点 / 2 `endpoint` / 1,490 findings | `[观察]`，但覆盖率明显低于原树（933→98 解析），**不可**将 #11 视为 #10 的等价替代 |

---

## 八、产出清单

**代码还原（TypeScript，可编译片段 + 逐条来源标注）**

| 文件 | 内容 |
| :--- | :--- |
| `reconstructions/stateofleonida/StateOfLeonida_ApiAndMapClient.ts` | `StateOfLeonidaApiAction`（13）/`StateOfLeonidaAdminAction`（23）联合类型、`STATE_OF_LEONIDA_DATA_DATASETS`（17）、`callStateOfLeonidaApi`、`fetchGtadbPublicSource`、`fetchScreenshotChallengeLeaderboard` |
| `reconstructions/gta6-leonida-atlas/LeonidaAtlas_CoordsAndRoutes.ts` | 前版本产物（保留原样，未覆盖）；符号名与真实源码不一致，勘误见 `reconstructions/gta6-leonida-atlas/VERIFICATION_NOTE.md` |
| `reconstructions/gta6-leonida-atlas/LeonidaAtlas_Verified_Transforms.ts` | 本次新增：按真实符号与真实行号复原坐标/路由（`gtadbToCanonical`、`canonicalToWorld`、`worldToGtadb`、`worldToMap`/`mapToWorld`、`WORLD_BOUNDS`/`MAP_BOUNDS` 含 `width`/`height`、`publicPath`/`projectPath`/`explorerPath`、`resolveRoute` 及其遗留 `place/`、`viewpoint/` 分支） |
| `reconstructions/gta6-leonida-atlas/VERIFICATION_NOTE.md` | 9 个自造符号在 bundle/还原/源码三侧 0 命中的逐条核验表 + 变换配对错误说明 |
| `reconstructions/ocarina-atlas/OcarinaAtlas_StageProjection.ts` | `Fraction/Size/Box/View`、`INITIAL_VIEW`、`MIN_ZOOM/MAX_ZOOM`、`fractionToBox`、`boxToFraction`、`fitSize`、`stageBox` |
| `reconstructions/noclip-and-tootr/NoclipAndTootr_SceneRegistry.ts` | `NoclipSceneDesc`、`NOCLIP_OOT3D_SCENE_PATHS`（8 项，逐条 bundle/仓库命中计数） |

**投影表与说明**：`01`(153)、`02`(82)、`03`(49)、`04`(11)、`05`(13)、`06`(59)+`_unmatched.json`、`08_gta6`(20)、`08_npwd`(25)、`08_noclip`(1)、`10`(11)、`11_evidence_ledger.csv`(26)、`12_input_digest_ledger.md`、`13_obfuscated_to_recovered_forms.md`、`14_coordinate_transforms.md`、`15_endpoint_trace_and_compare.md`、`16_stateofleonida_string_pool.csv`(192)、`17_minified_coordinate_chunk_symbol_map.md`、`00_README.md`。
投影脚本：`notes/export_reconstructions.py`、`notes/string_pool.py`；字面量保真对照结果：`notes/leonida_endpoint_literal_stability.json`；运行日志：`notes/local_runs.log`、`notes/recovery_runs.log`。
**输入/产物**：`bundles/`（17 个抓取文件）、`recovered/<4 目标>/modules/`、`live/`（`observed.tsv`、`observed_assets.har`、`vimap_headers.txt`、页面 HTML）、`local/`（作用域子图输入）。

---

## 九、对既有 63 行版本的核验与更正

表 9-1

| # | 原断言 | 核验 | 证据 |
| :--- | :--- | :--- | :--- |
| 1 | `worldToSvg(e)` = `{x: e.east*2, z: e.north*-2}`；`svgToWorld(e)` = `{x: e.x/2, y: e.z/-2}`；`SVG_BOUNDS_XZ`；`build3dExplorerUrl` | **符号不存在（4/4，bundle/还原/源码 0 命中），且变换对配对错误** | 真实为 `canonicalToWorld`（`leonida-coordinates.ts:40`）与 `worldToGtadb`（`:51`）；SVG 对是 `worldToMap`/`mapToWorld`（`:58,:62`，无缩放）；3D 链接是 `explorerPath`（`public-path.ts:12-16`） |
| 2 | "`20,000 × 20,000` 世界单位；SVG 跨度 `40,000 × 40,000`" | **成立**（canonical 跨距 20,000；×2 后 40,000） | `CANONICAL_BOUNDS` `:28-33`、`WORLD_METRES_PER_GTADB_UNIT` `:26` |
| 3 | "`data/*.json` 共 17 个，含 `en-emojibase-data.json`" | **计数成立但成员错误**：第 17 项是模板 `data/maps/${id}.json`；`en-emojibase-data.json` 的真实路径为 `assets/libs/emoji-picker-element-data/en-emojibase-data.json` | `16_stateofleonida_string_pool.csv`（`data_json` 17 行）、`recovered/stateofleonida/modules/bundle.js:596`、bundle 字符串上下文 |
| 4 | "`auth/api.php` 36 个 action，其中 23 个 `admin_*`" | **成立**（13 公开 + 23 管理） | 字符串池 `php_action` 36 行；33 行有 REA 观测 |
| 5 | "`tootr` = WebGL/React 场景与过场查看器" | **错误**：`tootr` 是 OoT 随机化器追踪器；无 WebGL/Cutscene 证据 | `oot-peripherals/tootr` 的依赖与 `src/components/Tracker.tsx`、`<title>TOoTR</title>`、#8 节点分布（WebGL 0） |
| 6 | 表头把 `noclip.website`、`tootr` 标为"线上"，数值 933/34,937,823/2,647,300/10,072 与 57/737,885/60,715/1,264 | **数值成立，主体错位**：来自本地工作树 `#8`/`#10`；线上包对应的是 `#9`/`#11`（还原输入 `bundles/tootr/index-CHfdltUy.js`、`bundles/noclip.website/static/js/8.15df99b7.js`） | 各 `__app.json` 的 `subject.local_path` |
| 7 | "`root_artifact_sha256 f62c7c0d…`（SHA-256）" | **概念混用**：`f62c7c0d…` 是 REA 规范化产物标识；原始文件 SHA-256 为 `0f5ab96c…` | `evidence/stateofleonida__summary.json` vs `sha256sum` |
| 8 | "`Ocarina-Atlas` 行留空（`-`），证据仅 `notes/local_runs.log`" | **低估**：实际有 1,216 节点完整图 + 49 项 Electron 暴露面 + 14 个 `ipcMain.handle` 行号 | #7、`03_ocarina_atlas_electron_surface.csv` |
| 9 | "`gta6-leonida-atlas` 证据为 `bundles/gta6-leonida-atlas/`" | **成立且补充**：5 解析 / 602,622 B / 48 节点 / 0 endpoint（生产包无 source map） | #3 |
| 10 | "8 个 `oot3d/*` 场景路径（bdan/bmori1/ddan/entra_day/entra_night/entra_ruins/ganon/daiyousei_izumi）" | **成立**：bundle 与仓库两侧均命中（次数见 §7.2） | `bundles/noclip.website/static/js/*`、`src/DefaultSaveStates.json` |
| 11 | "外部依赖含 `map.gtadb.org/photos`、`gta.wiki/api.php`、`earth.google.com/web`、YouTube/Vimeo/Spotify" | **成立（主机级）**：24 主机字符串池全部命中；但这些为主机字面量，非 REA 端点观测 | `16_stateofleonida_string_pool.csv`（`url_host` 24 行，`contained_in_rea_endpoint_observations` 全为 `no`） |
| 12 | 原报告未记录任何失败/未知项 | **补充**：3 次 REA 失败（§3.7、§4.4、§6.7）与 5 个仅字符串池可见的 PHP 脚本在原版缺失 | 同上 |

---

## 十、未知与未找到证据清单

1. `[未知]` 后端真实响应与权限模型：全程未调用任何 `api.php`/`*.php`；`admin_*` 是否要求会话、是否有二次鉴权，静态层无法判定。
2. `[未知]` `auth/community-api.php`、`auth/login.php`、`image-proxy.php`、`proxy.php`、`save-changelog.php` 的参数契约：仅有字符串字面量，无调用点观测。
3. `[未知]` `stateofleonida` WebSocket 端点 URL（存在 `call:WebSocket` 机制观测，URL 动态构造）。
4. `[未知]` 线上 `tootr` Bundle（5,339,984 B）的应用图谱：还原后 `parsed_javascript_files = 0`。
5. `[未知]` `Ocarina-Atlas` 坐标特征的 REA 入向闭包（内存约束，§6.7）。
6. `[未知]` `stateofleonida` 的"静态 ↔ 观测"端点差集（投影预算，§3.7）。
7. `[未知]` `gta6-leonida-atlas` 源码 ↔ 生产 Bundle 的文件级映射（无 source map，167 × `unknown`）。
8. `[未知]` `analyze-web-bundle` 类运行时事实：SPA 动态注入 URL、`import()` 实际解析、参数驱动的服务端分支。
9. `[未知]` `https://gtaweb.eu/`（403，未绕过）、`https://vimap.saamexe.com/` 之外的镜像是否共享同一 Bundle。

---

## 附录 A：关键可复现命令

```bash
# 别名与抓取事实
curl -sSI https://vimap.saamexe.com/ | sed -n '1,4p'
curl -sS -o bundles/stateofleonida/solmain2809_4.js \
  https://map.stateofleonida.net/solmain2809_4.js && sha256sum "$_"

# 线上 Bundle 建图（混淆态）
rea analyze-javascript-application \
  /root/wsl-demo/rea-reconstructions/web-and-js/bundles/stateofleonida \
  --json > evidence/stateofleonida__app.json

# 反混淆还原（必须 prlimit 包裹 + 显式 Wakaru 可执行文件 + 全新输出目录）
prlimit --as=8G --memlock=unlimited \
  env REA_WAKARU_COMMAND=/root/.local/share/rea-tools/node_modules/@wakaru/cli-linux-x64/wakaru \
  rea recover-javascript-sources \
    /root/wsl-demo/rea-reconstructions/web-and-js/bundles/stateofleonida/solmain2809_4.js \
    --output_root /root/wsl-demo/rea-reconstructions/web-and-js/recovered/stateofleonida --json

# 还原态二次建图 + 字面量保真对照
rea analyze-javascript-application \
  /root/wsl-demo/rea-reconstructions/web-and-js/recovered/stateofleonida/modules --json \
  > evidence/stateofleonida_recovered__app.json
python3 notes/string_pool.py          # -> reconstructions/16_stateofleonida_string_pool.csv
python3 notes/export_reconstructions.py

# 特征追踪 / 源码↔Bundle 对照 / 静态↔观测端点
rea trace-application-feature evidence/gta6-leonida-atlas_recovered__app.json \
  --seed '{"kind":"node-id","value":"jag_node_3d7a3972aa7751a32268b943c7cc3616a8551971a890ad0f741c9114eca4163e","match":"exact"}' \
  --direction incoming --json > notes/trace_leonida_coordinates_incoming.json
rea import-reference-source /root/wsl-demo/gta-projects/gta6-leonida-atlas \
  --exclude-paths '[]' --json > notes/leonida_source_graph.json
rea compare-source-to-bundle --reference notes/leonida_source_graph.json \
  --application evidence/gta6-leonida-atlas_recovered__app.json \
  --evidence-bundle evidence --json > notes/compare_gta6_src_to_bundle.json
rea inspect-web-network-capture live/observed_assets.har live/observed_assets.har \
  --detail summary --evidence_output evidence/observed_assets__har.json
```

## 附录 B：证据标识（节选）

| Evidence | ID（前缀） | 用途 |
| :--- | :--- | :--- |
| `stateofleonida__app.json` | `ev_f5951034…` | 混淆态主图 |
| `stateofleonida_recovered__app.json` | `ev_5a75e525…` | 还原态主图 |
| `leonida_source_coords__app.json` | `ev_32825cf5…` | 坐标模块子图 |
| `gta6-leonida-atlas_recovered__app.json` | `ev_aabf94a7…` | 追踪种子来源图 |
| `trace_leonida_coordinates_incoming.json` | `ev_cac3e667…` | 坐标入向追踪 |
| `compare_gta6_src_to_bundle.json` | `ev_7abd6455…` | 源码↔Bundle 对照 |
| `txadmin_router_scoped__app.json` | `ev_868fb827…` | txAdmin 作用域图（56 端点） |
| `endpoint_comparison_txadmin_vs_observed.json` | `ev_f9d019ea…` | 静态↔观测对照 |
| `observed_assets__har.json` | `ev_a5343e0e…`（视图 `ev_4727ec49…`） | 15 条目网络观测 |
| `tootr__app.json` / `noclip-website__app.json` | `ev_52f57551…` / `ev_709df08d…` | 时之笛本地源码树图 |

完整 26 项见 `reconstructions/11_evidence_ledger.csv`；输入摘要见 `reconstructions/12_input_digest_ledger.md`。

## 附录 C：工具与版本

REA 6.9.0（`/usr/local/bin/rea`）；Wakaru 1.14.0（`@wakaru/cli-linux-x64`）；Node.js v26.3.0；平台 `6.18.58.1-microsoft-standard-WSL2` / x64。所有 REA 调用均在 `dir_path` 固定于本项目、以绝对路径传参执行（本工作区存在会话目录漂移，已规避）。
