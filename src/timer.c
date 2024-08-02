#include "timer.h"

uint64_t timer_overhead = 0;

uint64_t time_access(addr_t addr) {
  uint64_t t1, t2, val;

  asm volatile("dsb sy\n\t"
               "isb\n\t"
               "mrs %[t1], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "ldr %[val], [%[addr]]\n\t"
               "isb\n\t"
               "mrs %[t2], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "dsb sy\n\t"
               : [t1] "=r"(t1), [val] "=r"(val), [t2] "=r"(t2)
               : [addr] "r"(addr));

  return t2 - t1 - timer_overhead;
}

uint64_t time_store(addr_t addr) {
  uint64_t t1, t2;
  uint64_t val = 0x1122334455667788;

  asm volatile("dsb sy\n\t"
               "isb\n\t"
               "mrs %[t1], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "str %[val], [%[addr]]\n\t"
               "isb\n\t"
               "mrs %[t2], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "dsb sy\n\t"
               : [t1] "=r"(t1), [t2] "=r"(t2)
               : [addr] "r"(addr), [val] "r"(val));

  return t2 - t1 - timer_overhead;
}

uint64_t time_exec(addr_t addr) {
  uint64_t t1, t2;

  asm volatile("dsb sy\n\t"
               "isb\n\t"
               "mrs %[t1], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "blr [%[addr]]\n\t"
               "isb\n\t"
               "mrs %[t2], PMCCNTR_EL0\n\t"
               "isb\n\t"
               "dsb sy\n\t"
               : [t1] "=r"(t1), [t2] "=r"(t2)
               : [addr] "r"(addr));

  return t2 - t1 - timer_overhead;
}

uint64_t time_overhead() {
  uint64_t t1, t2;

  asm volatile(
      // "dsb sy\n\t"
      "isb\n\t"
      "mrs %[t1], PMCCNTR_EL0\n\t"
      "isb\n\t"
      "nop\n\t"
      "isb\n\t"
      "mrs %[t2], PMCCNTR_EL0\n\t"
      "isb\n\t"
      // "dsb sy\n\t"
      : [t1] "=r"(t1), [t2] "=r"(t2));

  timer_overhead = t2 - t1;

  return t2 - t1;
}
