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
C_FLAGS         = -c -ffreestanding -O2 -Wall -Wextra -m32 -fno-pic -fno-stack-protector -fno-builtin -I$(SRC_DIR)
DEP_FLAGS       = -MMD -MP
C_FLAGS        += $(DEP_FLAGS)
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
IO_C            = $(SRC_DIR)/io.c
IO_H            = $(SRC_DIR)/io.h

# System Call, Shell, Filesystem, and RTC files
SYSCALL_C       = $(SRC_DIR)/syscall/syscall.c
SYSCALL_H       = $(SRC_DIR)/syscall/syscall.h
SHELL_C         = $(SRC_DIR)/shell/shell.c
SHELL_H         = $(SRC_DIR)/shell/shell.h
COMMANDS_C      = $(SRC_DIR)/shell/commands.c
COMMANDS_H      = $(SRC_DIR)/shell/commands.h
FILESYSTEM_C    = $(SRC_DIR)/fs/filesystem.c
FILESYSTEM_H    = $(SRC_DIR)/fs/filesystem.h
RTC_C           = $(SRC_DIR)/drivers/rtc.c
RTC_H           = $(SRC_DIR)/drivers/rtc.h

# Boot Menu files
BOOT_MENU_C     = $(SRC_DIR)/boot_menu.c
BOOT_MENU_H     = $(SRC_DIR)/boot_menu.h

# Game files
SNAKE_C         = $(SRC_DIR)/snake/snake.c
SNAKE_H         = $(SRC_DIR)/snake/snake.h

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
IO_C_O          = $(BIN_DIR)/io.o

# System Call, Shell, Filesystem, and RTC object files
SYSCALL_C_O     = $(BIN_DIR)/syscall.o
SHELL_C_O       = $(BIN_DIR)/shell.o
COMMANDS_C_O    = $(BIN_DIR)/commands.o
FILESYSTEM_C_O  = $(BIN_DIR)/filesystem.o
RTC_C_O         = $(BIN_DIR)/rtc.o

# Boot Menu object files
BOOT_MENU_C_O   = $(BIN_DIR)/boot_menu.o

# Game object files
SNAKE_C_O       = $(BIN_DIR)/snake.o

KERNEL_ELF      = $(BIN_DIR)/kernel.elf
KERNEL_BIN      = $(BIN_DIR)/kernel.bin
OS_IMAGE        = $(BIN_DIR)/os-image.bin

# Dependency files
DEPS = $(shell find $(BIN_DIR) -name "*.d" 2>/dev/null)

.PHONY: all clean run run-vga run-vbe debug quick rebuild test

# Default target - build complete OS image
all: $(OS_IMAGE)

# Create final OS image by combining bootloader and kernel
$(OS_IMAGE): $(BOOT_BIN) $(KERNEL_BIN) | $(BIN_DIR)
	$(CAT) $(BOOT_BIN) $(KERNEL_BIN) > $@
	dd if=/dev/zero bs=512 count=64 >> $@ 2>/dev/null || true

# Convert ELF kernel to raw binary format
$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

# Link all kernel object files into ELF executable
$(KERNEL_ELF): $(KERNEL_ASM_O) $(KERNEL_C_O) $(VGA_C_O) $(GRAPHICS_C_O) $(VBE_C_O) $(IDT_C_O) $(ISR_ASM_O) $(KEYBOARD_C_O) $(MOUSE_C_O) $(IO_C_O) $(SYSCALL_C_O) $(SHELL_C_O) $(COMMANDS_C_O) $(FILESYSTEM_C_O) $(RTC_C_O) $(BOOT_MENU_C_O) $(SNAKE_C_O) | $(BIN_DIR)
	$(LD) $(LD_FLAGS) -o $@ $^

# Compile C sources in dependency order

# First compile basic utilities and filesystem
$(FILESYSTEM_C_O): $(FILESYSTEM_C) $(FILESYSTEM_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(IO_C_O): $(IO_C) $(IO_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile RTC driver (needs IO)
$(RTC_C_O): $(RTC_C) $(RTC_H) $(IO_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile graphics components
$(VGA_C_O): $(VGA_C) $(VGA_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(GRAPHICS_C_O): $(GRAPHICS_C) $(GRAPHICS_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(VBE_C_O): $(VBE_C) $(VBE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile system components
$(IDT_C_O): $(IDT_C) $(IDT_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(SYSCALL_C_O): $(SYSCALL_C) $(SYSCALL_H) $(IDT_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile boot menu (needs VBE, keyboard, shell, and snake)
$(BOOT_MENU_C_O): $(BOOT_MENU_C) $(BOOT_MENU_H) $(VBE_H) $(KEYBOARD_H) $(SHELL_H) $(SNAKE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile snake game (needs VBE and keyboard)
$(SNAKE_C_O): $(SNAKE_C) $(SNAKE_H) $(VBE_H) $(KEYBOARD_H) $(IO_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile commands (needs filesystem, graphics, RTC, and shell headers)
$(COMMANDS_C_O): $(COMMANDS_C) $(COMMANDS_H) $(VBE_H) $(FILESYSTEM_H) $(RTC_H) $(SHELL_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile drivers (needs IO and graphics)
$(KEYBOARD_C_O): $(KEYBOARD_C) $(KEYBOARD_H) $(IO_H) $(VBE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

$(MOUSE_C_O): $(MOUSE_C) $(MOUSE_H) $(IO_H) $(VBE_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Then compile shell (needs filesystem, graphics, drivers, RTC, and commands)
$(SHELL_C_O): $(SHELL_C) $(SHELL_H) $(VBE_H) $(FILESYSTEM_H) $(KEYBOARD_H) $(RTC_H) $(COMMANDS_H) | $(BIN_DIR)
	$(CC) $(C_FLAGS) $< -o $@

# Finally compile kernel (needs everything)
$(KERNEL_C_O): $(KERNEL_C) $(VGA_H) $(GRAPHICS_H) $(VBE_H) $(IDT_H) $(SYSCALL_H) $(SHELL_H) $(FILESYSTEM_H) $(KEYBOARD_H) $(MOUSE_H) $(RTC_H) $(COMMANDS_H) $(BOOT_MENU_H) $(SNAKE_H) | $(BIN_DIR)
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

# Include dependency files
-include $(DEPS)

# Run targets
run: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy

run-vga: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std

run-vbe: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std -display sdl,gl=on

run-mouse: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std -usb -device usb-mouse

run-game: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -vga std -display sdl,gl=on

debug: $(OS_IMAGE)
	qemu-system-i386 -drive file=$(OS_IMAGE),format=raw,if=floppy -s -S &
	@echo "Now run: gdb -ex 'target remote localhost:1234' -ex 'symbol-file bin/kernel.elf'"

quick: all run

# Clean and rebuild
rebuild: clean all

# Test build and run
test: quick

clean:
	@echo "Cleaning build artifacts..."
	$(RM) -r $(BIN_DIR)
	$(RM) $(DEPS)
	@echo "Clean complete."

# Help target
help:
	@echo "Available targets:"
	@echo "  all      - Build the complete OS image"
	@echo "  clean    - Remove all build artifacts"
	@echo "  run      - Run in QEMU with default settings"
	@echo "  run-vga  - Run with VGA graphics"
	@echo "  run-vbe  - Run with VBE graphics (recommended)"
	@echo "  run-game - Run with game-optimized settings"
	@echo "  debug    - Run with GDB debugging"
	@echo "  quick    - Build and run quickly"
	@echo "  rebuild  - Clean and rebuild everything"
	@echo "  test     - Build and run (same as quick)"
	@echo "  help     - Show this help message"