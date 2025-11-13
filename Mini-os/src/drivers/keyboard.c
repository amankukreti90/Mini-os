#include "../drivers/keyboard.h"
#include "../graphic/vbe.h"
#include "../io.h"
#include "../shell/shell.h"
#include <stdint.h>

// Remove buffer variables, keep only scale
static int text_scale = 2;

// Key state tracking
bool shift_pressed = false;
bool ctrl_pressed = false;

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
            // Send to shell instead of buffer
            shell_add_char(key);
        }
    }
}

// Initialize keyboard driver
void init_keyboard(void) {
    shift_pressed = false;
    ctrl_pressed = false;
    text_scale = 2;

}

// Get current text scale
int get_text_scale(void) {
    return text_scale;
}

// ADD THIS FUNCTION TO src/drivers/keyboard.c (at the end of the file)
uint8_t keyboard_get_scancode(void) {
    return inb(0x60);
}