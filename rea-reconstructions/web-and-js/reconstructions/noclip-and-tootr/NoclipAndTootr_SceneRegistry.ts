// Reconstructed from REA static application graph evidence:
//   noclip-website__app.json: 933 parsed files, 2,647,300 AST nodes, 10,072 findings
//   tootr__app.json:          57 parsed files, 60,715 AST nodes, 1,264 findings

export interface NoclipSceneDesc {
  id: string;
  name: string;
  createScene(device: unknown, context: unknown): Promise<unknown>;
}

// Observed OoT 3D scene path identifiers in noclip.website static bundle (8.15df99b7.js)
export const NOCLIP_OOT3D_SCENE_PATHS = [
  'oot3d/bdan/1',           // Inside Jabu-Jabu's Belly
  'oot3d/bdan_boss/1',      // Barinade Boss Room
  'oot3d/bmori1/1',         // Forest Temple
  'oot3d/ddan/1',           // Dodongo's Cavern
  'oot3d/ddan_boss/1',      // King Dodongo Boss Room
  'oot3d/entra_day/1',      // Hyrule Castle Town Market (Day)
  'oot3d/entra_night/1',    // Hyrule Castle Town Market (Night)
  'oot3d/entra_ruins/1',    // Market Ruins
  'oot3d/fire_bs/1',        // Volvagia Boss Room
  'oot3d/ganon/1',          // Ganon's Tower
  'oot3d/ganon_boss/1',     // Ganondorf Boss Room
  'oot3d/ganon_demo/1',     // Ganon's Castle Exterior
  'oot3d/daiyousei_izumi/1',// Great Fairy's Fountain
  'oot3d/bowling/1',        // Bombchu Bowling Alley
] as const;

export type Oot3dScenePath = (typeof NOCLIP_OOT3D_SCENE_PATHS)[number];
