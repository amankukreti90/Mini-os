#include "../graphic/vbe.h"
#include "../idt.h"
#include "../drivers/keyboard.h"
#include "../boot_menu.h"

void kernel_main(void) {
    // Clear screen immediately
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Initialize IDT and interrupts
    init_idt();
    
    // Initialize keyboard driver
    init_keyboard();
    
    // Main kernel loop - always returns to boot menu
    while(1) {
        show_boot_menu();
        // When shell or snake game exit, control returns here
        // and boot menu is displayed again
    }
}