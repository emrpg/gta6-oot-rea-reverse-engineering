/*
 * Reconstructed C architecture for OoT-Randomizer gzinject & minibsdiff
 * Evidence source:
 *   - gzinject_elf__inspect-elf-unwind-ranges.json (69 FDEs, 4 exports)
 *   - gzinject_exe__inspect-pe-imports.json (PE32+ x86_64, dynamic_base=disabled, KERNEL32+msvcrt 89 imports)
 *   - minibsdiff_elf__inspect-elf-unwind-ranges.json (23 FDEs, 0 exports)
 *   - minibsdiff_exe__inspect-pe-imports.json (PE32+ x86_64, KERNEL32+msvcrt 55 imports)
 *   - Observed strings: "wadextract", "Writing metadata.tmd.", "Changing region in the TMD",
 *     "%s-inject.wad", "bsdiff() failed!"
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma pack(push, 1)
typedef struct {
    uint32_t header_size;  /* 0x00000020 BE */
    uint32_t wad_type;     /* "Is\0\0" = 0x49730000 */
    uint32_t cert_size;
    uint32_t reserved;
    uint32_t ticket_size;
    uint32_t tmd_size;
    uint32_t data_size;
    uint32_t footer_size;
} wii_wad_header_t;
#pragma pack(pop)

static uint32_t be32(uint32_t v) {
    return ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) |
           ((v & 0x00FF0000u) >> 8)  | ((v & 0xFF000000u) >> 24);
}

int gzinject_validate_wad(const char* path, wii_wad_header_t* out_hdr) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Could not open %s wad file\n", path);
        return -1;
    }
    if (fread(out_hdr, sizeof(*out_hdr), 1, f) != 1 || be32(out_hdr->header_size) != 0x20) {
        fprintf(stderr, "%s is an invalid wad file!\n", path);
        fclose(f);
        return -1;
    }
    fclose(f);
    return 0;
}

int gzinject_patch_tmd_region(uint8_t* tmd_buf, size_t tmd_len, uint16_t region_code) {
    if (tmd_len < 0x19E) return -1;
    /* Wii TMD region field at offset 0x19C (big-endian u16) */
    tmd_buf[0x19C] = (uint8_t)(region_code >> 8);
    tmd_buf[0x19D] = (uint8_t)(region_code & 0xFF);
    return 0;
}

int minibsdiff_apply_header_check(const uint8_t* patch_buf, size_t len,
                                  uint64_t* ctrl_len, uint64_t* diff_len, uint64_t* new_size) {
    if (len < 32 || memcmp(patch_buf, "BSDIFF40", 8) != 0) {
        fprintf(stderr, "minibsdiff: ERROR: invalid BSDIFF40 header\n");
        return -1;
    }
    auto decode_off64 = [](const uint8_t* b) -> uint64_t {
        uint64_t y = b[7] & 0x7F;
        for (int i = 6; i >= 0; i--) y = (y << 8) | b[i];
        return y;
    };
    *ctrl_len = decode_off64(patch_buf + 8);
    *diff_len = decode_off64(patch_buf + 16);
    *new_size = decode_off64(patch_buf + 24);
    return 0;
}
