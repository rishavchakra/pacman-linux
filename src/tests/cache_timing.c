/**
 * This file tests the ability to time memory access times and detect
 * differences between accesses from different levels of cache/memory.
 * The tested pointer and eviction set are made up of userspace memory.
 */

#include "../kmodule.h"
#include "../timer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  uint64_t time_dram, time_cache_maybe, time_cache_definite;

  // Define a pointer
  void *ptr = malloc(1);
  // Get its eviction set

  // Clear (evict) pointer's cache set (at all levels)
  kmod_flush_cache(ptr);

  // Time the memory access (time from DRAM)
  time_dram = time_access(ptr);

  // The item should be in cache now
  // Time the memory access (time from L1 cache)
  time_cache_maybe = time_access(ptr);

  // Useless data write
  *(char *)ptr = 'a';

  // DEFINITELY in cache now!
  time_cache_definite = time_access(ptr);

  // Clear L1 caches (not L2, L3)

  // Time the memory access (time from L2 cache)

  // Clear L1, L2 caches (not L3)

  // Time the memory access (time from L3 cache)

  // Process and print everything!
  long double ftime_dram, ftime_cache_m, ftime_cache_d, ratio_ram_cache_m,
      ratio_ram_cache_d;
  ftime_dram = (long double)time_dram;
  ftime_cache_m = (long double)time_cache_maybe;
  ftime_cache_d = (long double)time_cache_definite;

  ratio_ram_cache_m = ftime_dram / ftime_cache_m;
  ratio_ram_cache_d = ftime_dram / ftime_cache_d;

  printf("DRAM: %lu\n"
         "Cache (?): %lu\n"
         "DRAM/cache: %Lf\n"
         "Cache (!): %lu\n"
         "DRAM/cache: %Lf\n",
         time_dram, time_cache_maybe, ratio_ram_cache_m, time_cache_definite,
         ratio_ram_cache_d);

  return 0;
}
