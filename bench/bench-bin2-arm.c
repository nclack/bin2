#include <arm_neon.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_RUNS (1<<4)
#define PROGRESS_BAR_WIDTH 50

extern void bin2x2(uint8_t* im, int w, int h);

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

static volatile float dummy_sum = 0;

static BenchmarkStats benchmark(size_t size) {
    uint64_t times[NUM_RUNS];
    uint64_t min_time = UINT64_MAX, max_time = 0;
    uint64_t total_time = 0;

    const size_t w=size;
    const size_t h=size;
    const size_t nbytes=w*h;

    uint8_t* input = (uint8_t*)aligned_alloc(32, nbytes);
    if(!input) {
      printf("Memory allocation failed for size %dx%d\n",(int)w,(int)h);
      free(input);
      exit(1);
    }

    for (int i = 0; i < NUM_RUNS; i++) {
        for (size_t i = 0; i < nbytes; i++) input[i] = rand() % 256;

        uint64_t start = read_cycle_counter();
        bin2x2(input,w,h);
        uint64_t end = read_cycle_counter();
        dummy_sum+=input[(nbytes>>2)-1];
        times[i] = end - start;
        
        min_time = (times[i] < min_time) ? times[i] : min_time;
        max_time = (times[i] > max_time) ? times[i] : max_time;
        total_time += times[i];

        print_progress_bar(i + 1, NUM_RUNS);
    }
    printf("\nBenchmark completed.\n\n");
    free(input);

    double mean = (double)total_time / NUM_RUNS;
    double sum_sq_diff = 0;
    for (int i = 0; i < NUM_RUNS; i++) {
        double diff = (double)times[i] - mean;
        sum_sq_diff += diff * diff;
    }
    double stddev = sqrt(sum_sq_diff / NUM_RUNS);

    double ipc = (double)nbytes / min_time;  // Assuming one instruction per element

    return (BenchmarkStats){min_time, max_time, mean, stddev, ipc};
}



int main() {
  enable_cycle_counter();
  dummy_sum=0.0f;

  int sizes[] = {8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16};
  int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
  BenchmarkStats *stats=(BenchmarkStats*)malloc(sizeof(BenchmarkStats)*num_sizes);
  if(!stats) {
    printf("Allocation of benchmark stats failed\n");
    exit(1);
  }
  
  for(int i=0;i<num_sizes;++i) {
    printf("Running benchmark %dx%d:\n",sizes[i],sizes[i]);
    stats[i] = benchmark(sizes[i]);
#if 0
    printf("bin2x2 %dx%d:\n",sizes[i],sizes[i]);
    printf("  Min: %lu cycles\n  Max: %lu cycles\n  Mean: %.2f cycles\n  "
           "StdDev: %.2f cycles\n  Elements per cycle: %.4f\n\n",
           stats[i].min, stats[i].max, stats[i].mean,
           stats[i].stddev, stats[i].ipc);
  #endif
  }
  printf("Benchmarks complete (dummy=%f)\n",dummy_sum);
  

  for(int i=0;i<num_sizes;++i) {
    printf("%5d x %-5d %f elements per cycle\n",sizes[i],sizes[i],stats[i].ipc);
  }

  return 0;
}
