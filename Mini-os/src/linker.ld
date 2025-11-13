ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
    . = 0x100000;    /* Kernel loaded at 1MB */

    .text : {
        *(.text)     /* Code section */
    }

    .rodata : {
        *(.rodata)   /* Read-only data */
        *(.rodata.*)
    }

    .data : {
        *(.data)     /* Initialized data */
    }

    .bss : {
        _bss_start = .;
        *(COMMON)    /* Common symbols */
        *(.bss)      /* Uninitialized data */
        _bss_end = .;
    }
}