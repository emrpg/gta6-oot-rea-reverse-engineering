// Reconstructed TypeScript interface for OoT N64 SRAM (.sra/.srm) & zzrtl DMA Table
// Evidence source:
//   - oot-save-edit/models/savefile.ts & oot_save_edit__analyze.json
//   - audio-model/zzrtl/src/zzrtl.c & main_compress.c

export const OOT_SRAM_SIZE = 0x8000; // 32 KB
export const OOT_SLOT_SIZE = 0x1450; // 5200 bytes per save slot (checksum covers 0x1352 bytes)

export function swap32InPlace(buf: Uint8Array): Uint8Array {
  for (let i = 0; i + 3 < buf.length; i += 4) {
    const b0 = buf[i];
    const b1 = buf[i + 1];
    buf[i] = buf[i + 3];
    buf[i + 1] = buf[i + 2];
    buf[i + 2] = b1;
    buf[i + 3] = b0;
  }
  return buf;
}

export function computeOotSlotChecksum(slotBytes: Uint8Array): number {
  let sum = 0;
  // Standard OoT N64 save slot checksum over first 0x1352 bytes (16-bit big-endian words)
  for (let i = 0; i + 1 < 0x1352 && i + 1 < slotBytes.length; i += 2) {
    const word = (slotBytes[i] << 8) | slotBytes[i + 1];
    sum = (sum + word) & 0xffff;
  }
  return sum;
}

export interface DmaTableEntry {
  vromStart: number;
  vromEnd: number;
  romStart: number;
  romEnd: number;
}

export function parseDmaEntry(rom: Uint8Array, offset: number): DmaTableEntry {
  const view = new DataView(rom.buffer, rom.byteOffset + offset, 16);
  return {
    vromStart: view.getUint32(0, false),
    vromEnd: view.getUint32(4, false),
    romStart: view.getUint32(8, false),
    romEnd: view.getUint32(12, false),
  };
}
