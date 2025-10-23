#include "idt.h"
#include "graphic/vbe.h"
#include "io.h"
#include "drivers/keyboard.h"

// IDT with 256 entries and IDT Register
idt_entry_t idt_entries[256];
idtr_t idtr;

// Configure an IDT gate entry for interrupt handler
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].offset_low = base & 0xFFFF;
    idt_entries[num].selector = sel;
    idt_entries[num].zero = 0;
    idt_entries[num].flags = flags;
    idt_entries[num].offset_high = (base >> 16) & 0xFFFF;
}

// Load IDT register with LIDT instruction
void idt_load(void) {
    asm volatile("lidt (%0)" : : "r"(&idtr));
}

// Display interrupt debug information on screen
void debug_interrupt(registers_t *regs, const char* type, int y_offset) {

    for (int y = y_offset; y < y_offset + 40; y++) {
        for (int x = 50; x < 350; x++) {
            vbe_put_pixel(x, y, vbe_rgb(0, 0, 0));
        }
    }
    
    // Show interrupt type and number
    vbe_draw_string(50, y_offset, type, vbe_rgb(255, 255, 0), 2);
    
    char int_str[8];
    int_str[0] = '0' + (regs->int_no / 100);
    int_str[1] = '0' + ((regs->int_no / 10) % 10);
    int_str[2] = '0' + (regs->int_no % 10);
    int_str[3] = '\0';
    vbe_draw_string(300, y_offset, int_str, vbe_rgb(255, 255, 255), 2);
    
    // Show raw hexadecimal value
    vbe_draw_string(50, y_offset + 20, "RAW:", vbe_rgb(255, 128, 0), 2);
    
    char raw_str[16];
    raw_str[0] = '0' + ((regs->int_no >> 12) & 0xF);
    raw_str[1] = '0' + ((regs->int_no >> 8) & 0xF);
    raw_str[2] = '0' + ((regs->int_no >> 4) & 0xF);
    raw_str[3] = '0' + (regs->int_no & 0xF);
    raw_str[4] = '\0';
    vbe_draw_string(300, y_offset + 20, raw_str, vbe_rgb(255, 128, 128), 2);
}

// CPU Exception Handler (Interrupts 0-31)
void isr_handler(registers_t *regs) {
    debug_interrupt(regs, "ISR:", 10);
    
    // Handle specific CPU exceptions
    if (regs->int_no == 13) {  // General Protection Fault
        vbe_draw_string(50, 150, "GP FAULT!", vbe_rgb(255, 0, 0), 3);
    }
}

// Initialize Programmable Interrupt Controller
void init_pic(void) {
    // Initialize PICs with ICW1-4 sequence
    outb(0x20, 0x11);  // ICW1: initialization command
    outb(0xA0, 0x11);
    
    outb(0x21, 0x20);  // ICW2: Master PIC vector offset = 32
    outb(0xA1, 0x28);  // ICW2: Slave PIC vector offset = 40
    
    outb(0x21, 0x04);  // ICW3: Master has slave at IRQ2
    outb(0xA1, 0x02);  // ICW3: Slave identity
    
    outb(0x21, 0x01);  // ICW4: 8086 mode
    outb(0xA1, 0x01);
    
    // Mask all interrupts except keyboard (IRQ1)
    outb(0x21, 0xFD);  // 11111101 - enable only IRQ1 (keyboard)
    outb(0xA1, 0xFF);  // 11111111 - disable all slave PIC interrupts
}

// Initialize Interrupt Descriptor Table
void init_idt(void) {
    // Set up IDT register
    idtr.limit = sizeof(idt_entry_t) * 256 - 1;
    idtr.base = (uint32_t)&idt_entries;
    
    // Clear all IDT entries to safe state
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0x08, 0x8E);
    }

    // Set up CPU exception handlers (0-31)
    extern void isr0(), isr1(), isr2(), isr3(), isr13();
    idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);   // Divide Error
    idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);   // Debug
    idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);   // NMI
    idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);   // Breakpoint
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // General Protection Fault
    
    // Set up hardware interrupt handlers (32-47)
    extern void irq0(), irq1();
    idt_set_gate(32, (uint32_t)irq0, 0x08, 0x8E);  // Timer (IRQ0)
    idt_set_gate(33, (uint32_t)irq1, 0x08, 0x8E);  // Keyboard (IRQ1)
    
    // Load IDT into CPU
    idt_load();

    // Initialize PIC controller
    init_pic();
    
    // Enable interrupts (STI instruction)
    asm volatile("sti");
}

// Hardware Interrupt Handler (Interrupts 32-47)
void irq_handler(registers_t *regs) {
    // Send End of Interrupt to PIC
    outb(0x20, 0x20);
    
    // Handle keyboard interrupts
    handle_keyboard();
}