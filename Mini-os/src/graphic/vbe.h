#ifndef VBE_H
#define VBE_H

#include <stdint.h>

// VBE mode info structure placed by bootloader at 0x5000
#define VBE_INFO_ADDR 0x5000

// VBE mode information structure (from VBE 2.0+ specification)
struct vbe_mode_info {
    uint16_t attributes;        // Memory model and capabilities
    uint8_t window_a, window_b; // Window A and B attributes  
    uint16_t granularity;       // Window granularity in KB
    uint16_t window_size;       // Window size in KB
    uint16_t segment_a, segment_b; // Window segment addresses
    uint32_t win_func_ptr;      // Pointer to window function
    uint16_t pitch;             // Bytes per scanline
    uint16_t width, height;     // Screen resolution in pixels
    uint8_t w_char, y_char;     // Character cell dimensions
    uint8_t planes, bpp, banks; // Color planes, bits per pixel, memory banks
    uint8_t memory_model;       // Memory model type
    uint8_t bank_size;          // Bank size in KB
    uint8_t image_pages;        // Number of image pages
    uint8_t reserved0;          // Reserved
    uint8_t red_mask, red_position;     // Red color mask and bit position
    uint8_t green_mask, green_position; // Green color mask and bit position  
    uint8_t blue_mask, blue_position;   // Blue color mask and bit position
    uint8_t reserved_mask, reserved_position; // Reserved mask/position
    uint8_t direct_color_attributes;    // Direct color mode attributes
    uint32_t framebuffer;       // Linear frame buffer address
    uint32_t off_screen_mem_off;// Off-screen memory offset
    uint16_t off_screen_mem_size; // Off-screen memory size
    uint8_t reserved1[206];     // Reserved for future expansion
};

// VBE graphics functions

// Clear entire screen with specified color
void vbe_clear_screen(uint32_t color);

// Draw single pixel at specified coordinates
void vbe_put_pixel(int x, int y, uint32_t color);

// Convert RGB values to framebuffer color format
uint32_t vbe_rgb(uint8_t r, uint8_t g, uint8_t b);

// Draw a character at specified position with scaling
void vbe_draw_char(int x, int y, char c, uint32_t color, int scale);

// Draw a string at specified position with scaling
void vbe_draw_string(int x, int y, const char* str, uint32_t color, int scale);

#endif