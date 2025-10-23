#ifndef IO_H
#define IO_H

#include <stdint.h>

// Output byte to specified I/O port
void outb(uint16_t port, uint8_t value);

// Input byte from specified I/O port
uint8_t inb(uint16_t port);

// Create small delay using I/O wait (writes to unused port 0x80)
void io_wait(void);

#endif