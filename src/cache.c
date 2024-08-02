#include "cache.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

cache_system_t glob_cache_system;

//////////////// Helper functions ////////////////

int read_cache_info(cache_t *, int index);

//////////////// Function implementations ////////////////

void init_system_cache_info() {
  int rc;
  int cache_ind = 0;
  bool l1_is_unified = 0;
  cache_t cache_read;
  while (1) {
    rc = read_cache_info(&cache_read, cache_ind);
    // Will stop the loop automatically as soon as there are no more
    // caches to read
    if (rc != 0) {
      break;
    }
    cache_t *dest;

    if (cache_ind == 0 && cache_read.type == CACHE_UNIFIED) {
      printf("L1 cache is unified\n");
      l1_is_unified = 1;
    }

    if (cache_ind == 0) {
      dest = &glob_cache_system.l1d;
    } else if (cache_ind + l1_is_unified == 1) {
      dest = &glob_cache_system.l1i;
    } else if (cache_ind + l1_is_unified == 2) {
      dest = &glob_cache_system.l2;
    } else if (cache_ind + l1_is_unified == 3) {
      dest = &glob_cache_system.l3;
    } else {
      fprintf(stderr, "WARNING: Unknown cache detected: %d\n", cache_ind);
      break;
    }
    printf("Cache %d info registered\n", cache_ind);
    cache_ind++;
  }
}

uint64_t cache_get_l1_tag(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l1i.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l1i.sets)));
  return addr_num >> (offset_length + set_length);
}

uint64_t cache_get_l1_set(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l1i.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l1i.sets)));
  // Clear set-> bits
  mask >>= offset_length + set_length;
  mask <<= offset_length + set_length;
  // Invert; set-> bits all 1
  mask = ~mask;
  uint64_t set_offset_bits = mask & addr_num;
  return set_offset_bits >> offset_length;
}

uint64_t cache_get_l1_offset(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l1i.line_size)));
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  mask >>= offset_length;
  mask <<= offset_length;
  mask = ~mask;
  return addr_num & mask;
}

uint64_t cache_get_l2_tag(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l2.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l2.sets)));
  return addr_num >> (offset_length + set_length);
}

uint64_t cache_get_l2_set(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l2.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l2.sets)));
  // Clear set-> bits
  mask >>= offset_length + set_length;
  mask <<= offset_length + set_length;
  // Invert; set-> bits all 1
  mask = ~mask;
  uint64_t set_offset_bits = mask & addr_num;
  return set_offset_bits >> offset_length;
}

uint64_t cache_get_l2_offset(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l2.line_size)));
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  mask >>= offset_length;
  mask <<= offset_length;
  mask = ~mask;
  return addr_num & mask;
}

uint64_t cache_get_l3_tag(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l3.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l3.sets)));
  return addr_num >> (offset_length + set_length);
}

uint64_t cache_get_l3_set(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l3.line_size)));
  int set_length = (int)(round(log2((double)glob_cache_system.l3.sets)));
  // Clear set-> bits
  mask >>= offset_length + set_length;
  mask <<= offset_length + set_length;
  // Invert; set-> bits all 1
  mask = ~mask;
  uint64_t set_offset_bits = mask & addr_num;
  return set_offset_bits >> offset_length;
}

uint64_t cache_get_l3_offset(void *addr) {
  uint64_t addr_num = (uint64_t)addr;
  int offset_length =
      (int)(round(log2((double)glob_cache_system.l3.line_size)));
  uint64_t mask = 0xFFFFFFFFFFFFFFFF;
  mask >>= offset_length;
  mask <<= offset_length;
  mask = ~mask;
  return addr_num & mask;
}

// TODO
// (how do I even flush the instruction cache? will it work to just load in a
// bunch of functions into function pointers?)
void cache_flush_l1i() {}

void cache_flush_l1d() {}

void cache_flush_l2() {}

void cache_flush_l3() {}

int read_cache_info(cache_t *cache, int index) {
  const char *base_path = "/sys/devices/system/cpu/cpu0/cache/index";
  FILE *fptr;

  char level_path[64];
  char type_path[64];
  char line_size_path[64];
  char sets_path[64];
  char ways_path[64];
  char size_path[64];

  // These should be large enough to read cache info
  char level_val[2];
  char type_val[32];
  char line_size_val[8];
  char sets_val[8];
  char ways_val[8];
  char size_val[16];

  sprintf(level_path, "%s%d/level", base_path, index);
  sprintf(type_path, "%s%d/type", base_path, index);
  sprintf(line_size_path, "%s%d/coherency_line_size", base_path, index);
  sprintf(sets_path, "%s%d/number_of_sets", base_path, index);
  sprintf(ways_path, "%s%d/ways_of_associativity", base_path, index);
  sprintf(size_path, "%s%d/size", base_path, index);

  {
    fptr = fopen(level_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d not found\n", index);
      return -1;
    }
    fgets(level_val, 2, fptr);
    fclose(fptr);
    int level = atoi(level_val);
    if (!level) {
      fprintf(stderr, "Cache %d level parsing failed: %s\n", index, level_val);
      return -1;
    }
    cache->level = level;
  }

  {
    fptr = fopen(type_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d type not found\n", index);
      return -1;
    }
    fgets(type_val, 32, fptr);
    fclose(fptr);
    if (strncmp(type_val, "Data", 4) == 0) {
      cache->type = CACHE_DATA;
    } else if (strncmp(type_val, "Instruction", 11) == 0) {
      cache->type = CACHE_INSTRUCTION;
    } else if (strncmp(type_val, "Unified", 7) == 0) {
      cache->type = CACHE_UNIFIED;
    } else {
      fprintf(stderr, "Cache %d type parsing failed: %s\n", index, type_val);
      return -1;
    }
  }

  {
    fptr = fopen(line_size_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d line size not found\n", index);
      return -1;
    }
    fgets(line_size_val, 8, fptr);
    fclose(fptr);
    int line_size = atoi(line_size_val);
    if (!line_size) {
      fprintf(stderr, "Cache %d line size parsing failure: %s\n", index,
              line_size_val);
      return -1;
    }
    cache->line_size = line_size;
  }

  {
    fptr = fopen(ways_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d associative ways not found\n", index);
      return -1;
    }
    fgets(ways_val, 8, fptr);
    fclose(fptr);
    int ways = atoi(ways_val);
    if (!ways) {
      fprintf(stderr, "Cache %d ways parsing failure: %s\n", index, ways_val);
      return -1;
    }
    cache->ways = ways;
  }

  {
    fptr = fopen(sets_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d sets number not found\n", index);
      return -1;
    }
    fgets(sets_val, 8, fptr);
    fclose(fptr);
    int sets = atoi(sets_val);
    if (!sets) {
      fprintf(stderr, "Cache %d sets number parsing failure: %s\n", index,
              sets_val);
      return -1;
    }
    cache->sets = sets;
  }

  {
    fptr = fopen(size_path, "r");
    if (!fptr) {
      fprintf(stderr, "Cache %d size not found\n", index);
      return -1;
    }
    fgets(size_val, 16, fptr);
    fclose(fptr);
    int size_len = strlen(size_val);
    // Overwrite the 'K' at the end of the string
    size_val[size_len - 1] = '\0';
    int size = atoi(size_val);
    if (!size) {
      fprintf(stderr, "Cache %d size parsing failure: %s\n", index, size_val);
      return -1;
    }
  }

  return 0;
}
