// idt.h
#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// Interrupt register structure - MUST MATCH ASSEMBLY STACK LAYOUT
typedef struct {
    uint32_t ds;                                    // Data segment
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // pusha order
    uint32_t  err_code , int_no;                      // Interrupt number & error code
    uint32_t eip, cs, eflags, useresp, ss;          // Auto-pushed by CPU
} registers_t;

// IDT entry structure
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t flags;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

// IDTR structure
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idtr_t;

// Function declarations
void init_idt(void);                                // Initialize Interrupt Descriptor Table
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags); // Set IDT gate entry
void idt_load(void);                               // Load IDT register
void isr_handler(registers_t *regs);               // CPU exception handler (interrupts 0-31)
void irq_handler(registers_t *regs);               // Hardware interrupt handler (interrupts 32-47)

#endif