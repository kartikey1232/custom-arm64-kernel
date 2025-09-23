// Basic Memory Manager for ARM64 OS
// Save as: ~/OS_proj/src/memory.c

#include <stdint.h>
#include <stddef.h>

// External UART functions
extern void uart_puts(const char* str);
extern void uart_putc(char c);

// Memory layout definitions
#define KERNEL_START    0x40080000
#define HEAP_START      0x40200000  // Start heap at 2MB mark
#define HEAP_SIZE       0x00800000  // 8MB heap
#define HEAP_END        (HEAP_START + HEAP_SIZE)

// Simple block header for heap management
typedef struct block_header {
    size_t size;              // Size of this block (including header)
    int is_free;             // 1 if free, 0 if allocated
    struct block_header* next; // Next block in the list
} block_header_t;

// Global heap state
static block_header_t* heap_start = NULL;
static uint8_t* heap_memory = (uint8_t*)HEAP_START;
static size_t heap_initialized = 0;

// Simple utility functions
static void print_hex(uint64_t value) {
    uart_puts("0x");
    for (int i = 15; i >= 0; i--) {
        int digit = (value >> (i * 4)) & 0xF;
        char c = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        uart_putc(c);
    }
}

static void print_decimal(uint64_t value) {
    if (value == 0) {
        uart_putc('0');
        return;
    }
    
    char buffer[20];
    int pos = 0;
    
    while (value > 0 && pos < 19) {
        buffer[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Print in reverse order
    for (int i = pos - 1; i >= 0; i--) {
        uart_putc(buffer[i]);
    }
}

// Initialize the heap
void init_memory(void) {
    uart_puts("Initializing memory management...\n");
    
    // Print memory layout
    uart_puts("Kernel start: ");
    print_hex(KERNEL_START);
    uart_puts("\nHeap start: ");
    print_hex(HEAP_START);
    uart_puts("\nHeap size: ");
    print_decimal(HEAP_SIZE / 1024);
    uart_puts(" KB\n");
    
    // Initialize the first block (entire heap is one free block)
    heap_start = (block_header_t*)heap_memory;
    heap_start->size = HEAP_SIZE - sizeof(block_header_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    
    heap_initialized = 1;
    uart_puts("Memory management initialized.\n");
}

// Simple malloc implementation
void* kmalloc(size_t size) {
    if (!heap_initialized) {
        return NULL;
    }
    
    // Add padding for alignment
    size = (size + 7) & ~7;  // 8-byte alignment
    
    block_header_t* current = heap_start;
    
    // First-fit allocation
    while (current != NULL) {
        if (current->is_free && current->size >= size) {
            // Found a suitable block
            if (current->size > size + sizeof(block_header_t) + 8) {
                // Split the block
                block_header_t* new_block = (block_header_t*)((uint8_t*)current + sizeof(block_header_t) + size);
                new_block->size = current->size - size - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            return (uint8_t*)current + sizeof(block_header_t);
        }
        current = current->next;
    }
    
    return NULL; // No suitable block found
}

// Simple free implementation
void kfree(void* ptr) {
    if (!ptr || !heap_initialized) {
        return;
    }
    
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    block->is_free = 1;
    
    // Simple coalescing with next block
    if (block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(block_header_t);
        block->next = block->next->next;
    }
}

// Memory statistics
void print_memory_stats(void) {
    if (!heap_initialized) {
        uart_puts("Memory not initialized.\n");
        return;
    }
    
    uart_puts("\n=== Memory Statistics ===\n");
    
    size_t total_free = 0;
    size_t total_used = 0;
    int free_blocks = 0;
    int used_blocks = 0;
    
    block_header_t* current = heap_start;
    while (current != NULL) {
        if (current->is_free) {
            total_free += current->size;
            free_blocks++;
        } else {
            total_used += current->size;
            used_blocks++;
        }
        current = current->next;
    }
    
    uart_puts("Free memory: ");
    print_decimal(total_free);
    uart_puts(" bytes (");
    print_decimal(free_blocks);
    uart_puts(" blocks)\n");
    
    uart_puts("Used memory: ");
    print_decimal(total_used);
    uart_puts(" bytes (");
    print_decimal(used_blocks);
    uart_puts(" blocks)\n");
    
    uart_puts("Total heap: ");
    print_decimal(HEAP_SIZE);
    uart_puts(" bytes\n");
    uart_puts("========================\n\n");
}

// Test memory allocation
void test_memory(void) {
    uart_puts("Testing memory allocation...\n");
    
    // Test 1: Basic allocation
    void* ptr1 = kmalloc(100);
    uart_puts("Allocated 100 bytes: ");
    print_hex((uint64_t)ptr1);
    uart_puts("\n");
    
    // Test 2: Multiple allocations
    void* ptr2 = kmalloc(200);
    void* ptr3 = kmalloc(50);
    
    uart_puts("Allocated 200 bytes: ");
    print_hex((uint64_t)ptr2);
    uart_puts("\n");
    uart_puts("Allocated 50 bytes: ");
    print_hex((uint64_t)ptr3);
    uart_puts("\n");
    
    print_memory_stats();
    
    // Test 3: Free and reallocate
    uart_puts("Freeing first allocation...\n");
    kfree(ptr1);
    
    print_memory_stats();
    
    // Test 4: Allocate after free
    void* ptr4 = kmalloc(80);
    uart_puts("Allocated 80 bytes: ");
    print_hex((uint64_t)ptr4);
    uart_puts("\n");
    
    print_memory_stats();
    
    uart_puts("Memory test completed.\n");
}