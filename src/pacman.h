#ifndef __PACMAN_H
#define __PACMAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "pac.h"

typedef struct {
  PAC next_pac;
  bool pac_checked_last;
  void *cur_addr;
  uint64_t (*time_use_fn)(void *);
  uint64_t (*try_speculative)();
  uint64_t (*try_non_speculative)();
} pacman_data_t;

extern pacman_data_t pacman_data;

extern PAC beep;

void *get_next_pac();

uint64_t *pacman_try_one(
  void *ptr_to_forge,
  void *known_good_ptr,
  void *guess_ptr,
  uint64_t *forge_eviction_set,
  size_t *forge_eviction_set_indices,
  uint64_t *limit_eviction_set,
  size_t *limit_eviction_set_indices
);

#endif // !__PACMAN_H
