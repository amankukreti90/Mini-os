#include "../graphic/vbe.h"
#include "../drivers/keyboard.h"
#include "../idt.h"
#include <stdint.h>

// Temparory funntion to display task completed //

void display_os_task_completed(void) {
    // Clear screen with black background
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Display title in top center
    int screen_width = 1024; // 1024x768 resolution
    int title_x = (screen_width - (8 * 4 * 14)) / 2; 
    vbe_draw_string(title_x, 30, "OS PROJECT STATUS", vbe_rgb(0, 255, 255), 3);
    
    // Draw a separator line under title
    for (int x = 50; x < screen_width - 50; x++) {
        vbe_put_pixel(x, 70, vbe_rgb(255, 255, 0));
    }
    
    int start_y = 100;
    int line_height = 40;
    int text_scale = 2;
    
    // Completed task list
    vbe_draw_string(80, start_y, "Bootloader: Working", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height, "Protected Mode: Active", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 2, "Kernel: Loaded", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 3, "GDT Table: Configured", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 4, "IDT Table: Set up", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 5, "Keyboard Driver: Ready", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 6, "VBE: Graphics mode", vbe_rgb(0, 255, 0), text_scale);
    vbe_draw_string(80, start_y + line_height * 7, "VGA: Text display", vbe_rgb(0, 255, 0), text_scale);
    
    // separator line
    for (int x = 50; x < screen_width - 50; x++) {
        vbe_put_pixel(x, start_y + line_height * 8 + 10, vbe_rgb(255, 255, 0));
    }
    
    // Display keyboard input area B
    int keyboard_area_y = start_y + line_height * 9 + 30;
    vbe_draw_string(80, keyboard_area_y, "KEYBOARD TEST AREA:", vbe_rgb(255, 255, 0), text_scale);

}

void kernel_main() {

    init_idt();          // Initialize Interrupt Descriptor Table
    init_keyboard();     // Initialize keyboard driver
    
    display_os_task_completed();
    
    // Main loop
    while(1) {
        // The keyboard interrupts will handle input automatically
        // through the IDT and keyboard driver
        asm volatile("hlt"); // Halt until next interrupt
    }
}