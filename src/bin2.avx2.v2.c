#ifdef __AVX2__
#include <immintrin.h>

#define LANES (32)
#define CEIL_BLOCKS(n) ((n + LANES - 1) / LANES)
#define FLOOR_BLOCKS(n) (n / LANES)

void bin2x2(uint8_t* im_, int w, int h)
{
    __m256i* const im = (__m256i*)im_;
    const int dy = w / LANES;

    for (int y = 0; y < h / 2; y += 2) {
        __m256i* row = im + 2 * y * dy;
        __m256i* out = im + y * dy;
        for (int x = 0; x < CEIL_BLOCKS(w); x += 2) {
            // Prefetch next iterations
            _mm_prefetch((const char*)(row + x + 2), _MM_HINT_T0);
            _mm_prefetch((const char*)(row + x + 2 + dy), _MM_HINT_T0);

            // Process two columns at once
            __m256i a0 = _mm256_avg_epu8(row[x], row[x + dy]);
            __m256i a1 = _mm256_avg_epu8(row[x + 1], row[x + 1 + dy]);
            out[x] = a0;
            out[x + 1] = a1;
        }
    }

    const __m256i mask = _mm256_set1_epi16(0x00ff);
    for (int x = 0; x < FLOOR_BLOCKS(w * h / 2); x += 2) {
        // Prefetch next iteration
        _mm_prefetch((const char*)(im + x + 2), _MM_HINT_T0);

        // Process two vectors at once
        __m256i b0 = _mm256_srli_epi16(im[x], 8);
        __m256i b1 = _mm256_srli_epi16(im[x + 1], 8);
        __m256i v0 = _mm256_avg_epu8(im[x], b0);
        __m256i v1 = _mm256_avg_epu8(im[x + 1], b1);
        im[x] = _mm256_and_si256(v0, mask);
        im[x + 1] = _mm256_and_si256(v1, mask);
    }

    for (int x = 0; x < FLOOR_BLOCKS(w * h / 4); x += 2) {
        // Prefetch next iteration
        _mm_prefetch((const char*)(im + x + 4), _MM_HINT_T0);

        // Process two pairs at once
        __m256i v0 = _mm256_packus_epi16(im[2 * x], im[2 * x + 1]);
        __m256i v1 = _mm256_packus_epi16(im[2 * x + 2], im[2 * x + 3]);
        im[x] = _mm256_permute4x64_epi64(v0, 0xD8);  // 0xD8 = (3 << 6) | (1 << 4) | (2 << 2) | 0
        im[x + 1] = _mm256_permute4x64_epi64(v1, 0xD8);
    }
}
#endif // __AVX2__
