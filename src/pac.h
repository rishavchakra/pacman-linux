#ifndef __PAC_H
#define __PAC_H

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t PAC;

#define PAC_MASK 0xFFFF800000000000
#define PAC_KERNEL_BIT 0x0080000000000000
#define MAX_PAC UINT16_MAX

PAC pac_extract(void *);

void *pac_encode(void *, PAC);

bool ptr_is_kernel_ptr(void *);

#endif
