// ARM64 Context Switching Assembly
// Save as: ~/OS_proj/src/context_switch.s

.section ".text"

// Function: switch_context(old_context*, new_context*)
// x0 = pointer to old process context
// x1 = pointer to new process context
.global switch_context
switch_context:
    // Save current process context (if old_context is not NULL)
    cbz x0, restore_new_context
    
    // Save all general purpose registers
    stp x2, x3,   [x0, #16]   // x2, x3
    stp x4, x5,   [x0, #32]   // x4, x5
    stp x6, x7,   [x0, #48]   // x6, x7
    stp x8, x9,   [x0, #64]   // x8, x9
    stp x10, x11, [x0, #80]   // x10, x11
    stp x12, x13, [x0, #96]   // x12, x13
    stp x14, x15, [x0, #112]  // x14, x15
    stp x16, x17, [x0, #128]  // x16, x17
    stp x18, x19, [x0, #144]  // x18, x19
    stp x20, x21, [x0, #160]  // x20, x21
    stp x22, x23, [x0, #176]  // x22, x23
    stp x24, x25, [x0, #192]  // x24, x25
    stp x26, x27, [x0, #208]  // x26, x27
    stp x28, x29, [x0, #224]  // x28, x29
    str x30,      [x0, #240]  // x30 (link register)
    
    // Save stack pointer
    mov x2, sp
    str x2,       [x0, #248]  // sp
    
    // Save return address as PC
    str x30,      [x0, #256]  // pc
    
    // Save processor state
    mrs x2, daif
    str x2,       [x0, #264]  // pstate

restore_new_context:
    // Restore new process context (if new_context is not NULL)
    cbz x1, context_switch_done
    
    // Restore processor state
    ldr x2,       [x1, #264]  // pstate
    msr daif, x2
    
    // Restore stack pointer
    ldr x2,       [x1, #248]  // sp
    mov sp, x2
    
    // Restore general purpose registers
    ldp x2, x3,   [x1, #16]   // x2, x3
    ldp x4, x5,   [x1, #32]   // x4, x5
    ldp x6, x7,   [x1, #48]   // x6, x7
    ldp x8, x9,   [x1, #64]   // x8, x9
    ldp x10, x11, [x1, #80]   // x10, x11
    ldp x12, x13, [x1, #96]   // x12, x13
    ldp x14, x15, [x1, #112]  // x14, x15
    ldp x16, x17, [x1, #128]  // x16, x17
    ldp x18, x19, [x1, #144]  // x18, x19
    ldp x20, x21, [x1, #160]  // x20, x21
    ldp x22, x23, [x1, #176]  // x22, x23
    ldp x24, x25, [x1, #192]  // x24, x25
    ldp x26, x27, [x1, #208]  // x26, x27
    ldp x28, x29, [x1, #224]  // x28, x29
    ldr x30,      [x1, #240]  // x30 (link register)
    
    // Load PC for new process
    ldr x0,       [x1, #256]  // pc
    
    // For initial process start, jump to entry point
    // For resumed process, return normally
    br x0

context_switch_done:
    ret

// Function: start_first_process(context*)
// x0 = pointer to first process context
.global start_first_process
start_first_process:
    // Simple approach - just jump to PC with correct stack
    ldr x1, [x0, #248]    // Load SP
    mov sp, x1            // Set stack pointer
    
    ldr x1, [x0, #256]    // Load PC  
    br x1                 // Jump to process entry point
