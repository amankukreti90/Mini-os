#include "pmm.h"

// Simple bitmap-based physical memory manager.
// Frame size: 4KiB.

#define FRAME_SIZE 4096u

// Default cap to keep bitmap static and simple.
// 64MiB / 4KiB = 16384 frames.
#define PMM_MAX_MEMORY_BYTES (64u * 1024u * 1024u)
#define PMM_MAX_FRAMES (PMM_MAX_MEMORY_BYTES / FRAME_SIZE)
#define PMM_BITMAP_WORDS (PMM_MAX_FRAMES / 32u)

static uint32_t pmm_bitmap[PMM_BITMAP_WORDS];
static uint32_t pmm_total_frames = 0;

extern uint8_t _kernel_start;
extern uint8_t _kernel_end;

static void bitmap_set(uint32_t frame) {
    pmm_bitmap[frame / 32u] |= (1u << (frame % 32u));
}

static void bitmap_clear(uint32_t frame) {
    pmm_bitmap[frame / 32u] &= ~(1u << (frame % 32u));
}

static int bitmap_test(uint32_t frame) {
    return (pmm_bitmap[frame / 32u] >> (frame % 32u)) & 1u;
}

static void bitmap_clear_all(void) {
    for (uint32_t i = 0; i < PMM_BITMAP_WORDS; i++) {
        pmm_bitmap[i] = 0;
    }
}

static uint32_t align_up(uint32_t val, uint32_t align) {
    return (val + align - 1u) & ~(align - 1u);
}

void pmm_init(uint32_t total_memory_bytes) {
    if (total_memory_bytes > PMM_MAX_MEMORY_BYTES) {
        total_memory_bytes = PMM_MAX_MEMORY_BYTES;
    }

    pmm_total_frames = total_memory_bytes / FRAME_SIZE;
    bitmap_clear_all();

    // Reserve frame 0 (NULL / BIOS structures, etc.)
    bitmap_set(0);

    // Reserve low memory below 1MiB (real-mode stuff, IVT/BDA, etc.)
    for (uint32_t addr = 0; addr < 0x00100000u; addr += FRAME_SIZE) {
        bitmap_set(addr / FRAME_SIZE);
    }

    // Reserve kernel image frames.
    uint32_t kstart = (uint32_t)(uintptr_t)&_kernel_start;
    uint32_t kend = (uint32_t)(uintptr_t)&_kernel_end;
    uint32_t kaddr = kstart & ~(FRAME_SIZE - 1u);
    uint32_t kend_aligned = align_up(kend, FRAME_SIZE);

    for (uint32_t addr = kaddr; addr < kend_aligned; addr += FRAME_SIZE) {
        uint32_t frame = addr / FRAME_SIZE;
        if (frame < pmm_total_frames) {
            bitmap_set(frame);
        }
    }
}

uint32_t pmm_alloc_frame(void) {
    for (uint32_t frame = 0; frame < pmm_total_frames; frame++) {
        if (!bitmap_test(frame)) {
            bitmap_set(frame);
            return frame * FRAME_SIZE;
        }
    }
    return 0;
}

void pmm_free_frame(uint32_t phys_addr) {
    if (phys_addr % FRAME_SIZE) return;
    uint32_t frame = phys_addr / FRAME_SIZE;
    if (frame == 0 || frame >= pmm_total_frames) return;
    bitmap_clear(frame);
}

