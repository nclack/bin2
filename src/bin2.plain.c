#include <stdint.h>
#include <string.h>

void
bin2x2(uint8_t* im_, int w, int h)
{
    const uint8_t* end = im_ + w * h;

    // horizontal
    for (uint8_t* row = im_; row < end; row += w) {
        const uint8_t* row_end = row + w;
        for (uint8_t* p = row; p < row_end; p += 2) {
            p[0] = (uint8_t)(((uint16_t)p[0] + (uint16_t)p[1])>>1);
        }
        for (uint8_t *p = row, *s = row; s < row_end; ++p, s += 2) {
            *p = *s;
        }
    }
    // vertical
    for (uint8_t* row = im_ + w; row < end; row += 2 * w) {
        const uint8_t* row_end = row + w/2;
        for (uint8_t* p = row; p < row_end; ++p) {
            p[-w] = (uint8_t)(((uint16_t)p[0] + (uint16_t)p[-w])>>1);
        }
    }
    for (uint8_t *src_row = im_, *dst_row = im_; src_row < end;
         src_row += 2 * w, dst_row += w/2) {
        memcpy(dst_row, src_row, w/2);
    }
}

