#ifndef VGA_H
#define VGA_H

#include <stdint.h>

// VGA color definitions
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW = 14,
    VGA_COLOR_WHITE = 15,
};

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)
#define VGA_MEMORY 0xB8000

// Create color byte from foreground and background
static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | (bg << 4);
}

// VGA text mode functions
void vga_clear_screen(uint8_t color);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_putchar(char c, uint32_t x, uint32_t y);
void vga_puts(const char* str, uint32_t x, uint32_t y);
void vga_print(const char* str);
void vga_print_color(const char* str, uint8_t fg, uint8_t bg);
void vga_scroll(void);
void vga_set_cursor(uint32_t x, uint32_t y);
void vga_draw_box(int x, int y, int width, int height, uint8_t color);

#endif