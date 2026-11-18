// Reconstructed from REA static evidence of GTAIV.EFLC.FusionFix.asi & XLivelessAddon.asi
// GTAIV.EFLC.FusionFix.asi:
//   SHA-256: 14f6ac879bcc7df5fb18ce7b410bc436e16675b0dad640378dcb1e6dd3be98b3
//   Format: PE32 (32-bit x86), 14,526,680 bytes, Authenticode signed (7,392 bytes @ offset 14519288)
//   Hardening: dynamic_base=enabled, IMAGE_GUARD_CF_INSTRUMENTED, SEH handlers=813, TLS=present
//   Exposed PDB (via audit-release-exposure): D:\a\GTAIV.EFLC.FusionFix\GTAIV.EFLC.FusionFix\bin\GTAIV.EFLC.FusionFix.pdb
//
// XLivelessAddon.asi:
//   Format: PE32 (32-bit x86), 232,960 bytes
//   Exposed PDB: Z:\GitHub\xlivelessaddon\Release\plugins\XLivelessAddon.pdb
#pragma once

#include <cstdint>
#include <windows.h>

namespace FusionFixBinary {

// 42 RT_RCDATA (type ID = 10) embedded shader/config/font resources observed via rea inspect-pe-resources
enum class EmbeddedResourceId : uint16_t {
    ShaderPackStart = 101, // 101..105, 110..111, 113, 116..121, 126...
};

// Reconstructed XLivelessAddon.asi configuration & window/savegame hook contract
struct XLivelessAddonConfig {
    bool customSavePath = false;       // "CustomSavePath" / "savegames"
    bool doNotPauseOnMinimize = false; // "DoNotPauseOnMinimize"
};

// Observed imports: KERNEL32.dll (VirtualProtect, CreateThread), USER32.dll (SetWindowLongA, GetAsyncKeyState), ADVAPI32.dll (RegOpenKeyExA)
inline void PatchMemoryBytes(uintptr_t address, const void* bytes, size_t size) {
    DWORD oldProtect = 0;
    if (VirtualProtect(reinterpret_cast<LPVOID>(address), size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        memcpy(reinterpret_cast<void*>(address), bytes, size);
        VirtualProtect(reinterpret_cast<LPVOID>(address), size, oldProtect, &oldProtect);
    }
}

} // namespace FusionFixBinary
