/*
 * Reconstructed C++ architecture for seq64_console.exe
 * Evidence source:
 *   - seq64_console_exe__inspect-pe-imports.json (PE32+ x86_64, 22 DLLs / 342 imports:
 *     KERNEL32, USER32, SHELL32, ole32, ADVAPI32, MSVCP140, WININET, WS2_32, VERSION, SHLWAPI...)
 *   - Observed strings: "seq64 - Sequenced music editor for first-party N64 games",
 *     "tsections internal consistency error!", "Error parsing XML of ",
 *     "Error creating XML for currently loaded ABI!", "--help", "SEQ64 operation completed successfully"
 */

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>

namespace seq64 {

struct CliOptions {
    std::string input_path;
    std::string output_path;
    std::string abi_name;
    bool show_help = false;
};

static bool parse_cli(int argc, char** argv, CliOptions& opts) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            opts.show_help = true;
            return true;
        } else if (arg == "--in" || arg == "-i") {
            if (!opts.input_path.empty()) {
                std::fprintf(stderr, "Error, multiple input files specified!\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::fprintf(stderr, "Error, no file specified with input argument!\n");
                return false;
            }
            opts.input_path = argv[++i];
        } else if (arg == "--out" || arg == "-o") {
            if (!opts.output_path.empty()) {
                std::fprintf(stderr, "Error, multiple output files specified!\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::fprintf(stderr, "Error, no file specified with output argument!\n");
                return false;
            }
            opts.output_path = argv[++i];
        } else if (arg == "--abi" || arg == "-a") {
            if (!opts.abi_name.empty()) {
                std::fprintf(stderr, "Error, multiple ABIs specified!\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::fprintf(stderr, "Error, no argument specified with ABI!\n");
                return false;
            }
            opts.abi_name = argv[++i];
        }
    }
    if (opts.input_path.empty()) {
        std::fprintf(stderr, "Error, no input file specified!\n");
        return false;
    }
    if (opts.output_path.empty()) {
        std::fprintf(stderr, "Error, no output file specified!\n");
        return false;
    }
    return true;
}

} // namespace seq64

int main(int argc, char** argv) {
    seq64::CliOptions opts;
    if (!seq64::parse_cli(argc, argv, opts)) {
        return 1;
    }
    if (opts.show_help) {
        std::printf("seq64 - Sequenced music editor for first-party N64 games\n");
        return 0;
    }
    std::printf("SEQ64 operation completed successfully\n");
    return 0;
}
