section .text

; Macro for CPU exceptions without error codes
; Pushes dummy error code (0) and interrupt number
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte %1            ; Push interrupt number
    jmp isr_common_stub     ; Jump to common handler
%endmacro

; Macro for CPU exceptions with error codes  
; CPU automatically pushes error code, we push interrupt number
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli                     ; Disable interrupts
    push byte %1            ; Push interrupt number (error code already pushed by CPU)
    jmp isr_common_stub     ; Jump to common handler
%endmacro

; Create CPU Exception Handlers (Interrupts 0-31)
ISR_NOERRCODE 0   ; Divide by zero exception
ISR_NOERRCODE 1   ; Debug exception
ISR_NOERRCODE 2   ; Non-maskable interrupt
ISR_NOERRCODE 3   ; Breakpoint exception
ISR_NOERRCODE 4   ; Overflow exception
ISR_NOERRCODE 5   ; Bound range exceeded
ISR_NOERRCODE 6   ; Invalid opcode
ISR_NOERRCODE 7   ; Device not available
ISR_ERRCODE   8   ; Double fault (with error code)
ISR_NOERRCODE 9   ; Coprocessor segment overrun
ISR_ERRCODE   10  ; Invalid TSS (with error code)
ISR_ERRCODE   11  ; Segment not present (with error code)
ISR_ERRCODE   12  ; Stack-segment fault (with error code)
ISR_ERRCODE   13  ; General protection fault (with error code)
ISR_ERRCODE   14  ; Page fault (with error code)
ISR_NOERRCODE 15  ; Reserved
ISR_NOERRCODE 16  ; x87 floating-point exception
ISR_ERRCODE   17  ; Alignment check (with error code)
ISR_NOERRCODE 18  ; Machine check
ISR_NOERRCODE 19  ; SIMD floating-point exception
ISR_NOERRCODE 20  ; Virtualization exception
ISR_NOERRCODE 21  ; Control protection exception
ISR_NOERRCODE 22  ; Reserved
ISR_NOERRCODE 23  ; Reserved
ISR_NOERRCODE 24  ; Reserved
ISR_NOERRCODE 25  ; Reserved
ISR_NOERRCODE 26  ; Reserved
ISR_NOERRCODE 27  ; Reserved
ISR_NOERRCODE 28  ; Hypervisor injection exception
ISR_NOERRCODE 29  ; VMM communication exception
ISR_NOERRCODE 30  ; Security exception
ISR_NOERRCODE 31  ; Reserved

; System Call Interrupt (0x80 = 128)
global isr128
isr128:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte 128           ; Push interrupt number (128 = 0x80)
    jmp isr_common_stub     ; Jump to common handler

; Macro for Hardware Interrupts (IRQs)
; Maps IRQ numbers to interrupt vectors 32-47
%macro IRQ 2
global irq%1
irq%1:
    cli                     ; Disable interrupts
    push byte 0             ; Push dummy error code
    push byte %2            ; Push interrupt vector (32 + IRQ)
    jmp irq_common_stub     ; Jump to common handler
%endmacro

; Create Hardware Interrupt Handlers (IRQ0-15 -> INT 32-47)
IRQ 0, 32   ; Timer (IRQ0 -> INT 32)
IRQ 1, 33   ; Keyboard (IRQ1 -> INT 33) - Primary input device
IRQ 2, 34   ; Cascade (IRQ2 -> INT 34)
IRQ 3, 35   ; COM2 (IRQ3 -> INT 35)
IRQ 4, 36   ; COM1 (IRQ4 -> INT 36)
IRQ 5, 37   ; LPT2 (IRQ5 -> INT 37)
IRQ 6, 38   ; Floppy (IRQ6 -> INT 38)
IRQ 7, 39   ; LPT1 (IRQ7 -> INT 39)
IRQ 8, 40   ; RTC (IRQ8 -> INT 40)
IRQ 9, 41   ; Legacy (IRQ9 -> INT 41)
IRQ 10, 42  ; Reserved (IRQ10 -> INT 42)
IRQ 11, 43  ; Reserved (IRQ11 -> INT 43)
IRQ 12, 44  ; PS/2 Mouse (IRQ12 -> INT 44)
IRQ 13, 45  ; FPU (IRQ13 -> INT 45)
IRQ 14, 46  ; Primary ATA (IRQ14 -> INT 46)
IRQ 15, 47  ; Secondary ATA (IRQ15 -> INT 47)

; External C function for exception handling
extern isr_handler

; Common stub for all CPU exceptions (0-31)
; Saves processor state and calls C handler
isr_common_stub:
    ; Save all general-purpose registers (EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI)
    pusha
    
    ; Save segment registers
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment (0x10 from GDT)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Push current stack pointer (points to registers_t structure)
    push esp
    
    ; Call C ISR handler with register state
    call isr_handler
    
    ; Clean up stack pointer parameter
    add esp, 4
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore general-purpose registers
    popa
    
    ; Clean up error code and interrupt number from stack
    add esp, 8
    
    ; Re-enable interrupts and return from interrupt
    sti
    iret

; External C function for hardware interrupt handling  
extern irq_handler

; Common stub for all hardware interrupts (32-47)
; Saves processor state and calls C handler
irq_common_stub:
    ; Save all general-purpose registers
    pusha
    
    ; Save segment registers
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment (0x10 from GDT)
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Push current stack pointer (points to registers_t structure)
    push esp
    
    ; Call C IRQ handler with register state
    call irq_handler
    
    ; Clean up stack pointer parameter
    add esp, 4
    
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds
    
    ; Restore general-purpose registers
    popa
    
    ; Clean up error code and interrupt number from stack
    add esp, 8
    
    ; Re-enable interrupts and return from interrupt
    sti
    iret