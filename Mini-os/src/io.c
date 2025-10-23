#include "io.h"

// Output byte to specified I/O port
// Uses inline assembly for direct port I/O
void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

// Input byte from specified I/O port  
// Uses inline assembly for direct port I/O
uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Create small I/O delay by writing to unused port 0x80
// Used for timing-sensitive hardware operations
void io_wait(void) {
    outb(0x80, 0);  // Write to unused port to create a small delay
}