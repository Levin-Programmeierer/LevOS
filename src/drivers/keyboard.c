#include "drivers/keyboard.h"
#include "drivers/io.h"
#include "drivers/terminal.h"

#define KEYBOARD_BUFFER_SIZE 128

static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile unsigned int buffer_head = 0;
static volatile unsigned int buffer_tail = 0;

char table[256] = {
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',

    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',

    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',

    [0x1C] = '\n',
    [0x39] = ' ',
    [0x0E] = '\b'
};

char keyboard_getchar(void){
    if (buffer_head == buffer_tail) {
        return 0; // buffer empty
    }

    char c = keyboard_buffer[buffer_tail];
    buffer_tail = (buffer_tail + 1) % KEYBOARD_BUFFER_SIZE;

    return c;
}

static void keyboard_buffer_put(char c){
    unsigned int next = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;

    if (next == buffer_tail) {
        return; // buffer full
    }

    keyboard_buffer[buffer_head] = c;
    buffer_head = next;
}

void keyboard_irq(){
    unsigned char scancode = inb(0x60); // get scancode
    if((scancode & 0x80) == 0){ // key press
        char asciichar = scancode_to_ascii(scancode);
        if(asciichar != 0){
            keyboard_buffer_put(asciichar);
        }
    }
}

char scancode_to_ascii(unsigned char scancode){
    if(table[scancode] != 0){
            return table[scancode];
        } else {
            return (char)0;
    }
}

void init_keyboard(void){
    outb(0x21, 0xFD);
}