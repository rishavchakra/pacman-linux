#include <stdio.h>
void *kmod_virt_to_phys(void *vaddr) {
  FILE *fptr;
  void *paddr;
  fptr = fopen("/proc/PACMAN-kmod", "w");
  if (fptr == NULL) {
    fprintf(stderr, "PACMAN kernel module is not running!\n");
    goto err; // ew... but also :)
  }

  // Write to the /proc/PACMAN-kmod file
  fprintf(fptr, "%zu", (size_t)vaddr);
  fclose(fptr);

  fptr = fopen("/proc/PACMAN-kmod", "r");
  // The kernel module writes back to it
  int rc = fscanf(fptr, "%p", &paddr);
  if (rc == EOF || rc == 0) {
    goto err;
  }
  fclose(fptr);
  return paddr;

err:
  fclose(fptr);
  return NULL;
}
