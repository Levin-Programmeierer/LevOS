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

void picremap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

unsigned char pic_get_irr(void) {
    outb(0x20, 0x0A);
    return inb(0x20);
}

unsigned char pic_get_isr(void)
{
    outb(0x20, 0x0B);
    return inb(0x20);
}