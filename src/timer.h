#ifndef __TIMER_H
#define __TIMER_H

#include "memory.h"
#include <stdint.h>

// Overhead (in cycles?) of the time access instructions
// I don't think it's too important, just offsets the times
extern uint64_t timer_overhead;

uint64_t time_access(addr_t);

uint64_t time_store(addr_t);

uint64_t time_exec(addr_t);

uint64_t time_overhead();

#endif // !__TIMER_H
