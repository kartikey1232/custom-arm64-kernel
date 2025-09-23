/* ARM64 Kernel Linker Script */
/* Save as: ~/OS_proj/src/kernel.ld */

ENTRY(_start)

SECTIONS
{
    /* Kernel loads at 1MB */
    . = 0x40080000;
    
    /* Boot section first */
    .text.boot : {
        *(.text.boot)
    }
    
    /* Main text section */
    .text : {
        . = ALIGN(4096);
        *(.text)
    }
    
    /* Read-only data */
    .rodata : {
        . = ALIGN(4096);
        *(.rodata)
    }
    
    /* Read-write data */
    .data : {
        . = ALIGN(4096);
        *(.data)
    }
    
    /* BSS section */
    .bss : {
        . = ALIGN(4096);
        __bss_start = .;
        *(.bss)
        __bss_end = .;
    }
    
    /* Calculate BSS size */
    __bss_size = (__bss_end - __bss_start);
    
    /* End of kernel */
    . = ALIGN(4096);
    __end = .;
}