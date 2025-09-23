// ARM64 Exception Vector Table
// Save as: ~/OS_proj/src/exceptions.s

.section ".text"

// Exception vector table alignment
.align 11
.global exception_vector_table
exception_vector_table:

// Current EL with SP_EL0
.align 7
el1_sp0_sync:
    b exception_handler
.align 7
el1_sp0_irq:
    b irq_handler
.align 7
el1_sp0_fiq:
    b exception_handler
.align 7
el1_sp0_error:
    b exception_handler

// Current EL with SP_ELx
.align 7
el1_spx_sync:
    b exception_handler
.align 7
el1_spx_irq:
    b irq_handler
.align 7
el1_spx_fiq:
    b exception_handler
.align 7
el1_spx_error:
    b exception_handler

// Lower EL using AArch64
.align 7
el0_sync:
    b syscall_handler
.align 7
el0_irq:
    b irq_handler
.align 7
el0_fiq:
    b exception_handler
.align 7
el0_error:
    b exception_handler

// Lower EL using AArch32
.align 7
el0_32_sync:
    b exception_handler
.align 7
el0_32_irq:
    b irq_handler
.align 7
el0_32_fiq:
    b exception_handler
.align 7
el0_32_error:
    b exception_handler

// Exception handler entry point
exception_handler:
    // Save registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x8, x9, [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    str x30, [sp, #-8]!

    // Call C exception handler
    bl handle_exception

    // Restore registers
    ldr x30, [sp], #8
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8, x9, [sp], #16
    ldp x6, x7, [sp], #16
    ldp x4, x5, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    
    eret

// IRQ handler entry point
irq_handler:
    // Save registers (simplified for now)
    stp x0, x1, [sp, #-16]!
    stp x29, x30, [sp, #-16]!

    // Call C IRQ handler
    bl handle_irq

    // Restore registers
    ldp x29, x30, [sp], #16
    ldp x0, x1, [sp], #16
    
    eret

// System call handler entry point
syscall_handler:
    // Save registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x29, x30, [sp, #-16]!

    // Call C syscall handler
    bl handle_syscall

    // Restore registers
    ldp x29, x30, [sp], #16
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    
    eret

// Function to install exception vector table
.global install_exception_table
install_exception_table:
    adr x0, exception_vector_table
    msr vbar_el1, x0
    ret