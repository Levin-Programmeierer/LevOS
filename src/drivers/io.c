#include "io.h"

unsigned char inb(unsigned short port){
    unsigned char value;

    __asm__ volatile (
        "inb %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

void outb(unsigned short port, unsigned char value){
    __asm__ volatile (
        "outb %1, %0"
        :
        : "Nd"(port), "a"(value)
    );
}

void picremap(void){
    outb(0x20, 0x11); // set master command to ICW1
    outb(0xA0, 0x11); // set slave command to ICW1 aka ICW 4 will follow
    outb(0x21, 0x20); // set master data port to 0x20
    outb(0xA1, 0x28); // set slave data port to 0x28
    outb(0x21, 0x04); // tell IRQ2 is connected  to slave
    outb(0xA1, 0x02); // tell slave it is connected to IRQ2
    outb(0x21, 0x01); // tell master to operate in 8086/88 mode
    outb(0xA1, 0x01); // tell slave to operate in 8086/88 mode

    outb(0x21, 0xFF); // set pic data ports to 1111111 to block all ports
    outb(0xA1, 0xFF); // set pic data ports to 1111111 to block all ports
}