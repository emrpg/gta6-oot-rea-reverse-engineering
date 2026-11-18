# 端点比对、特性追踪与源码↔产物比对：真实结果（含失败）

工具版本：`rea 6.9.0`（`/usr/local/bin/rea`）。以下每条结果都保留 REA 原始错误/统计字段，
失败也是交付物的一部分。

## 1. 观测流量：`live/observed_assets.har`

15 条事务全部来自本会话的真实 `curl` 观测（状态码、传输字节、`Content-Type`、耗时为测量值；
非浏览器捕获，故无 Cookie/Referer 等请求头细节）。抓取范围只有**静态资源 GET**：
页面 HTML 与 JS/CSS 分包。**没有调用任何后端 API**（`auth/api.php`、`media-admin.php`、
`get-media-files.php` 等一律未触发），因此本报告对服务端行为不作任何断言。

字节一致性核验（本次重取 vs 早前存档，SHA-256 全等）：

| 线上 URL | SHA-256 前缀 | 结论 |
|---|---|---|
| `https://map.stateofleonida.net/solmain2809_4.js` | `0f5ab96c788a6e8d…` | 与存档一致（2,928,855 B） |
| `https://noclip.website/static/js/8.15df99b7.js` | `ef73ed494db8e130…` | 一致（7,677,171 B） |
| `https://tootr.mracsys.com/assets/index-CHfdltUy.js` | `f1e0ea9556b4b928…` | 一致（5,339,984 B） |
| `https://gta6state.com/…/leonida-coordinates-CXGuqbYF.js` | `c65bc30cb288ba0d…` | 一致（586 B） |
| `https://gta6state.com/…/index-B34Su3Ax.js` | `d81d24b7da29b91c…` | 一致（579,650 B） |

域名解析事实：`https://vimap.saamexe.com/` 以 **HTTP 308** 永久重定向到
`https://map.stateofleonida.net/`（响应头 `server: cloudflare`、`location: https://map.stateofleonida.net/`，
见 `../live/vimap_headers.txt`），落地页与 `bundles/leonida.html` 同为 19,064 字节。
工单点名的 VImap 与 Leonida 地图是同一应用的别名域。
`https://gtaweb.eu/` 持续返回 **403**（Cloudflare 挑战页），本次**未尝试绕过**。

HAR → Evidence：`rea inspect-web-network-capture live/observed_assets.har har`，
父 Evidence `ev_a5343e0ed30783a814668a521bd02d7771b684e193a8a483bb76f98dc06230d2`，
`coverage.status: complete-within-view`，`examined: 15`；摘要视图
`ev_4727ec4954c2ff769a597a3847c83295a620dbec8031e2766b5f2176218a1e31`。

## 2. `compare-static-and-observed-endpoints`

### 成功样本：txAdmin 路由模块

输入：`application = ev_868fb827…`（`local/txadmin-router` 的切片分析，1,337,463 B Evidence，
56 个 `endpoint` 节点 / 58 条网络观测）；`capture = ` 上文 HAR Evidence（**内联**记录）。
输出 Evidence：`../evidence/endpoint_comparison_txadmin_vs_observed.json`
（`ev_f9d019eac1c2e363878b9995a7241e3518ab0ebb55539c70f52219b8b1a21db2`）。

| summary 字段 | 值 |
|---|---|
| `static_network_endpoint_observations` | 58 |
| `observed_groups` | 14 |
| `matched` / `matches` | 0 / 0 |
| `observed_without_static_match` | 14 |
| `not_observed_in_capture` | 0 |
| `unknown` | 58 |

全部静态端点落入 `unknown`，且 REA 的 limitations 明确了口径：
「Not-observed-in-capture refers only to this capture; it does not establish unused, hidden,
inaccessible or absent server behavior.」⇒ **静态路由存在、但本次未观测**，不得写成「路由未使用」。
（txAdmin 是本地 FiveM 管理端，本会话未运行它，因此 0 匹配是预期的空结果，只用于验证工作流可用。）

### 失败样本：stateofleonida 全量 API 面

`application = ev_5a75e525…`（131 端点 / 125 个字面量，含长模板串与 44,831 行源范围）时，
无论 `--detail complete` 还是 `--detail summary + --evidence_output`，均返回：

```
code: resource_constraint
reason: API-surface retained state and temporary work exceed the projection representation budget.
reported_limits.output_bytes: 100663296
diagnostics.input_path: ["application"]
retryable: false
```

REA 说明该投影预算是**固定额度**，提高 worker 内存或主机空闲内存都无效。
因此该站的静态↔观测比对留在「未获得比对 Evidence」状态；端点清单本身仍完整可用
（`01_stateofleonida_endpoints.csv`）。绕行方案（更小切片、`inspect-analysis-view` 分页）记在待办里。

## 3. `trace-application-feature`

