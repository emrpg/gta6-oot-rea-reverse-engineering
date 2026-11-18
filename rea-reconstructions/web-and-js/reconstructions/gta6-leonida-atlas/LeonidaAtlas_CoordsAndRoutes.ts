// Deobfuscated & reconstructed from gta6-leonida-atlas production bundle:
//   bundles/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js
//   bundles/gta6-leonida-atlas/assets/public-path-DoqNwZJ4.js

export interface GeoPoint2D {
  east: number;
  north: number;
}

export interface WorldPosition2D {
  x: number;
  y: number;
}

export interface SvgPoint3D {
  x: number;
  z: number;
}

export const CANONICAL_BOUNDS = Object.freeze({
  west: -16000,
  east: 4000,
  south: -8000,
  north: 12000,
});

export function normalizePoint(input: WorldPosition2D | [number, number]): GeoPoint2D {
  return 'x' in input
    ? { east: input.x, north: input.y }
    : { east: input[0], north: input[1] };
}

export function geoToSvg(pt: GeoPoint2D): SvgPoint3D {
  return { x: pt.east * 2, z: pt.north * -2 };
}

export function worldToSvg(input: WorldPosition2D | [number, number]): SvgPoint3D {
  return geoToSvg(normalizePoint(input));
}

export function svgToWorld(pt: SvgPoint3D): WorldPosition2D {
  return { x: pt.x / 2, y: pt.z / -2 };
}

export function svgToPlanar(pt: SvgPoint3D): WorldPosition2D {
  return { x: pt.x, y: pt.z };
}

const topLeft = geoToSvg({ east: CANONICAL_BOUNDS.west, north: CANONICAL_BOUNDS.north });
const bottomRight = geoToSvg({ east: CANONICAL_BOUNDS.east, north: CANONICAL_BOUNDS.south });

export const SVG_BOUNDS_XZ = Object.freeze({
  minX: topLeft.x,
  maxX: bottomRight.x,
  minZ: topLeft.z,
  maxZ: bottomRight.z,
  width: bottomRight.x - topLeft.x,
  height: bottomRight.z - topLeft.z,
});

export const SVG_BOUNDS_XY = Object.freeze({
  minX: SVG_BOUNDS_XZ.minX,
  maxX: SVG_BOUNDS_XZ.maxX,
  minY: SVG_BOUNDS_XZ.minZ,
  maxY: SVG_BOUNDS_XZ.maxZ,
  width: SVG_BOUNDS_XZ.width,
  height: SVG_BOUNDS_XZ.height,
});

export function resolvePublicPath(assetPath: string, basePath = '/gta6-leonida-atlas/'): string {
  const cleanBase = `/${basePath.replace(/^\/+|\/+$/g, '')}`.replace(/\/$/, '');
  const cleanAsset = assetPath.replace(/^\/+/, '');
  return `${cleanBase}/${cleanAsset}`;
}

export function build3dExplorerUrl(placeId?: string, basePath?: string): string {
  const params = new URLSearchParams({ view: '3d' });
  if (placeId) {
    params.set('place', placeId);
  }
  return `${resolvePublicPath('', basePath)}?${params.toString()}`;
}
