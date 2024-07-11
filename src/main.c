#include "stdio.h"
#include "cache.h"
#include "memory.h"

int main() {
  printf("Starting PACMAN\n");
  init_system_cache_info();
  init_system_mem_info();
  return 0;
} 
