// Basic ARM64 kernel for testing
// Save as: ~/OS_proj/src/kernel.c

#include <stdint.h>

// UART0 base address for ARM Virt machine
#define UART0_BASE 0x09000000
#define UART0_DR   ((volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR   ((volatile uint32_t*)(UART0_BASE + 0x18))

// Simple UART functions
void uart_putc(char c) {
    // Wait until transmit FIFO is not full
    while (*UART0_FR & (1 << 5));
    *UART0_DR = c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*str++);
    }
}

// External memory management functions
extern void init_memory(void);
extern void test_memory(void);

// External process management functions
extern void init_process_manager(void);
extern void test_processes(void);

// Kernel main function
void kernel_main(void) {
    uart_puts("Hello from your ARM64 OS!\n");
    uart_puts("Kernel successfully booted.\n");
    uart_puts("System ready for development.\n");
    
    // Initialize memory management
    uart_puts("\n=== Memory Management Setup ===\n");
    init_memory();
    
    // Test memory allocation
    uart_puts("\n=== Memory Allocation Test ===\n");
    test_memory();
    
    // Initialize process management
    uart_puts("\n=== Process Management Setup ===\n");
    init_process_manager();
    
    // Test process creation
    uart_puts("\n=== Process Management Test ===\n");
    test_processes();
    
    uart_puts("\n=== Kernel Ready ===\n");
    uart_puts("Kernel entering main loop...\n");
    
    // Simple loop to keep kernel running
    while (1) {
        // Kernel idle loop
        asm volatile("wfi"); // Wait for interrupt
    }
}

// Entry point is handled by boot.s