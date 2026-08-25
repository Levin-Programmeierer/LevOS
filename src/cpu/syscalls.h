#ifndef SYSCALLS_H
#define SYSCALLS_H

#define SYS_WRITE 1
#define SYS_EXIT  2

void syscall_handler(
    unsigned int number,
    unsigned int arg1,
    unsigned int arg2
);

#endif