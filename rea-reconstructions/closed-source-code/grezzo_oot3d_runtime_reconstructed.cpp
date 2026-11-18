/*
 * Reconstructed C++20 runtime for Grezzo OoT3D Formats (ZAR / ZSI / CMB / CSAB)
 * Evidence source:
 *   - noclip.website/src/OcarinaOfTime3D/{zar.ts, zsi.ts, cmb.ts, csab.ts}
 *   - oot3d_practice_menu & oot3d/disassembly/tools/{ctrtool, n3dsdisasm}
 */

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace grezzo_oot3d {

static uint16_t rd_u16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

static uint32_t rd_u32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

struct ZarFileEntry {
    std::string name;
    uint32_t offset = 0;
    uint32_t size   = 0;
};

struct ZarArchive {
    uint32_t total_size = 0;
    std::string codename;
    std::vector<ZarFileEntry> files;

    static ZarArchive parse(const uint8_t* data, size_t len) {
        if (len < 0x20 || std::memcmp(data, "ZAR\x01", 4) != 0) {
            throw std::runtime_error("Invalid ZAR1 header");
        }
        ZarArchive zar;
        zar.total_size = rd_u32(data + 0x04);
        uint16_t num_files = rd_u16(data + 0x0A);
        uint32_t file_tbl_off = rd_u32(data + 0x10);
        uint32_t data_tbl_off = rd_u32(data + 0x14);
        zar.codename = std::string(reinterpret_cast<const char*>(data + 0x18),
                                   strnlen(reinterpret_cast<const char*>(data + 0x18), 8));

        for (uint16_t i = 0; i < num_files; i++) {
            size_t ft = file_tbl_off + static_cast<size_t>(i) * 8;
            size_t dt = data_tbl_off + static_cast<size_t>(i) * 4;
            if (ft + 8 > len || dt + 4 > len) break;

            uint32_t fsize = rd_u32(data + ft);
            uint32_t name_off = rd_u32(data + ft + 4);
            uint32_t data_off = rd_u32(data + dt);
            std::string fname = (name_off < len)
                ? std::string(reinterpret_cast<const char*>(data + name_off))
                : "";
            zar.files.push_back(ZarFileEntry{std::move(fname), data_off, fsize});
        }
        return zar;
    }
};

struct ZsiCommand {
    uint8_t  opcode;
    uint8_t  param0;
    uint16_t param1;
    uint32_t payload;
};

struct CmbHeader {
    uint32_t magic;       /* "cmb\0" = 0x00626D63 */
    uint32_t file_size;
    uint32_t version;     /* 6 = OoT3D */
    uint32_t reserved;
    char     name[0x10];
    uint32_t num_chunks;
};

struct CsabHeader {
    uint32_t magic;       /* "csab" */
    uint32_t file_size;
    uint32_t version;
    uint32_t duration;
    uint32_t num_nodes;
};

} // namespace grezzo_oot3d
