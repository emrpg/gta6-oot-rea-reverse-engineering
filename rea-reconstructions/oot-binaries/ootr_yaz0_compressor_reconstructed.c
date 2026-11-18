/*
 * Reconstructed C implementation of OoT-Randomizer Compress & Decompress
 * Evidence source:
 *   - compress_elf__inspect-elf-unwind-ranges.json (23 FDEs, 2 exports)
 *   - compress_exe__inspect-pe-imports.json (PE32+ x86_64, KERNEL32.dll + msvcrt.dll, 119 imports)
 *   - decompress_exe__inspect-pe-imports.json (PE32 x86 32-bit, KERNEL32.dll + msvcrt.dll, 59 imports)
 *   - Observed strings in Compress: "Zelda64 Compressor, Version 2.1", "compressor.c",
 *     "yaz0_internal", "yaz0_encode", "Error: Entry %d in dmaTable.dat is out of bounds",
 *     "Error: Couldn't find dma table in ROM!", "Usage: %s [Input ROM] <Output ROM>"
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t vrom_start;
    uint32_t vrom_end;
    uint32_t rom_start;
    uint32_t rom_end;
} dma_entry_t;

static uint32_t read_be32(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  | (uint32_t)p[3];
}

static void write_be32(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)(v >> 16);
    p[2] = (uint8_t)(v >> 8);
    p[3] = (uint8_t)(v);
}

static size_t yaz0_internal(const uint8_t* src, size_t src_size,
                            size_t pos, size_t* out_dist) {
    size_t best_len = 0;
    size_t best_dist = 0;
    size_t max_dist = (pos > 0x1000) ? 0x1000 : pos;
    size_t max_len = src_size - pos;
    if (max_len > 0x111) max_len = 0x111;

    for (size_t d = 1; d <= max_dist; d++) {
        size_t len = 0;
        while (len < max_len && src[pos - d + len] == src[pos + len]) {
            len++;
        }
        if (len >= 3 && len > best_len) {
            best_len = len;
            best_dist = d;
            if (best_len == 0x111) break;
        }
    }
    *out_dist = best_dist;
    return best_len;
}

size_t yaz0_encode(const uint8_t* src, size_t src_size, uint8_t* dst) {
    memcpy(dst, "Yaz0", 4);
    write_be32(dst + 4, (uint32_t)src_size);
    memset(dst + 8, 0, 8);

    size_t src_pos = 0;
    size_t dst_pos = 16;

    while (src_pos < src_size) {
        size_t header_pos = dst_pos++;
        uint8_t code_byte = 0;

        for (int bit = 7; bit >= 0 && src_pos < src_size; bit--) {
            size_t dist = 0;
            size_t len = yaz0_internal(src, src_size, src_pos, &dist);
            if (len >= 3) {
                uint16_t d = (uint16_t)(dist - 1);
                if (len >= 0x12) {
                    dst[dst_pos++] = (uint8_t)((d >> 8) & 0x0F);
                    dst[dst_pos++] = (uint8_t)(d & 0xFF);
                    dst[dst_pos++] = (uint8_t)(len - 0x12);
                } else {
                    dst[dst_pos++] = (uint8_t)(((len - 2) << 4) | ((d >> 8) & 0x0F));
                    dst[dst_pos++] = (uint8_t)(d & 0xFF);
                }
                src_pos += len;
            } else {
                code_byte |= (uint8_t)(1u << bit);
                dst[dst_pos++] = src[src_pos++];
            }
        }
        dst[header_pos] = code_byte;
    }
    return dst_pos;
}

size_t yaz0_decode(const uint8_t* src, uint8_t* dst) {
    if (memcmp(src, "Yaz0", 4) != 0) return 0;
    uint32_t uncompressed_size = read_be32(src + 4);
    size_t src_pos = 16;
    size_t dst_pos = 0;

    while (dst_pos < uncompressed_size) {
        uint8_t code_byte = src[src_pos++];
        for (int bit = 7; bit >= 0 && dst_pos < uncompressed_size; bit--) {
            if (code_byte & (1u << bit)) {
                dst[dst_pos++] = src[src_pos++];
            } else {
                uint8_t b0 = src[src_pos++];
                uint8_t b1 = src[src_pos++];
                size_t dist = (((size_t)(b0 & 0x0F) << 8) | b1) + 1;
                size_t len = (b0 >> 4) == 0 ? (size_t)src[src_pos++] + 0x12 : (size_t)(b0 >> 4) + 2;
                for (size_t i = 0; i < len; i++) {
                    dst[dst_pos] = dst[dst_pos - dist];
                    dst_pos++;
                }
            }
        }
    }
    return uncompressed_size;
}
