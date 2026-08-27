#include "process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"

static struct process processes[MAX_PROCESSES];

static unsigned int next_pid = 1;


struct process *process_get(int index)
{
    if (index < 0 || index >= MAX_PROCESSES) {
        return 0;
    }

    return &processes[index];
}


unsigned int process_get_page_directory(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid == (unsigned int)pid) {
            return processes[i].page_directory;
        }
    }

    return 0;
}


void process_init(void)
{
    print("process_init A\n", WHITE);

    for (int i = 0; i < MAX_PROCESSES; i++) {

        processes[i].pid = 0;
        processes[i].state = PROCESS_UNUSED;

        processes[i].page_directory = 0;

        processes[i].entry_point = 0;
        processes[i].user_stack = 0;

        processes[i].esp = 0;
        processes[i].ebp = 0;

        processes[i].kernel_stack = 0;
        processes[i].kernel_frame = 0;

        processes[i].context.edi = 0;
        processes[i].context.esi = 0;
        processes[i].context.ebp = 0;
        processes[i].context.esp = 0;
        processes[i].context.ebx = 0;
        processes[i].context.edx = 0;
        processes[i].context.ecx = 0;
        processes[i].context.eax = 0;

        processes[i].context.irq = 0;
        processes[i].context.error = 0;

        processes[i].context.eip = 0;
        processes[i].context.cs = 0;
        processes[i].context.eflags = 0;

        processes[i].context.user_esp = 0;
        processes[i].context.user_ss = 0;
    }

    next_pid = 1;

    print("process_init done\n", WHITE);
}


/*
 * Create a user process.
 *
 * User code:
 *     0x00400000
 *
 * User stack:
 *     0x00800000 - 0x00800FFF
 *
 * Kernel stack:
 *     physical page allocated by PMM
 */
int process_create(unsigned char *program, unsigned int size)
{
    int slot = -1;


    /*
     * Find free process slot first.
     */
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].state == PROCESS_UNUSED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {

        print(
            "ERROR: process table full\n",
            WHITE
        );

        return -1;
    }


    /*
     * Create address space.
     */
    unsigned int *directory =
        create_page_directory();

    if (directory == 0) {

        print(
            "ERROR: could not create page directory\n",
            WHITE
        );

        return -1;
    }


    /*
     * Keep scheduler scratch page available
     * in this address space.
     */
    extern unsigned int scheduler_scratch_phys;
    extern unsigned int scheduler_scratch_vaddr;

    if (scheduler_scratch_phys != 0) {

        map_page_in(
            directory,
            scheduler_scratch_vaddr,
            scheduler_scratch_phys,
            'K'
        );
    }


    /*
     * Allocate user code pages.
     */
    unsigned int code_pages =
        (size + 4095) / 4096;

    print("Code pages: ", WHITE);
    print_hex_dword(code_pages);
    putchar('\n');


    for (unsigned int i = 0; i < code_pages; i++) {

        unsigned int physical =
            allocate_page();

        unsigned int virtual_address =
            0x00400000 + (i * 4096);

        map_page_in(
            directory,
            virtual_address,
            physical,
            'U'
        );
    }


    /*
     * Allocate user stack.
     *
     * Virtual:
     *
     *     0x00800000
     *     0x00801000 = top of stack
     */
    unsigned int stack_page =
        allocate_page();

    map_page_in(
        directory,
        0x00800000,
        stack_page,
        'U'
    );


    /*
     * Allocate kernel stack.
     */
    unsigned int kernel_stack_page =
        allocate_page();

    unsigned int kernel_stack_top =
        kernel_stack_page + 4096;

    processes[slot].kernel_stack =
        kernel_stack_top;


    /*
     * Identity map kernel stack into this address space.
     *
     * This is temporary/simple and works with your current
     * kernel identity mapping.
     */
    map_page_in(
        directory,
        kernel_stack_page,
        kernel_stack_page,
        'K'
    );


    /*
     * Initial user context.
     *
     * IMPORTANT:
     *
     * This is NOT the actual stack layout yet.
     * It is the logical CPU context that the scheduler
     * will eventually copy to its scratch frame.
     */
    processes[slot].context.edi = 0;
    processes[slot].context.esi = 0;
    processes[slot].context.ebp = 0;
    processes[slot].context.esp = kernel_stack_top;

    processes[slot].context.ebx = 0;
    processes[slot].context.edx = 0;
    processes[slot].context.ecx = 0;
    processes[slot].context.eax = 0;

    processes[slot].context.irq = 0;
    processes[slot].context.error = 0;

    processes[slot].context.eip =
        0x00400000;

    processes[slot].context.cs =
        0x1B;

    processes[slot].context.eflags =
        0x202;

    processes[slot].context.user_esp =
        0x00801000;

    processes[slot].context.user_ss =
        0x23;


    /*
     * Save process metadata.
     */
    processes[slot].pid =
        next_pid++;

    processes[slot].state =
        PROCESS_READY;

    processes[slot].page_directory =
        (unsigned int)directory;

    processes[slot].entry_point =
        0x00400000;

    processes[slot].user_stack =
        0x00801000;

    processes[slot].esp =
        0x00801000;

    processes[slot].ebp =
        0x00801000;


    /*
     * Load the new address space temporarily so
     * 0x00400000 refers to the process's code pages.
     */
    load_page_directory(directory);


    unsigned char *destination =
        (unsigned char *)0x00400000;


    for (unsigned int i = 0; i < size; i++) {
        destination[i] = program[i];
    }


    /*
     * Return to kernel address space.
     */
    reload_page_directory();


    print("Created process PID ", WHITE);
    print_hex_dword(processes[slot].pid);
    putchar('\n');


    return processes[slot].pid;
}

