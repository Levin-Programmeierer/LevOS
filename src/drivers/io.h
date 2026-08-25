#ifndef IO_H
#define IO_H

unsigned char inb(unsigned short port);
void outb(unsigned short port, unsigned char value);
void picremap(void);
unsigned char pic_get_irr(void);
unsigned char pic_get_isr(void);

#endif