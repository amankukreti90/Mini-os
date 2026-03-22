#ifndef PMM_H
#define PMM_H

#include <stdint.h>

// Initialize physical memory manager with a fixed total memory size.
// For now, this OS assumes a contiguous RAM region starting at 0.
void pmm_init(uint32_t total_memory_bytes);

// Allocate a 4KiB physical frame. Returns physical address, or 0 on failure.
uint32_t pmm_alloc_frame(void);

// Free a previously allocated 4KiB physical frame address.
void pmm_free_frame(uint32_t phys_addr);

#endif

