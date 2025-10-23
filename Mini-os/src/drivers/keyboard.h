#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

// Main keyboard interrupt handler - called by IDT when keyboard interrupt occurs
void handle_keyboard(void);

// Initialize keyboard driver and display
void init_keyboard(void);

// Get pointer to the keyboard buffer containing typed characters
char* get_keyboard_buffer(void);

// Clear the keyboard buffer and reset display
void clear_keyboard_buffer(void);

// Add a character to the keyboard buffer 
void add_to_buffer(char c);

// Update the keyboard display area on screen
void update_buffer_display(void);

// Convert keyboard scan code to ASCII character
char scancode_to_ascii(uint8_t scancode);  

// Set text display scale 
void set_text_scale(int scale);

// Get current text display scale
int get_text_scale(void);

// Convert integer to string for display
void int_to_string(int num, char* buffer);

// I/O port functions for hardware communication

void outb(uint16_t port, uint8_t value);        // Output byte to specified port
uint8_t inb(uint16_t port);                     // Input byte from specified port

#endif