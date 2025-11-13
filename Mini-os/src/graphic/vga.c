#include "vga.h"

static uint16_t* const VGA_BUFFER = (uint16_t*)VGA_MEMORY;
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static uint8_t current_color = 0x07; 

// Output byte to port
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Create VGA entry from character and color
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t)uc | (uint16_t)color << 8;
}

// Clear entire screen with specified color
void vga_clear_screen(uint8_t color) {
    uint16_t blank = vga_entry(' ', color);
    
    for (int i = 0; i < VGA_BUFFER_SIZE; i++) {
        VGA_BUFFER[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Set current text color
void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = vga_entry_color(fg, bg);
}

// Put character at specific coordinates
void vga_putchar(char c, uint32_t x, uint32_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    
    int index = y * VGA_WIDTH + x;
    VGA_BUFFER[index] = vga_entry(c, current_color);
}

// Put string at specific coordinates
void vga_puts(const char* str, uint32_t x, uint32_t y) {
    int i = 0;
    while (str[i] != '\0') {
        vga_putchar(str[i], x + i, y);
        i++;
    }
}

// Print string with newline and scrolling support
void vga_print(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            vga_putchar(str[i], cursor_x, cursor_y);
            cursor_x++;
        }
        
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        
        if (cursor_y >= VGA_HEIGHT) {
            vga_scroll();
            cursor_y = VGA_HEIGHT - 1;
        }
        i++;
    }
}

// Print string with specified colors
void vga_print_color(const char* str, uint8_t fg, uint8_t bg) {
    uint8_t old_color = current_color;
    vga_set_color(fg, bg);
    vga_print(str);
    current_color = old_color;
}

// Scroll screen up by one line
void vga_scroll(void) {
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            int from_index = y * VGA_WIDTH + x;
            int to_index = (y - 1) * VGA_WIDTH + x;
            VGA_BUFFER[to_index] = VGA_BUFFER[from_index];
        }
    }
    
    // Clear bottom line
    uint16_t blank = vga_entry(' ', current_color);
    for (int x = 0; x < VGA_WIDTH; x++) {
        VGA_BUFFER[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = blank;
    }
}

// Set hardware cursor position
void vga_set_cursor(uint32_t x, uint32_t y) {
    uint16_t pos = y * VGA_WIDTH + x;
    
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Draw box with border characters
void vga_draw_box(int x, int y, int width, int height, uint8_t color) {
    uint8_t old_color = current_color;
    vga_set_color(color, VGA_COLOR_BLACK);
    
    // Draw horizontal borders
    for (int i = x; i < x + width; i++) {
        vga_putchar('-', i, y);
        vga_putchar('-', i, y + height - 1);
    }
    
    // Draw vertical borders
    for (int i = y; i < y + height; i++) {
        vga_putchar('|', x, i);
        vga_putchar('|', x + width - 1, i);
    }
    
    // Draw corners
    vga_putchar('+', x, y);
    vga_putchar('+', x + width - 1, y);
    vga_putchar('+', x, y + height - 1);
    vga_putchar('+', x + width - 1, y + height - 1);
    
    current_color = old_color;
}