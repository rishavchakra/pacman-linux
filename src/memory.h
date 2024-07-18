#ifndef __MEMORY_H
#define __MEMORY_H

#include <stddef.h>
#include <stdint.h>

// TODO: double check these
#define INSTR_RET 0xd65f03c0
#define INSTR_NOP 0xd503201f

typedef void *addr_t;

typedef struct {
  uint64_t page_size;
  uint64_t page_offset_mask;
  uint64_t tlb_offset_mask;
} glob_mem_info_t;

extern glob_mem_info_t glob_mem_info;

void init_system_mem_info();

/*
 * Creates a page full of ret instructions
 * with executable permissions
 *
 * # In
 *
 * page: a buffer starting at the beginning of a page boundary
 * that is at least as long as a memory page
 *
 * # Out
 *
 * 0 on success, -1 on failure
 */
int mem_create_ret_page(void *page);

int mem_create_ret_range(void *buffer, size_t size);

#endif
