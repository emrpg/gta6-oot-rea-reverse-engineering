/*
 * Reconstructed C++20 implementation of ScriptHookV Native Fiber Scheduler & TLS Context Switcher
 * Evidence source:
 *   - ScriptHookVDotNet_asi__inspect-pe-imports.json (exact mangled ScriptHookV.dll exports:
 *     scriptRegister, scriptUnregister, scriptWait, keyboardHandlerRegister, keyboardHandlerUnregister)
 *   - ScriptHookVDotNet_asi__inspect-pe-load-config.json (IMAGE_GUARD_CF_INSTRUMENTED)
 *   - ScriptHookVDotNet2_dll / ScriptHookVDotNet3_dll managed boundaries
 */

#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace shv_reconstructed {

using ScriptMainFn = void (*)();
using KeyboardHandlerFn = void (*)(uint32_t key, uint16_t repeats, uint8_t scan_code,
                                   int is_extended, int is_with_alt,
                                   int was_down_before, int is_up_now);

struct RageTlsContext {
    uint8_t  reserved[0x2D8];
    void*    sys_mem_allocator;   /* RAGE allocator pointer */
    void*    scr_thread;          /* Active rage::scrThread* */
    uint8_t  is_main_thread;
};

struct ScriptFiber {
    void*        module_handle = nullptr;
    ScriptMainFn entry         = nullptr;
    uint64_t     wake_tick_ms  = 0;
    bool         finished      = false;
    void*        native_fiber  = nullptr; /* Win32 LPVOID fiber handle */
};

class FiberScheduler {
public:
    void register_script(void* hmodule, ScriptMainFn fn) {
        std::lock_guard<std::mutex> lock(mu_);
        fibers_.push_back(ScriptFiber{hmodule, fn, 0, false, nullptr});
    }

    void unregister_script(void* hmodule) {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto it = fibers_.begin(); it != fibers_.end();) {
            if (it->module_handle == hmodule) {
                it = fibers_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void wait_current(uint32_t ms) {
        if (current_idx_ < fibers_.size()) {
            fibers_[current_idx_].wake_tick_ms = now_ms() + ms;
        }
        /* On Win32: SwitchToFiber(main_fiber_) */
    }

    void register_keyboard(KeyboardHandlerFn cb) {
        std::lock_guard<std::mutex> lock(mu_);
        kb_handlers_.push_back(cb);
    }

    void unregister_keyboard(KeyboardHandlerFn cb) {
        std::lock_guard<std::mutex> lock(mu_);
        for (auto it = kb_handlers_.begin(); it != kb_handlers_.end();) {
            if (*it == cb) it = kb_handlers_.erase(it);
            else ++it;
        }
    }

    // Executes one game tick while preserving RAGE TLS scrThread & allocator slots
    void tick(RageTlsContext* game_tls, void* target_scr_thread) {
        void* saved_thread = game_tls ? game_tls->scr_thread : nullptr;
        if (game_tls) {
            game_tls->scr_thread = target_scr_thread;
        }

        uint64_t t = now_ms();
        for (size_t i = 0; i < fibers_.size(); i++) {
            if (!fibers_[i].finished && t >= fibers_[i].wake_tick_ms) {
                current_idx_ = i;
                /* On Win32: SwitchToFiber(fibers_[i].native_fiber) */
                if (fibers_[i].entry) {
                    fibers_[i].entry();
                }
            }
        }

        if (game_tls) {
            game_tls->scr_thread = saved_thread;
        }
    }

private:
    static uint64_t now_ms() {
        using namespace std::chrono;
        return static_cast<uint64_t>(
            duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
    }

    std::mutex mu_;
    std::vector<ScriptFiber> fibers_;
    std::vector<KeyboardHandlerFn> kb_handlers_;
    size_t current_idx_ = 0;
};

} // namespace shv_reconstructed
