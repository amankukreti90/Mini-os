[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Set up stack pointer to 0x90000 (576KB mark)
    mov esp, 0x90000
    
    ; Declare external C kernel_main function
    extern kernel_main
    
    ; Call the main C kernel function
    call kernel_main
    
    ; AFTER kernel_main returns, loop forever
    ; This is a safety measure in case kernel_main ever returns
.halt:
    cli                   ; Disable interrupts
    hlt                   ; Halt CPU
    jmp .halt             ; Jump back in case of wake-up*



    