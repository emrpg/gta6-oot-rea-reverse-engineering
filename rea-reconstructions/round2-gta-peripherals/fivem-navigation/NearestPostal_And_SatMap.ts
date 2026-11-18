// Reconstructed from Round 2 incremental scan:
//   DevBlocky/nearest-postal (cl.lua, cl_render.lua)
//   Oulsen/oulsen_satmap (fxmanifest.lua, client.lua, oulsen_satmap_postals.json)

export interface PostalEntry {
  x: number;
  y: number;
  code: string;
}

export interface NearestPostalResult {
  code: string;
  dist: number;
}

export const RADAR_MINIMAP_TILES = [
  'minimap_0_0', 'minimap_0_1',
  'minimap_1_0', 'minimap_1_1',
  'minimap_2_0', 'minimap_2_1',
  'minimap_sea_0_0', 'minimap_sea_0_1',
  'minimap_sea_1_0', 'minimap_sea_1_1',
  'minimap_sea_2_0', 'minimap_sea_2_1',
] as const;

export const OULSEN_DEFAULT_RADAR_ZOOM = 1100;

export function findNearestPostal(
  playerX: number,
  playerY: number,
  postals: readonly PostalEntry[],
): NearestPostalResult | null {
  let bestCode: string | null = null;
  let bestDist = Infinity;
  for (const p of postals) {
    const dx = playerX - p.x;
    const dy = playerY - p.y;
    const d = Math.hypot(dx, dy);
    if (d < bestDist) {
      bestDist = d;
      bestCode = p.code;
    }
  }
  return bestCode !== null ? { code: bestCode, dist: bestDist } : null;
}

export function shouldClearPostalWaypoint(
  playerX: number,
  playerY: number,
  target: PostalEntry,
  deleteDist: number,
): boolean {
  return Math.hypot(playerX - target.x, playerY - target.y) < deleteDist;
}
