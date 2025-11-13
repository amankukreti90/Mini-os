#include "mouse.h"
#include "../graphic/vbe.h"
#include "../io.h"

// Current mouse position
static int mouse_x = 400;
static int mouse_y = 300;
static int prev_mouse_x = 400;
static int prev_mouse_y = 300;

// Mouse button state
static int mouse_buttons = 0;

// Screen bounds (will be set from VBE info)
static int screen_width = 1024;
static int screen_height = 768;

// Set screen dimensions from VBE
void mouse_set_screen_bounds(int width, int height) {
    screen_width = width;
    screen_height = height;
    // Center mouse initially
    mouse_x = width / 2;
    mouse_y = height / 2;
    prev_mouse_x = mouse_x;
    prev_mouse_y = mouse_y;
}

// Wait for mouse controller to be ready
static void mouse_wait(unsigned char type) {
    unsigned int timeout = 100000;
    if (type == 0) {
        // Wait for data to be ready to read
        while (timeout--) {
            if ((inb(0x64) & 1) == 1) return;
        }
    } else {
        // Wait for ready to send command
        while (timeout--) {
            if ((inb(0x64) & 2) == 0) return;
        }
    }
}

// Write command to mouse
static void mouse_write(unsigned char data) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, data);
}

// Read from mouse
static unsigned char mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

// Initialize PS/2 mouse controller
void init_mouse(void) {
    unsigned char status;
    
    // Enable the auxiliary mouse device
    mouse_wait(1);
    outb(0x64, 0xA8);
    
    // Enable mouse interrupts
    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    status = inb(0x60) | 2;
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);
    
    // Set default settings
    mouse_write(0xF6);
    mouse_read();
    
    // Enable the mouse
    mouse_write(0xF4);
    mouse_read();
}

// Handle mouse interrupt
void handle_mouse(void) {
    static unsigned char mouse_cycle = 0;
    static unsigned char mouse_data[3];
    
    unsigned char status = inb(0x64);
    
    if (!(status & 1)) return; // No data available
    
    if (status & 0x20) { // Mouse data
        mouse_data[mouse_cycle] = inb(0x60);
        mouse_cycle++;
        
        if (mouse_cycle == 3) {
            // Store previous position for erasing
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
            
            // Parse mouse data
            mouse_buttons = mouse_data[0] & 0x07;
            
            int delta_x = (int)mouse_data[1];
            int delta_y = (int)mouse_data[2];
            
            // Handle overflow (sign extend)
            if (mouse_data[0] & 0x10) delta_x -= 256;
            if (mouse_data[0] & 0x20) delta_y -= 256;
            
            // Update mouse position
            mouse_x += delta_x;
            mouse_y -= delta_y; // Y is inverted in mouse data
            
            // Keep within screen bounds
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= screen_width) mouse_x = screen_width - 1;
            if (mouse_y >= screen_height) mouse_y = screen_height - 1;
            
            // Erase old cursor and draw new one
            erase_mouse_cursor();
            draw_mouse_cursor();
            
            mouse_cycle = 0;
        }
    }
}

// Erase mouse cursor from previous position
void erase_mouse_cursor(void) {
    for (int i = -5; i <= 5; i++) {
        vbe_put_pixel(prev_mouse_x + i, prev_mouse_y, vbe_rgb(0, 0, 0));
        vbe_put_pixel(prev_mouse_x, prev_mouse_y + i, vbe_rgb(0, 0, 0));
    }
}

// Draw a simple crosshair mouse cursor
void draw_mouse_cursor(void) {
    for (int i = -5; i <= 5; i++) {
        vbe_put_pixel(mouse_x + i, mouse_y, vbe_rgb(255, 255, 255));
        vbe_put_pixel(mouse_x, mouse_y + i, vbe_rgb(255, 255, 255));
    }
}

// Get current mouse X coordinate
int get_mouse_x(void) { return mouse_x; }

// Get current mouse Y coordinate
int get_mouse_y(void) { return mouse_y; }

// Get current mouse button state
int get_mouse_buttons(void) { return mouse_buttons; }