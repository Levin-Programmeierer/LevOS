#include "process/process.h"
#include "memory/paging.h"
#include "memory/pmm.h"
#include "memory/gdt.h"
#include "process/scheduler.h"

#define MAX_PROCESSES 16
#define KERNEL_CS 0x08
#define USER_CS 0x1b
#define USER_DS 0x23
#define IF_FLAG 0x202

static struct process processes[MAX_PROCESSES];
static uint32_t next_pid = 1;
static struct process *running;
#define BOOTSTRAP_STACK_SIZE 4096u
static uint8_t bootstrap_stack[BOOTSTRAP_STACK_SIZE] __attribute__((aligned(16)));
static struct cpu_context *bootstrap_context;

static void process_bootstrap(void)
{
    struct process *process = running;
    if (process != 0 && process->entry != 0)
        process->entry(process->argument);
    process_exit();
}

void process_system_init(void)
{
    bootstrap_context =
        (struct cpu_context *)(uintptr_t)(bootstrap_stack +
                                          BOOTSTRAP_STACK_SIZE -
                                          sizeof(struct cpu_context));
    for (uint32_t i = 0; i < sizeof(struct cpu_context) / 4; ++i)
        ((uint32_t *)bootstrap_context)[i] = 0;

    for (uint32_t i = 0; i < MAX_PROCESSES; ++i) {
        processes[i].pid = 0;
        processes[i].state = PROCESS_UNUSED;
        processes[i].context = 0;
        processes[i].stack_base = 0;
        processes[i].directory = 0;
        processes[i].entry = 0;
        processes[i].argument = 0;
        processes[i].bootstrap = 0;
        processes[i].user = 0;
        processes[i].user_code_base = 0;
        processes[i].user_stack_base = 0;
    }

    running = &processes[0];
    running->pid = 0;
    running->state = PROCESS_RUNNING;
    running->directory = (uint32_t)page_directory;
    running->context = bootstrap_context;
    running->bootstrap = 1;
    tss_set_kernel_stack(process_kernel_stack_top(running));
}

int process_create(process_entry_t entry, void *argument)
{
    if (entry == 0)
        return -1;

    for (uint32_t i = 1; i < MAX_PROCESSES; ++i) {
        if (processes[i].state != PROCESS_UNUSED &&
            processes[i].state != PROCESS_ZOMBIE)
            continue;

        uint32_t stack = allocate_page();
        uint32_t *directory = create_page_directory();
        if (stack == 0 || directory == 0) {
            if (stack != 0)
                free_page(stack);
            return -1;
        }

        struct process *process = &processes[i];
        process->pid = next_pid++;
        process->state = PROCESS_READY;
        process->directory = (uint32_t)directory;
        process->stack_base = stack;
        process->entry = entry;
        process->argument = argument;
        process->bootstrap = 0;
        process->user = 0;
        process->user_code_base = 0;
        process->user_stack_base = 0;

        /* Keep the initial frame in the stack page.  The common iretd path
           consumes the frame and starts the process on its kernel stack. */
        process->context =
            (struct cpu_context *)(uintptr_t)(stack + 4096 -
                                               sizeof(struct cpu_context));
        for (uint32_t j = 0; j < sizeof(struct cpu_context) / 4; ++j)
            ((uint32_t *)process->context)[j] = 0;
        process->context->eip = (uint32_t)process_bootstrap;
        process->context->cs = KERNEL_CS;
        process->context->eflags = IF_FLAG;
        /* The interrupt stubs consume 20 bytes before pusha. */
        process->context->esp = stack + 4096 - 20;
        process->context->user_esp = stack + 4096;
        process->context->user_ss = 0x10;
        return (int)process->pid;
    }
    return -1;
}

