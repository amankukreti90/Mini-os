#include "paging.h"
#include "pmm.h"
#include "../graphic/vbe.h"

#define PAGE_SIZE 4096u

// Page directory/table entry flags
#define PAGE_PRESENT 0x001u
#define PAGE_RW      0x002u

static uint32_t align_down(uint32_t val, uint32_t align) {
    return val & ~(align - 1u);
}

static uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + align - 1u) & ~(align - 1u);
}

static void mem_zero(void* p, uint32_t bytes) {
    uint8_t* b = (uint8_t*)p;
    for (uint32_t i = 0; i < bytes; i++) b[i] = 0;
}

static inline void load_cr3(uint32_t phys_pd) {
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(phys_pd) : "memory");
}

static inline uint32_t read_cr0(void) {
    uint32_t v;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(v));
    return v;
}

static inline void write_cr0(uint32_t v) {
    __asm__ __volatile__("mov %0, %%cr0" : : "r"(v) : "memory");
}

static uint32_t* page_directory = 0;
static uint32_t page_directory_phys = 0;

static uint32_t* get_page_table(uint32_t pd_index, uint32_t create) {
    uint32_t pde = page_directory[pd_index];
    if (pde & PAGE_PRESENT) {
        return (uint32_t*)(pde & 0xFFFFF000u);
    }

    if (!create) return 0;

    uint32_t pt_phys = pmm_alloc_frame();
    if (!pt_phys) return 0;
    mem_zero((void*)pt_phys, PAGE_SIZE);

    page_directory[pd_index] = (pt_phys & 0xFFFFF000u) | PAGE_PRESENT | PAGE_RW;
    return (uint32_t*)pt_phys;
}

void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FFu;

    uint32_t* pt = get_page_table(pd_index, 1);
    if (!pt) return;

    pt[pt_index] = (phys & 0xFFFFF000u) | (flags & 0xFFFu) | PAGE_PRESENT;
}

void paging_init(void) {
    // Allocate and zero a page directory.
    page_directory_phys = pmm_alloc_frame();
    if (!page_directory_phys) return;

    page_directory = (uint32_t*)page_directory_phys;
    mem_zero(page_directory, PAGE_SIZE);

    // Identity map first 64MiB (enough for kernel + stacks + early allocations).
    const uint32_t identity_limit = 64u * 1024u * 1024u;
    for (uint32_t addr = 0; addr < identity_limit; addr += PAGE_SIZE) {
        paging_map_page(addr, addr, PAGE_RW);
    }

    // Map VBE framebuffer (physical address provided by bootloader).
    extern struct vbe_mode_info* vbe_info;
    uint32_t fb = vbe_info ? vbe_info->framebuffer : 0;
    if (fb) {
        uint32_t fb_size = 0;
        if (vbe_info->pitch && vbe_info->height) {
            fb_size = (uint32_t)vbe_info->pitch * (uint32_t)vbe_info->height;
        }
        if (fb_size == 0) fb_size = 4u * 1024u * 1024u; // fallback

        uint32_t start = align_down(fb, PAGE_SIZE);
        uint32_t end = align_up(fb + fb_size, PAGE_SIZE);
        for (uint32_t addr = start; addr < end; addr += PAGE_SIZE) {
            paging_map_page(addr, addr, PAGE_RW);
        }
    }

    // Load page directory and enable paging.
    load_cr3(page_directory_phys);
    uint32_t cr0 = read_cr0();
    cr0 |= 0x80000000u; // CR0.PG
    write_cr0(cr0);
}

