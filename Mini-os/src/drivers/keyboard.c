#include "keyboard.h"
#include "../graphic/vbe.h"
#include "../io.h"
#include <stdint.h>

// Keyboard buffer to store typed characters
static char keyboard_buffer[1024];

// Current position in the buffer
static int buffer_pos = 0;

// Text scale for display (1=small, 2=medium, 3=large, 4=very large)
static int text_scale = 2;

// For reducing blinking - tracks last buffer state
static int last_buffer_pos = -1;

// Convert keyboard scan code to ASCII character
char scancode_to_ascii(uint8_t scancode) {
    // US QWERTY keyboard layout mapping
    static const char keymap[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', // Backspace at 14
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',  // Enter at 28
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };
    
    if (scancode < 128) {
        return keymap[scancode];
    }
    return 0;
}

// Convert integer to string for display
void int_to_string(int num, char* buffer) {
    if (num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }
    
    int i = 0;
    int n = num;
    
    // Count digits in the number
    while (n > 0) {
        n /= 10;
        i++;
    }
    
    // Convert digits to string (reverse order)
    n = num;
    buffer[i] = '\0';
    for (int j = i - 1; j >= 0; j--) {
        buffer[j] = '0' + (n % 10);
        n /= 10;
    }
}

// Change text display scale (1-4)
void set_text_scale(int scale) {
    if (scale >= 1 && scale <= 4) {
        text_scale = scale;
        update_buffer_display();
    }
}

// Add character to keyboard buffer
void add_to_buffer(char c) {
    if (buffer_pos < 1023) {
        if (c == '\b') {
            // Handle backspace - remove last character
            if (buffer_pos > 0) {
                buffer_pos--;
            }
        } else {
            // Add regular character or newline
            keyboard_buffer[buffer_pos++] = c;
        }
        keyboard_buffer[buffer_pos] = '\0'; // Null terminate string
    }
    update_buffer_display();
}

void update_buffer_display(void) {

    int keyboard_area_start_y = 450; 
    int length_display_y = 550;      
    
    // Clear only the keyboard area (not the entire screen)
    for (int y = keyboard_area_start_y - 20; y < 600; y++) {
        for (int x = 50; x < 1000; x++) {
            vbe_put_pixel(x, y, vbe_rgb(0, 0, 0));
        }
    }
    
    // Show buffer header
    vbe_draw_string(50, keyboard_area_start_y, "BUFFER:", vbe_rgb(0, 255, 0), text_scale);
    
    if (buffer_pos > 0) {
        // Draw buffer content with text wrapping
        int current_x = 50;
        int current_y = keyboard_area_start_y + 30;
        int char_width = 8 * text_scale;     // Character width based on scale
        int max_line_width = 950;            // Maximum line width before wrapping
        
        // Draw each character in the buffer
        for (int i = 0; i < buffer_pos; i++) {
            if (keyboard_buffer[i] == '\n') {
                // Manual newline - move to next line
                current_y += 12 * text_scale;
                current_x = 50;
            } else if (current_x + char_width > max_line_width) {
                // Auto-wrap at screen edge
                current_y += 12 * text_scale;
                current_x = 50;
            }
            
            // Draw the character
            vbe_draw_char(current_x, current_y, keyboard_buffer[i], vbe_rgb(255, 255, 255), text_scale);
            current_x += char_width;
        }
        
        // Show length of buffer
        vbe_draw_string(50, length_display_y, "LENGTH:", vbe_rgb(255, 128, 0), text_scale);
        char len_str[16];
        int_to_string(buffer_pos, len_str);
        vbe_draw_string(200, length_display_y, len_str, vbe_rgb(255, 255, 255), text_scale);
        
    } else {
        // Buffer is empty
        vbe_draw_string(50, keyboard_area_start_y + 30, "EMPTY", vbe_rgb(255, 0, 0), text_scale);
        
        // Show zero length
        vbe_draw_string(50, length_display_y, "LENGTH:", vbe_rgb(255, 128, 0), text_scale);
        vbe_draw_string(200, length_display_y, "0", vbe_rgb(255, 255, 255), text_scale);
    }
}

// Handle keyboard interrupts
void handle_keyboard(void) {
    uint8_t scancode = inb(0x60); // Read from keyboard controller
    
    // Only process key presses (ignoring releases)
    if (!(scancode & 0x80)) {
        char key = scancode_to_ascii(scancode);
        if (key != 0) {
            add_to_buffer(key);
            
            // Quick text scale change using number keys
            if (key == '1') set_text_scale(1);
            if (key == '2') set_text_scale(2);
            if (key == '3') set_text_scale(3);
            if (key == '4') set_text_scale(4);
        }
    }
}

// Initialize keyboard driver
void init_keyboard(void) {
    buffer_pos = 0;
    keyboard_buffer[0] = '\0';
    last_buffer_pos = -1; 
    text_scale = 2;
    update_buffer_display();
}

// Get pointer to keyboard buffer
char* get_keyboard_buffer(void) {
    return keyboard_buffer;
}

// Clear the keyboard buffer
void clear_keyboard_buffer(void) {
    buffer_pos = 0;
    keyboard_buffer[0] = '\0';
    last_buffer_pos = -1; 
    update_buffer_display();
}

// Get current text scale
int get_text_scale(void) {
    return text_scale;
}