// Reconstructed from Z5Client REA analysis (Z5Client__app.json) & lua/ootMulti.lua
// Electron: 2 BrowserWindows, 4 contextBridge APIs, 17 exposed members, 15 main IPC handlers

export const OOT_NTSC_10_RAM = {
  SAVE_CONTEXT_OFFSET: 0x11a5d0,
  EQUIPMENT_OFFSET: 0x11a640,        // +0x70
  SCENE_FLAGS_OFFSET: 0x11a6a4,      // +0xD4 (stride 0x1C per scene)
  SCENE_STRIDE: 0x1c,
  SCENE_CHEST_OFFSET: 0x0,
  SCENE_SWITCH_OFFSET: 0x4,
  SCENE_COLLECTIBLE_OFFSET: 0xc,
  SHOP_CONTEXT_OFFSET: 0x11ab84,     // +0x5B4
  SKULLTULA_FLAGS_OFFSET: 0x11b46c,  // +0xE9C
  BIG_POE_POINTS_OFFSET: 0x11b48c,   // +0xEBC
  FISHING_CONTEXT_OFFSET: 0x11b490,  // +0xEC0
  EVENT_CONTEXT_OFFSET: 0x11b4a4,    // +0xED4
  ITEM_GET_INF_OFFSET: 0x11b4c0,     // +0xEF0
  INF_TABLE_OFFSET: 0x11b4c8,        // +0xEF8
  TEMP_MULTIWORLD_CTX: 0x40002c,     // 4-byte remote item check buffer
} as const;

export enum MultiworldLocationType {
  Chest = 0x01,
  Freestanding = 0x02,
}

export function decodeTempMultiworldContext(bytes: Uint8Array): {
  isRemoteItem: boolean;
  sceneId: number;
  locationType: MultiworldLocationType;
  locationBit: number;
} | null {
  if (bytes.length < 4 || bytes[0] !== 0x00) return null;
  return {
    isRemoteItem: true,
    sceneId: bytes[1],
    locationType: bytes[2] as MultiworldLocationType,
    locationBit: bytes[3],
  };
}
