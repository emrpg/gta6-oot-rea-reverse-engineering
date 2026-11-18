# 生产包坐标模块的逐符号反混淆对照表

**对象**：`bundles/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js`（586 B，单行）
**参照**：`gta-projects/gta6-leonida-atlas/src/features/street-leonida/leonida-coordinates.ts`（112 行）
**方法声明**：`[推断]`——本表由人工将最小化 AST 与源码逐式对齐得到，REA 在该 chunk 的图中未产出命名符号
（`evidence/gta6-leonida-atlas-bundle__app.json` `ev_7816f81f…`：48 节点 / **0 endpoint** / 5 解析文件 / 602,622 B）。
标识符在生产包中**全部消失**（对 `CANONICAL_BOUNDS`、`canonicalToWorld`、`worldToGtadb`、`worldToMap`、
`mapToWorld`、`WORLD_BOUNDS`、`MAP_BOUNDS`、`WORLD_METRES_PER_GTADB_UNIT`、`explorerPath`、`publicPath`
逐词 `grep -r` 结果均为 bundle=0 / recovered=0 / 仅 src 命中），因此下表以**结构与数值**为配对依据。

## 最小化名 ↔ 真实符号

| 生产包 | 生产包体 | 真实符号 | 源码位置 | 一致性依据 |
| :--- | :--- | :--- | :--- | :--- |
| `e` | `Object.freeze({west:-16e3,east:4e3,south:-8e3,north:12e3})` | `CANONICAL_BOUNDS` | `:28-33` | 四个数值逐字对应（`16e3` = 16000 指数写法，故 `-16000` 字面 grep 为 0 命中） |
| `t(p)` | `` `x`in p?{east:p.x,north:p.y}:{east:p[0],north:p[1]} `` | `gtadbToCanonical` | `:35-38` | 判别属性 `x` 与两种返回形态完全一致 |
| `n(p)` | `{x:p.east*2,z:p.north*-2}` | `canonicalToWorld` | `:40-45` | 系数 `2` 即 `WORLD_METRES_PER_GTADB_UNIT`（`:26`），Z 取反一致 |
| `r(p)` | `n(t(p))` | `gtadbToWorld` | `:47-49` | 复合顺序一致 |
| `i(p)` | `{x:p.x/2,y:p.z/-2}` | `worldToGtadb` | `:51-56` | 与 `n` 互为逆（÷2 与 Z 取反），**不是** SVG 逆变换 |
| `a(p)` | `{x:p.x,y:p.z}` | `worldToMap` | `:58-60` | 无缩放的轴重命名，印证 `:17` 注释 |
| — | （map→world 未出现在 chunk 中） | `mapToWorld` | `:62-64` | `[未知]` 被 tree-shaking 移除或未使用，不能推断其不存在于源码 |
| `o` | `n({east:e.west,north:e.north})` | `northWestWorld` | `:84-87` | 由 `CANONICAL_BOUNDS` 西北角构造 |
| `s` | `n({east:e.east,north:e.south})` | `southEastWorld` | `:88-91` | 东南角 |
| `c` | `Object.freeze({minX:o.x,maxX:s.x,minZ:o.z,maxZ:s.z,width:s.x-o.x,height:s.z-o.z})` | `WORLD_BOUNDS` | `:92-99` | 六字段顺序与表达式一致 ⇒ 实际值 `−32000, 8000, −24000, 16000, 40000, 40000` |
| `l` | `Object.freeze({minX:c.minX,…,height:c.height})` | `MAP_BOUNDS` | `:101-108` | `minY/minMax` 由 `minZ/maxZ` 改名，正是 SVG Y ≡ world Z 的体现 |
| — | 距离三件套未出现在 chunk 中 | `gtadbDistance` / `gtadbDistanceToWorldMetres` / `worldDistance` | `:66-82` | `[未知]` 同上（未被该入口引用） |

## 导出别名表（chunk 尾部）

```js
export{i as a, r as i, l as n, a as o, c as r, e as t};
```

即外部 chunk 看到的是：`a = worldToGtadb`、`i = gtadbToWorld`、`n = MAP_BOUNDS`、
`o = worldToMap`、`r = WORLD_BOUNDS`、`t = CANONICAL_BOUNDS`。
注意别名与被别名符号自身名称冲突（同一 chunk 内 `r as i`、`i as a` 交叉），
任何按名字检索导出符号的下游分析都会得到错误配对——这是"命名复原"必须依赖结构而非字符串的直接理由。

## 对 `STATE_*` 别名的说明

源码 `:110-112` 的 `STATE_CANONICAL_BOUNDS` / `STATE_WORLD_BOUNDS` / `STATE_MAP_BOUNDS` 未在 chunk 中出现
`[未知]`：该入口未引用它们，或其在构建中被消除。它们在源码侧真实存在（`grep` 命中同一文件），
故不可据此认为向后兼容层已移除。

## 同批 chunk：`public-path-DoqNwZJ4.js`（300 B）

```js
function e(e,t=`/gta6-leonida-atlas/`){return`${`/${t.replace(/^\/+|\/+$/g,``)}`.replace(/\/$`,``)}/${e.replace(/^\/+/,``)}`}
function t(t){return e(encodeURIComponent(t))}
function n(t,n){let r=new URLSearchParams({view:`3d`});return t&&r.set(`place`,t),`${e(``,n)}?${r}`}
export{t as n,e as r,n as t};
```

| 最小化名 | 真实符号 | 源码位置 | 结构依据 |
| :--- | :--- | :--- | :--- |
| `e` | `publicPath` | `public-path.ts:2-5` | 两次 `replace` 链与返回模板逐字符一致 |
| `t` | `projectPath` | `:7-9` | `e(encodeURIComponent(t))` 复合一致 |
| `n` | `explorerPath` | `:12-16` | `new URLSearchParams({view:'3d'})` + 条件 `set('place',·)` + `` `${e(``,n)}?${r}` `` 三步一致 |

**构建期差异（真实发现）**：源码 `publicPath` 的默认参数是 `import.meta.env?.BASE_URL ?? '/'`，
生产 chunk 中已被内联为字面量 `` `/gta6-leonida-atlas/` ``。
说明该构建的 `BASE_URL` 被固化为 `/gta6-leonida-atlas/`，与 `gta6state.com/gta6-leonida-atlas/…` 的部署前缀相符
（`live/observed.tsv:10-11`）。
