#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>
#include "../idt.h"

void sched_init(void);

// Called from timer IRQ (IRQ0 / vector 32). May return a new regs pointer
// (i.e., a different task's saved stack) to switch tasks on interrupt return.
registers_t* sched_on_tick(registers_t* regs);

#endif

