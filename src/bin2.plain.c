#include <stdint.h>

void
bin2x2(uint8_t* im_, int w, int h)
{
    const uint8_t* end = im_ + w * h;

    // vertical
    uint8_t *out=im_;
    for (uint8_t* row = im_ + w; row < end; row += 2 * w, out+=w) {
        const uint8_t* row_end = row + w;
        uint8_t *o=out;
        for (uint8_t* p = row; p < row_end; ++p, ++o) {
             *o = (uint8_t)(((uint16_t)p[0] + (uint16_t)p[-w] + 1)>>1);
        }
    }

    // horizontal
    const uint8_t* hend = im_ + w*h/2;
    out=im_;
    for (uint8_t* row = im_; row < hend; row += w, out+=w/2) {
        const uint8_t* row_end = row + w;
        uint8_t *o=out;
        for (uint8_t* p = row; p < row_end; p += 2,++o) {
            *o = (uint8_t)(((uint16_t)p[0] + (uint16_t)p[1] + 1)>>1);
        }
    }
}

