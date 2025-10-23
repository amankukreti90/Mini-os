#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

// Initialize PS/2 mouse controller and enable interrupts
void init_mouse(void);

// Handle mouse interrupt - process movement and button data  
void handle_mouse(void);

// Draw crosshair cursor at current mouse position
void draw_mouse_cursor(void);

// Get current X coordinate (0-1023)
int get_mouse_x(void);

// Get current Y coordinate (0-767)  
int get_mouse_y(void);

// Get button state: bit 0=left, bit 1=right, bit 2=middle
int get_mouse_buttons(void);

// I/O port functions for PS/2 controller

void outb(uint16_t port, uint8_t value);        // Write value to I/O port
uint8_t inb(uint16_t port);                     // Read value from I/O port

#endif