#ifndef __PACMAN_H
#define __PACMAN_H

#include "pac.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// This needs to be sufficiently high so that the branch predictor WILL think a
// path is certain. If not, we run into a kernel crash (stakes are high!)
#define NUM_BRANCHPRED_TRAIN 24

// MIT code used something like this
// not necessary for a demo
// typedef struct {
//   PAC next_pac;
//   bool pac_checked_last;
//   void *cur_addr;
//   // from MIT code
//   // uint64_t (*time_use_fn)(void *);
//   // uint64_t (*try_speculative)();
//   // uint64_t (*try_non_speculative)();
// } pacman_data_t;

// extern pacman_data_t pacman_data;

// void *get_next_pac();

// Function prototype from MIT code
// uint64_t *pacman_try_one(
//   void *ptr_to_forge,
//   void *known_good_ptr,
//   void *guess_ptr,
//   uint64_t *forge_eviction_set,
//   size_t *forge_eviction_set_indices,
//   uint64_t *limit_eviction_set,
//   size_t *limit_eviction_set_indices
// );

void pacman_run();

#endif // !__PACMAN_H
