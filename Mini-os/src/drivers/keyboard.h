#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

// Function declarations
void init_keyboard(void);
void handle_keyboard(void);
void keyboard_poll_test(void);
char scancode_to_ascii(uint8_t scancode, bool shift);
void handle_shift(bool pressed);
void handle_ctrl(bool pressed);
void increase_text_scale(void);
void decrease_text_scale(void);
void set_text_scale(int scale);
int get_text_scale(void);

// Scancode queue helpers (IRQ-driven)
int keyboard_scancode_available(void);
uint8_t keyboard_read_scancode(void);   // blocks/spins until a scancode is available
void keyboard_flush_scancodes(void);

// Control whether keyboard IRQ forwards ASCII to the shell.
void keyboard_set_shell_input(int enabled);

// External shell function
extern void shell_add_char(char c);

uint8_t keyboard_get_scancode(void);

#endif