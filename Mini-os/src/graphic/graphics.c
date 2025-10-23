#include "graphics.h"
#include <stdint.h>

static uint8_t* const VGA_BUFFER = (uint8_t*)VGA_GRAPHICS_MEMORY;

// Initialize 320x200x256 color graphics mode
void graphics_init() {
    asm volatile (
        "mov $0x13, %%ax\n"
        "int $0x10\n"
        : : : "ax"
    );
}

// Clear entire screen with specified color
void graphics_clear_screen(uint8_t color) {
    for (int i = 0; i < SCREEN_SIZE; i++) {
        VGA_BUFFER[i] = color;
    }
}

// Draw single pixel with bounds checking
void graphics_plot_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        VGA_BUFFER[y * SCREEN_WIDTH + x] = color;
    }
}

// Draw filled rectangle
void graphics_draw_rectangle(int x, int y, int width, int height, uint8_t color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            graphics_plot_pixel(x + dx, y + dy, color);
        }
    }
}

// Simple block text rendering
void graphics_draw_text(const char* str, int x, int y, uint8_t color) {
    while (*str) {
        graphics_draw_rectangle(x, y, 6, 8, color);
        x += 8;
        str++;
    }
}