int process_create_user(const uint8_t *image, uint32_t image_size)
{
    if (image == 0 || image_size == 0 || image_size > 4096)
        return -1;

    for (uint32_t i = 1; i < MAX_PROCESSES; ++i) {
        if (processes[i].state != PROCESS_UNUSED &&
            processes[i].state != PROCESS_ZOMBIE)
            continue;

        uint32_t kernel_stack = allocate_page();
        uint32_t code = allocate_page();
        uint32_t user_stack = allocate_page();
        uint32_t *directory = create_page_directory();
        if (kernel_stack == 0 || code == 0 || user_stack == 0 ||
            directory == 0) {
            if (kernel_stack != 0)
                free_page(kernel_stack);
            if (code != 0)
                free_page(code);
            if (user_stack != 0)
                free_page(user_stack);
            return -1;
        }

        map_page_in(directory, USER_CODE_ADDRESS, code, 'U');
        map_page_in(directory, USER_STACK_TOP - 4096, user_stack, 'U');
        for (uint32_t j = 0; j < image_size; ++j)
            ((uint8_t *)(uintptr_t)code)[j] = image[j];

        struct process *process = &processes[i];
        process->pid = next_pid++;
        process->state = PROCESS_READY;
        process->directory = (uint32_t)directory;
        process->stack_base = kernel_stack;
        process->entry = 0;
        process->argument = 0;
        process->bootstrap = 0;
        process->user = 1;
        process->user_code_base = code;
        process->user_stack_base = user_stack;
        process->context =
            (struct cpu_context *)(uintptr_t)(kernel_stack + 4096 -
                                               sizeof(struct cpu_context));
        for (uint32_t j = 0; j < sizeof(struct cpu_context) / 4; ++j)
            ((uint32_t *)process->context)[j] = 0;
        process->context->eip = USER_CODE_ADDRESS;
        process->context->cs = USER_CS;
        process->context->eflags = IF_FLAG;
        process->context->user_esp = USER_STACK_TOP - 16;
        process->context->user_ss = USER_DS;
        return (int)process->pid;
    }
    return -1;
}

void process_exit(void)
{
    if (running != 0 && !running->bootstrap)
        running->state = PROCESS_ZOMBIE;

    /*
     * process_exit can be reached from a process entry point, where there is
     * no interrupt context to return through.  Enter the scheduler using the
     * same frame format as a timer interrupt.  A hlt loop is only a fallback
     * when there are no runnable processes.
     */
    __asm__ volatile ("int $0x20" : : : "memory");
    for (;;)
        __asm__ volatile ("hlt");
}

void process_terminate_current(void)
{
    if (running != 0 && !running->bootstrap)
        running->state = PROCESS_ZOMBIE;
}

struct process *process_current(void)
{
    return running;
}

struct process *process_by_pid(uint32_t pid)
{
    for (uint32_t i = 0; i < MAX_PROCESSES; ++i)
        if (processes[i].state != PROCESS_UNUSED && processes[i].pid == pid)
            return &processes[i];
    return 0;
}

/* Scheduler-private accessors keep process bookkeeping in this module. */
uint32_t process_count(void)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; ++i)
        if (processes[i].state == PROCESS_READY ||
            processes[i].state == PROCESS_RUNNING)
            ++count;
    return count;
}

struct process *process_next_round_robin(void)
{
    uint32_t current_index = 0;
    for (uint32_t i = 0; i < MAX_PROCESSES; ++i)
        if (&processes[i] == running)
            current_index = i;

    for (uint32_t step = 1; step <= MAX_PROCESSES; ++step) {
        uint32_t index = (current_index + step) % MAX_PROCESSES;
        if (processes[index].state == PROCESS_READY ||
            processes[index].state == PROCESS_RUNNING) {
            running = &processes[index];
            running->state = PROCESS_RUNNING;
            tss_set_kernel_stack(process_kernel_stack_top(running));
            return running;
        }
    }

    return running;
}

void process_save_context(struct cpu_context *context)
{
    if (running == 0)
        return;
    if (running->bootstrap) {
        /*
         * The bootstrap process uses the TSS stack for user-to-kernel
         * transitions.  Keep its saved frame out of that reusable stack.
         */
        *bootstrap_context = *context;
        running->context = bootstrap_context;
        return;
    }
    if (running->context == context)
        return;
    *running->context = *context;
}

uint32_t process_kernel_stack_top(const struct process *process)
{
    if (process == 0)
        return 0;
    if (process->bootstrap)
        return (uint32_t)(bootstrap_stack + sizeof(bootstrap_stack));
    return process->stack_base + 4096;
}
