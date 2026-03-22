#include "../graphic/vbe.h"
#include "../idt.h"
#include "../drivers/keyboard.h"
#include "../drivers/pit.h"
#include "../sched/sched.h"
#include "../mem/pmm.h"
#include "../mem/paging.h"
#include "../mem/kheap.h"
#include "../boot_menu.h"

// Exposed for on-screen demo overlay (see `src/sched/sched.c`)
int g_heap_ok = 0;

void kernel_main(void) {
    // Clear screen immediately
    vbe_clear_screen(vbe_rgb(0, 0, 0));
    
    // Initialize IDT and interrupts
    init_idt();
    
    // Initialize keyboard driver
    init_keyboard();

    // Initialize PIT timer (drives IRQ0 ticks)
    pit_init(100);

    // Initialize scheduler (preemptive RR via IRQ0)
    sched_init();

    // Initialize physical memory manager (assume 64MiB for now)
    pmm_init(64u * 1024u * 1024u);

    // Enable paging (kernel-only, identity + framebuffer mapping)
    asm volatile("cli");
    paging_init();
    kheap_init();
    asm volatile("sti");

    // Quick heap smoke-test (paging+PMM+kheap path)
    void* a = kmalloc(256);
    void* b = kmalloc(8192);
    g_heap_ok = (a && b) ? 1 : 0;
    kfree(b);
    kfree(a);
    
    // Main kernel loop - always returns to boot menu
    while(1) {
        show_boot_menu();
        // When shell or snake game exit, control returns here
        // and boot menu is displayed again
    }
}