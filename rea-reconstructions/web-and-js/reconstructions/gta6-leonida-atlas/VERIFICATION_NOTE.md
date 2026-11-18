# 核验说明：`LeonidaAtlas_CoordsAndRoutes.ts`

本目录同时存在两份还原文件，请勿合并：

| 文件 | 状态 |
| :--- | :--- |
| `LeonidaAtlas_CoordsAndRoutes.ts` | 工单早期产出（他人会话提交于 `bcb44ef`）。数学结论与 `CANONICAL_BOUNDS`、`?view=3d&place=` 语义正确，**但标识符为重构自造名** |
| `LeonidaAtlas_Verified_Transforms.ts` | 本次核验产出，逐符号对齐真实源码位置 |

## 自造符号命中数核验（方法：`grep -r`，非 REA 输出）

搜索范围：`bundles/gta6-leonida-atlas/assets/*.js`（线上生产包）、
`recovered/gta6-leonida-atlas/modules/`（Wakaru 还原产物）、
`/root/wsl-demo/gta-projects/gta6-leonida-atlas/src`（源码树）。

| `LeonidaAtlas_CoordsAndRoutes.ts` 中的符号 | bundle | recovered | src | 真实等价物 |
| :--- | ---: | ---: | ---: | :--- |
| `worldToSvg` | 0 | 0 | 0 | `canonicalToWorld`（`leonida-coordinates.ts:40-45`，输出 World 而非 SVG） |
| `svgToWorld` | 0 | 0 | 0 | `worldToGtadb`（`:51-56`，world→gtadb.org 单位）；SVG 逆变换实为 `mapToWorld`（`:62-64`） |
| `geoToSvg` / `svgToPlanar` | 0 | 0 | 0 | 无对应符号；`worldToMap`（`:58-60`）承担 world→SVG |
| `normalizePoint` | 0 | 0 | 0 | `gtadbToCanonical`（`:35-38`） |
| `SVG_BOUNDS_XZ` / `SVG_BOUNDS_XY` | 0 | 0 | 0 | `WORLD_BOUNDS`（`:92`）/ `MAP_BOUNDS`（`:101`） |
| `build3dExplorerUrl` | 0 | 0 | 0 | `explorerPath`（`public-path.ts:12-16`） |
| `resolvePublicPath` | 0 | 0 | 0 | `publicPath`（`public-path.ts:2-5`） |
| `CANONICAL_BOUNDS` | 0 | 0 | 是（`:28-33`） | 同名真实存在，但**生产包中只以数值出现**（`{west:-16e3,east:4e3,south:-8e3,north:12e3}`），名称已随最小化消失；见 `../17_minified_coordinate_chunk_symbol_map.md` |

## 一处配对错误（非命名问题）

原文件把 `{x: east*2, z: north*-2}` 命名为 "SVG 变换"、把 `{x: x/2, y: z/-2}` 命名为其 "SVG 逆变换"。
真实源码中前者是 **canonical→world**、后者是 **world→gtadb.org**；world↔SVG 的那一对是
`worldToMap`/`mapToWorld`，只做 `y ← z` 的轴重命名，**不含任何 `×2` / `÷2`**
（`leonida-coordinates.ts:17` 的注释即为设计意图："SVG map Y is deliberately the same value as Three.js world Z"）。
按原文件的配对关系做投影会得到缩放错误 2 倍的坐标，故不可直接复用。
