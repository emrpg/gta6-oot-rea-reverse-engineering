// Reconstructed from Tracker-of-Time (soh.vb)
// Ship of Harkinian (SoH) PC Port memory offset table across releases 3.0.0 -> 5.1.4
using System;

namespace Round2OotTrackers
{
    public sealed class SohVersionProfile
    {
        public int VersionCode { get; init; }
        public string BuildMarker { get; init; } = "";
        public long MarkerOffset { get; init; }
        public long SaveContextOffset { get; init; }
        public long SceneOffset { get; init; }
        public long RoomOffset { get; init; }
        public int SceneShift { get; init; }
    }

    public static class SohMemoryProfiles
    {
        public static readonly SohVersionProfile[] Profiles =
        {
            new() { VersionCode = 300, BuildMarker = "RACHAEL ALFA (3.0.0)",  MarkerOffset = 0xAECB78, SaveContextOffset = 0xEC8560, SceneOffset = 0xE4D878, RoomOffset = 0,        SceneShift = 0 },
            new() { VersionCode = 301, BuildMarker = "RACHAEL BRAVO (3.0.1)", MarkerOffset = 0xAECD38, SaveContextOffset = 0xECA260, SceneOffset = 0xE7D878, RoomOffset = 0,        SceneShift = 0 },
            new() { VersionCode = 402, BuildMarker = "ZHORA CHARLIE (4.0.2)", MarkerOffset = 0xAA7FE0, SaveContextOffset = 0xEC56A0, SceneOffset = 0xE492E8, RoomOffset = 0,        SceneShift = -0x40 },
            new() { VersionCode = 403, BuildMarker = "ZHORA DELTA (4.0.3)",   MarkerOffset = 0xAA7570, SaveContextOffset = 0xEC56E0, SceneOffset = 0xE78398, RoomOffset = 0,        SceneShift = -0x40 },
            new() { VersionCode = 501, BuildMarker = "FLYNN BRAVO (5.0.1)",   MarkerOffset = 0xAEFD10, SaveContextOffset = 0xF07AE0, SceneOffset = 0xE9A698, RoomOffset = 0xD1D858, SceneShift = -0x40 },
            new() { VersionCode = 514, BuildMarker = "BRADLEY ECHO (5.1.4)",  MarkerOffset = 0xF15490, SaveContextOffset = 0x1298074 - 0x2E, SceneOffset = 0x1238FF8, RoomOffset = 0x1061F74, SceneShift = -0x40 },
        };

        public static long AdjustSaveOffset(int versionCode, long baseSaveCtxOff, int offset)
        {
            if (versionCode == 514)
            {
                if (offset == 0x4) offset = -2;
                if (offset >= 0x78) offset += 0xE; // 5.0.1 -> 5.1.4 inventory shift
            }
            return baseSaveCtxOff + offset;
        }
    }
}
