#include "kheap.h"
#include "pmm.h"
#include "paging.h"
#include <stdint.h>

#define PAGE_SIZE 4096u
#define HEAP_BASE 0x02000000u   // 32MiB
#define HEAP_MAX  (0x01000000u) // 16MiB

typedef struct block_header {
    uint32_t size;               // bytes after header
    uint32_t free;               // 1 if free
    struct block_header* next;
} block_header_t;

static uint32_t heap_end = HEAP_BASE;
static block_header_t* heap_head = 0;

static uint32_t align_up(uint32_t v, uint32_t a) {
    return (v + a - 1u) & ~(a - 1u);
}

static void mem_zero(void* p, uint32_t bytes) {
    uint8_t* b = (uint8_t*)p;
    for (uint32_t i = 0; i < bytes; i++) b[i] = 0;
}

static int heap_grow_pages(uint32_t pages) {
    uint32_t new_end = heap_end + pages * PAGE_SIZE;
    if (new_end - HEAP_BASE > HEAP_MAX) return 0;

    for (uint32_t v = heap_end; v < new_end; v += PAGE_SIZE) {
        uint32_t phys = pmm_alloc_frame();
        if (!phys) return 0;
        paging_map_page(v, phys, 0x002u); // RW
        mem_zero((void*)v, PAGE_SIZE);
    }

    heap_end = new_end;
    return 1;
}

void kheap_init(void) {
    heap_end = HEAP_BASE;
    heap_head = 0;

    // Start with 4 pages (~16KiB).
    if (!heap_grow_pages(4)) {
        return;
    }

    heap_head = (block_header_t*)HEAP_BASE;
    heap_head->size = (heap_end - HEAP_BASE) - (uint32_t)sizeof(block_header_t);
    heap_head->free = 1;
    heap_head->next = 0;
}

static void split_block(block_header_t* blk, uint32_t needed) {
    // needed is aligned payload size
    uint32_t remaining = blk->size - needed;
    if (remaining <= sizeof(block_header_t) + 8u) return;

    uint8_t* base = (uint8_t*)blk;
    block_header_t* next = (block_header_t*)(base + sizeof(block_header_t) + needed);
    next->size = remaining - (uint32_t)sizeof(block_header_t);
    next->free = 1;
    next->next = blk->next;

    blk->size = needed;
    blk->next = next;
}

void* kmalloc(size_t size) {
    if (size == 0) return 0;

    uint32_t needed = align_up((uint32_t)size, 8u);

    block_header_t* cur = heap_head;
    while (cur) {
        if (cur->free && cur->size >= needed) {
            cur->free = 0;
            split_block(cur, needed);
            return (void*)((uint8_t*)cur + sizeof(block_header_t));
        }
        cur = cur->next;
    }

    // No block found: grow heap by enough pages and retry once.
    uint32_t total_needed = needed + (uint32_t)sizeof(block_header_t);
    uint32_t pages = align_up(total_needed, PAGE_SIZE) / PAGE_SIZE;
    if (!heap_grow_pages(pages)) return 0;

    // Append a new free block at the previous end.
    block_header_t* tail = heap_head;
    while (tail && tail->next) tail = tail->next;

    block_header_t* new_blk = (block_header_t*)(heap_end - pages * PAGE_SIZE);
    new_blk->size = pages * PAGE_SIZE - (uint32_t)sizeof(block_header_t);
    new_blk->free = 1;
    new_blk->next = 0;

    if (tail) tail->next = new_blk;
    else heap_head = new_blk;

    // Retry allocation.
    return kmalloc(size);
}

static void coalesce(void) {
    block_header_t* cur = heap_head;
    while (cur && cur->next) {
        uint8_t* cur_end = (uint8_t*)cur + sizeof(block_header_t) + cur->size;
        if (cur->free && cur->next->free && (uint8_t*)cur->next == cur_end) {
            cur->size += (uint32_t)sizeof(block_header_t) + cur->next->size;
            cur->next = cur->next->next;
            continue;
        }
        cur = cur->next;
    }
}

void kfree(void* ptr) {
    if (!ptr) return;
    block_header_t* blk = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    blk->free = 1;
    coalesce();
}

