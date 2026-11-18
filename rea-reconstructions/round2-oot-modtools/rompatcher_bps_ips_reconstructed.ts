// Reconstructed TypeScript interface for RomPatcher.js (BPS1 & IPS32/IPS Patcher)
// Evidence source:
//   - RomPatcher.js/rom-patcher-js/modules/RomPatcher.format.bps.js
//   - rompatcher_js__analyze.json

export const enum BpsAction {
  SourceRead = 0,
  TargetRead = 1,
  SourceCopy = 2,
  TargetCopy = 3,
}

export interface BpsPatch {
  sourceSize: number;
  targetSize: number;
  metadata: string;
  sourceCrc32: number;
  targetCrc32: number;
  patchCrc32: number;
}

export function decodeVarInt(buf: Uint8Array, cursor: { pos: number }): number {
  let data = 0;
  let shift = 1;
  while (cursor.pos < buf.length) {
    const x = buf[cursor.pos++];
    data += (x & 0x7f) * shift;
    if (x & 0x80) break;
    shift <<= 7;
    data += shift;
  }
  return data;
}

export function applyBpsPatch(source: Uint8Array, patch: Uint8Array): Uint8Array {
  if (
    patch.length < 16 ||
    patch[0] !== 0x42 || // 'B'
    patch[1] !== 0x50 || // 'P'
    patch[2] !== 0x53 || // 'S'
    patch[3] !== 0x31    // '1'
  ) {
    throw new Error("Invalid BPS1 magic header");
  }

  const cur = { pos: 4 };
  const sourceSize = decodeVarInt(patch, cur);
  const targetSize = decodeVarInt(patch, cur);
  const metaLen = decodeVarInt(patch, cur);
  cur.pos += metaLen;

  if (source.length < sourceSize) {
    throw new Error(`Source size mismatch: expected ${sourceSize}, got ${source.length}`);
  }

  const target = new Uint8Array(targetSize);
  let outPos = 0;
  let srcRel = 0;
  let dstRel = 0;
  const actionEnd = patch.length - 12;

  while (cur.pos < actionEnd) {
    const cmd = decodeVarInt(patch, cur);
    const action = (cmd & 3) as BpsAction;
    const length = (cmd >> 2) + 1;

    if (action === BpsAction.SourceRead) {
      target.set(source.subarray(outPos, outPos + length), outPos);
      outPos += length;
    } else if (action === BpsAction.TargetRead) {
      target.set(patch.subarray(cur.pos, cur.pos + length), outPos);
      cur.pos += length;
      outPos += length;
    } else if (action === BpsAction.SourceCopy) {
      const d = decodeVarInt(patch, cur);
      srcRel += (d & 1 ? -1 : 1) * (d >> 1);
      target.set(source.subarray(srcRel, srcRel + length), outPos);
      srcRel += length;
      outPos += length;
    } else if (action === BpsAction.TargetCopy) {
      const d = decodeVarInt(patch, cur);
      dstRel += (d & 1 ? -1 : 1) * (d >> 1);
      for (let i = 0; i < length; i++) {
        target[outPos++] = target[dstRel++];
      }
    }
  }
  return target;
}
