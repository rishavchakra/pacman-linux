#include "kmodule.h"
#include <stdio.h>

#define PROC_NAME "/proc/PACMAN-kmod"
void *kmod_virt_to_phys(void *vaddr) {
  FILE *fptr;
  void *paddr;
  fptr = fopen(PROC_NAME, "w");
  if (fptr == NULL) {
    fprintf(stderr, "PACMAN kernel module is not running!\n");
    goto err; // ew... but also :)
  }

  // Write to the /proc/PACMAN-kmod file
  // v prefix denotes a Virt->phys translation request
  fprintf(fptr, "v%zu", (size_t)vaddr);
  fclose(fptr);

  fptr = fopen(PROC_NAME, "r");
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

int kmod_data_gadget(bool take_pac_path, void *guess_ptr) {
  FILE *fptr;

  fptr = fopen(PROC_NAME, "w");
  if (fptr == NULL) {
    fprintf(stderr, "PACMAN kernel module is not running!\n");
    goto err;
  }

  if (!take_pac_path) {
    // Doesn't really matter, I just want to make sure that the parsing works ok
    // on the module end
    guess_ptr = &fptr;
  }

  fprintf(fptr, "d%c%zu", take_pac_path ? 'y' : 'n', (size_t)guess_ptr);
  fclose(fptr);

  fptr = fopen(PROC_NAME, "r");
  int proc_rc;
  int rc = fscanf(fptr, "%d", &proc_rc);
  if (rc == EOF || rc == 0) {
    goto err;
  }

  if (proc_rc == 0) {
    printf("Kernel module took PAC authorization path!\n");
  } else if (proc_rc == 1) {
    printf("Kernel module took base path!\n");
  } else if (proc_rc == -1) {
    printf("Kernel module data gadget errored! Check dmesg\n");
  } else {
    printf("what?\n");
  }

  fclose(fptr);
  return 0;

err:
  printf("ERROR: kmod data gadget\n");
  fclose(fptr);
  return -1;
}

void *kmod_get_paddr() {
  FILE *fptr;

  fptr = fopen(PROC_NAME, "w");
  if (fptr == NULL) {
    fprintf(stderr, "PACMAN kernel module is not running!\n");
    goto err;
  }

  fprintf(fptr, "t");
  fclose(fptr);

  fptr = fopen(PROC_NAME, "r");
  void *ret_paddr;
  int rc = fscanf(fptr, "%p", &ret_paddr);
  if (rc == EOF || rc == 0) {
    goto err;
  }
  fclose(fptr);
  return ret_paddr;

err:
  fclose(fptr);
  return NULL;
}
