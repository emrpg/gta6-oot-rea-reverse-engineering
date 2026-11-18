/*
 * Reconstructed C++20 implementation of fxReloader Thread-Safe Timecycle & VisualSettings Hot-Reloader
 * Evidence source:
 *   - fxReloader/FINDINGS.md (CTimeCycle::Update marshal, CVisualSettings::LoadAll,
 *     tcCycle::Load, CTimeCycle::Init, RevertModifierFile / LoadModifierFile / FinishModifierLoad)
 *   - gtav-timecycle-editor/source/
 */

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>

namespace fxreloader_reconstructed {

enum ReloadBits : uint32_t {
    RELOAD_NONE            = 0,
    RELOAD_VISUAL_SETTINGS = 1u << 0,
    RELOAD_WEATHER_CYCLES  = 1u << 1,
    RELOAD_MODIFIERS       = 1u << 2,
};

struct GameReloadPrimitives {
    void (*visual_settings_load_all)() = nullptr;
    void (*timecycle_init)(void* g_timecycle, int mode) = nullptr;
    void (*revert_modifier_file)(const char* path) = nullptr;
    void (*load_modifier_file)(const char* path) = nullptr;
    void (*finish_modifier_load)() = nullptr;
    void* g_timecycle_ptr = nullptr;
};

class HotReloadMarshal {
public:
    void request_reload(uint32_t bits) {
        pending_mask_.fetch_or(bits, std::memory_order_release);
    }

    // Must be called strictly from the main simulation thread (hooked inside CTimeCycle::Update)
    void on_timecycle_update(const GameReloadPrimitives& api,
                             const std::string& modifier_path) {
        uint32_t mask = pending_mask_.exchange(RELOAD_NONE, std::memory_order_acq_rel);
        if (mask == RELOAD_NONE) return;

        if ((mask & RELOAD_VISUAL_SETTINGS) && api.visual_settings_load_all) {
            api.visual_settings_load_all();
        }
        if ((mask & RELOAD_WEATHER_CYCLES) && api.timecycle_init && api.g_timecycle_ptr) {
            api.timecycle_init(api.g_timecycle_ptr, 4);
        }
        if ((mask & RELOAD_MODIFIERS) && api.revert_modifier_file &&
            api.load_modifier_file && api.finish_modifier_load) {
            api.revert_modifier_file(modifier_path.c_str());
            api.load_modifier_file(modifier_path.c_str());
            api.finish_modifier_load();
        }
    }

    static std::string redirect_path_if_present(const std::string& game_path,
                                                const std::filesystem::path& override_dir) {
        auto filename = std::filesystem::path(game_path).filename();
        auto candidate = override_dir / filename;
        if (std::filesystem::exists(candidate)) {
            return candidate.string();
        }
        return game_path;
    }

private:
    std::atomic<uint32_t> pending_mask_{RELOAD_NONE};
};

} // namespace fxreloader_reconstructed
