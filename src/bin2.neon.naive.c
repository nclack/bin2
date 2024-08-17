#include <arm_neon.h>
#include <stdint.h>

void bin2x2(uint8_t* image, int width, int height) {
    int new_width = width / 2;
    int new_height = height / 2;

    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; x += 8) {
            uint8x16_t row1 = vld1q_u8(image + (2 * y) * width + 2 * x);
            uint8x16_t row2 = vld1q_u8(image + (2 * y + 1) * width + 2 * x);


            const uint16x8_t one=vdupq_n_u16(1);
            const uint8x16_t rows=vrhaddq_u8(row1,row2);
            const uint8x8_t avg=vshrn_n_u16(vaddq_u16(vpaddlq_u8(rows),one),1);

            vst1_u8(image + y * new_width + x, avg);
        }
    }
}

