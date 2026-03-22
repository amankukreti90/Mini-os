#include "sched.h"
#include "../graphic/vbe.h"
#include "../drivers/pit.h"

extern int g_heap_ok;

// Extremely small round-robin kernel-thread scheduler.
// Tasks are represented by a saved stack pointer that points to the interrupt
// frame layout used by `isr.asm`'s irq_common_stub (i.e., registers_t*).

#define MAX_TASKS 8
#define STACK_SIZE_DWORDS (4096) // 16 KiB stack (4096 * 4)

typedef enum {
    TASK_UNUSED = 0,
    TASK_RUNNABLE = 1,
} task_state_t;

typedef struct {
    task_state_t state;
    registers_t* regs;   // saved "regs pointer" (top of irq frame stack)
    uint32_t* stack_base;
} task_t;

static task_t tasks[MAX_TASKS];
static int current_task = -1;
static int bootstrap_registered = 0;

static uint32_t idle_stack[STACK_SIZE_DWORDS];
static uint32_t demo_stack[STACK_SIZE_DWORDS];

__attribute__((noreturn)) static void demo_task(void) {
    uint64_t last = 0;
    for (;;) {
        uint64_t t = pit_get_ticks();
        if (t != last) {
            last = t;

            // Draw a tiny tick counter in the corner (proof of preemption).
            // Keep it small and non-invasive.
            char buf[32];
            int pos = 0;
            const char* prefix = "ticks:";
            for (int i = 0; prefix[i]; i++) buf[pos++] = prefix[i];

            // Convert ticks to decimal (simple, no libc)
            uint64_t v = t;
            char tmp[20];
            int tp = 0;
            if (v == 0) tmp[tp++] = '0';
            while (v > 0 && tp < (int)sizeof(tmp)) {
                tmp[tp++] = '0' + (char)(v % 10);
                v /= 10;
            }
            for (int i = tp - 1; i >= 0; i--) buf[pos++] = tmp[i];
            buf[pos] = '\0';

            vbe_draw_string(10, 10, buf, vbe_rgb(255, 255, 255), 2);

            if (g_heap_ok) {
                vbe_draw_string(10, 40, "heap: ok", vbe_rgb(0, 255, 0), 2);
            } else {
                vbe_draw_string(10, 40, "heap: fail", vbe_rgb(255, 0, 0), 2);
            }
        }
    }
}

__attribute__((noreturn)) static void idle_task(void) {
    for (;;) {
        __asm__ __volatile__("hlt");
    }
}

static registers_t* build_initial_regs(uint32_t* stack_top, void (*entry)(void)) {
    // Build a fake interrupt frame so that when `irq_common_stub` restores
    // registers and executes `iret`, it will start executing `entry`.
    //
    // Layout at `regs` pointer (top of stack) must match registers_t.
    uint32_t* sp = stack_top;

    // iret frame (ring0): eip, cs, eflags
    // plus our stub expects int_no + err_code beneath the saved GP regs.
    // Build from the bottom up (push in reverse order).

    // iret frame (pushed by CPU on real interrupt)
    *(--sp) = 0x00000202u;            // eflags (IF=1)
    *(--sp) = 0x00000008u;            // cs (kernel code segment)
    *(--sp) = (uint32_t)(uintptr_t)entry; // eip

    // int_no, err_code (removed by stub via add esp,8)
    *(--sp) = 0; // int_no
    *(--sp) = 0; // err_code

    // pusha regs (eax..edi in memory order expected by registers_t)
    *(--sp) = 0; // eax
    *(--sp) = 0; // ecx
    *(--sp) = 0; // edx
    *(--sp) = 0; // ebx
    *(--sp) = 0; // esp (ignored)
    *(--sp) = 0; // ebp
    *(--sp) = 0; // esi
    *(--sp) = 0; // edi

    // segment regs (ds/es/fs/gs) as saved by stub (but struct begins with gs)
    *(--sp) = 0x10; // ds
    *(--sp) = 0x10; // es
    *(--sp) = 0x10; // fs
    *(--sp) = 0x10; // gs

    return (registers_t*)sp;
}

void sched_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_UNUSED;
        tasks[i].regs = 0;
        tasks[i].stack_base = 0;
    }

    // Task 0: idle
    tasks[0].state = TASK_RUNNABLE;
    tasks[0].stack_base = idle_stack;
    tasks[0].regs = build_initial_regs(&idle_stack[STACK_SIZE_DWORDS], idle_task);

    // Task 2: demo task (shows tick counter)
    tasks[2].state = TASK_RUNNABLE;
    tasks[2].stack_base = demo_stack;
    tasks[2].regs = build_initial_regs(&demo_stack[STACK_SIZE_DWORDS], demo_task);

    current_task = 0;
    bootstrap_registered = 0;
}

static int pick_next_task(void) {
    for (int i = 1; i <= MAX_TASKS; i++) {
        int idx = (current_task + i) % MAX_TASKS;
        if (tasks[idx].state == TASK_RUNNABLE && tasks[idx].regs != 0) {
            return idx;
        }
    }
    return current_task;
}

registers_t* sched_on_tick(registers_t* regs) {
    // Lazily register the currently-running bootstrap context as a task.
    if (!bootstrap_registered) {
        tasks[1].state = TASK_RUNNABLE;
        tasks[1].regs = regs;
        tasks[1].stack_base = 0;
        current_task = 1;
        bootstrap_registered = 1;
        return regs;
    }

    // Save current regs pointer into current task slot.
    tasks[current_task].regs = regs;

    int next = pick_next_task();
    current_task = next;
    return tasks[current_task].regs;
}

