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
    const int newh=h/2;
    const int maxx=FLOOR_BLOCKS(w);

    // Average rows
    for (int y = 0; y < newh; ++y) {
        uint8x16_t* row = im + 2 * y * dy;
        uint8x16_t* out = im + y * dy / 2;
        // Process two lane-wide columns at a time.
        __builtin_prefetch(row+2*dy);
        for (int x = 0; x < maxx; x += 2) {
            // interleaved load, v0.val[0] gets even columns, v0.val[1] the odds
            const uint8x16x2_t u0 = vld2q_u8((uint8_t*)&row[x]);
            const uint8x16x2_t v0 = vld2q_u8((uint8_t*)&row[dy+x]);

            // vertical average
            const uint8x16_t a=vrhaddq_u8(u0.val[0],v0.val[0]);
            const uint8x16_t b=vrhaddq_u8(u0.val[1],v0.val[1]);

            // horizontal average
            const uint8x16_t r0 = vrhaddq_u8(a,b);

            out[(x>>1)] = r0;
        }
    }
}
