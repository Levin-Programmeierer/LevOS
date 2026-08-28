#ifndef KEYBOARD_H
#define KEYBOARD_H

char keyboard_getchar(void);
char scancode_to_ascii(unsigned char scancode);
void init_keyboard(void);
void keyboard_irq(void);

#endif