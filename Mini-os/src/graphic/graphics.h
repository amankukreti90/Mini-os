#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

// VGA Mode 13h - 320x200x256 color graphics mode
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define VGA_GRAPHICS_MEMORY 0xA0000

// Standard 16-color VGA palette indices
#define COLOR_BLACK 0
#define COLOR_BLUE 1
#define COLOR_GREEN 2
#define COLOR_CYAN 3
#define COLOR_RED 4
#define COLOR_MAGENTA 5
#define COLOR_BROWN 6
#define COLOR_LIGHT_GREY 7
#define COLOR_DARK_GREY 8
#define COLOR_LIGHT_BLUE 9
#define COLOR_LIGHT_GREEN 10
#define COLOR_LIGHT_CYAN 11
#define COLOR_LIGHT_RED 12
#define COLOR_LIGHT_MAGENTA 13
#define COLOR_YELLOW 14
#define COLOR_WHITE 15




// Graphics functions

// Initialize VGA 320x200x256 graphics mode
void graphics_init();

// Clear entire screen with specified color
void graphics_clear_screen(uint8_t color);

// Draw single pixel at specified coordinates
void graphics_plot_pixel(int x, int y, uint8_t color);

// Draw filled rectangle at specified position and size
void graphics_draw_rectangle(int x, int y, int width, int height, uint8_t color);

// Draw text string at specified position
void graphics_draw_text(const char* str, int x, int y, uint8_t color);

#endif