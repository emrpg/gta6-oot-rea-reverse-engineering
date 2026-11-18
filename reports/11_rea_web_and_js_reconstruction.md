# 基于 REA 的 GTA 6 与《塞尔达传说：时之笛》Web/JS 应用图谱逆向与源码重构报告

> **工单关联**：`emrpg/gta6-oot-rea-reverse-engineering#3` (`[REA-Web-And-JS]`)  
> **分析工具**：`/usr/local/bin/rea` (`analyze-javascript-application`)  
> **证据归档目录**：`/root/wsl-demo/rea-reconstructions/web-and-js/evidence/`  
> **源码重构目录**：`/root/wsl-demo/rea-reconstructions/web-and-js/reconstructions/`

---

## 一、 REA 静态应用图谱分析总览

本次针对 GTA 6 与《时之笛》（OoT）生态的线上抓取 Bundle 及本地前端工程执行了 `rea analyze-javascript-application`，核心规模与静态取证统计如下：

| 目标应用 | 类型 | 解析 JS/TS 文件数 | 扫描源码字节数 | 遍历 AST 节点数 | REA Findings | 证据归档 |
| :--- | :--- | :---: | :---: | :---: | :---: | :--- |
| **`noclip.website`** | 线上 3D 关卡博物馆 (OoT/OoT3D) | 933 | 34,937,823 B | 2,647,300 | 10,072 | `noclip-website__app.json` (148 MB) |
| **`stateofleonida`** | 线上闭源 GTA 6 交互地图 (`solmain2809_4.js`) | 1 (混淆 Bundle) | 2,928,855 B | 381,100 | 264 | `stateofleonida__app.json` (45 MB) |
| **`tootr`** | 《时之笛》WebGL/React 场景与过场查看器 | 57 | 737,885 B | 60,715 | 1,264 | `tootr__app.json` (63 MB) |
| **`gta6-leonida-atlas`** | GTA 6 Leonida 地图与 3D 街景 (生产构建) | ES Modules | 601 KB (assets) | - | - | `bundles/gta6-leonida-atlas/` |
| **`Ocarina-Atlas`** | 《时之笛》多楼层交互地图 (`src/lib/*`) | TypeScript 源码 | - | - | - | `notes/local_runs.log` |

---

## 二、 GTA 6 线上闭源地图 `stateofleonida`（`solmain2809_4.js`）深度逆向

通过 REA 解析 `bundles/stateofleonida/solmain2809_4.js`（SHA-256: `f62c7c0d7c384d6ac3f0b13101277f134989e01cdd1e8d35af264602906e779f`）及 `gta6state.html`，完整提取了该闭源地图的后端接口契约与隐藏管理路由：

1. **隐藏的 PHP 后端 API 端点（`auth/api.php?action=...`）**：
   - **公开与玩家交互接口**：`access_config`、`access_verify`、`gtadb_source_public`（与 `map.gtadb.org` 联动）、`site_settings_public`、`ranks`、`notifications&limit=40`、`notifications_read`、`marker_list&type=`、`marker_save`、`marker_update`、`marker_delete`、`game_stats&game=screenshot_challenge`、`leaderboard&game=screenshot_challenge&limit=10`。
   - **隐藏的管理后台（Admin）接口（共 23 个）**：`admin_community_access_save`、`admin_credits`、`admin_credits_save`、`admin_db_export&id=`、`admin_db_integrity&id=`、`admin_db_overview`、`admin_gtadb_save`、`admin_gtadb_scheduled`、`admin_gtadb_status`、`admin_gtadb_sync`、`admin_gtadb_sync_status`、`admin_news`、`admin_news_save`、`admin_ranks`、`admin_seo`、`admin_seo_save`、`admin_site_settings`、`admin_site_settings_save`、`admin_stats`、`admin_user_delete`、`admin_user_details&id=`、`admin_user_set_rank`、`admin_users`。
