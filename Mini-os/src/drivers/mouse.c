#include "mouse.h"
#include "../graphic/vbe.h"
#include "../io.h"

// Current mouse position (centered initially)
static int mouse_x = 400;
static int mouse_y = 300;

// Mouse button state (bit 0 = left, bit 1 = right, bit 2 = middle)
static int mouse_buttons = 0;

// Initialize PS/2 mouse controller
void init_mouse(void) {
    // Enable the auxiliary mouse device
    outb(0x64, 0xA8);
    
    // Enable mouse interrupts
    outb(0x64, 0x20);
    unsigned char status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);
    

    outb(0x64, 0xF4);
}

//handeling mouse cursor
void handle_mouse(void) {
    static unsigned char mouse_cycle = 0;
    static char mouse_data[3];
    
    unsigned char status = inb(0x64);
    
    if (status & 0x20) {  
        mouse_data[mouse_cycle] = inb(0x60);
        mouse_cycle++;
        
        if (mouse_cycle == 3) {
    
            mouse_buttons = mouse_data[0] & 0x07;
            int delta_x = (int)mouse_data[1] - ((mouse_data[0] & 0x10) ? 256 : 0);
            int delta_y = (int)mouse_data[2] - ((mouse_data[0] & 0x20) ? 256 : 0);
            
            // Update mouse position with movement deltas
            mouse_x += delta_x;
            mouse_y -= delta_y;  
            
            // Keep within screen bounds (1024x768 assumed)
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x > 1023) mouse_x = 1023;
            if (mouse_y > 767) mouse_y = 767;
            
            // Draw updated mouse cursor
            draw_mouse_cursor();
            
            mouse_cycle = 0;
        }
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

// Get current mouse button state (bitmask: 1=left, 2=right, 4=middle)
int get_mouse_buttons(void) { return mouse_buttons; }