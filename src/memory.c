#include "memory.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

glob_mem_info_t glob_mem_info;

void init_system_mem_info() {
  glob_mem_info.page_size = getpagesize();
  glob_mem_info.page_offset_mask = glob_mem_info.page_size - 1;
  // I don't really know where this number comes from
  glob_mem_info.tlb_offset_mask = 0x7FFFFFFFFF;
}

int mem_create_ret_page(void *page) {
  int rc;
  int i;

  rc = mprotect(page, glob_mem_info.page_size, PROT_READ | PROT_WRITE);
  if (rc == -1) {
    fprintf(stderr, "Could not create page of returns\n");
    return -1;
  }

  // AArch64 opcodes are 32 bits long
  uint32_t *instr_page = (uint32_t *)page;
  for (i = 0; i < glob_mem_info.page_size; ++i) {
    instr_page[i] = INSTR_RET;
  }

  rc = mprotect(instr_page, glob_mem_info.page_size, PROT_READ | PROT_EXEC);
  if (rc == -1) {
    fprintf(stderr, "Could not create page of returns\n");
    return -1;
  }

  return 0;
}

int mem_create_ret_range(void *buffer, size_t size) {
  int rc;
  int i;

  rc = mprotect(buffer, size, PROT_READ | PROT_WRITE);
  if (rc == -1) {
    fprintf(stderr, "Could not create range of returns\n");
    return -1;
  }

  // AArch64 opcodes are 32 bits long
  uint32_t *instr_buf = (uint32_t *)buffer;
  for (i = 0; i < size; ++i) {
    instr_buf[i] = INSTR_RET;
  }

  rc = mprotect(instr_buf, size, PROT_READ | PROT_EXEC);
  if (rc == -1) {
    fprintf(stderr, "Could not create range of returns\n");
    return -1;
  }

  return 0;
}
