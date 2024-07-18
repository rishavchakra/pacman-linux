/**
 * This test file tests the interaction with the PACMAN-kmod kernel module.
 */

#include "../kmodule.h"
#include <stdio.h>
#include <stdlib.h>
int main() {
  {
    printf("================================\n");
    printf("Virtual -> Physical address translation test\n");
    // NOTE:
    // I really have no way of knowing if the result is correct.
    // This test mostly makes sure that the userspace<- ->kmod communication
    // works
    void *vaddr = malloc(10);
    void *paddr = kmod_virt_to_phys(vaddr);
    printf("Virtual:  %p\nPhysical: %p\n", vaddr, paddr);
    paddr = kmod_virt_to_phys(vaddr);
    printf("Again! (should be the same)\nVirtual:  %p\nPhysical: %p\n", vaddr, paddr);
    free(vaddr);
  }

  {
    printf("================================\n");
    printf("Data gadget testing\n");
    // NOTE:
    // This is more useful when you're also looking at dmesg to see what the
    // kernel outputs. The return value is purposely not very descriptive
    // because the module function needs to be fast.
    void *buf1 = malloc(10);
    void *buf2 = malloc(10);
    printf("Request: Take the PAC path for guess pointer %p\n", buf1);
    kmod_data_gadget(true, buf1);
    printf("Request: Take the PAC path for guess pointer %p\n", buf2);
    kmod_data_gadget(true, buf2);
    printf("Request: Take the base path for guess pointer %p\n", buf1);
    kmod_data_gadget(false, buf1);
    printf("Request: Take the base path for guess pointer %p\n", buf2);
    kmod_data_gadget(false, buf2);
    free(buf1);
    free(buf2);
  }

  return 0;
}