2. **静态 JSON 数据集矩阵（`data/*.json`）**：
   - 共发现 17 个结构化数据源：`data/backgrounds.json`、`data/content.json`、`data/credits.json`、`data/database_changelog.json`、`data/el_screenshots.json`、`data/en-emojibase-data.json`、`data/lang.json`、`data/leaks.json`、`data/markers.json`、`data/news.json`、`data/screenshots.json`、`data/seo.json`、`data/supporters.json`、`data/trailer1.json`、`data/trailer2.json`、`data/trailer3.json`、`data/wiki-cache.json`。
3. **外部生态依赖**：集成 `https://map.gtadb.org/photos`、`https://gta.wiki/api.php`、`https://earth.google.com/web/`（现实地标对比）及 YouTube/Vimeo/Spotify 媒体嵌入。

---

## 三、 `gta6-leonida-atlas` 生产 Bundle 反混淆与坐标/路由还原

对 `bundles/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js` 与 `public-path-DoqNwZJ4.js` 进行反混淆重构，还原了其精确的二维/三维坐标变换管线：
- **标准世界边界（`CANONICAL_BOUNDS`）**：`west: -16000, east: 4000, south: -8000, north: 12000`（`20,000 × 20,000` 世界单位）。
- **SVG / 3D 视图空间映射**：`worldToSvg(e)` 执行 `{ x: e.east * 2, z: e.north * -2 }`，`svgToWorld(e)` 执行逆变换 `{ x: e.x / 2, y: e.z / -2 }`，从而使 SVG 坐标跨度对齐为 `40,000 × 40,000`。
- **3D 探索器路由构造**：`build3dExplorerUrl(placeId)` 通过 `?view=3d&place=<placeId>` 切换至 Three.js 街景模式。

---

## 四、 《时之笛》生态（`noclip.website`、`tootr`、`Ocarina-Atlas`）逆向对比

1. **`noclip.website`（933 模块，264.7 万 AST 节点）**：
   - 从 `bundles/noclip.website/static/js/8.15df99b7.js` 中提取出完整的 `oot3d` 场景注册路径（如 `oot3d/bdan/1`、`oot3d/bmori1/1`、`oot3d/ddan/1`、`oot3d/entra_day/1`、`oot3d/entra_night/1`、`oot3d/entra_ruins/1`、`oot3d/ganon/1`、`oot3d/daiyousei_izumi/1` 等），底层通过自研 WebGL2 `GfxDevice` 抽象层与 `Zelda` 渲染管线按需流式加载场景二进制。
2. **`tootr`（57 模块，6.07 万 AST 节点）**：
   - 相比 `noclip.website` 的自研 WebGL2 管线，`tootr` 采用 React + Babel 运行时生态，专注于《时之笛》场景与过场动画（Cutscene）的轻量化浏览器解析。
3. **`Ocarina-Atlas`（`src/lib/coords.ts` & `src/lib/floorGrid.ts`）**：
   - 采用**归一化图像分数坐标系（`Fraction ∈ [0, 1]`）**与多楼层等距网格布局（`CELL_WIDTH = 1`, `CELL_HEIGHT = 0.75`, `GAP = 0.06`, `LABEL_HEIGHT = 0.08`），通过 `fractionToBox` / `stageBox` / `clampView` 在 `MIN_ZOOM = 1` 到 `MAX_ZOOM = 10` 之间实现无失真缩放与多楼层地牢（Dungeon）坐标统一映射。

---

## 五、 TypeScript 源码级重构产出清单

所有重构代码已写入 `/root/wsl-demo/rea-reconstructions/web-and-js/reconstructions/`：
1. `stateofleonida/StateOfLeonida_ApiAndMapClient.ts`：完整还原 36 个 `auth/api.php` 公开与隐藏管理 Action、17 个 `data/*.json` 常量表与请求封装。
2. `gta6-leonida-atlas/LeonidaAtlas_CoordsAndRoutes.ts`：反混淆还原 `CANONICAL_BOUNDS`、`worldToSvg` / `svgToWorld`、`SVG_BOUNDS_XZ` 与 `build3dExplorerUrl`。
3. `ocarina-atlas/OcarinaAtlas_StageProjection.ts`：还原归一化分数坐标投影与视口缩放约束逻辑。
4. `noclip-and-tootr/NoclipAndTootr_SceneRegistry.ts`：还原 `oot3d` 场景路径注册表与接口契约。
