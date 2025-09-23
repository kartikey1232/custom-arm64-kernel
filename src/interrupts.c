// ARM64 Interrupt and Exception Handlers
// Save as: ~/OS_proj/src/interrupts.c

#include <stdint.h>

// External UART functions from kernel.c
extern void uart_puts(const char* str);

// Timer control bits
#define TIMER_CTRL_ENABLE    (1 << 0)
#define TIMER_CTRL_IMASK     (1 << 1)
#define TIMER_CTRL_ISTATUS   (1 << 2)

// Global tick counter
static volatile uint64_t system_ticks = 0;

// Initialize a simple timer (without actual timer for now)
void init_timer(void) {
    uart_puts("Timer system initialized (basic mode).\n");
    // For now, we'll just set up the framework
    // Real timer setup will come later when we have proper GIC setup
}

// Exception handler
void handle_exception(void) {
    uart_puts("Exception occurred!\n");
    
    // Read exception syndrome register
    uint64_t esr;
    asm volatile("mrs %0, esr_el1" : "=r"(esr));
    
    // Read faulting address register  
    uint64_t far;
    asm volatile("mrs %0, far_el1" : "=r"(far));
    
    // Print some debug info
    uart_puts("ESR_EL1: ");
    // Simple hex print for debugging
    char buffer[20];
    for (int i = 15; i >= 0; i--) {
        int digit = (esr >> (i * 4)) & 0xF;
        buffer[15-i] = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
    }
    buffer[16] = '\n';
    buffer[17] = '\0';
    uart_puts(buffer);
    
    uart_puts("System continuing after exception...\n");
    // Don't halt - just return and continue
}

// IRQ handler
void handle_irq(void) {
    system_ticks++;
    uart_puts("IRQ received!\n");
    
    // For now, just acknowledge we got an interrupt
    // Real timer handling will be added later
}

// System call handler
void handle_syscall(void) {
    uart_puts("System call received!\n");
    // For now, just acknowledge the syscall
    // Later we'll implement actual syscall functionality
}

// Function to enable interrupts
void enable_interrupts(void) {
    uart_puts("Interrupt framework ready.\n");
    // For now, don't actually enable interrupts until we have proper setup
    // asm volatile("msr daifclr, #2"); // Clear IRQ mask bit
    uart_puts("(Interrupts will be enabled in future version)\n");
}

// Function to disable interrupts  
void disable_interrupts(void) {
    asm volatile("msr daifset, #2"); // Set IRQ mask bit
}