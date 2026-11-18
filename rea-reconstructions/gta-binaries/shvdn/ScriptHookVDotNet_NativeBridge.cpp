// Reconstructed from REA static evidence of ScriptHookVDotNet.asi
// Mixed-mode C++/CLI CLR host & Fiber-based script scheduler
#include "ScriptHookVDotNet_NativeBridge.hpp"

namespace SHVDN_Loader {

static LPVOID g_mainFiber = nullptr;
static LPVOID g_scriptFiber = nullptr;

// Reconstructed from KERNEL32.dll imports: CreateFiber, SwitchToFiber, DeleteFiber, GetCurrentThreadId
void __stdcall ScriptFiberEntry(LPVOID /*lpParameter*/) {
    while (true) {
        // Tick managed ScriptDomain via CLR interop
        // Switch back to main game fiber when slice yields
        SwitchToFiber(g_mainFiber);
    }
}

void ScriptMainLoop() {
    if (!g_mainFiber) {
        g_mainFiber = ConvertThreadToFiber(nullptr);
    }
    if (!g_scriptFiber) {
        g_scriptFiber = CreateFiber(0, ScriptFiberEntry, nullptr);
    }

    while (true) {
        if (g_scriptFiber) {
            SwitchToFiber(g_scriptFiber);
        }
        ScriptHookV::scriptWait(0);
    }
}

void OnKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow) {
    // Dispatched to managed ScriptDomain keyboard queue
    (void)key; (void)repeats; (void)scanCode; (void)isExtended; (void)isWithAlt; (void)wasDownBefore; (void)isUpNow;
}

} // namespace SHVDN_Loader

// Note: Mixed-mode DLL entry delegates to mscoree.dll!_CorDllMain
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        ScriptHookV::scriptRegister(hModule, SHVDN_Loader::ScriptMainLoop);
        ScriptHookV::keyboardHandlerRegister(SHVDN_Loader::OnKeyboardMessage);
        break;
    case DLL_PROCESS_DETACH:
        ScriptHookV::keyboardHandlerUnregister(SHVDN_Loader::OnKeyboardMessage);
        ScriptHookV::scriptUnregister(hModule);
        if (SHVDN_Loader::g_scriptFiber) {
            DeleteFiber(SHVDN_Loader::g_scriptFiber);
            SHVDN_Loader::g_scriptFiber = nullptr;
        }
        break;
    }
    return TRUE;
}
