#include "process/scheduler.h"
#include "process/process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "drivers/terminal.h"
#include <stdint.h>

uint32_t scheduler_scratch_phys = 0;
uint32_t scheduler_scratch_vaddr = 0x00300000;

uint32_t scheduler_next_directory = 0;

static int current_index = -1;
static unsigned int scheduler_ticks = 0;
static int current_pid = -1;


int scheduler_current(void)
{
    return current_index;
}


int scheduler_next(void)
{
    return -1;
}


void scheduler_init(void)
{
    current_index = -1;
    current_pid = -1;
    scheduler_ticks = 0;

    scheduler_scratch_phys =
        allocate_page();

    map_page_in(
        page_directory,
        scheduler_scratch_vaddr,
        scheduler_scratch_phys,
        'K'
    );

    print(
        "Scheduler initialized\n",
        WHITE
    );
}


struct cpu_context *scheduler_tick(
    struct cpu_context *context
)
{
    scheduler_ticks++;

    /*
     * DEBUG:
     *
     * Don't switch anything yet.
     */
    if ((scheduler_ticks % 10) == 0) {

        print("TICK 10\n", WHITE);
    }

    return context;
}