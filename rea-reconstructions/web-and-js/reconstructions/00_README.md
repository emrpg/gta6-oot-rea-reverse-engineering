# `web-and-js` 逆向重建目录索引（工单 #3）

工单：`emrpg/gta6-oot-rea-reverse-engineering#3` `[REA-Web-And-JS]`
工具：`/usr/local/bin/rea` 6.9.0（`analyze-javascript-application`、`recover-javascript-sources`
（Wakaru 1.14.0）、`import-reference-source`、`compare-source-to-bundle`、`trace-application-feature`、
`inspect-web-network-capture`、`compare-static-and-observed-endpoints`）
深度报告：`/root/wsl-demo/reports/11_rea_web_and_js_reconstruction.md`

## 目录

| 路径 | 内容 |
|---|---|
| `bundles/` | 线上抓取的原样产物（4 站点、17 个文件、17 MB；SHA-256 与线上当前字节逐条核对） |
| `live/` | 真实观测事务 `observed_assets.har`（15 条）、`observed.tsv`、`vimap_headers.txt`、落地 HTML |
| `evidence/` | REA 完整 Evidence 束（26 份 = 22 JSON + 4 YAML 分析视图摘要，1.3 KB – 148 MB；整目录 924 MB，`.gitignore` 排除） |
| `recovered/` | Wakaru 结构还原输出（`inputs/ modules/ reports/ manifest.json`，4 站点，60 MB） |
| `local/` | 为控制内存而切片的最小输入（`leonida-source-coords/`、`txadmin-router/`） |
| `notes/` | 运行脚本、进度日志、`xtract.py` 投影器、`export_reconstructions.py`、trace/compare 原始 JSON |
| `notes/profiles/` | 每份 Evidence 的紧凑图谱画像（节点类型、端点、存储键、语义计数、limitations） |
| `reconstructions/*.csv`、`*.md` | 下表所列的可读重建表与专论 |
| `reconstructions/<项目>/` | 5 个可运行还原模块 + 1 份勘误说明（`gta6-leonida-atlas/VERIFICATION_NOTE.md`） |

## 重建表

| 文件 | 行数 | 主题 |
|---|---|---|
| `01_stateofleonida_endpoints.csv` | 153 | 闭源 Leonida 地图的全部静态端点观测（131 节点 / 125 字面量 / 9 种机制） |
| `02_stateofleonida_storage_keys.csv` | 82 | 客户端存储观测：64 localStorage + 18 sessionStorage；17 个字面量键（44 行）+ 38 行动态键 |
| `03_ocarina_atlas_electron_surface.csv` | 49 | Electron IPC / contextBridge / BrowserWindow / service-worker 观测 |
| `04_ocarina_atlas_endpoints.csv` | 11 | 遥测与同步端点（`/api/lichii/*`、`/api/sync/state`、GitHub/OpenAI、路由候选） |
| `05_ocarina_atlas_storage_keys.csv` | 13 | 3 个 `oot-map:v1:*` 字面量键（4 行）+ 9 行动态键（i18n/全屏/PWA 缓存） |
| `06_txadmin_routes.csv` | 59 | txAdmin 路由表（方法、路径、`router.ts:行`、REA 端点节点数、证据等级） |
| `06_txadmin_routes_unmatched.json` | — | 源码↔图谱反向差集：仅 `/insights` 未被 REA 端点节点覆盖 |
| `07_ocarina_atlas_electron_ipc.md` | — | 启动器 IPC 面专论（含 `setup-sync` 写 `nx2_switches.txt` 的隐藏配置） |
| `08_gta6_leonida_atlas_endpoints.csv` | 20 | 源码侧端点/URL 状态参数（`x/y/z/t/layer/place/page/view`） |
| `08_npwd_endpoints.csv` | 25 | FiveM 侧车 `app.post` + NUI 路径 + ESX 玩家字段 |
| `08_noclip_endpoints.csv` | 1 | noclip 源码侧唯一“端点”其实是存档缓存键 `static_arc.crg1` |
| `10_electron_main_node_audit.csv` | 11 | `electron-main` 节点审计：npwd 的 10 个节点全是 `static-relationship-inference` |
| `11_evidence_ledger.csv` | 26 | 每个 Evidence 文件的字节数、operation、evidence_id、SHA-256 |
| `12_input_digest_ledger.md` | — | 输入产物原始摘要 ↔ REA 产物身份摘要 ↔ Evidence ID 对照 |
| `13_obfuscated_to_recovered_forms.md` | — | 混淆形态↔还原形态 28 对逐条配对（0 未配对） |
| `14_coordinate_transforms.md` | — | Leonida GTADB↔world↔map 与 OoT fraction↔stage 数学 |
| `15_endpoint_trace_and_compare.md` | — | HAR/比对/追踪/源码-产物比对的真实结果，含 3 类失败与固定预算限制 |
| `16_stateofleonida_string_pool.csv` | 192 | Leonida 字符串池清单（9 PHP / 36 action / 17 data-json / 24 主机 / 2 media action / 102 疑似存储前缀），并逐行标注是否被 REA 端点观测覆盖 |
| `17_minified_coordinate_chunk_symbol_map.md` | — | 生产 chunk `leonida-coordinates-CXGuqbYF.js`(586 B) 与 `public-path-DoqNwZJ4.js`(300 B) 的最小化名 ↔ 真实符号配对表，含导出别名冲突与构建期 `BASE_URL` 内联证据 |

## 方法与边界（摘要）

1. 工单点名的 `analyze-web-bundle` 需要回环 CDP（`http://127.0.0.1:PORT` + `targetId`）。本机无
   Chromium/Firefox/Playwright 缓存，**未安装浏览器**，故改用「公开静态资源 GET 抓取 → 应用图谱重构 →
   Wakaru 模块还原 → 二次图谱」的等价静态路线，并把这一替代写进报告。
2. `gtaweb.eu` 的 403 挑战未被绕过；所有后端 API 端点只作为**静态候选**记录，一次也未调用。
3. 报告严格区分：观察（`ast-static-analysis` + `exact`）/ 推断（`static-relationship-inference`、
   `inferred`）/ 未知（`unknown` 节点、`partial` 覆盖、`resource_constraint`）。
4. `evidence/`、`bundles/`、`recovered/`、`notes/profiles/`、`notes/*.log`、`notes/leonida_source_graph.json`
   合计约 1,006 MB（其中 Evidence 924 MB），已在仓库 `.gitignore` 中排除；版本库只收可读重建表、
   还原模块、脚本、trace/compare 原始 JSON、`live/` 观测与报告。
5. 可读产物之外的复现路径见深度报告附录 A。
