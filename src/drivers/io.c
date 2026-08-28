#include "io.h"
// Defines from OSDEV wiki
#define PIC1		0x20
#define PIC2		0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)


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

unsigned int inl(unsigned short port) {
    unsigned int value;

    __asm__ volatile (
        "inl %1, %0"
        : "=a"(value)
        : "Nd"(port)
    );

    return value;
}

void outl(unsigned short port, unsigned int value) {
    __asm__ volatile (
        "outl %1, %0"
        :
        : "Nd"(port), "a"(value)
    );
}

void picremap(void) {
    // goal: remap pic to 32-47 so in hex 0x20 to 0x2F 
    outb(PIC1_COMMAND, 0x11); // initialisation sequence
    outb(PIC2_COMMAND, 0x11); // initialisation sequence

    outb(PIC1_DATA, 0x20); // set PIC1 start at 0x20 so it gets 32-39
    outb(PIC2_DATA, 0x28); // set PIC2 start at 0x28 so it gets 40-47

    outb(PIC1_DATA, 0x04); // tell that g that he got slave at 4
    outb(PIC2_DATA, 0x02); // tell that g he is a slave and his master is at 2

    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01); // tell these bros to run at 8086 mode

    outb(PIC1_DATA, 0xFD); // tell these gangsters to take those masks off
    outb(PIC2_DATA, 0xFF);
}

void end_interrupt(unsigned int irq){
    if(irq >= 8) // set PIC 2 here
        outb(PIC2_COMMAND, 0x20);
    
    outb(PIC1_COMMAND, 0x20);
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