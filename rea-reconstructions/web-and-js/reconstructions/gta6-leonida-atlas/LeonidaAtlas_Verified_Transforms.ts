/**
 * gta6-leonida-atlas — 坐标与路由的**核验版**还原
 *
 * 本文件与同目录 `LeonidaAtlas_CoordsAndRoutes.ts` 并存而非覆盖：后者的 `worldToSvg`、
 * `svgToWorld`、`svgToPlanar`、`geoToSvg`、`SVG_BOUNDS_XZ`、`SVG_BOUNDS_XY`、
 * `normalizePoint`、`build3dExplorerUrl`、`resolvePublicPath` 在下列真实产物中均为 0 命中
 * （`bundles/gta6-leonida-atlas/assets/*.js`、`recovered/gta6-leonida-atlas/modules/`、
 * `gta-projects/gta6-leonida-atlas/src/`），属于重构时的自造命名。
 * 此处按证据逐字节复述真实符号与来源位置。
 *
 * 证据：
 *   evidence/gta6-leonida-atlas__app.json            ev_136c2b50…  （源码树图，15 endpoint）
 *   evidence/leonida_source_coords__app.json         ev_32825cf5…  （坐标模块子图，38 节点）
 *   evidence/gta6-leonida-atlas-bundle__app.json     ev_7816f81f…  （线上生产包，48 节点 / 0 endpoint）
 *   notes/trace_leonida_coordinates_incoming.json    ev_cac3e667…  （入向追踪，4 节点 / 3 边）
 *   notes/compare_gta6_src_to_bundle.json            ev_7abd6455…  （167 文件全部 unknown，生产包无 source map）
 */

// ---------------------------------------------------------------------------
// src/features/street-leonida/leonida-coordinates.ts
// 与本地副本 local/leonida-source-coords/leonida-coordinates.ts 逐字节一致（diff 无输出）
// ---------------------------------------------------------------------------

/** :2 GtadbPoint{x,y} —— gtadb.org 原生单位 */
export interface GtadbPoint { x: number; y: number }
/** :7 CanonicalPoint{east,north} */
export interface CanonicalPoint { east: number; north: number }
/** :12 WorldPoint{x,z} —— three.js 世界坐标（米） */
export interface WorldPoint { x: number; z: number }
/** :18 MapPoint{x,y} —— SVG/地图平面坐标；:17 注释明确 "SVG map Y is deliberately the same value as Three.js world Z" */
export interface MapPoint { x: number; y: number }
/** :23 */
export type GtadbPointInput = GtadbPoint | readonly [x: number, y: number];

/** :26 —— 每个 gtadb.org 单位 = 2 世界米 */
export const WORLD_METRES_PER_GTADB_UNIT = 2 as const;

/** :28-33 —— 跨度恰为 20,000 × 20,000 canonical 单位 */
export const CANONICAL_BOUNDS = Object.freeze({
  west: -16000,
  east: 4000,
  south: -8000,
  north: 12000,
});

/** :35-38 */
export function gtadbToCanonical(point: GtadbPointInput): CanonicalPoint {
  if ('x' in point) return { east: point.x, north: point.y };
  return { east: point[0], north: point[1] };
}

/**
 * :40-45 真实"geo → world"变换。
 * 注意：先前版本把该函数体命名为 `worldToSvg`，但它产出的是 **WorldPoint**（three.js），
 * 不是 SVG 点。
 */
export function canonicalToWorld(point: CanonicalPoint): WorldPoint {
  return {
    x: point.east * WORLD_METRES_PER_GTADB_UNIT,
    z: point.north * -WORLD_METRES_PER_GTADB_UNIT,
  };
}

/** :47-49 */
export function gtadbToWorld(point: GtadbPointInput): WorldPoint {
  return canonicalToWorld(gtadbToCanonical(point));
}

/**
 * :51-56 world → gtadb.org 单位的**逆映射**（含 ÷2 与 Z 取反）。
 * 先前版本把它命名为 `svgToWorld`，造成"SVG ↔ 世界"配对错觉：真实的 SVG 配对是下面两个函数。
 */
export function worldToGtadb(point: WorldPoint): GtadbPoint {
  return {
    x: point.x / WORLD_METRES_PER_GTADB_UNIT,
    y: point.z / -WORLD_METRES_PER_GTADB_UNIT,
  };
}

/** :58-60 world → SVG/地图：**纯轴重命名，无任何缩放** */
export function worldToMap(point: WorldPoint): MapPoint {
  return { x: point.x, y: point.z };
}

/** :62-64 SVG/地图 → world：`worldToMap` 的逆，同样无缩放 */
export function mapToWorld(point: MapPoint): WorldPoint {
  return { x: point.x, z: point.y };
}

/** :66 / :72 / :79 距离换算三件套 */
export function gtadbDistance(first: GtadbPointInput, second: GtadbPointInput): number {
  const a = gtadbToCanonical(first);
  const b = gtadbToCanonical(second);
  return Math.hypot(b.east - a.east, b.north - a.north);
}
/** :72-77 —— 注意入参仍是**两点**，而非已算好的距离 */
export function gtadbDistanceToWorldMetres(
  first: GtadbPointInput,
  second: GtadbPointInput,
): number {
  return gtadbDistance(first, second) * WORLD_METRES_PER_GTADB_UNIT;
}
export function worldDistance(first: WorldPoint, second: WorldPoint): number {
  return Math.hypot(second.x - first.x, second.z - first.z);
}

