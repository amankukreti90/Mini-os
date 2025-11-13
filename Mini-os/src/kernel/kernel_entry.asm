[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Set up stack pointer with proper alignment
    mov esp, _stack_top
    
    ; Clear direction flag for string operations
    cld
    
    ; Clear the BSS section (uninitialized data)
    extern _bss_start
    extern _bss_end
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, _bss_start
    xor eax, eax
    rep stosb
    
    ; Declare external C kernel_main function
    extern kernel_main
    
    ; Call the main C kernel function
    call kernel_main
    
    ; If kernel_main returns, halt safely
.halt:
    cli                   ; Disable interrupts
    hlt                   ; Halt CPU
    jmp .halt             ; Jump back in case of wake-up

; Stack space definition
section .bss
align 16
_stack_bottom:
    resb 32768           ; 32KB stack for Snake game
_stack_top: