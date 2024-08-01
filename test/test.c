#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void bin2x2(uint8_t* im, int w, int h);

static void bin2x2_reference(uint8_t* im, int w, int h) {
    int new_w = w / 2, new_h = h / 2;
    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            uint16_t sum = im[2*y*w + 2*x] + im[2*y*w + 2*x + 1] +
                           im[(2*y+1)*w + 2*x] + im[(2*y+1)*w + 2*x + 1];
            im[y*new_w + x] = (uint8_t)(sum / 4);
        }
    }
}

static int compare_images(uint8_t* im1, uint8_t* im2, int w, int h) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            if (im1[y*w + x] != im2[y*w + x]) return 1;
        }
    }
    return 0;
}

int main() {
    int max_size = 8192;
    uint8_t* test_im = (uint8_t*)aligned_alloc(32, max_size * max_size);
    uint8_t* ref_im = (uint8_t*)aligned_alloc(32, max_size * max_size);
    
    for (int i = 0; i < max_size * max_size; i++) test_im[i] = rand() % 256;
    memcpy(ref_im, test_im, max_size * max_size);
    
    int sizes[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64, 32};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        bin2x2(test_im, size, size);
        bin2x2_reference(ref_im, size, size);
        
        int result = compare_images(test_im, ref_im, size/2, size/2);
        if (result != 0) {
            printf("Failed for size %dx%d\n", size, size);
            free(test_im);
            free(ref_im);
            return 1;
        }
    }
    
    free(test_im);
    free(ref_im);
    return 0;
}
