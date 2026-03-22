#include "../drivers/keyboard.h"
#include "../graphic/vbe.h"
#include "../io.h"
#include "../shell/shell.h"
#include <stdint.h>

// Remove buffer variables, keep only scale
static int text_scale = 2;

// IRQ-driven scancode ring buffer
#define KBD_BUF_SIZE 64
static volatile uint8_t kbd_buf[KBD_BUF_SIZE];
static volatile uint8_t kbd_head = 0;
static volatile uint8_t kbd_tail = 0;

static volatile int shell_input_enabled = 0;

// Key state tracking
bool shift_pressed = false;
bool ctrl_pressed = false;

static void kbd_buf_push(uint8_t scancode) {
    uint8_t next = (uint8_t)((kbd_head + 1) % KBD_BUF_SIZE);
    if (next == kbd_tail) {
        // Buffer full; drop scancode.
        return;
    }
    kbd_buf[kbd_head] = scancode;
    kbd_head = next;
}

int keyboard_scancode_available(void) {
    return kbd_head != kbd_tail;
}

uint8_t keyboard_read_scancode(void) {
    while (!keyboard_scancode_available()) {
        // spin
    }
    uint8_t sc = kbd_buf[kbd_tail];
    kbd_tail = (uint8_t)((kbd_tail + 1) % KBD_BUF_SIZE);
    return sc;
}

void keyboard_flush_scancodes(void) {
    kbd_tail = kbd_head;
}

void keyboard_set_shell_input(int enabled) {
    shell_input_enabled = enabled ? 1 : 0;
}

// Extended keymap with shift characters
char scancode_to_ascii(uint8_t scancode, bool shift) {
    // Regular keymap (without shift)
    static const char keymap_normal[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };
    
    // Shift keymap
    static const char keymap_shift[128] = {
        0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
        'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0
    };
    
    if (scancode < 128) {
        return shift ? keymap_shift[scancode] : keymap_normal[scancode];
    }
    return 0;
}

// Handle shift key press/release
void handle_shift(bool pressed) {
    shift_pressed = pressed;
}

// Handle ctrl key press/release  
void handle_ctrl(bool pressed) {
    ctrl_pressed = pressed;
}

// Increase text scale (max 4)
void increase_text_scale(void) {
    if (text_scale < 4) {
        text_scale++;
    }
}

// Decrease text scale (min 1)
void decrease_text_scale(void) {
    if (text_scale > 1) {
        text_scale--;
    }
}

// Change text display scale
void set_text_scale(int scale) {
    if (scale >= 1 && scale <= 4) {
        text_scale = scale;
    }
}

void handle_keyboard(void) {
    uint8_t scancode = inb(0x60);
    char key = 0;

    // Always enqueue raw scancode for menu/game consumers.
    kbd_buf_push(scancode);

    // Check for special keys first
    switch(scancode) {
        case 0x2A: // Left Shift pressed
            handle_shift(true);
            return;
        case 0xAA: // Left Shift released
            handle_shift(false);
            return;
        case 0x36: // Right Shift pressed
            handle_shift(true);
            return;
        case 0xB6: // Right Shift released
            handle_shift(false);
            return;
        case 0x1D: // Left Ctrl pressed
            handle_ctrl(true);
            return;
        case 0x9D: // Left Ctrl released
            handle_ctrl(false);
            return;
    }
    
    // Only process key presses (not releases)
    if (!(scancode & 0x80)) {
        key = scancode_to_ascii(scancode, shift_pressed);
  
        
        if (ctrl_pressed) {
            // Handle Ctrl+ combinations
            switch(key) {
                case '1': 
                    set_text_scale(1);
                    break;
                case '2': 
                    set_text_scale(2);
                    break;
                case '3': 
                    set_text_scale(3);
                    break;
                case '4': 
                    set_text_scale(4);
                    break;
                case '+': 
                case '=': 
                    increase_text_scale();
                    break;
                case '-': 
                case '_': 
                    decrease_text_scale();
                    break;
                default: 
                    break;
            }
        } else if (key != 0) {
            // Forward to shell only when shell input is enabled.
            if (shell_input_enabled) {
                shell_add_char(key);
            }
        }
    }
}

// Initialize keyboard driver
void init_keyboard(void) {
    shift_pressed = false;
    ctrl_pressed = false;
    text_scale = 2;
    kbd_head = 0;
    kbd_tail = 0;
    shell_input_enabled = 0;

}

// Get current text scale
int get_text_scale(void) {
    return text_scale;
}

// ADD THIS FUNCTION TO src/drivers/keyboard.c (at the end of the file)
uint8_t keyboard_get_scancode(void) {
    return keyboard_read_scancode();
}