static void enter_user_process(
    unsigned int entry_point,
    unsigned int user_stack
)
{
    __asm__ volatile (
        "cli\n"

        /* User data segments */
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        /* SS */
        "push $0x23\n"

        /* ESP */
        "push %0\n"

        /* EFLAGS */
        "pushf\n"
        "pop %%eax\n"
        "or $0x200, %%eax\n"
        "push %%eax\n"

        /* CS */
        "push $0x1B\n"

        /* EIP */
        "push %1\n"

        /* Ring 3 */
        "iret\n"

        :
        : "r"(user_stack),
          "r"(entry_point)
        : "eax"
    );
}
/*
 * Run a process directly.
 *
 * This is the important part for the current test.
 */
void process_run(int pid)
{
    struct process *process = 0;


    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid == (unsigned int)pid &&
            processes[i].state == PROCESS_READY) {

            process = &processes[i];
            break;
        }
    }


    if (process == 0) {

        print(
            "ERROR: process not found\n",
            WHITE
        );

        return;
    }


    print("Running PID ", WHITE);
    print_hex_dword(process->pid);
    putchar('\n');


    process->state =
        PROCESS_RUNNING;


    /*
     * CRITICAL:
     *
     * Load THIS process's page directory.
     *
     * Do NOT call reload_page_directory() here,
     * because that loads the kernel directory.
     */
    load_page_directory(
        (unsigned int *)process->page_directory
    );


    /*
     * Enter ring 3.
     */
    enter_user_process(
        process->entry_point,
        process->user_stack
    );


    /*
     * We should never return here normally.
     */
    process->state =
        PROCESS_DEAD;
}


/*
 * Save a context captured by the IRQ.
 */
void process_save_context(
    int pid,
    struct cpu_context *context
)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid ==
            (unsigned int)pid) {

            processes[i].context =
                *context;

            return;
        }
    }
}


/*
 * IMPORTANT:
 *
 * Always return process.context.
 *
 * Do NOT return kernel_frame.
 */
struct cpu_context *process_get_context(int pid)
{
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid ==
            (unsigned int)pid) {

            return &processes[i].context;
        }
    }

    return 0;
}

void process_exit(void)
{
    print("\nProcess exited.\n", WHITE);

    while (1) {
        __asm__ volatile ("hlt");
    }
}