/**
 * :92 与 :101 —— 由 CANONICAL_BOUNDS × 2 导出，源文件先算两个角点（:84-91）再冻结边界。
 * 跨度为 40,000 × 40,000 米（先前版本一度写作 64,000 × 40,000，算术上不成立：
 * east−west = 4000−(−16000) = 20000 → ×2 = 40000；north−south = 12000−(−8000) = 20000 → ×2 = 40000）。
 */
const northWestWorld = canonicalToWorld({
  east: CANONICAL_BOUNDS.west,
  north: CANONICAL_BOUNDS.north,
});
const southEastWorld = canonicalToWorld({
  east: CANONICAL_BOUNDS.east,
  north: CANONICAL_BOUNDS.south,
});

export const WORLD_BOUNDS = Object.freeze({
  minX: northWestWorld.x, // −32000
  maxX: southEastWorld.x, //   8000
  minZ: northWestWorld.z, // −24000
  maxZ: southEastWorld.z, //  16000
  width: southEastWorld.x - northWestWorld.x, // 40000
  height: southEastWorld.z - northWestWorld.z, // 40000
});
export const MAP_BOUNDS = Object.freeze({
  minX: WORLD_BOUNDS.minX,
  maxX: WORLD_BOUNDS.maxX,
  minY: WORLD_BOUNDS.minZ,
  maxY: WORLD_BOUNDS.maxZ,
  width: WORLD_BOUNDS.width,
  height: WORLD_BOUNDS.height,
});

/** :110-112 向后兼容别名（真实存在，非命名虚构） */
export const STATE_CANONICAL_BOUNDS = CANONICAL_BOUNDS;
export const STATE_WORLD_BOUNDS = WORLD_BOUNDS;
export const STATE_MAP_BOUNDS = MAP_BOUNDS;

// ---------------------------------------------------------------------------
// src/features/explorer/public-path.ts —— 3D 探索器链接的真实构造者
// ---------------------------------------------------------------------------

/** public-path.ts:2-5 基路径归一化（读取 import.meta.env.BASE_URL，兼容 fork 部署） */
export function publicPath(
  path: string,
  base: string = import.meta.env?.BASE_URL ?? '/',
): string {
  const prefix = `/${base.replace(/^\/+|\/+$/g, '')}`.replace(/\/$/, '');
  return `${prefix}/${path.replace(/^\/+/, '')}`;
}

/** public-path.ts:7-9 */
export function projectPath(page: string): string {
  return publicPath(encodeURIComponent(page));
}

/**
 * public-path.ts:12-16 —— 先前版本命名为 `build3dExplorerUrl`；真实符号是 `explorerPath`，
 * 语义一致（`?view=3d` 且可选 `place`），注释说明其目的是"在可重载链接中保留地图选择，
 * 且不暴露本地 marker 内容"。
 */
export function explorerPath(placeId?: string | null, base?: string): string {
  const query = new URLSearchParams({ view: '3d' });
  if (placeId) query.set('place', placeId);
  return `${publicPath('', base)}?${query}`;
}

// ---------------------------------------------------------------------------
// src/app/routes.ts:9-33 —— 视图判定与遗留路径兼容（先前版本未记录）
// ---------------------------------------------------------------------------

export type AtlasView = 'map' | 'explorer' | 'project';

const projectPages = new Set([
  'about',
  'documentation',
  'credits',
  'contributing',
  'changelog',
  'licenses',
]);

export function resolveRoute(
  pathname: string,
  search: string,
): { view: AtlasView; page: string; placeId: string | null } {
  const segments = pathname.split('/').filter(Boolean);
  const params = new URLSearchParams(search);
  const tail = params.get('page') ?? segments.at(-1) ?? '';
  const placeIndex = segments.indexOf('place');
  const viewpointIndex = segments.indexOf('viewpoint');
  const oldSlug =
    placeIndex >= 0
      ? segments[placeIndex + 1]
      : viewpointIndex >= 0
        ? segments[viewpointIndex + 1]?.replace(/-regional-entry$/, '')
        : null;
  return {
    view: params.get('view') === '3d' ? 'explorer' : projectPages.has(tail) ? 'project' : 'map',
    page: tail,
    placeId: params.get('place') ?? (oldSlug ? `region:${oldSlug}` : null),
  };
}

/**
 * src/features/project/static-page.tsx:54 —— 落地页真实链接（注意目录是 features/project，
 * 与坐标模块所在的 features/street-leonida 不同）：
 *   <a href={`${publicPath("")}?view=3d`}>Open the optional 3D explorer</a>
 */

/**
 * 地标数据源（reconstructions/08_gta6_leonida_atlas_endpoints.csv 行内观测）：
 *   构建产物内：`${base.endsWith('/') ? base : `${base}/`}assets/street-leonida/maps/gtadb-landmarks-5c1d69f.json`
 *   上游固定快照：https://raw.githubusercontent.com/rolux/gtadb.org/5c1d69ff864623d181da5457afaacd00b623e1a4/map/data/6/landmarks.json
 */
