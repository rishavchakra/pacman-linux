#ifndef __KMODULE_H
#define __KMODULE_H

#include <stdbool.h>

void *kmod_virt_to_phys(void *);

int kmod_data_gadget(bool take_pac_path, void *guess_ptr);

#endif // !__KMODULE_H
