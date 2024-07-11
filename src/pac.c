#include "pac.h"

PAC pac_extract(void *ptr) {
  uint64_t ptr_num = (uint64_t)ptr;
  PAC pac_lower = (ptr_num >> 47) & 0xFF;
  PAC pac_upper = (ptr_num >> 56) & 0xFF;
  return (pac_upper << 8) | pac_lower;
}

void *pac_encode(void *ptr, PAC pac) {
  uint64_t ptr_num = (uint64_t)ptr;
  uint64_t ptr_bare = ptr_num & (!PAC_MASK);
  uint64_t pac_lower = ( pac & 0x00FF );
  uint64_t pac_upper = ( pac & 0xFF00 );
  uint64_t pac_kernel_bit = pac & PAC_KERNEL_BIT;
  ptr_num = ptr_bare | (pac_lower << 47) | (pac_upper << 56) | pac_kernel_bit;
  return (void *)ptr_num;
}

bool ptr_is_kernel_ptr(void *ptr) {
  uint64_t ptr_num = (uint64_t)ptr;
  return (ptr_num & PAC_KERNEL_BIT) != 0;
}
