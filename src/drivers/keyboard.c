#include "drivers/keyboard.h"
#include "drivers/io.h"
#include "drivers/terminal.h"

#define KEYBOARD_BUFFER_SIZE 128
static volatile char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
static volatile unsigned int buffer_head = 0;
static volatile unsigned int buffer_tail = 0;
static volatile int left_shift;
static volatile int right_shift;
static volatile int caps_lock;

/* Set 1 scancodes, without the high bit used for key releases. */
char table[256] = {
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
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
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2B] = '\\',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
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

static const char shifted_table[256] = {
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',
    [0x1A] = '{',
    [0x1B] = '}',
    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = ':',
    [0x28] = '"',
    [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?'
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

char scancode_to_ascii(unsigned char scancode){
    char value;
    int upper = left_shift || right_shift;

    if (scancode & 0x80)
        return 0;
    value = table[scancode];
    if (value >= 'a' && value <= 'z') {
        if (caps_lock != 0)
            upper = !upper;
        if (upper)
            value = (char)(value - 'a' + 'A');
    } else if (upper && shifted_table[scancode & 0x7F] != 0) {
        value = shifted_table[scancode & 0x7F];
    }
    return value;
}

void keyboard_irq(){
    unsigned char scancode = inb(0x60); // get scancode
    unsigned char code = scancode & 0x7F;

    if (code == 0x2A) {
        left_shift = (scancode & 0x80) == 0;
        return;
    }
    if (code == 0x36) {
        right_shift = (scancode & 0x80) == 0;
        return;
    }
    if (code == 0x3A) {
        if ((scancode & 0x80) == 0)
            caps_lock = !caps_lock;
        return;
    }
    if((scancode & 0x80) == 0){ // key press
        char asciichar = scancode_to_ascii(code);
        if(asciichar != 0)
            keyboard_buffer_put(asciichar);
    }
}

void init_keyboard(void){
    unsigned char mask = inb(0x21);
    left_shift = 0;
    right_shift = 0;
    caps_lock = 0;
    mask &= ~(1 << 1);
    outb(0x21, mask);
}