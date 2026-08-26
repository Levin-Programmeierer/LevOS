#ifndef FONT_H
#define FONT_H

#include <stdint.h>

int font_init(void);

const uint8_t *font_get_glyph(unsigned char c);

uint8_t font_get_height(void);

uint16_t font_get_glyph_count(void);

#endif