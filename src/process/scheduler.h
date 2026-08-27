#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process/cpu_context.h"

#include <stdint.h>

void scheduler_init(void);

struct cpu_context *scheduler_tick(
    struct cpu_context *context
);

/* Scratch page used for safe context switching: virtual address visible
   under each process' page directory. */
extern unsigned int scheduler_scratch_phys;
extern unsigned int scheduler_scratch_vaddr;

/* Directory that the IRQ assembly should load into CR3 when performing the
   context switch. */
extern uint32_t scheduler_next_directory;

/* Runtime toggles */
extern int scheduler_diag_enabled;

/* Deferred scheduling flag (set by timer IRQ) */
extern int scheduler_pending;

/* Called from non-IRQ context to run the scheduler and perform the context switch
   (this may not return — it can iret directly into the next user task). */
void scheduler_do_pending(void);

#endif