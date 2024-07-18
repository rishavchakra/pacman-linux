/**
 * This test file tests the interaction with the PACMAN-kmod kernel module.
 */

#include "../kmodule.h"
#include <stdio.h>
#include <stdlib.h>
int main() {
  printf("================================\n");
  printf("Virtual -> Physical address translation test\n");
  // NOTE:
  // I really have no way of knowing if the result is correct.
  // This test mostly makes sure that the userspace<- ->kmod communication works
  void *vaddr = malloc(100);
  void *paddr = kmod_virt_to_phys(vaddr);
  printf("Virtual:  %p\nPhysical: %p\n", vaddr, paddr);
  free(vaddr);

  return 0;
}
