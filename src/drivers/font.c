#include <stdint.h>
#include "drivers/font.h"
#include "drivers/terminal.h"

extern unsigned char _binary_fonts_cp865_8x16_psf_start[];
extern unsigned char _binary_fonts_cp865_8x16_psf_end[];

static const uint8_t *font_data = 0;
static uint8_t font_height = 0;
static uint16_t font_glyph_count = 0;

int font_init(void)
{
    uint8_t *data = _binary_fonts_cp865_8x16_psf_start;

    print("FONT INIT\n", 0x0F);

    print("MAGIC 0: ", 0x0F);
    print_hex_dword(data[0]);
    print("\n", 0x0F);

    print("MAGIC 1: ", 0x0F);
    print_hex_dword(data[1]);
    print("\n", 0x0F);

    print("MODE: ", 0x0F);
    print_hex_dword(data[2]);
    print("\n", 0x0F);

    print("HEIGHT: ", 0x0F);
    print_hex_dword(data[3]);
    print("\n", 0x0F);

    if (data[0] != 0x36 || data[1] != 0x04) {
        print("NOT PSF1!\n", 0x0F);
        return 0;
    }

    font_height = data[3];

    if (data[2] & 0x01)
        font_glyph_count = 512;
    else
        font_glyph_count = 256;

    font_data = data;

    print("FONT OK\n", 0x0F);

    return 1;
}

const uint8_t *font_get_glyph(unsigned char c)
{
    if (!font_data)
        return 0;

    return font_data + 4 +
           ((uint32_t)c * font_height);
}

uint8_t font_get_height(void)
{
    return font_height;
}

uint16_t font_get_glyph_count(void)
{
    return font_glyph_count;
}