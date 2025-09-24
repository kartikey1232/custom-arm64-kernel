// ARM64 Process Management System
// Save as: ~/OS_proj/src/process.c

#include <stdint.h>
#include <stddef.h>

// External functions
extern void uart_puts(const char* str);
extern void uart_putc(char c);
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);


// Process states
typedef enum {
    PROCESS_READY = 0,
    PROCESS_RUNNING = 1,
    PROCESS_BLOCKED = 2,
    PROCESS_TERMINATED = 3
} process_state_t;

// ARM64 CPU context for process switching
typedef struct {
    uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27, x28, x29, x30;
    uint64_t sp;        // Stack pointer
    uint64_t pc;        // Program counter
    uint64_t pstate;    // Processor state
} cpu_context_t;
extern void switch_context(cpu_context_t* old_ctx, cpu_context_t* new_ctx);
extern void start_first_process(cpu_context_t* ctx);
void schedule(void);

// Process Control Block
typedef struct process {
    int pid;                    // Process ID
    char name[32];             // Process name
    process_state_t state;     // Current state
    cpu_context_t context;     // Saved CPU context
    uint8_t* stack_base;       // Base of process stack
    size_t stack_size;         // Stack size
    uint64_t time_slice;       // Time slice remaining
    struct process* next;      // Next process in list
} process_t;

// Process management globals
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static process_t* ready_queue = NULL;
static int next_pid = 1;
static uint64_t scheduler_ticks = 0;

// Stack size for each process (64KB)
#define PROCESS_STACK_SIZE 0x10000
#define TIME_SLICE_TICKS 10

// Utility functions
static void print_hex(uint64_t value) {
    uart_puts("0x");
    for (int i = 15; i >= 0; i--) {
        int digit = (value >> (i * 4)) & 0xF;
        char c = (digit < 10) ? ('0' + digit) : ('A' + digit - 10);
        uart_putc(c);
    }
}

