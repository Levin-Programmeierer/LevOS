#include "process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "drivers/terminal.h"
#include "drivers/keyboard.h"

static struct process processes[MAX_PROCESSES];

static unsigned int next_pid = 1;


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

struct process *process_get(int index) {
    if (index < 0 || index >= MAX_PROCESSES) {
        return 0;
    }

    return &processes[index];
}

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
    print("process_init A\n", WHITE);

    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_UNUSED;
        processes[i].page_directory = 0;
        processes[i].entry_point = 0;
        processes[i].user_stack = 0;
        processes[i].esp = 0;
        processes[i].ebp = 0;
        processes[i].context.edi = 0;
        processes[i].context.esi = 0;
        processes[i].context.ebp = 0;
        processes[i].context.esp = 0;
        processes[i].context.ebx = 0;
        processes[i].context.edx = 0;
        processes[i].context.ecx = 0;
        processes[i].context.eax = 0;

        processes[i].kernel_stack = 0;

        processes[i].context.irq = 0;
        processes[i].context.error = 0;

        processes[i].context.eip = processes[i].entry_point;

        processes[i].context.cs = 0x1B;

        processes[i].context.eflags = 0x202;

        processes[i].context.user_ss = 0x23;
        processes[i].context.user_esp = processes[i].user_stack;    }

    next_pid = 1;

    print("process_init done\n", WHITE);
}

unsigned int process_get_page_directory(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == (unsigned int)pid) {
            return processes[i].page_directory;
        }
    }

    return 0;
}



int process_create(unsigned char *program, unsigned int size) {
    int slot = -1;

    unsigned int *directory = create_page_directory();

    if (directory == 0) {
        print("ERROR: could not create page directory\n", WHITE);
        return -1;
    }

    /* Ensure the scheduler scratch page is mapped into this new directory so the
       kernel can copy the context there safely before switching CR3. */
    extern unsigned int scheduler_scratch_phys;
    extern unsigned int scheduler_scratch_vaddr;
    if (scheduler_scratch_phys != 0) {
        map_page_in(directory, scheduler_scratch_vaddr, scheduler_scratch_phys, 'K');
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROCESS_UNUSED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        print("ERROR: process table full\n", WHITE);
        return -1;
    }

    unsigned int code_pages =
        (size + 4095) / 4096;

    print("Code pages: ", WHITE);
    print_hex_dword(code_pages);
    print("\n", WHITE);

    for (unsigned int i = 0; i < code_pages; i++) {

        unsigned int code_page =
            allocate_page();

        unsigned int virtual_address =
            0x00400000 + (i * 4096);

        map_page_in(
            directory,
            virtual_address,
            code_page,
            'U'
        );
    }

    unsigned int stack_page = allocate_page();

    map_page_in(
        directory,
        0x00800000,
        stack_page,
        'U'
    );

    unsigned int kernel_stack_page = allocate_page();

    processes[slot].kernel_stack = kernel_stack_page + 4096;

    /* Map the kernel stack page into the new page directory at the same
       virtual address so it remains accessible after switching CR3. */
    map_page_in(
        directory,
        kernel_stack_page,
        kernel_stack_page,
        'K'
    );

    /* Build an initial kernel stack frame for this process in the kernel stack page.
       Place the struct cpu_context at the top of the stack so the IRQ stub can
       movl %eax, %esp to that address and then popa/iret will restore registers and
       return to user EIP. */
    struct cpu_context *frame = (struct cpu_context *)(kernel_stack_page + 4096 - sizeof(struct cpu_context));

    frame->edi = 0;
    frame->esi = 0;
    frame->ebp = processes[slot].kernel_stack;
    frame->esp = processes[slot].kernel_stack;
    frame->ebx = 0;
    frame->edx = 0;
    frame->ecx = 0;
    frame->eax = 0;

    frame->irq = 0;
    frame->error = 0;

    frame->eip = 0x00400000; /* entry point */
    frame->cs = 0x1B;
    frame->eflags = 0x202;
    frame->user_ss = 0x23;
    frame->user_esp = 0x00801000;

    processes[slot].kernel_frame = frame;

    /* initialize the saved kernel context fields for bookkeeping too */
    processes[slot].context = *frame;

    load_page_directory(directory);

    unsigned char *destination =
        (unsigned char *)0x00400000;

    for (unsigned int i = 0; i < size; i++) {
        destination[i] = program[i];
    }

    /* Return to kernel address space */
    reload_page_directory();

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

    processes[slot].context.edi = 0;
    processes[slot].context.esi = 0;
    processes[slot].context.ebp = 0;
    processes[slot].context.esp = 0;
    processes[slot].context.ebx = 0;
    processes[slot].context.edx = 0;
    processes[slot].context.ecx = 0;
    processes[slot].context.eax = 0;

    processes[slot].context.irq = 0;
    processes[slot].context.error = 0;

    processes[slot].context.eip =
        processes[slot].entry_point;

    processes[slot].context.cs = 0x1B;

    processes[slot].context.eflags = 0x202;

    processes[slot].context.user_ss = 0x23;

    processes[slot].context.user_esp = processes[slot].user_stack;

    processes[slot].esp =
        0x00801000;

    processes[slot].ebp =
        0x00801000;

    print("Created process PID ", WHITE);

    print_hex_dword(
        processes[slot].pid
    );

    print("\n", WHITE);

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
        print("ERROR: process not found\n", WHITE);
        return;
    }

    print("Running PID ", WHITE);

    print_hex_dword(
        process->pid
    );

    print("\n", WHITE);

    process->state = PROCESS_RUNNING;

    reload_page_directory();

    enter_user_process(
        process->entry_point,
        process->user_stack
    );

    process->state = PROCESS_DEAD;
}

int process_next_ready(int current_pid) {
    int start = 0;

    if (current_pid >= 0) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (processes[i].pid == (unsigned int)current_pid) {
                start = i + 1;
                break;
            }
        }
    }

    for (int offset = 0; offset < MAX_PROCESSES; offset++) {

        int index =
            (start + offset) % MAX_PROCESSES;

        if (processes[index].state == PROCESS_READY ||
            processes[index].state == PROCESS_RUNNING) {

            return processes[index].pid;
        }
    }

    return -1;
}

void process_exit(void) {
    print("\nProcess exited.\n", WHITE);

    while (1) {
        __asm__ volatile ("hlt");
    }
}

void process_save_context(int pid, struct cpu_context *context) {
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid == (unsigned int)pid) {

            processes[i].context = *context;

            return;
        }
    }
}

struct cpu_context *process_get_context(int pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {

        if (processes[i].pid == (unsigned int)pid) {
            if (processes[i].kernel_frame)
                return processes[i].kernel_frame;
            return &processes[i].context;
        }
    }

    return 0;
}