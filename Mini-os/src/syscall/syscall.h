#ifndef SYSCALL_H
#define SYSCALL_H

#include "../idt.h"  // For registers_t

// System call numbers
#define SYSCALL_WRITE   0  // Write to output
#define SYSCALL_READ    1  // Read from input  
#define SYSCALL_OPEN    2  // Open file
#define SYSCALL_CLOSE   3  // Close file
#define SYSCALL_EXEC    4  // Execute program
#define SYSCALL_EXIT    5  // Exit process

// System call dispatcher - routes interrupts to handler functions
void syscall_dispatcher(registers_t *regs);

// System call handler functions
int sys_write(char *buffer, int length);  // Write data to output
int sys_read(char *buffer, int length);   // Read data from input
void sys_exit(int status);                // Terminate process

#endif