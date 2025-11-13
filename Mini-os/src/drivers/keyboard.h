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

// External shell function
extern void shell_add_char(char c);

uint8_t keyboard_get_scancode(void);

#endif