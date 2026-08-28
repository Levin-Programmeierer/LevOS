#ifndef SYSCALL_H
#define SYSCALL_H

#include "process/cpu_context.h"

#define SYS_WRITE 1u
#define SYS_GETPID 2u
#define SYS_YIELD 3u
#define SYS_EXIT 4u

struct cpu_context *syscall_dispatch(struct cpu_context *context);

#endif
