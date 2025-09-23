// Software Timer System for Process Scheduling
// Save as: ~/OS_proj/src/timer.c

#include <stdint.h>

// External functions
extern void uart_puts(const char* str);
extern void schedule(void);

// Timer state
static volatile uint64_t timer_counter = 0;
static volatile uint64_t schedule_interval = 1000000; // Process switch interval

// Initialize software timer
void init_software_timer(void) {
    uart_puts("Initializing software timer for preemptive scheduling...\n");
    timer_counter = 0;
}

// Software timer tick - call this from a loop
void software_timer_tick(void) {
    timer_counter++;
    
    // Check if it's time to schedule
    if (timer_counter >= schedule_interval) {
        timer_counter = 0;
        
        // Call scheduler for process switching
        uart_puts("[TIMER] Process switch time\n");
        schedule();
    }
}

// Get current timer value
uint64_t get_timer_ticks(void) {
    return timer_counter;
}

// Set scheduling interval
void set_schedule_interval(uint64_t interval) {
    schedule_interval = interval;
}