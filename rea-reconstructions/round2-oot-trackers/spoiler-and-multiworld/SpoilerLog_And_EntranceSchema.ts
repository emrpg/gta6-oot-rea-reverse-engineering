// Reconstructed from spoilizer REA analysis (spoilizer__app.json, src/utils/parseLog.ts, src/schema/entrance.ts)
// and ootr-multiworld (crate/multiworld/src/ws/mod.rs)

export type EntranceTarget = string | { region: string; from: string };

export interface OotrSingleSpoilerLog {
  ':seed': string;
  ':settings_string': string;
  settings: Record<string, unknown>;
  locations: Record<string, unknown>;
  ':woth_locations': Record<string, unknown>;
  ':playthrough': Record<string, unknown>;
  entrances: Record<string, EntranceTarget>;
  ':entrance_playthrough': Record<string, unknown>;
}

export interface OotrMultiworldSpoilerLog {
  ':seed': string;
  ':settings_string': string;
  settings: Record<string, unknown>;
  locations: Record<string, Record<string, unknown>>;
  ':woth_locations': Record<string, Record<string, unknown>>;
  ':playthrough': Record<string, unknown>;
  entrances: Record<string, Record<string, EntranceTarget>>;
  ':entrance_playthrough': Record<string, unknown>;
}

export enum OotrMultiworldServerErrorCode {
  WrongPassword = 1,
  RoomExists = 2,
  // Discriminant >= 3 maps to Future(u8) in async_proto
}
