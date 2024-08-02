#ifndef __KMODULE_H
#define __KMODULE_H

#include "memory.h"
#include <stdbool.h>

void *kmod_virt_to_phys(void *);

int kmod_data_gadget(bool take_pac_path, void *guess_ptr);

void kmod_auth_gadget(bool take_pac_path, char *str);

// Returns a pointer to some target function/instruction
// physical address, for use in generating an eviction set
addr_t kmod_get_paddr();

// Returns a pointer to some target function/instruction
// virtual address, for use in the actual PAC generation and testing
addr_t kmod_get_vaddr();

#endif // !__KMODULE_H
