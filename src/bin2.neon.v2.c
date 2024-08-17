#include <arm_neon.h>
#include <stdint.h>

#define LANES (16)  // NEON vectors are 128-bit (16 bytes) wide
#define CEIL_BLOCKS(n) ((n + LANES - 1) / LANES)
#define FLOOR_BLOCKS(n) (n / LANES)

void
bin2x2(uint8_t* im_, int w, int h)
{
    uint8x16_t* const im = (uint8x16_t*)im_;
    const int dy = w / LANES;

    // Average rows
    for (int y = 0; y < h / 2; ++y) {
        uint8x16_t* row = im + 2 * y * dy;
        uint8x16_t* out = im + y * dy;
        // Process two lane-wide columns at a time.
        for (int x = 0; x < FLOOR_BLOCKS(w); x += 2) {
            __builtin_prefetch(row + x + 1, 0);
            __builtin_prefetch(row + x + 2, 0);
            __builtin_prefetch(row + x + 1 + dy, 0);
            __builtin_prefetch(row + x + 2 + dy, 0);

            // Process two columns at once
            uint8x16_t a0 = vrhaddq_u8(row[x], row[x + dy]);
            uint8x16_t a1 = vrhaddq_u8(row[x + 1], row[x + 1 + dy]);
            
            out[x] = a0;
            out[x + 1] = a1;
        }
    }
    for (int x = 0; x < FLOOR_BLOCKS(w * h / 2); x += 4) {
        __builtin_prefetch(im + x + 4, 0);

        // Process four lanes 
        // interleaved load, v0.val[0] gets even columns, v0.val[1] the odds
        uint8x16x2_t v0 = vld2q_u8((uint8_t*)&im[x]);
        uint8x16x2_t v1 = vld2q_u8((uint8_t*)&im[x + 2]);

        uint8x16_t r0 = vrhaddq_u8(v0.val[0], v0.val[1]);
        uint8x16_t r1 = vrhaddq_u8(v1.val[0], v1.val[1]);
        
        im[(x>>1)] = r0;
        im[(x>>1) + 1] = r1;
    }
}
