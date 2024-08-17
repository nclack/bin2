#include <arm_neon.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE (1ULL << 24)
#define NUM_RUNS (1<<11)
#define PROGRESS_BAR_WIDTH 50

static void print_progress_bar(int progress, int total) {
    int filled_width = PROGRESS_BAR_WIDTH * progress / total;
    printf("\r[");
    for (int i = 0; i < PROGRESS_BAR_WIDTH; ++i) {
        if (i < filled_width) printf("#");
        else printf(" ");
    }
    printf("] %3d%%", progress * 100 / total);
    fflush(stdout);
}

// These aren't true cumsum's but they do something close with a single 
// load, compute, store.
static void cumsum_inplace(float *input, float *output, uint64_t size) {
  float32x4_t sum = vdupq_n_f32(0);
  for (uint64_t i = 0; i < size; i += 4) {
    float32x4_t v = vld1q_f32(&input[i]);
    sum = vaddq_f32(sum, v);
    vst1q_f32(&input[i], sum);
  }
  __asm__ __volatile__ ("dmb ish" ::: "memory"); // memory barrier
}

static void cumsum_outofplace(float *input, float *output, uint64_t size) {
  float32x4_t sum = vdupq_n_f32(0);
  for (uint64_t i = 0; i < size; i += 4) {
    float32x4_t v = vld1q_f32(&input[i]);
    sum = vaddq_f32(sum, v);
    vst1q_f32(&output[i], sum);
  }
  __asm__ __volatile__ ("dmb ish" ::: "memory"); // memory barrier
}

typedef struct {
  uint64_t min;
  uint64_t max;
  double mean;
  double stddev;
  double ipc; // instructions per cycle
} BenchmarkStats;

#ifdef __aarch64__
static inline uint64_t read_cycle_counter(void) {
  uint64_t val;
  __asm__ __volatile__("mrs %0, pmccntr_el0" : "=r"(val));
  return val;
}
#else
static inline uint32_t read_cycle_counter(void) {
  uint32_t val;
  __asm__ __volatile__("mrc p15, 0, %0, c9, c13, 0" : "=r"(val));
  return val;
}
#endif

// Enable cycle counter
static inline void enable_cycle_counter(void) {
#ifdef __aarch64__
  uint64_t val;
  __asm__ __volatile__("mrs %0, pmcr_el0" : "=r"(val));
  val |= 1; // Enable bit
  __asm__ __volatile__("msr pmcr_el0, %0" : : "r"(val));

  __asm__ __volatile__("mrs %0, pmcntenset_el0" : "=r"(val));
  val |= (1 << 31); // Enable cycle counter
  __asm__ __volatile__("msr pmcntenset_el0, %0" : : "r"(val));
#else
  uint32_t val = 1;
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 0" : : "r"(val));
  val = 0x80000000;
  __asm__ __volatile__("mcr p15, 0, %0, c9, c12, 1" : : "r"(val));
#endif
}

static BenchmarkStats benchmark(void (*func)(float*, float*, uint64_t), float* input, float* output, uint64_t size, int runs) {
    uint64_t times[NUM_RUNS];
    uint64_t min_time = UINT64_MAX, max_time = 0;
    uint64_t total_time = 0;

    printf("Running benchmark: \n");
    for (int i = 0; i < runs; i++) {
        uint64_t start = read_cycle_counter();
        func(input, output, size);
        uint64_t end = read_cycle_counter();
        times[i] = end - start;
        
        min_time = (times[i] < min_time) ? times[i] : min_time;
        max_time = (times[i] > max_time) ? times[i] : max_time;
        total_time += times[i];

        print_progress_bar(i + 1, runs);
    }
    printf("\nBenchmark completed.\n\n");

    double mean = (double)total_time / runs;
    double sum_sq_diff = 0;
    for (int i = 0; i < runs; i++) {
        double diff = (double)times[i] - mean;
        sum_sq_diff += diff * diff;
    }
    double stddev = sqrt(sum_sq_diff / runs);

    double ipc = (double)size / min_time;  // Assuming one instruction per element

    return (BenchmarkStats){min_time, max_time, mean, stddev, ipc};
}

static volatile float dummy_sum = 0;

static void use_output(float *arr, uint64_t size) {
  for (uint64_t i = 0; i < size; i++) {
    dummy_sum += arr[i];
  }
}

int main() {
  enable_cycle_counter();
  float *input =
      (float *)aligned_alloc(16, ARRAY_SIZE * sizeof(float));
  float *output =
      (float *)aligned_alloc(16, ARRAY_SIZE * sizeof(float));

  if (!input || !output) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
    input[i] = (float)rand() / (float)RAND_MAX;
  }

  dummy_sum = 0;
  BenchmarkStats inplace_stats =
      benchmark(cumsum_inplace, input, output, ARRAY_SIZE, NUM_RUNS);
  use_output(input, ARRAY_SIZE);

  for (uint64_t i = 0; i < ARRAY_SIZE; i++) {
    input[i] = (float)rand() / (float) RAND_MAX;
  }

  dummy_sum = 0;
  BenchmarkStats outofplace_stats =
      benchmark(cumsum_outofplace, input, output, ARRAY_SIZE, NUM_RUNS);
  use_output(output, ARRAY_SIZE);

  printf("In-place cumsum:\n");
  printf("  Min: %lu cycles\n  Max: %lu cycles\n  Mean: %.2f cycles\n  "
         "StdDev: %.2f cycles\n  Elements per cycle: %.4f\n\n",
         inplace_stats.min, inplace_stats.max, inplace_stats.mean,
         inplace_stats.stddev, inplace_stats.ipc);

  printf("Out-of-place cumsum:\n");
  printf("  Min: %lu cycles\n  Max: %lu cycles\n  Mean: %.2f cycles\n  "
         "StdDev: %.2f cycles\n  Elements per cycle: %.4f\n",
         outofplace_stats.min, outofplace_stats.max, outofplace_stats.mean,
         outofplace_stats.stddev, outofplace_stats.ipc);

  printf("Dummy sum: %f\n", dummy_sum);

  free(input);
  free(output);

  return 0;
}
