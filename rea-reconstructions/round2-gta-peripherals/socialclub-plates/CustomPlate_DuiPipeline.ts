// Reconstructed from Round 2 incremental scan:
//   MASRPNathan-G/customplates (CustomPlate/plate.lua)
// Models the FiveM DUI -> RuntimeTxd -> vehshare plate01..plate05 texture replacement pipeline.

export const VEHSHARE_PLATE_DIFFUSE_SLOTS = [
  'plate01', 'plate02', 'plate03', 'plate04', 'plate05',
] as const;

export const VEHSHARE_PLATE_NORMAL_SLOTS = [
  'plate01_n', 'plate02_n', 'plate03_n', 'plate04_n', 'plate05_n',
] as const;

export interface CustomPlateConfig {
  diffuseImageUrl: string;
  normalImageUrl: string;
  width: number;  // default 540
  height: number; // default 300
  txdName: string; // default 'duiTxd'
}

export function buildPlateReplacementPlan(cfg: CustomPlateConfig) {
  return {
    txdName: cfg.txdName,
    diffuseReplacements: VEHSHARE_PLATE_DIFFUSE_SLOTS.map((slot) => ({
      origTxd: 'vehshare',
      origTxn: slot,
      newTxd: cfg.txdName,
      newTxn: 'duiTex',
      sourceUrl: cfg.diffuseImageUrl,
    })),
    normalReplacements: VEHSHARE_PLATE_NORMAL_SLOTS.map((slot) => ({
      origTxd: 'vehshare',
      origTxn: slot,
      newTxd: cfg.txdName,
      newTxn: 'duiTex2',
      sourceUrl: cfg.normalImageUrl,
    })),
  };
}
