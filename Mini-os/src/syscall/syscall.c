#include "syscall.h"
#include "../graphic/vbe.h"
#include "../drivers/keyboard.h"
#include <stdint.h>

// System call dispatcher - routes syscalls to appropriate handlers
void syscall_dispatcher(registers_t *regs) {
    switch(regs->eax) {
        case SYSCALL_WRITE:
            regs->eax = sys_write((char*)regs->ebx, regs->ecx);
            break;
        case SYSCALL_READ:
            regs->eax = sys_read((char*)regs->ebx, regs->ecx);
            break;
        case SYSCALL_EXIT:
            sys_exit(regs->ebx);
            break;
        default:
            regs->eax = -1; // Invalid syscall
    }
}

// Write system call - outputs text to screen
int sys_write(char *buffer, int length) {
    static int sys_write_x = 50;    // Current X position for text
    static int sys_write_y = 400;   // Current Y position for text
    
    for(int i = 0; i < length; i++) {
        if (buffer[i] == '\n') {
            // Handle newline - move to next line
            sys_write_x = 50;
            sys_write_y += 16;
        } else {
            // Draw character and advance position
            vbe_draw_char(sys_write_x, sys_write_y, buffer[i], vbe_rgb(255, 255, 255), 2);
            sys_write_x += 16;
            
            // Wrap to next line if at screen edge
            if (sys_write_x > 1000) {
                sys_write_x = 50;
                sys_write_y += 16;
            }
        }
    }
    return length; // Return number of characters written
}

// Read system call - placeholder implementation
int sys_read(char *buffer, int length) {
    (void)buffer;  // Mark parameters as unused
    (void)length;
    return 0;      // Return 0 bytes read
}

// Exit system call - terminates process
void sys_exit(int status) {
    // Basic exit - show exit message
    (void)status; // Mark parameter as unused
    vbe_draw_string(50, 100, "Process exited", vbe_rgb(255, 0, 0), 2);
}