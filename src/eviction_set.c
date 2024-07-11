#include "eviction_set.h"
#include "cache.h"
#include "memory.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct eviction_set {
  addr_t *buf;
  size_t length;
  size_t capacity;
} eviction_set_t;

eviction_set_t *eviction_set_new() {
  eviction_set_t *evset = malloc(sizeof(eviction_set_t));
  evset->buf = malloc(sizeof(addr_t) * 8);
  evset->length = 0;
  evset->capacity = 8;
  return evset;
}

void eviction_set_push(eviction_set_t *evset, addr_t el) {
  if (evset->length == evset->capacity) {
    addr_t *new_buf = malloc(sizeof(addr_t *) * evset->capacity * 2);
    memcpy(new_buf, evset->buf, evset->capacity);
    free(evset->buf);
    evset->buf = new_buf;
    evset->capacity *= 2;
  }

  // TODO: FIX THIS
  evset->buf[evset->length] = el;
  evset->length++;
}

void eviction_set_free(eviction_set_t *evset) { free(evset->buf); }

eviction_set_t *evset_inst(addr_t addr, addr_t region_base, size_t region_len) {
  eviction_set_t *evset = eviction_set_new();

  int i;
  for (i = 0; i < glob_cache_system.l1i.ways; ++i) {
    size_t offset =
        (i + 1) * glob_cache_system.l1i.sets * glob_cache_system.l1i.line_size;

    if (offset > region_len / 4) {
      fprintf(stderr, "Error creating instruction eviction set");
      return NULL;
    }

    addr_t entry = region_base + offset;
    eviction_set_push(evset, entry);
  }

  return evset;
}

eviction_set_t *evset_data(addr_t addr, addr_t region_base, size_t region_len) {
  eviction_set_t *evset = eviction_set_new();

  int i;
  for (i = 0; i < glob_cache_system.l1d.ways; ++i) {
    size_t offset =
        (i + 1) * glob_cache_system.l1d.sets * glob_cache_system.l1d.line_size;

    if (offset > region_len / 4) {
      fprintf(stderr, "Error creating instruction eviction set");
      return NULL;
    }

    addr_t entry = region_base + offset;
    eviction_set_push(evset, entry);
  }

  return evset;
}

eviction_set_t *evset_p_data(addr_t target_vaddr, addr_t target_paddr,
                             addr_t region_base, size_t region_len) {
  eviction_set_t *evset = eviction_set_new();

  size_t offset = ((uint64_t)target_vaddr) & glob_mem_info.tlb_offset_mask;

  return evset;
}
