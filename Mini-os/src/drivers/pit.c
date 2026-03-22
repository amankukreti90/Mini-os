#include "pit.h"
#include "../io.h"

// PIT I/O ports
#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

// PIT base frequency
#define PIT_BASE_HZ 1193182u

static volatile uint64_t pit_ticks = 0;

void pit_init(uint32_t frequency_hz) {
    if (frequency_hz == 0) {
        frequency_hz = 100;
    }

    uint32_t divisor = PIT_BASE_HZ / frequency_hz;
    if (divisor == 0) divisor = 1;
    if (divisor > 0xFFFF) divisor = 0xFFFF;

    // Command: channel 0, access mode lobyte/hibyte, mode 3 (square wave), binary
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));

    pit_ticks = 0;
}

void pit_on_tick(void) {
    pit_ticks++;
}

uint64_t pit_get_ticks(void) {
    return pit_ticks;
}

