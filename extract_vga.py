#!/usr/bin/env python3
# Usage: python3 extract_vga.py /tmp/screen.ppm
from PIL import Image
import sys
ppm = sys.argv[1]
font_path = "fonts/cp865-8x16.psf"
img = Image.open(ppm).convert("RGB")
w,h = img.size
cols,rows = 80,25
cell_w = w//cols
cell_h = h//rows
# Load PSF1 font
with open(font_path,"rb") as f:
    data = f.read()
if data[0]!=0x36 or data[1]!=0x04:
    print("Not PSF1 font or font not found", file=sys.stderr); sys.exit(1)
mode = data[2]; charsize = data[3]
count = 512 if (mode & 0x01) else 256
ptr = 4
font_glyphs = []
for i in range(count):
    font_glyphs.append(data[ptr:ptr+charsize]); ptr += charsize
def lum(rgb):
    r,g,b = rgb; return 0.2126*r + 0.7152*g + 0.0722*b
lines = []
for ry in range(rows):
    line = ""
    for cx in range(cols):
        x0, y0 = cx*cell_w, ry*cell_h
        bg = img.getpixel((x0,y0)); bg_l = lum(bg)
        bits = []
        for r in range(cell_h):
            rowbits = 0
            for c in range(8):
                px = img.getpixel((x0+c, y0+r))
                rowbits = (rowbits<<1) | (1 if abs(lum(px)-bg_l) > 30 else 0)
            bits.append(rowbits & 0xFF)
        best = None; bestscore = -1
        for ch in range(256):
            glyph = font_glyphs[ch]
            if len(glyph) < cell_h: continue
            score = 0
            for r in range(cell_h):
                score += 8 - bin(glyph[r] ^ bits[r]).count("1")
            if score > bestscore:
                bestscore = score; best = ch
        if bestscore < 0.6 * (cell_h*8):
            line += '?'
        else:
            line += chr(best)
    lines.append(line.rstrip())
print("\n".join(lines))