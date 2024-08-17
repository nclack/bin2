#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define min(a,b) ((a)<(b)?(a):(b))

void bin2x2(uint8_t* im, int w, int h);

static void bin2x2_reference(uint8_t* im, int w, int h) {
    int new_w = w / 2;
    int new_h = h / 2;
    
    for (int y = 0; y < new_h; y++) {
        for (int x = 0; x < new_w; x++) {
            const int a=
                im[2*y*w + 2*x] +
                im[(2*y+1)*w + 2*x] + 1;
            const int b=
                      im[2*y*w + 2*x + 1] +
                      im[(2*y+1)*w + 2*x + 1] +1;
            
            im[y*new_w + x] = ((a/2)+(b/2)+1)/2;
        }
    }
}


static int compare_images(uint8_t* im1, uint8_t* im2, int w, int h) {
    int max_diff = 0;
    int diff_count = 0;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int diff = abs((int)im1[y*w+ x] - (int)im2[y*w+ x]);
            if (diff > 0) {
                diff_count++;
                if (diff > max_diff) max_diff = diff;
                if(diff_count<10) {
                    printf("Difference: (%d,%d)=%d\n",x,y,diff);
                }
            }
        }
    }
    if (diff_count > 0) {
        printf("Differences found: count=%d, max_diff=%d\n", diff_count, max_diff);
        return 1;
    }
    return 0;
}

static void print_image(const char* name, uint8_t* im, int w, int h) {
    const int pw=min(w,64);
    const int ph=min(h,64);
    printf("Image %dx%d: %s\n", w, h, name);
    
    // Print column headers
    printf("    ");
    for (int x = 0; x < pw; x++) {
        printf("%3d ", x);
    }
    printf("\n");
    
    // Print horizontal separator
    printf("   +");
    for (int x = 0; x < pw; x++) {
        printf("---+");
    }
    printf("\n");
    
    // Print rows
    for (int y = 0; y < ph; y++) {
        printf("%2d |", y);
        for (int x = 0; x < pw; x++) {
            printf("%3d|", im[y*w + x]);
        }
        printf("\n");
        
        // Print horizontal separator
        printf("   +");
        for (int x = 0; x < pw; x++) {
            printf("---+");
        }
        printf("\n");
    }
}

static int test_size(int w, int h) {
    printf("Testing size %dx%d\n", w, h);
    size_t size = (size_t)w * h;
    uint8_t* test_im = (uint8_t*)aligned_alloc(32, size);
    uint8_t* ref_im = (uint8_t*)aligned_alloc(32, size);
    
    if (!test_im || !ref_im) {
        printf("Memory allocation failed for size %dx%d\n", w, h);
        free(test_im);
        free(ref_im);
        return 1;
    }

    for (size_t i = 0; i < size; i++) test_im[i] = rand() % 256;
    memcpy(ref_im, test_im, size);
    // print_image("input",ref_im,w,h);
    
    bin2x2(test_im, w, h);
    bin2x2_reference(ref_im, w, h);
    // print_image("test",test_im,w/2,h/2);
    // print_image("ref",ref_im,w/2,h/2);
    
    int result = compare_images(test_im, ref_im, w/2, h/2);
    
    free(test_im);
    free(ref_im);
    return result;
}

int main() {
    int sizes[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        int result = test_size(size, size);
        if (result != 0) {
            printf("Failed for size %dx%d\n", size, size);
            return 1;
        } else {
            printf("Passed for size %dx%d\n", size, size);
        }
    }
    
    printf("All tests passed!\n");
    return 0;
}
