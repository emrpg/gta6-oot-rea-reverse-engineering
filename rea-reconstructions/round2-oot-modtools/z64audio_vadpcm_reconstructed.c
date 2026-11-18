/*
 * Reconstructed C implementation of z64audio.exe VADPCM Codebook & Frame Encoder
 * Evidence source:
 *   - z64audio_exe__inspect-pe-imports.json (PE32 x86, 6 DLLs / 270 imports: GDI32, KERNEL32, msvcrt, OPENGL32, SHELL32, USER32)
 *   - z64audio_exe__inspect-binary-hardening.json (sha256: 3d173b43b807, dynamic_base=enabled, nx_compat=enabled)
 *   - Observed strings: z64audio.toml, [bin/wav/aiff/c], book_iteration, book_frame_size, book_bits, book_order, book_threshold
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int32_t order;        /* book_order (typically 2) */
    int32_t npredictors;  /* 1 << book_bits (typically 2 or 4) */
    int32_t frame_size;   /* book_frame_size (16 samples per VADPCM frame) */
    int32_t iterations;   /* book_iteration */
    double  threshold;    /* book_threshold */
} z64_vadpcm_config_t;

typedef struct {
    uint8_t header;       /* (scale << 4) | (predictor_index & 0x0F) */
    uint8_t nibbles[8];   /* 16 signed 4-bit ADPCM residuals */
} vadpcm_frame_t;

static int16_t clamp_s16(int32_t v) {
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (int16_t)v;
}

void vadpcm_encode_frame(const int16_t pcm16[16],
                         const int16_t codebook_row[2][8],
                         int32_t prev2, int32_t prev1,
                         uint8_t pred_idx, uint8_t scale_shift,
                         vadpcm_frame_t* out_frame) {
    out_frame->header = (uint8_t)((scale_shift << 4) | (pred_idx & 0x0Fu));
    memset(out_frame->nibbles, 0, sizeof(out_frame->nibbles));

    int32_t scale = 1 << scale_shift;
    for (int i = 0; i < 16; i++) {
        int32_t pred = (codebook_row[0][0] * prev2 + codebook_row[1][0] * prev1) >> 11;
        int32_t err = static_cast<int32_t>(pcm16[i]) - pred;
        int32_t q = err / scale;
        if (q > 7) q = 7;
        if (q < -8) q = -8;

        uint8_t nib = (uint8_t)(q & 0x0F);
        if ((i & 1) == 0) out_frame->nibbles[i >> 1] |= (nib << 4);
        else              out_frame->nibbles[i >> 1] |= nib;

        int32_t recon = clamp_s16(pred + q * scale);
        prev2 = prev1;
        prev1 = recon;
    }
}
