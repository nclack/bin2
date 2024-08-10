#include <arm_neon.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <float.h>

#define ARRAY_SIZE (1<<20)
#define NUM_RUNS 100

static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

void cumsum_inplace(float* arr, int size) {
    float32x4_t sum = vdupq_n_f32(0);
    for (int i = 0; i < size; i += 4) {
        float32x4_t v = vld1q_f32(&arr[i]);
        sum = vaddq_f32(sum, v);
        vst1q_f32(&arr[i], sum);
    }
}

void cumsum_outofplace(const float* input, float* output, int size) {
    float32x4_t sum = vdupq_n_f32(0);
    for (int i = 0; i < size; i += 4) {
        float32x4_t v = vld1q_f32(&input[i]);
        sum = vaddq_f32(sum, v);
        vst1q_f32(&output[i], sum);
    }
}

typedef struct {
    double min;
    double max;
    double mean;
    double stddev;
    double throughput;
} BenchmarkStats;

BenchmarkStats benchmark(void (*func)(float*, int), float* arr, int size, int runs) {
    double times[NUM_RUNS];
    double min_time = DBL_MAX, max_time = 0, total_time = 0;

    for (int i = 0; i < runs; i++) {
        uint64_t start = get_time_ns();
        func(arr, size);
        uint64_t end = get_time_ns();
        times[i] = (end - start) / 1e9;
        
        min_time = fmin(min_time, times[i]);
        max_time = fmax(max_time, times[i]);
        total_time += times[i];
    }

    double mean = total_time / runs;
    double sum_sq_diff = 0;
    for (int i = 0; i < runs; i++) {
        double diff = times[i] - mean;
        sum_sq_diff += diff * diff;
    }
    double stddev = sqrt(sum_sq_diff / runs);

    double ops = (double)size * runs;
    double throughput = ops / total_time / 1e9;  // GFLOPS

    return (BenchmarkStats){min_time, max_time, mean, stddev, throughput};
}

int main() {
    float* arr1 = (float*)aligned_alloc(16, ARRAY_SIZE * sizeof(float));
    float* arr2 = (float*)aligned_alloc(16, ARRAY_SIZE * sizeof(float));

    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (float)rand() / RAND_MAX;
        arr2[i] = arr1[i];
    }

    BenchmarkStats inplace_stats = benchmark(cumsum_inplace, arr1, ARRAY_SIZE, NUM_RUNS);
    BenchmarkStats outofplace_stats = benchmark((void (*)(float*, int))cumsum_outofplace, arr2, ARRAY_SIZE, NUM_RUNS);

    printf("In-place cumsum:\n");
    printf("  Min: %.9f s\n  Max: %.9f s\n  Mean: %.9f s\n  StdDev: %.9f s\n  Throughput: %.2f GFLOPS\n\n",
           inplace_stats.min, inplace_stats.max, inplace_stats.mean, inplace_stats.stddev, inplace_stats.throughput);

    printf("Out-of-place cumsum:\n");
    printf("  Min: %.9f s\n  Max: %.9f s\n  Mean: %.9f s\n  StdDev: %.9f s\n  Throughput: %.2f GFLOPS\n",
           outofplace_stats.min, outofplace_stats.max, outofplace_stats.mean, outofplace_stats.stddev, outofplace_stats.throughput);

    free(arr1);
    free(arr2);

    return 0;
}
