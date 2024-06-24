/**
 * This test file implements and compares several different timers
 * to determine resolution, precision, and performance
 */

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#if !defined(__aarch64__)
#error Test can only be compiled on ARM64.
#endif

//////////////// Function Definitions ////////////////

uint64_t get_time_pmccntr();

uint64_t get_time_cntpct();

uint64_t get_time_clock(clockid_t clock);

struct timespec clock_ts;

int main() {
  printf("PACMAN high frequency timers test\n");

  printf("----------------------------------------\n");
  uint64_t time_pmccntr[2];
  uint64_t time_cntpct[2];
  uint64_t time_clock_realtime[2];
  uint64_t time_clock_realtime_coarse[2];
  uint64_t time_clock_monotonic[2];
  uint64_t time_clock_monotonic_coarse[2];
  uint64_t time_clock_process[2];
  uint64_t time_clock_thread[2];

  for (int i = 0; i < 5; ++i) {
    time_pmccntr[0] = get_time_pmccntr();
    time_cntpct[0] = get_time_cntpct();
    time_clock_realtime[0] = get_time_clock(CLOCK_REALTIME);
    time_clock_realtime_coarse[0] = get_time_clock(CLOCK_REALTIME_COARSE);
    time_clock_monotonic[0] = get_time_clock(CLOCK_MONOTONIC);
    time_clock_monotonic_coarse[0] = get_time_clock(CLOCK_MONOTONIC_COARSE);
    time_clock_process[0] = get_time_clock(CLOCK_PROCESS_CPUTIME_ID);
    time_clock_thread[0] = get_time_clock(CLOCK_THREAD_CPUTIME_ID);

    sleep(i + 2);

    time_pmccntr[1] = get_time_pmccntr();
    time_cntpct[1] = get_time_cntpct();
    time_clock_realtime[1] = get_time_clock(CLOCK_REALTIME);
    time_clock_realtime_coarse[1] = get_time_clock(CLOCK_REALTIME_COARSE);
    time_clock_monotonic[1] = get_time_clock(CLOCK_MONOTONIC);
    time_clock_monotonic_coarse[1] = get_time_clock(CLOCK_MONOTONIC_COARSE);
    time_clock_process[1] = get_time_clock(CLOCK_PROCESS_CPUTIME_ID);
    time_clock_thread[1] = get_time_clock(CLOCK_THREAD_CPUTIME_ID);

    printf("Waited ~%d seconds\n", i + 2);
    printf("%-20s:%24lu\n", "PMCCNTR", time_pmccntr[1] - time_pmccntr[0]);
    printf("%-20s:%24lu\n", "CNTPCT", time_cntpct[1] - time_cntpct[0]);
    printf("%-20s:%24lu\n", "Realtime",
           time_clock_realtime[1] - time_clock_realtime[0]);
    printf("%-20s:%24lu\n", "Realtime coarse",
           time_clock_realtime_coarse[1] - time_clock_realtime_coarse[0]);
    printf("%-20s:%24lu\n", "Monotonic",
           time_clock_monotonic[1] - time_clock_monotonic[0]);
    printf("%-20s:%24lu\n", "Monotonic coarse",
           time_clock_monotonic_coarse[1] - time_clock_monotonic_coarse[0]);
    printf("%-20s:%24lu\n", "Process",
           time_clock_process[1] - time_clock_process[0]);
    printf("%-20s:%24lu\n", "Thread",
           time_clock_thread[1] - time_clock_thread[0]);
    printf("----------------------------------------\n");
  }
}

uint64_t get_time_pmccntr() {
  uint64_t time;
  // Hesitant to use this because it doesn't include pipeline flushes
  // time = __arm_rsr64("PMCCNTR_EL0");

  asm volatile("dsb sy\n\t"
               "isb\n\t"
               "mrs %0, PMCCNTR_EL0\n\t"
               "isb\n\t"
               "dsb sy\n\t"
               : "=r"(time));
  return time;
}

uint64_t get_time_cntpct() {
  uint64_t time;
  // Hesitant to use this because it doesn't include pipeline flushes
  // time = __arm_rsr64("CNTPCT_EL0");

  asm volatile("dsb sy\n\t"
               "isb\n\t"
               "mrs %0, CNTPCT_EL0\n\t"
               "isb\n\t"
               "dsb sy\n\t"
               : "=r"(time));
  return time;
}

uint64_t get_time_clock(clockid_t clock) {
  clock_gettime(clock, &clock_ts);
  uint64_t sec_ns = clock_ts.tv_sec * 1000000000;
  return (uint64_t)clock_ts.tv_nsec + sec_ns;
}
