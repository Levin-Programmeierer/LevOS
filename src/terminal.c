#include "terminal.h"   

int cursor_x = 0;
int cursor_y = 0;

volatile unsigned short *video =
        (unsigned short *)0xB8000;

void print(const char *text) {
     // To move a character y * 80 + x to move it and call video
    int position = 0;
    for(int i = 0; i < 100; i++) {
        if(text[i] != '\0' && text[i] != '\n'){
            position = cursor_y * 80 + cursor_x;
            video[position] = (0x0F << 8) | text[i];
            cursor_x++;
        } else if(text[i] == '\n'){
            cursor_x = 0;
            cursor_y++;
        } else{
            i = 100;
        }
    }

}

void putchar(char c){
    if(c == '\n'){
        cursor_y++;
        cursor_x = 0;
    }else if (c == '\b'){
        backspace();
    }
    else {
    int position = cursor_y * 80 + cursor_x;
    video[position] = (0x0F << 8) | c;
    cursor_x++;
    }
}

void clear(void){
    for(int i = 0; i < 2000; i++){
        video[i] = 0x0F20;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void backspace(void)
{
    if (cursor_x > 0) {
        cursor_x--;
        video[cursor_y * 80 + cursor_x] = 0x0F20;
    }
}