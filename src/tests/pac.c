/**
 * This test file tests inline assembly for signing pointers with a PAC.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if !defined(__aarch64__)
#error Test can only be compiled on ARM64.
#endif

#define PAC_SALT 0x0123456789ABCDEF

int test_fn(void);

int main() {
  {
    printf("Data pointer signing\n");
    void *buf = malloc(10);
    uint64_t salt = PAC_SALT;

    printf("Pointer before signing:\n%p\n", buf);

    asm("pacda %[ptr], %[salt]" : [ptr] "+r"(buf) : [salt] "r"(salt));

    printf("Pointer after signing:\n%p\n", buf);

    asm("autda %[ptr], %[salt]" : [ptr] "+r"(buf) : [salt] "r"(salt));

    printf("Pointer after authenticating:\n%p\n", buf);

    printf("Data pointer authentication successful!\n");
  }

  printf("================================\n");

  {
    printf("Instruction pointer signing\n");
    int (*fn_ptr)() = test_fn;
    uint64_t salt = PAC_SALT;

    printf("Pointer before signing:\n%p\n", fn_ptr);

    asm("pacia %[ptr], %[salt]" : [ptr] "+r"(fn_ptr) : [salt] "r"(salt));

    printf("Pointer after signing:\n%p\n", fn_ptr);

    asm("autia %[ptr], %[salt]" : [ptr] "+r"(fn_ptr) : [salt] "r"(salt));

    printf("Pointer after authenticating:\n%p\n", fn_ptr);

    printf("Instruction pointer authentication successful!\n");
  }

  return 0;
}

int test_fn(void) {
  printf("Test function!\n");
  return 0;
}