| 目标 | 种子 | 方向 | 结果 |
|---|---|---|---|
| `gta6-leonida-atlas_recovered__app.json` | `node-id jag_node_3d7a3972aa…`（`leonida-coordinates-CXGuqbYF.js` 模块节点） | incoming | 4 节点 / 3 边，`paths: []`；`coverage.status: partial`，`total_seed_matches: 1`；Evidence `ev_cac3e6677f0abbc0…` |
| `Ocarina-Atlas__app.json` | `string fractionToBox`（`seed` 需为对象：`{kind,value,match}`） | both | **失败**：`resource_constraint / memory`，`authentication_bytes 2,004,904,248` > `available_heap_bytes 1,873,791,640`（122 MB Evidence 的认证峰值超出共享主机可用堆） |

Leonida 侧的追踪结论要如实表述：还原后的图谱里只出现
`artifact → javascript-asset → javascript-module`（同一 chunk 的两个导出键）这一条**包含链**，
`paths` 为空 ⇒ 跨 chunk 的 `import()` 调用（Vite 运行时 `__vite__mapDeps` 拼接的哈希文件名）在静态层
未被解析成引用边。这不是「坐标模块没人用」，而是**引用关系未知**；使用者需从产物侧的
`index-B34Su3Ax.js` 里按运行时映射反查（该文件本轮因单文件资源约束未展开，见 `13_…` 与 §5）。

`seed` 契约（实测）：`{kind: node-id|route|string|channel|module|native-export|api, value, match:"exact"}`；
不接受 `entry_path`（未知参数）；`direction ∈ {incoming,outgoing,both}`。

## 4. `compare-source-to-bundle`

生产链：`rea import-reference-source <仓库> --exclude-paths '["node_modules","dist",".git","coverage"]'`
→ `HistoricalSourceGraph`（`notes/leonida_source_graph.json`，`root_sha256 3dcaa862…`，
`inventory_state: partial`）→ `compare-source-to-bundle`。
注意 `reference.kind` 只接受 `"file"` 且必须是 **Evidence/Graph JSON**（直接给 `.ts` 路径会报
`invalid-json`），`application` 只接受 `retained-evidence` 引用（配 `--evidence-bundle`），
`capture` 可内联整条记录。

打分信号与权重（REA 返回的 `scoring.weights`，`minimum_candidate_score = 20`）：

| signal | weight |
|---|---|
| `exact-source-digest` | 100 |
| `source-map-original-path` | 70 |
| `current-path-exact` | 60 |
| `current-path-suffix` | 40 |
| `unicode-equivalent-path` | 20 |
| `basename-match` | 20 |
| `language-extension` | 5 |

实际结果（`notes/compare_gta6_src_to_bundle.json`，`ev_7abd6455…`，`comparison_id stbc_fc8deee2…`）：

```
summary: {unchanged: 0, modified: 0, removed: 0, split: 0, merged: 0, duplicated: 0, unknown: 167}
coverage: {status: partial, retained_source_files: 167, retained_application_nodes: 53, candidate_evaluations: 0}
unmapped_current_node_ids: 53
```

原因清楚且可复核：`gta6state.com` 的生产构建**不含 `.map`**，chunk 文件名是内容哈希
（`index-B34Su3Ax.js` 等），与 TS 源路径既不 `exact` 也不同基名；于是 70/60/40 三档信号全部无从得分，
`candidate_evaluations` 直接为 0。REA 因此**拒绝猜测配对**，全部记 `unknown`，
limitations 同时写明「Application graph coverage is partial; unmatched files remain unknown」。
⇒ 该站的源码↔产物对应关系靠 §A 的逐式人工核对建立，而不是靠工具背书。

## 5. 还原后二次图谱的资源约束分布（诚实覆盖率）

| 二轮图谱 | 已解析 | 跳过 | `unknown` 节点 | 主要约束 |
|---|---|---|---|---|
| `stateofleonida_recovered__app.json` | 1 / 1 文件（3,229,701 B，44,831+ 行） | 0 | **0** | 语义图达 20,000 节点上限 ⇒ `partial` |
| `gta6-leonida-atlas_recovered__app.json` | 5 | 1 | 1 | `index-B34Su3Ax.js`（579,650 B 单行）触发「Owned JavaScript worker」约束 |
| `tootr_recovered__app.json` | 0 | 1 | 1 | 还原把 5,339,984 B 扩张到 14,879,069 B，单文件超预算 ⇒ 整模块只留 `unknown` |
| `noclip-website_recovered__app.json` | 98 | 1 | 97 | 43× 「The parent reserved」+43× 「Expanded application relationships」+8× 「Semantic graph」+2× 「Completed module」+1× worker |

一轮（源码/原始产物）图谱与二轮的差异说明：**还原提升可读性与行号可寻址性，
但体积膨胀会换来新的覆盖缺口**——这不是失败，而是必须在报告里成对给出的覆盖率事实。
