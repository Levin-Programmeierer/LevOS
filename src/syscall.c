#include "syscall.h"
#include "drivers/terminal.h"
#include "memory/paging.h"
#include "process/process.h"
#include "process/scheduler.h"

#define STDOUT_FD 1u
#define MAX_WRITE_SIZE 256u

static int syscall_write(uint32_t fd, uint32_t address, uint32_t length)
{
    struct process *current = process_current();
    if (current == 0 || !current->user || fd != STDOUT_FD ||
        length > MAX_WRITE_SIZE ||
        !user_range_valid((uint32_t *)(uintptr_t)current->directory,
                          address, length))
        return -1;

    const char *text = (const char *)(uintptr_t)address;
    for (uint32_t i = 0; i < length; ++i)
        putchar(text[i]);
    return (int)length;
}

struct cpu_context *syscall_dispatch(struct cpu_context *context)
{
    switch (context->eax) {
    case SYS_WRITE:
        context->eax = (uint32_t)syscall_write(context->ebx, context->ecx,
                                               context->edx);
        return context;
    case SYS_GETPID:
        context->eax = process_current() == 0 ? 0 : process_current()->pid;
        return context;
    case SYS_YIELD:
        context->eax = 0;
        return scheduler_yield(context);
    case SYS_EXIT:
        process_exit();
        return context;
    default:
        context->eax = (uint32_t)-1;
        return context;
    }
}
