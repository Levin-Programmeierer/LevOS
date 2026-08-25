#include "process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "drivers/terminal.h"

static struct process processes[MAX_PROCESSES];

static unsigned int next_pid = 1;


// chatgpt copied function cuz assembly
static void enter_user_process(unsigned int entry_point, unsigned int user_stack) {
    __asm__ volatile (
        "cli\n"

        /* User data segments */
        "mov $0x23, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "push $0x23\n"

        "push %0\n"

        "pushf\n"

        "pop %%eax\n"
        "or $0x200, %%eax\n"
        "push %%eax\n"

        "push $0x1B\n"

        "push %1\n"

        "iret\n"

        :
        : "r"(user_stack),
          "r"(entry_point)
        : "eax"
    );
}


void process_init(void) {
    print("process_init A\n");

    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_UNUSED;
        processes[i].page_directory = 0;
        processes[i].entry_point = 0;
        processes[i].user_stack = 0;
        processes[i].esp = 0;
        processes[i].ebp = 0;
    }

    next_pid = 1;

    print("process_init done\n");
}

int process_create(unsigned char *program, unsigned int size) {
    int slot = -1;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_UNUSED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        print("ERROR: process table full\n");
        return -1;
    }

    unsigned int code_pages =
        (size + 4095) / 4096;

    print("Code pages: ");
    print_hex_dword(code_pages);
    print("\n");

    for (unsigned int i = 0; i < code_pages; i++) {

        unsigned int code_page =
            allocate_page();

        unsigned int virtual_address =
            0x00400000 + (i * 4096);

        map_page(
            virtual_address,
            code_page,
            'U'
        );
    }

    unsigned int stack_page =
        allocate_page();

    map_page(
        0x00800000,
        stack_page,
        'U'
    );

    reload_page_directory();

    unsigned char *destination =
        (unsigned char *)0x00400000;

    for (unsigned int i = 0; i < size; i++) {
        destination[i] = program[i];
    }

    processes[slot].pid =
        next_pid++;

    processes[slot].state =
        PROCESS_READY;

    processes[slot].page_directory =
        (unsigned int)page_directory;

    processes[slot].entry_point =
        0x00400000;

    processes[slot].user_stack =
        0x00801000;

    processes[slot].esp =
        0x00801000;

    processes[slot].ebp =
        0x00801000;

    print("Created process PID ");

    print_hex_dword(
        processes[slot].pid
    );

    print("\n");

    return processes[slot].pid;
}


void process_run(int pid) {
    struct process *process = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == (unsigned int)pid &&
            processes[i].state == PROCESS_READY) {

            process = &processes[i];
            break;
        }
    }

    if (process == 0) {
        print("ERROR: process not found\n");
        return;
    }

    print("Running PID ");

    print_hex_dword(
        process->pid
    );

    print("\n");

    process->state = PROCESS_RUNNING;

    reload_page_directory();

    enter_user_process(
        process->entry_point,
        process->user_stack
    );

    process->state = PROCESS_DEAD;
}


void process_exit(void)
{
    print("\nProcess exited.\n");

    while (1) {
        __asm__ volatile ("hlt");
    }
}