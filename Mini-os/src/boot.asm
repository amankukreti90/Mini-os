[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments and stack to known state
    mov [boot_drive], dl  ; Save boot drive from DL
    xor ax, ax            ; Zero AX register
    mov ds, ax            ; Set data segment to 0
    mov es, ax            ; Set extra segment to 0
    mov ss, ax            ; Set stack segment to 0
    mov sp, 0x7C00        ; Set stack pointer just below bootloader
    
    ; Display boot message
    mov si, boot_msg
    call print_string
    
    ; Set VBE graphics mode 1024x768x24bpp
    mov ax, 0x4F02        ; VBE set mode function
    mov bx, 0x4118        ; Mode 0x118: 1024x768x24bpp
    int 0x10              ; Call VBE BIOS interrupt
    cmp ax, 0x004F        ; Check if successful (AX=0x004F)
    jne vbe_error         ; Jump if VBE failed
    
    mov si, vbe_success_msg
    call print_string
    
    ; Save VBE mode info for kernel at 0x5000
    call save_vbe_info
    
    ; Load kernel from disk to 0x10000 (64KB)
    mov ax, 0x1000        ; ES segment for 0x10000
    mov es, ax
    xor bx, bx            ; BX offset 0
    mov ah, 0x02          ; BIOS read sectors function
    mov al, 50            ; Number of sectors to read
    mov ch, 0             ; Cylinder 0
    mov cl, 2             ; Sector 2 (after boot sector)
    mov dh, 0             ; Head 0
    mov dl, [boot_drive]  ; Boot drive
    int 0x13              ; BIOS disk interrupt
    jc disk_error         ; Jump if disk error
    
    mov si, load_msg
    call print_string
    
    ; DISABLE INTERRUPTS BEFORE SWITCHING TO PROTECTED MODE
    cli
    
    ; Load Global Descriptor Table
    lgdt [gdt_descriptor]
    
    ; Switch to protected mode by setting CR0 bit 0
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ; Far jump to flush pipeline and load CS with code segment
    jmp CODE_SEG:init_pm

; === VBE Info Saving Function ===
save_vbe_info:
    mov ax, 0x4F01        ; VBE get mode info function
    mov cx, 0x4118        ; Mode 0x118
    mov di, 0x5000        ; Store info at 0x5000
    int 0x10              ; Call VBE BIOS
    ret

vbe_error:
    mov si, vbe_error_msg
    call print_string
    ; Continue booting despite VBE error

; === 16-bit Real Mode Functions ===
[BITS 16]
print_string:
    mov ah, 0x0E          ; BIOS teletype function
.loop:
    lodsb                 ; Load byte from SI into AL
    test al, al           ; Test for null terminator
    jz .done              ; If zero, string is done
    int 0x10              ; Print character
    jmp .loop             ; Continue loop
.done:
    ret

disk_error:
    mov si, error_msg
    call print_string
    jmp $                 ; Halt on disk error

; === 32-bit Protected Mode Initialization ===
[BITS 32]
init_pm:
    ; Initialize all segment registers to data segment
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000      ; Set stack pointer to 576KB
    
    ; Copy kernel from 0x10000 to 0x100000 (1MB)
    mov esi, 0x10000      ; Source address
    mov edi, 0x100000     ; Destination address
    mov ecx, 50 * 128     ; 50 sectors * 128 dwords per sector
    cld                   ; Clear direction flag (forward copy)
    rep movsd             ; Copy ECX dwords from ESI to EDI
    
    ; Set up temporary IDT with limit 0 to prevent interrupts
    lidt [idtr_temp]
    
    ; Far jump to kernel at 1MB
    jmp CODE_SEG:0x100000

; === Temporary IDT Descriptor (disables interrupts) ===
idtr_temp:
    dw 0                  ; Limit 0 = no valid IDT entries
    dd 0                  ; Base address 0

; === Data Section ===
boot_msg:        db 'Booting...', 0x0D, 0x0A, 0
vbe_success_msg: db 'VBE mode set', 0x0D, 0x0A, 0
vbe_error_msg:   db 'VBE failed', 0x0D, 0x0A, 0
load_msg:        db 'Kernel loaded', 0x0D, 0x0A, 0
error_msg:       db 'Error!', 0
boot_drive:      db 0

; === Global Descriptor Table ===
gdt_start:
    dq 0                  ; Null descriptor (required)
gdt_code:
    dw 0xFFFF             ; Limit 0-15
    dw 0x0000             ; Base 0-15
    db 0x00               ; Base 16-23
    db 0x9A               ; Access: Present, Ring 0, Exec/Read
    db 0xCF               ; Flags: 4K granular, 32-bit; Limit 16-19
    db 0x00               ; Base 24-31
gdt_data:
    dw 0xFFFF             ; Limit 0-15
    dw 0x0000             ; Base 0-15
    db 0x00               ; Base 16-23
    db 0x92               ; Access: Present, Ring 0, Read/Write
    db 0xCF               ; Flags: 4K granular, 32-bit; Limit 16-19
    db 0x00               ; Base 24-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT limit (size-1)
    dd gdt_start                ; GDT base address

CODE_SEG equ gdt_code - gdt_start  ; Code segment selector
DATA_SEG equ gdt_data - gdt_start  ; Data segment selector

; Boot signature (required by BIOS)
times 510-($-$$) db 0
dw 0xAA55