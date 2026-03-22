#ifndef PIT_H
#define PIT_H

#include <stdint.h>

// Initialize PIT to a target frequency (Hz), e.g. 100.
void pit_init(uint32_t frequency_hz);

// Called from IRQ0 handler to update ticks.
void pit_on_tick(void);

// Get number of PIT ticks since init.
uint64_t pit_get_ticks(void);

#endif

