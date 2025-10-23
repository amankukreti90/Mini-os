ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x100000;    /* Kernel loaded at 1MB */

    .text : {
        *(.text)     /* Code section */
    }

    .rodata : {
        *(.rodata)   /* Read-only data (string literals) */
        *(.rodata.*)
    }

    .data : {
        *(.data)     /* Initialized data */
    }

    .bss : {
        *(.bss)      /* Uninitialized data */
    }
}