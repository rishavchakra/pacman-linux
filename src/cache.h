#ifndef __CACHE_H
#define __CACHE_H

#include <stdint.h>

typedef enum {
  CACHE_DATA,
  CACHE_INSTRUCTION,
  CACHE_UNIFIED,
} cache_type_t;

typedef struct {
  int id;
  // Cache level
  int level;
  // Cache type (instruction/data/unified)
  cache_type_t type;
  int line_size;
  // Number of ways of set associativity
  int ways;
  // Number of sets in cache
  int sets;
  int size;
} cache_t;

typedef struct {
  int page_size;
  // Level 1 Data cache
  // Unified cache if no instruction cache exists
  cache_t l1d;
  // Level 1 Instruction cache
  cache_t l1i;
  // Level 2 Unified cache
  cache_t l2;
  // Level 3 Unified cache
  cache_t l3;
  // Other caches not supported yet. I'm not sure if there even is anything
  // other than l1i/d, l2, l3. Add handling logic to init_system_cache_info impl
} cache_system_t;

extern cache_system_t glob_cache_system;

// Initialized the global glob_cache_system struct
// Reads the kernel's saved data about the CPU cache
void init_system_cache_info();

// Get the tag of the address within the set
uint64_t cache_get_l1_tag(void *);

// Get the index of the set
uint64_t cache_get_l1_set(void *);

// Get the offset of the data within the cache line
uint64_t cache_get_l1_offset(void *);

// Get the tag of the address within the set
uint64_t cache_get_l2_tag(void *);

// Get the index of the set
uint64_t cache_get_l2_set(void *);

// Get the offset of the data within the cache line
uint64_t cache_get_l2_offset(void *);

// Get the tag of the address within the set
uint64_t cache_get_l3_tag(void *);

// Get the index of the set
uint64_t cache_get_l3_set(void *);

// Get the offset of the data within the cache line
uint64_t cache_get_l3_offset(void *);

#endif
