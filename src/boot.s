// ARM64 Boot Assembly Code
// Save as: ~/OS_proj/src/boot.s

.section ".text.boot"

.global _start

_start:
    // We enter here on all cores
    // Check if we're on core 0
    mrs x1, mpidr_el1
    and x1, x1, #3
    cbz x1, core0
    
    // If not core 0, put core to sleep
core_hang:
    wfe
    b core_hang

core0:
    // Set up stack pointer
    // Place stack at 0x80000 (before our kernel load address)
    mov sp, #0x80000
    
    // Clear BSS section
    ldr x1, =__bss_start
    ldr w2, =__bss_size
    
clear_bss:
    cbz w2, clear_done
    str xzr, [x1], #8
    sub w2, w2, #8
    cbnz w2, clear_bss

clear_done:
    // Jump to C kernel
    bl kernel_main
    
    // If kernel returns, hang
hang:
    wfe
    b hang

// Define BSS section symbols (will be defined by linker)
.section ".bss"
__bss_start:
.section ".bss"
__bss_end: