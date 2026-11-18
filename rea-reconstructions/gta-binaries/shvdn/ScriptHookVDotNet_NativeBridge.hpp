// Reconstructed from REA static evidence of ScriptHookVDotNet.asi
// SHA-256: c5461f63b5844a19945e72667ff0dd629a232a341b97910d7106405a5329f50e
// Format: PE32+ (AMD64 Mixed-Mode C++/CLI, 14 P/Invoke imports, 156 native implementations)
#pragma once

#include <cstdint>
#include <windows.h>

namespace ScriptHookV {

enum class eGameVersion : int32_t {
    VER_1_0_335_2_STEAM = 0,
    VER_1_0_335_2_NOSTEAM = 1,
    // ... additional versions
};

using KeyboardHandler = void(*)(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);
using ScriptMain = void(*)();

// Mangled C++ ABI imports from ScriptHookV.dll (observed via rea inspect-pe-imports & inspect-managed-native-boundaries)
extern "C" {
    // ?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z
    void scriptRegister(HINSTANCE module, ScriptMain LP_SCRIPT_MAIN);

    // ?scriptUnregister@@YAXPEAUHINSTANCE__@@@Z
    void scriptUnregister(HINSTANCE module);

    // ?scriptWait@@YAXK@Z
    void scriptWait(DWORD time);

    // ?keyboardHandlerRegister@@YAXP6AXKGEHHHH@Z@Z
    void keyboardHandlerRegister(KeyboardHandler handler);

    // ?keyboardHandlerUnregister@@YAXP6AXKGEHHHH@Z@Z
    void keyboardHandlerUnregister(KeyboardHandler handler);

    // ?nativeInit@@YAX_K@Z
    void nativeInit(uint64_t hash);

    // ?nativePush64@@YAX_K@Z
    void nativePush64(uint64_t val);

    // ?nativeCall@@YAPEA_KXZ
    uint64_t* nativeCall();

    // ?createTexture@@YAHPEBD@Z
    int32_t createTexture(const char* fileName);

    // ?drawTexture@@YAXHHHHMMMMMMMMMMMM@Z
    void drawTexture(
        int32_t id, int32_t index, int32_t level, int32_t time,
        float sizeX, float sizeY, float centerX, float centerY,
        float posX, float posY, float rotation, float screenHeightScaleFactor,
        float r, float g, float b, float a
    );

    // ?getGameVersion@@YA?AW4eGameVersion@@XZ
    eGameVersion getGameVersion();

    // ?getGlobalPtr@@YAPEA_KH@Z
    uint64_t* getGlobalPtr(int32_t globalId);
}

} // namespace ScriptHookV
