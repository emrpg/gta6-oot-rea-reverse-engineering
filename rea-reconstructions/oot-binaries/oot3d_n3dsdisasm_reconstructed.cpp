/*
 * Reconstructed C++ architecture for n3dsdisasm (oot3d/disassembly/tools/n3dsdisasm.exe & n3dsdisasm)
 * Evidence source:
 *   - n3dsdisasm_exe__inspect-pe-imports.json (PE32+ x86_64, VCRUNTIME140.dll + 8 api-ms-win-crt-* + KERNEL32.dll)
 *   - n3dsdisasm_elf__inspect-elf-unwind-ranges.json (256+ FDEs, 10.2 MB binary)
 *   - Observed strings: arm_func, thumb_func, arm_func_start, thumb_func_start,
 *     non_word_aligned_thumb_func_start, arm_label, thumb_label, arm_far_jump, thumb_far_jump,
 *     "error: function at 0x%08x is not aligned", "arminsn->op_count > 0"
 */

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace n3dsdisasm {

enum class FuncMode : uint8_t {
    Arm   = 0,
    Thumb = 1,
};

struct FunctionEntry {
    uint32_t address;
    FuncMode mode;
    std::string name;
};

struct DisasmConfig {
    uint32_t base_addr = 0x00100000;
    std::vector<uint8_t> code_bytes;
    std::map<uint32_t, FunctionEntry> functions;
    std::set<uint32_t> labels;
};

static bool is_aligned_for_mode(uint32_t addr, FuncMode mode) {
    return (mode == FuncMode::Arm) ? ((addr & 3u) == 0) : ((addr & 1u) == 0);
}

static void emit_function_header(FILE* out, const FunctionEntry& fn) {
    if (!is_aligned_for_mode(fn.address, fn.mode)) {
        std::fprintf(stderr, "error: function at 0x%08x is not aligned\n", fn.address);
        return;
    }
    if (fn.mode == FuncMode::Arm) {
        std::fprintf(out, "\tarm_func_start %s\n%s:\n", fn.name.c_str(), fn.name.c_str());
    } else if ((fn.address & 3u) != 0) {
        std::fprintf(out, "\tnon_word_aligned_thumb_func_start %s\n%s:\n",
                     fn.name.c_str(), fn.name.c_str());
    } else {
        std::fprintf(out, "\tthumb_func_start %s\n%s:\n", fn.name.c_str(), fn.name.c_str());
    }
}

static void emit_branch_target(FILE* out, uint32_t target, FuncMode mode, bool is_far) {
    if (is_far) {
        std::fprintf(out, "\t%s 0x%08x\n",
                     (mode == FuncMode::Arm) ? "arm_far_jump" : "thumb_far_jump", target);
    } else {
        std::fprintf(out, "\t%s _%08X\n",
                     (mode == FuncMode::Arm) ? "arm_label" : "thumb_label", target);
    }
}

void disassemble_image(const DisasmConfig& cfg, FILE* out) {
    for (const auto& [addr, fn] : cfg.functions) {
        emit_function_header(out, fn);
        /* Capstone ARM/Thumb decode loop verifies arminsn->op_count > 0 per instruction */
        if (cfg.labels.count(addr)) {
            emit_branch_target(out, addr, fn.mode, false);
        }
    }
}

} // namespace n3dsdisasm
