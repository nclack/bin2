#include <arm_neon.h>
#include <stdint.h>

void bin2x2(uint8_t* image, int width, int height) {
    int new_width = width / 2;
    int new_height = height / 2;

    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; x += 8) {
            uint8x16x2_t row1 = vld2q_u8(image + (2 * y) * width + 2 * x);
            uint8x16x2_t row2 = vld2q_u8(image + (2 * y + 1) * width + 2 * x);

            // Extract even elements (every other pixel)
            uint8x8_t row1_even = vget_low_u8(row1.val[0]);
            uint8x8_t row2_even = vget_low_u8(row2.val[0]);

            // Average the pixels
            uint8x8_t avg = vhadd_u8(row1_even, row2_even);

            // Store the result in-place
            vst1_u8(image + y * new_width + x, avg);
        }
    }
}