static void print_decimal(int value) {
    if (value == 0) {
        uart_putc('0');
        return;
    }
    
    if (value < 0) {
        uart_putc('-');
        value = -value;
    }
    
    char buffer[12];
    int pos = 0;
    
    while (value > 0 && pos < 11) {
        buffer[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Print in reverse order
    for (int i = pos - 1; i >= 0; i--) {
        uart_putc(buffer[i]);
    }
}

// Simple string copy
static void strcpy_simple(char* dest, const char* src, size_t max_len) {
    size_t i = 0;
    while (src[i] && i < max_len - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Initialize process management
void init_process_manager(void) {
    uart_puts("Initializing process management...\n");
    
    process_list = NULL;
    current_process = NULL;
    ready_queue = NULL;
    next_pid = 1;
    scheduler_ticks = 0;
    
    uart_puts("Process manager initialized.\n");
}

// Process wrapper function to handle process termination
static void process_wrapper(void (*entry_point)(void)) {
    // Call the actual process function
    entry_point();
    
    // If process returns, mark it as terminated
    if (current_process) {
        current_process->state = PROCESS_TERMINATED;
        uart_puts("Process ");
        print_decimal(current_process->pid);
        uart_puts(" terminated.\n");
    }
    
    // Yield to scheduler
    schedule();
    
    // Should never reach here
    while(1) {
        asm volatile("wfi");
    }
}

// Create a new process
process_t* create_process(const char* name, void (*entry_point)(void)) {
    uart_puts("Creating process: ");
    uart_puts(name);
    uart_puts("\n");
    
    // Allocate process control block
    process_t* proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) {
        uart_puts("Failed to allocate PCB!\n");
        return NULL;
    }
    
    // Allocate stack
    proc->stack_base = (uint8_t*)kmalloc(PROCESS_STACK_SIZE);
    if (!proc->stack_base) {
        uart_puts("Failed to allocate stack!\n");
        kfree(proc);
        return NULL;
    }
    
    // Initialize process fields
    proc->pid = next_pid++;
    strcpy_simple(proc->name, name, sizeof(proc->name));
    proc->state = PROCESS_READY;
    proc->stack_size = PROCESS_STACK_SIZE;
    proc->time_slice = TIME_SLICE_TICKS;
    proc->next = NULL;
    
    // Initialize context - simpler approach
    // Clear all registers  
    for (int i = 0; i < 31; i++) {
        *((uint64_t*)&proc->context + i) = 0;
    }
    
    // Set stack pointer to top of stack (stacks grow downward)
    proc->context.sp = (uint64_t)(proc->stack_base + PROCESS_STACK_SIZE - 16);
    
    // Set program counter to entry point directly
    proc->context.pc = (uint64_t)entry_point;
    
    // Set processor state (simple)
    proc->context.pstate = 0;
    
    // Add to process list
    proc->next = process_list;
    process_list = proc;
    
    uart_puts("Process created - PID: ");
    print_decimal(proc->pid);
    uart_puts(", Stack: ");
    print_hex((uint64_t)proc->stack_base);
    uart_puts("\n");
    
    return proc;
}

// Add process to ready queue
void schedule_process(process_t* proc) {
    if (!proc || proc->state != PROCESS_READY) {
        return;
    }
    
    // Simple FIFO ready queue
    if (!ready_queue) {
        ready_queue = proc;
        proc->next = NULL;
    } else {
        process_t* current = ready_queue;
        while (current->next) {
            current = current->next;
        }
        current->next = proc;
        proc->next = NULL;
    }
}

// Get next process from ready queue
process_t* get_next_process(void) {
    if (!ready_queue) {
        return NULL;
    }
    
    process_t* next = ready_queue;
    ready_queue = ready_queue->next;
    next->next = NULL;
    
    return next;
}

// Start first process (simplified version)
// Start first process (simplified version) 
void start_multitasking(void) {
    if (!current_process) {
        uart_puts("No process to start!\n");
        return;
    }
    
    uart_puts("Starting first process: ");
    print_decimal(current_process->pid);
    uart_puts("\n");
    
    current_process->state = PROCESS_RUNNING;
    
    // Jump directly to the first process
    start_first_process(&current_process->context);
}
void process_yield(void) {
    if (!current_process) {
        return;
    }
    
    uart_puts("Process ");
    print_decimal(current_process->pid);
    uart_puts(" yielding CPU\n");
    
    // Mark current process as ready and reschedule
    current_process->state = PROCESS_READY;
    schedule_process(current_process);
    
    // Get next process
    process_t* old_process = current_process;
    current_process = get_next_process();
    
    if (current_process && current_process != old_process) {
        current_process->state = PROCESS_RUNNING;
        current_process->time_slice = TIME_SLICE_TICKS;
        
        uart_puts("Switching to process ");
        print_decimal(current_process->pid);
        uart_puts("\n");
        
        // Perform context switch
        switch_context(&old_process->context, &current_process->context);
    } else {
        // No other process available, continue with current
        current_process = old_process;
        current_process->state = PROCESS_RUNNING;
    }
}

// Simple scheduler (called from timer interrupt or manually)
void schedule(void) {
    scheduler_ticks++;
    
    // If no current process, try to get one from ready queue
    if (!current_process) {
        current_process = get_next_process();
        if (current_process) {
            current_process->state = PROCESS_RUNNING;
            current_process->time_slice = TIME_SLICE_TICKS;
        }
        return;
    }
    
    // Decrease time slice
    current_process->time_slice--;
    
    // If time slice expired or current process is terminated, switch processes
    if (current_process->time_slice <= 0 || current_process->state == PROCESS_TERMINATED) {
        process_t* old_process = current_process;
        
        // Move current process back to ready queue (if not terminated)
        if (current_process->state != PROCESS_TERMINATED) {
            current_process->state = PROCESS_READY;
            current_process->time_slice = TIME_SLICE_TICKS;
            schedule_process(current_process);
        }
        
        // Get next process
        current_process = get_next_process();
        if (current_process) {
            current_process->state = PROCESS_RUNNING;
            current_process->time_slice = TIME_SLICE_TICKS;
        }
        
        // Perform context switch if we have a new process
        // if (current_process && current_process != old_process) {
        //     do_context_switch();
        // }
    }
}

// Print process information
void print_processes(void) {
    uart_puts("\n=== Process List ===\n");
    
    process_t* proc = process_list;
    int count = 0;
    
    while (proc) {
        uart_puts("PID ");
        print_decimal(proc->pid);
        uart_puts(": ");
        uart_puts(proc->name);
        uart_puts(" - ");
        
        switch (proc->state) {
            case PROCESS_READY:
                uart_puts("READY");
                break;
            case PROCESS_RUNNING:
                uart_puts("RUNNING");
                break;
            case PROCESS_BLOCKED:
                uart_puts("BLOCKED");
                break;
            case PROCESS_TERMINATED:
                uart_puts("TERMINATED");
                break;
        }
        
        uart_puts("\n");
        proc = proc->next;
        count++;
    }
    
    uart_puts("Total processes: ");
    print_decimal(count);
    uart_puts("\n");
    uart_puts("Scheduler ticks: ");
    print_decimal(scheduler_ticks);
    uart_puts("\n==================\n\n");
}

// Forward declaration for cooperative yielding
void process_yield(void);

// Test process functions with cooperative yielding
void test_process_1(void) {
    static int counter = 0;
    while (1) {
        uart_puts("Process 1 running - count: ");
        print_decimal(counter++);
        uart_puts("\n");
        
        // Shorter busy wait for better responsiveness
        for (volatile int i = 0; i < 50000; i++);
        
        // Yield every few iterations to allow process switching
        if (counter % 3 == 0) {
            uart_puts("Process 1 yielding...\n");
            process_yield();
        }
    }
}

void test_process_2(void) {
    static int counter = 0;
    while (1) {
        uart_puts("Process 2 running - count: ");
        print_decimal(counter++);
        uart_puts("\n");
        
        // Different busy wait timing
        for (volatile int i = 0; i < 75000; i++);
        
        // Yield every few iterations
        if (counter % 4 == 0) {
            uart_puts("Process 2 yielding...\n");
            process_yield();
        }
    }
}

// Test process management with cooperative multitasking
// Test process management with cooperative multitasking
void test_processes(void) {
    uart_puts("Testing process creation...\n");
    
    // Create test processes
    process_t* proc1 = create_process("test_proc_1", test_process_1);
    process_t* proc2 = create_process("test_proc_2", test_process_2);
    
    if (proc1) {
        schedule_process(proc1);
    }
    
    if (proc2) {
        schedule_process(proc2);
    }
    
    print_processes();
    
    uart_puts("Starting cooperative multitasking...\n");
    
    // Get the first process and start it
    current_process = get_next_process();
    if (current_process) {
        start_multitasking();  // Changed from start_first_process()
    } else {
        uart_puts("No processes to run!\n");
    }
}