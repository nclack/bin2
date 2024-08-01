#include <immintrin.h>
#include <stdint.h>

void bin2x2(uint8_t* image, int width, int height) {
    int new_width = width / 2;
    int new_height = height / 2;

    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; x += 16) {
            __m256i row1 = _mm256_loadu_si256((const __m256i*)(image + (2 * y) * width + 2 * x));
            __m256i row2 = _mm256_loadu_si256((const __m256i*)(image + (2 * y + 1) * width + 2 * x));

            // Interleave low bytes of row1 and row2
            __m256i interleaved = _mm256_unpacklo_epi8(row1, row2);

            // Sum adjacent elements
            __m256i summed = _mm256_maddubs_epi16(interleaved, _mm256_set1_epi16(0x0101));

            // Divide by 4 (right shift by 2)
            __m256i downsampled = _mm256_srli_epi16(summed, 2);

            // Pack 16-bit integers to 8-bit integers
            __m128i result = _mm_packus_epi16(_mm256_castsi256_si128(downsampled), _mm256_extracti128_si256(downsampled, 1));

            // Store the result in-place
            _mm_storeu_si128((__m128i*)(image + y * new_width + x), result);
        }
    }
}
