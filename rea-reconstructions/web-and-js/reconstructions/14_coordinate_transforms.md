# 坐标与投影变换：源码 ↔ 线上打包产物

## A. GTA 6 Leonida：GTADB ↔ canonical ↔ world ↔ map

源码：`/root/wsl-demo/gta-projects/gta6-leonida-atlas/src/features/street-leonida/leonida-coordinates.ts`
线上产物：`https://gta6state.com/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js`
（586 字节，本次抓取原始 SHA-256 `c65bc30c…`，与线上当前字节完全一致）
还原产物：`../recovered/gta6-leonida-atlas/modules/leonida-coordinates-CXGuqbYF.js`
图谱：`../evidence/gta6-leonida-atlas__app.json`（源码，`ev_136c2b5082f8981572ce…`）、
`../evidence/gta6-leonida-atlas_recovered__app.json`（产物，`ev_aabf94a72f4e489872d9…`）

| 变换 | 源码位置 | 还原后的产物形式 | 语义 |
|---|---|---|---|
| 单位比例 | `:26` `WORLD_METRES_PER_GTADB_UNIT = 2` | `e.east*2`、`e.north*-2` | 1 GTADB 单位 = 2 世界米（社区标定，非官方测量） |
| GTADB→canonical | `:35` `gtadbToCanonical` | 对象/元组双形态入口 | `{x,y} → {east,north}` |
| canonical→world | `:40` `canonicalToWorld` | `n(e){return{x:e.east*2,z:e.north*-2}}` | 北向取负 ⇒ Three.js `+Z` 朝南 |
| GTADB→world | `:47` `gtadbToWorld` | 组合调用 | 两步复合 |
| world→GTADB | `:51` `worldToGtadb` | 除以 2、`z/-2` | 逆变换 |
| world↔map | `:58` / `:62` | `mapToWorld`/`worldToMap` | SVG map Y 与 world Z 同值（`:17` 注释） |
| 距离 | `:66`、`:72`、`:79` | `Math.hypot` | canonical 距离 × 2 = 米 |
| canonical 边界 | `:28-33` west −16000 / east 4000 / south −8000 / north 12000 | `Object.freeze({west:-16e3,east:4e3,south:-8e3,north:12e3})` | 20,000 × 20,000 单位 |
| world 边界 | `:92` `WORLD_BOUNDS` | 由两角点派生 | `minX −32000 … maxX 8000`、`minZ −24000 … maxZ 16000`，即 `width = height = 40000` 米 |
| map 边界 | `:101` `MAP_BOUNDS` | 与 world 同值别名 | — |
| 州级别名 | `:110-112` | `STATE_*_BOUNDS` | Leonida 即当前唯一州 |

数据溯源（`gtadb-mx_UyCIg.js` 与 `scripts/sync-gtadb-landmarks.mjs:118`）：地标 JSON 钉在
`https://raw.githubusercontent.com/rolux/gtadb.org/5c1d69ff864623d181da5457afaacd00b623e1a4/map/data/6/landmarks.json`
（CC BY 4.0），发布产物内的读取路径为
`` `${base…}/assets/street-leonida/maps/gtadb-landmarks-5c1d69f.json` ``（`src/data/catalogue.ts:117`）。
不确定性标记（`unconfirmed`、`may-not-exist`、`cancelled`、`fictional`、`demolished`、`duplicate`）
随数据一并打包，见 `../reconstructions/gta6-leonida-atlas/`。

## B. 《时之笛》Ocarina-Atlas：fraction ↔ box ↔ stage ↔ viewport

源码：`/root/wsl-demo/oot-peripherals/Ocarina-Atlas/src/lib/coords.ts`（107 行）
Evidence：`../evidence/Ocarina-Atlas__app.json`
可运行还原件：`OcarinaAtlas_StageProjection.ts`（本目录）

| 函数 | 位置 | 契约 |
|---|---|---|
| `fractionToBox(f, box)` | `coords.ts:11-16` | 数据集分数 → 盒内像素；分数相对图像，原点左上、`y` 向下 |
| `boxToFraction(p, box)` | `:19-24` | 逆运算 |
| `pinPosition(f)` | `:27-30` | 以 `{left,top}` 百分比返回 CSS 偏移，使图钉随缩放/尺寸同步（内部用 100×100 参考盒） |
| `isInsideImage(f)` | `:32-34` | 闭区间 `[0,1]²` |
| `fitSize(container, aspect)` | `:43-46` | `width = min(cw, ch·aspect)`，高度等比 |
| `stageBox(view, container, aspect)` | `:49-59` | `left = cw/2 − cx·(fit·zoom)`、`top = ch/2 − cy·(fit·zoom)` |
| `clampAxis` | `:61-65` | 舞台小于容器时居中（返回 0.5），否则边距 `container/(2·stage)` |
| `clampView` | `:67-75` | 缩放夹在 `MIN_ZOOM=1` … `MAX_ZOOM=10`（`:40-41`）后夹轴 |
| `panView` | `:77-84` | 位移除以舞台像素 ⇒ 分数位移，再夹 |
| `zoomViewAt` | `:87-106` | 保持锚点下图像点不动：先 `boxToFraction` 取锚点下分数，再以新舞台尺寸反解中心 |

`INITIAL_VIEW = {cx:0.5, cy:0.5, zoom:1}`（`:39`）。整套数学只依赖 `(container, aspect, zoom, cx, cy)`，
因此同一 `Fraction` 在地图舞台、全屏舞台与导出图上必然一致；OoT 的多层/地牢坐标建在
`src/lib/floorGrid.ts` 之上（分数相对底图，见本目录还原件的注释）。

## C. 两条链路的方法差异

- Leonida 侧可用**产物→源码**的字节级对照（同一 586 字节 chunk 与 `.ts` 源逐式核对），
  但仓库产物**不含 `.map`**，所以 REA `compare-source-to-bundle` 只能报 `unknown`（见 `15_…`）。
- Ocarina-Atlas 侧直接有 TypeScript 源；本次以源码图谱为准，`coords.ts` 的分数数学没有线上混淆层。
