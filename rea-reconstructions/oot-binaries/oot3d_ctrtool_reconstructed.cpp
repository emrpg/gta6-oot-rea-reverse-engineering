/*
 * Reconstructed C++ architecture for ctrtool (oot3d/disassembly/tools/ctrtool.exe & ctrtool)
 * Evidence source:
 *   - ctrtool_exe__inspect-pe-imports.json   (PE32+ x86_64, KERNEL32.dll only: 88 static-CRT Win32 APIs)
 *   - ctrtool_elf__inspect-elf-unwind-ranges.json (256+ FDEs, 8 exports, 17 initializers)
 *   - ctrtool_elf__inspect-binary-hardening.json & observed strings ("=NCCHu", std::regex_error, etc.)
 */

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace ctrtool {

#pragma pack(push, 1)
struct NcchHeader {
    uint8_t  signature[0x100];
    uint32_t magic;            /* "NCCH" = 0x4843434E */
    uint32_t content_size;     /* media units (0x200 bytes) */
    uint64_t partition_id;
    uint16_t maker_code;
    uint16_t version;
    uint32_t content_lock_hash;
    uint64_t program_id;
    uint8_t  reserved0[0x10];
    uint8_t  logo_region_hash[0x20];
    char     product_code[0x10];
    uint8_t  exheader_hash[0x20];
    uint32_t exheader_size;
    uint32_t reserved1;
    uint8_t  flags[8];
    uint32_t plain_region_offset;
    uint32_t plain_region_size;
    uint32_t logo_region_offset;
    uint32_t logo_region_size;
    uint32_t exefs_offset;
    uint32_t exefs_size;
    uint32_t exefs_hash_region_size;
    uint32_t reserved2;
    uint32_t romfs_offset;
    uint32_t romfs_size;
    uint32_t romfs_hash_region_size;
    uint32_t reserved3;
    uint8_t  exefs_superblock_hash[0x20];
    uint8_t  romfs_superblock_hash[0x20];
};

struct ExefsFileHeader {
    char     name[8];
    uint32_t offset;
    uint32_t size;
};
#pragma pack(pop)

constexpr uint32_t kNcchMagic = 0x4843434E;
constexpr uint64_t kMediaUnit = 0x200;

class NcchReader {
public:
    explicit NcchReader(const std::filesystem::path& path)
        : stream_(path, std::ios::binary) {
        if (!stream_) {
            throw std::runtime_error("Failed to open CTR image: " + path.string());
        }
        stream_.read(reinterpret_cast<char*>(&hdr_), sizeof(hdr_));
        if (stream_.gcount() != sizeof(hdr_) || hdr_.magic != kNcchMagic) {
            throw std::runtime_error("Invalid NCCH magic (expected 'NCCH')");
        }
    }

    void extract_exefs(const std::filesystem::path& out_dir) {
        if (hdr_.exefs_size == 0) return;
        std::filesystem::create_directories(out_dir);
        uint64_t base = static_cast<uint64_t>(hdr_.exefs_offset) * kMediaUnit;
        stream_.seekg(static_cast<std::streamoff>(base));

        ExefsFileHeader entries[10]{};
        stream_.read(reinterpret_cast<char*>(entries), sizeof(entries));

        for (const auto& entry : entries) {
            if (entry.name[0] == '\0' || entry.size == 0) continue;
            std::string fname(entry.name, strnlen(entry.name, 8));
            std::vector<uint8_t> buf(entry.size);
            stream_.seekg(static_cast<std::streamoff>(base + 0x200 + entry.offset));
            stream_.read(reinterpret_cast<char*>(buf.data()), entry.size);
            std::ofstream out(out_dir / fname, std::ios::binary);
            out.write(reinterpret_cast<const char*>(buf.data()),
                      static_cast<std::streamsize>(buf.size()));
        }
    }

private:
    std::ifstream stream_;
    NcchHeader hdr_{};
};

} // namespace ctrtool

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s [options] <file>\n", argv[0]);
        return 1;
    }
    try {
        ctrtool::NcchReader reader(argv[argc - 1]);
        reader.extract_exefs("exefs_out");
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "error: %s\n", ex.what());
        return 1;
    }
    return 0;
}
