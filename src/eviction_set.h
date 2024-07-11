#ifndef __EVICTION_SET_H
#define __EVICTION_SET_H

#include <stddef.h>
#include <stdint.h>

#define EVSET_STRIDE (4096 * 4096)
#define EVSET_SIZE_MAX 1024

//////////////// Eviction Set Data Type ////////////////

typedef void *addr_t;

typedef struct eviction_set eviction_set_t;

eviction_set_t *eviction_set_new();

void eviction_set_add(eviction_set_t *, addr_t);

void eviction_set_free(eviction_set_t *evset);

//////////////// Eviction Set Generation (from memory) ////////////////

eviction_set_t *evset_inst(addr_t addr, void *region_base, size_t region_len);

eviction_set_t *evset_data(addr_t addr, addr_t region_base, size_t region_len);

eviction_set_t *evset_p_data(addr_t target_vaddr, addr_t target_paddr,
                             addr_t region_base, size_t region_len);

eviction_set_t *evset_p_inst();

eviction_set_t *evset_kp_data();

#endif // !__EVICTION_SET_H
