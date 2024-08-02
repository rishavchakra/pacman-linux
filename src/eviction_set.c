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

addr_t *get(eviction_set_t *evset) {
  return evset->buf;
}

size_t length(eviction_set_t *evset) {
  return evset->length;
}

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
  // May need use of kext kernel VA->PA conversion
  // but commented out in MIT code
  eviction_set_t *evset = eviction_set_new();

  size_t offset = ((uint64_t)target_vaddr) & glob_mem_info.tlb_offset_mask;
  // Used for kext VA->PA conversion
  // uint64_t ptarget_l2_set = cache_get_l2_tag(target_paddr);

  size_t i;
  for (i = 0; i < region_len; i += EVSET_STRIDE) {
    size_t idx = i + offset;

    if (idx > region_len) {
      break;
    }

    if (evset->length >= EVSET_SIZE_MAX) {
      break;
    }

    // CHECK: multiplication factor of the index (should it be sizeof(addr_t)?)
    eviction_set_push(evset, region_base + idx);
  }

  // CHECK: almost used for VA incongruency checking in MIT code
  // uint64_t vtarget_l1_set = cache_get_l1_set(target_vaddr);

  return evset;
}

eviction_set_t *evset_p_inst(addr_t target_vaddr, addr_t target_paddr,
                             addr_t region_base, size_t region_len) {
  eviction_set_t *evset =
      evset_p_data(target_vaddr, target_paddr, region_base, region_len);

  int i;
  for (i = 0; i < evset->length; ++i) {
    addr_t pg = evset->buf[i];
    mem_create_ret_page(pg);
  }

  return evset;
}

// TODO: connect to elevated service that can translate VA->PA
eviction_set_t *evset_kp_data(addr_t target_vaddr, addr_t target_paddr,
                              addr_t kregion_base, size_t kregion_len) {
  eviction_set_t *evset = eviction_set_new();

  uint64_t ptarget_l2_set = cache_get_l2_set(target_paddr);

  int i;
  // idk where 128 comes from. see MIT code
  for (i = 0; i < kregion_len; i += 128) {
    if (i > kregion_len) {
      break;
    }

    addr_t cur_va = kregion_base + i;
    // Use kext for this
    // addr_t cur_pa = virt_to_phys(cur_va);
    // In the mean time, do this (which doesn't work as intended)
    addr_t cur_pa = cur_va;
    if (cache_get_l2_set(cur_pa) == ptarget_l2_set) {
      eviction_set_push(evset, cur_va);
    }
  }

  uint64_t vtarget_l1_set = cache_get_l2_set(target_vaddr);
  for (i = 0; i < evset->length; ++i) {
    if (cache_get_l1_set(evset->buf[i]) != vtarget_l1_set) {
      fprintf(stderr, "Error: Incongruent Virtual Addresses\n");
      return NULL;
    }
  }

  return evset;
}
