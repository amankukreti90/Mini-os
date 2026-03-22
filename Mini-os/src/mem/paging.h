#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

// Enable kernel paging with identity mapping for low memory and VBE framebuffer.
void paging_init(void);

// Map a single 4KiB page (virt -> phys) with RW by default.
void paging_map_page(uint32_t virt, uint32_t phys, uint32_t flags);

#endif

