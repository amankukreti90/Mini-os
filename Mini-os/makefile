# Compiler and tool definitions
ASM      = nasm
CC       = gcc -m32
LD       = ld -m elf_i386
OBJCOPY  = objcopy
CAT      = cat
RM       = rm -f
MKDIR    = mkdir -p

# Compilation flags
AS_FLAGS_KERNEL = -f elf32
C_FLAGS         = -c -ffreestanding -O2 -Wall -Wextra -m32 -fno-pic -fno-stack-protector
LD_FLAGS        = -T src/linker.ld -nostdlib -static

# File paths
SRC_DIR = src
BIN_DIR = bin

# Source files
BOOT_ASM        = $(SRC_DIR)/boot.asm
KERNEL_ASM      = $(SRC_DIR)/kernel/kernel_entry.asm
KERNEL_C        = $(SRC_DIR)/kernel/kernel.c
VGA_C           = $(SRC_DIR)/graphic/vga.c
VGA_H           = $(SRC_DIR)/graphic/vga.h
GRAPHICS_C      = $(SRC_DIR)/graphic/graphics.c
GRAPHICS_H      = $(SRC_DIR)/graphic/graphics.h
VBE_C           = $(SRC_DIR)/graphic/vbe.c
VBE_H           = $(SRC_DIR)/graphic/vbe.h
IDT_C           = $(SRC_DIR)/idt.c
IDT_H           = $(SRC_DIR)/idt.h
ISR_ASM         = $(SRC_DIR)/isr.asm
KEYBOARD_C      = $(SRC_DIR)/drivers/keyboard.c
KEYBOARD_H      = $(SRC_DIR)/drivers/keyboard.h
MOUSE_C         = $(SRC_DIR)/drivers/mouse.c
MOUSE_H         = $(SRC_DIR)/drivers/mouse.h
IO_C            = $(SRC_DIR)/io.c          # ← ADD THIS
IO_H            = $(SRC_DIR)/io.h          # ← ADD THIS

# Object files
BOOT_BIN        = $(BIN_DIR)/boot.bin
KERNEL_ASM_O    = $(BIN_DIR)/kernel_asm.o
KERNEL_C_O      = $(BIN_DIR)/kernel_c.o
VGA_C_O         = $(BIN_DIR)/vga.o
GRAPHICS_C_O    = $(BIN_DIR)/graphics.o
VBE_C_O         = $(BIN_DIR)/vbe.o
IDT_C_O         = $(BIN_DIR)/idt.o
ISR_ASM_O       = $(BIN_DIR)/isr_asm.o
KEYBOARD_C_O    = $(BIN_DIR)/keyboard.o
MOUSE_C_O       = $(BIN_DIR)/mouse.o
IO_C_O          = $(BIN_DIR)/io.o          # ← ADD THIS
KERNEL_ELF      = $(BIN_DIR)/kernel.elf
KERNEL_BIN      = $(BIN_DIR)/kernel.bin
OS_IMAGE        = $(BIN_DIR)/os-image.bin

.PHONY: all clean run run-vga run-vbe debug quick info help

# Default target
all: $(OS_IMAGE)

# Create final OS image
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN) | $(BIN_DIR)
	$(CAT) $(BOOT_BIN) $(KERNEL_BIN) > $@
	dd if=/dev/zero bs=512 count=64 >> $@ 2>/dev/null || true

# Convert ELF to raw binary
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Link kernel objects - ADD io.o HERE
$(KERNEL_ELF): $(KERNEL_ASM_O) $(KERNEL_C_O) $(VGA_C_O) $(GRAPHICS_C_O) $(VBE_C_O) $(IDT_C_O) $(ISR_ASM_O) $(KEYBOARD_C_O) $(MOUSE_C_O) $(IO_C_O) | $(BIN_DIR)
	$(LD) $(LD_FLAGS) -o $@ $^

# Compile C sources
$(KERNEL_C_O): $(KERNEL_C) $(VGA_H) $(GRAPHICS_H) $(VBE_H) $(IDT_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(VGA_C_O): $(VGA_C) $(VGA_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(GRAPHICS_C_O): $(GRAPHICS_C) $(GRAPHICS_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(VBE_C_O): $(VBE_C) $(VBE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(IDT_C_O): $(IDT_C) $(IDT_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(KEYBOARD_C_O): $(KEYBOARD_C) $(KEYBOARD_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(MOUSE_C_O): $(MOUSE_C) $(MOUSE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# ADD COMPILATION RULE FOR io.c
$(IO_C_O): $(IO_C) $(IO_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Assemble ASM sources
$(KERNEL_ASM_O): $(KERNEL_ASM) | $(BIN_DIR)
	$(ASM) $(AS_FLAGS_KERNEL) $< -o $@

$(ISR_ASM_O): $(ISR_ASM) | $(BIN_DIR)
	$(ASM) $(AS_FLAGS_KERNEL) $< -o $@

$(BOOT_BIN): $(BOOT_ASM) | $(BIN_DIR)
	$(ASM) -f bin $< -o $@

# Create output directory
$(BIN_DIR):
	$(MKDIR) $(BIN_DIR)

# Run targets
run: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy

run-vga: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std

run-vbe: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std -display sdl,gl=on

debug: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -s -S &
	@echo "Now run: gdb -ex 'target remote localhost:1234' -ex 'symbol-file bin/kernel.elf'"

quick: all run

clean:
	@echo "Cleaning build artifacts..."
	$(RM) -r $(BIN_DIR)
	@echo "Clean complete."