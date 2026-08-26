#include "process/scheduler.h"
#include "process/process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "drivers/terminal.h"

unsigned int scheduler_scratch_phys = 0;
unsigned int scheduler_scratch_vaddr = 0x00300000; /* 3 MiB: inside low 4 MiB identity map */
unsigned int scheduler_next_directory = 0;

/* Runtime-debug toggle: printing inside IRQ handlers is dangerous (can
   re-enter emulator I/O locks). Keep disabled by default. */
int scheduler_diag_enabled = 0;

static int current_index = -1;

extern unsigned char LIGHT_BLUE;
extern unsigned char LIGHT_CYAN;
extern unsigned char LIGHT_GREEN;
extern unsigned char LIGHT_RED;
extern unsigned char LIGHT_PURPLE;
extern unsigned char WHITE;
extern unsigned char RED;
extern unsigned char BLUE;
extern unsigned char GREEN;
extern unsigned char CYAN;
extern unsigned char BROWN;
extern unsigned char PURPLE;
extern unsigned char YELLOW;
extern unsigned char DARK_GRAY;
extern unsigned char GRAY;

static unsigned int scheduler_ticks = 0;
static int current_pid = -1;

int scheduler_current(void){
    return current_index;
}

int scheduler_next(void){
    if (current_index < 0) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            struct process *process = process_get(i);
            if (process != 0 &&
                (process->state == PROCESS_READY ||
                 process->state == PROCESS_RUNNING)) {
                process->state = PROCESS_RUNNING;
                current_index = i;
                current_pid = (int)process->pid;
                return process->pid;
            }
        }

        return -1;
    }

    for (int i = 1; i <= MAX_PROCESSES; i++) {
        int index = (current_index + i) % MAX_PROCESSES;
        struct process *process = process_get(index);

        if (process == 0) {
            continue;
        }

        if (process->state == PROCESS_READY ||
            process->state == PROCESS_RUNNING) {

            struct process *current = process_get(current_index);
            if (current != 0 && current->state == PROCESS_RUNNING) {
                current->state = PROCESS_READY;
            }

            process->state = PROCESS_RUNNING;
            current_index = index;
            current_pid = (int)process->pid;
            return process->pid;
        }
    }

    return -1;
}

void scheduler_init(void){
    current_index = -1;
    current_pid = -1;
    scheduler_ticks = 0;

    /* Allocate a scratch page (identity-mapped area) for safe context switching. */
    scheduler_scratch_phys = allocate_page();

    /* Map it into the kernel page directory at scheduler_scratch_vaddr so the kernel
       can write into it before switching CR3. */
    map_page_in(page_directory, scheduler_scratch_vaddr, scheduler_scratch_phys, 'K');

    print("Scheduler initialized\n", WHITE);
}

struct cpu_context *scheduler_tick(
    struct cpu_context *context
)
{
    scheduler_ticks++;

    if ((scheduler_ticks % 10) != 0) {
        return context;
    }

    if (current_index >= 0) {
        struct process *current = process_get(current_index);
        if (current != 0 && current->state == PROCESS_RUNNING) {
            process_save_context(current->pid, context);
            current->state = PROCESS_READY;
        }
    }

    int next_pid = scheduler_next();
    if (next_pid < 0) {
        return context;
    }

    unsigned int directory = process_get_page_directory(next_pid);
    if (directory != 0) {
        /* Copy the target context into the global scratch page so it's available
           under both the current and next page directories. */
        extern unsigned int scheduler_scratch_vaddr;
        struct cpu_context *next_context = process_get_context(next_pid);
        if (next_context == 0) {
            return context;
        }

        struct cpu_context *scratch = (struct cpu_context *)scheduler_scratch_vaddr;

        /* Diagnostics are expensive inside interrupts and can re-enter host I/O
           causing emulator assertions. Only enable when debugging interactively. */
        extern int scheduler_diag_enabled;
        if (scheduler_diag_enabled) {
            print("Diag: scratch_vaddr: ", YELLOW); print_hex_dword(scheduler_scratch_vaddr); putchar('\n');
            print("Diag: scratch_phys: ", YELLOW); print_hex_dword(scheduler_scratch_phys); putchar('\n');
            print("Diag: next_context ptr: ", YELLOW); print_hex_dword((unsigned int)next_context); putchar('\n');

            print("Diag: next_context first dwords:\n", YELLOW);
            unsigned int *nc = (unsigned int *)next_context;
            for (int i=0;i<8;i++) { print_hex_dword(nc[i]); putchar(' '); }
            putchar('\n');
        }

        /* copy into scratch while current page directory (kernel) is active */
        unsigned int sz = sizeof(struct cpu_context);
        unsigned char *src = (unsigned char *)next_context;
        unsigned char *dst = (unsigned char *)scratch;
        for (unsigned int i = 0; i < sz; i++) dst[i] = src[i];

        /* Diagnostics: avoid printing here unless enabled */
        extern int scheduler_diag_enabled;
        if (scheduler_diag_enabled) {
            print("Diag: scratch first dwords after copy:\n", YELLOW);
            unsigned int *sc = (unsigned int *)scratch;
            for (int i=0;i<12;i++) { print_hex_dword(sc[i]); putchar(' '); }
            putchar('\n');

            print("Diag: next page directory addr: ", YELLOW); print_hex_dword(directory); putchar('\n');

            print("Switching to PID ", LIGHT_GREEN);
            print_hex_dword((unsigned int)next_pid);
            putchar('\n');
        }

        /* Make next page directory visible to the IRQ assembly so it can load CR3.
           We do NOT load CR3 here — the assembly stub will do it right after it sets ESP. */
        extern unsigned int scheduler_next_directory;
        scheduler_next_directory = directory;

        /* Return the scratch pointer (still valid because every process' page directory
           maps the same scratch physical page at the same virtual address). */
        return scratch;
    }

    return context;
}
