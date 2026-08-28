#include "process/scheduler.h"
#include "process/process.h"

extern uint32_t process_count(void);
extern struct process *process_next_round_robin(void);
extern void process_save_context(struct cpu_context *context);

volatile uint32_t scheduler_next_directory;
static uint32_t quantum_ticks;

void scheduler_init(void)
{
    process_system_init();
    scheduler_next_directory = 0;
    quantum_ticks = 0;
}

static struct cpu_context *schedule(struct cpu_context *context)
{
    scheduler_next_directory = 0;
    if (process_count() < 2 &&
        process_current()->state != PROCESS_ZOMBIE)
        return context;

    process_save_context(context);
    struct process *next = process_next_round_robin();
    if (next == 0 || next->context == 0)
        return context;

    if (next->context != context) {
        scheduler_next_directory = next->directory;
        return next->context;
    }
    return context;
}

struct cpu_context *scheduler_tick(struct cpu_context *context)
{
    ++quantum_ticks;
    return schedule(context);
}

struct cpu_context *scheduler_yield(struct cpu_context *context)
{
    return schedule(context);
}
