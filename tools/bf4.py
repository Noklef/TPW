"""Theme Park World .bf4 bitmap fonts -> PNG glyph sheet + metrics.

Layout (little-endian):
  0x00 "F4FB"
  0x04 u8  kind        1 = anti-aliased fonts, 2 = the plain 1-bit fonts (CONSOLE6, GAME10, ...)
  0x05 u8  height      line height in pixels
  0x06 u16 count       number of glyphs
  0x08 u32 offsets[count]   absolute, one per glyph record
  glyph record:
    u32 code       character (Unicode / Latin-1, e.g. 0xd3 = 'Ó'); not sorted
    u32 stored     bytes of pixel data that follow the 24-byte record header (padded to 4)
    u32 size4      w * h / 2 rounded up: the size of the pixels at 4 bits each
    u32 encoding   0 = raw 4bpp, 1 = nibble RLE 4bpp, 2 = 1bpp
    u16 w, u16 h
    i8  x offset, i8 y offset (from the top of the line; accents go negative)
    u16 advance
    pixels
Pixels are coverage values 0..15 (1bpp: 0/1), row by row; nibbles and bits run high first.
RLE: a non-zero nibble is one pixel; nibble 0 is followed by a count nibble and a value
nibble (count copies of value). Count 0 ends the glyph; the rest is 0. The encoder works in
whole bytes, so an odd pixel count can decode one pixel long: cut it to w * h.

Usage:
  python -I tools/bf4.py <font.bf4> [out.png]   writes the sheet and <out>.txt metrics
  python -I tools/bf4.py --survey <dir>          checks every font in a folder
"""
import glob
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from wct2png import write_png  # noqa: E402


def nibbles(data):
    out = []
    for b in data:
        out += (b >> 4, b & 15)
    return out


def unrle(data, total):
    n, out, i = nibbles(data), [], 0
    while len(out) < total and i < len(n):
        v = n[i]
        i += 1
        if v:
            out.append(v)
            continue
        count, v = n[i], n[i + 1]
        i += 2
        if count == 0:
            break
        out += [v] * count
    return (out + [0] * total)[:total]


def decode_pixels(enc, data, w, h):
    total = w * h
    if enc == 0:
        return nibbles(data)[:total]
    if enc == 1:
        return unrle(data, total)
    if enc == 2:
        return [15 * (data[i >> 3] >> (7 - (i & 7)) & 1) for i in range(total)]
    raise ValueError('unknown glyph encoding %d' % enc)


def read_font(path):
    d = open(path, 'rb').read()
    if d[:4] != b'F4FB':
        raise ValueError('not a .bf4 font')
    kind, height, count = struct.unpack_from('<BBH', d, 4)
    glyphs = []
    for off in struct.unpack_from('<%dI' % count, d, 8):
        code, stored, size4, enc, w, h, xo, yo, adv = struct.unpack_from('<4I2H2bH', d, off)
        if size4 != (w * h + 1) // 2:
            raise ValueError('glyph %#x: size %d for %dx%d' % (code, size4, w, h))
        glyphs.append({'code': code, 'enc': enc, 'w': w, 'h': h, 'x': xo, 'y': yo, 'advance': adv,
                       'pixels': decode_pixels(enc, d[off + 24:off + 24 + stored], w, h)})
    return {'kind': kind, 'height': height, 'glyphs': glyphs}


def write_sheet(font, png_path, columns=16):
    glyphs = sorted(font['glyphs'], key=lambda g: g['code'])
    cell_w = max(g['w'] for g in glyphs) + 2
    cell_h = max(g['h'] for g in glyphs) + 2
    rows = (len(glyphs) + columns - 1) // columns
    W, H = columns * cell_w, rows * cell_h
    rgba = bytearray(W * H * 4)
    lines = ['# height %d kind %d; code char x y w h xoff yoff advance' % (font['height'], font['kind'])]
    for i, g in enumerate(glyphs):
        cx, cy = i % columns * cell_w + 1, i // columns * cell_h + 1
        for y in range(g['h']):
            for x in range(g['w']):
                a = g['pixels'][y * g['w'] + x] * 17
                p = ((cy + y) * W + cx + x) * 4
                rgba[p:p + 4] = bytes((255, 255, 255, a))
        ch = chr(g['code']) if g['code'] > 32 else ''
        lines.append('%#06x %s %d %d %d %d %d %d %d' % (g['code'], ch or '-', cx, cy, g['w'], g['h'],
                                                      g['x'], g['y'], g['advance']))
    write_png(png_path, W, H, bytes(rgba))
    with open(os.path.splitext(png_path)[0] + '.txt', 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines) + '\n')


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    if sys.argv[1] == '--survey':
        for path in sorted(glob.glob(os.path.join(sys.argv[2], '*.bf4'))):
            font = read_font(path)
            enc = [sum(g['enc'] == e for g in font['glyphs']) for e in range(3)]
            codes = [g['code'] for g in font['glyphs']]
            print('%-16s kind %d height %2d glyphs %3d (raw %d, rle %d, 1bpp %d) codes %#x..%#x' % (
                os.path.basename(path), font['kind'], font['height'], len(codes), *enc, min(codes), max(codes)))
        return
    out = sys.argv[2] if len(sys.argv) > 2 else os.path.splitext(os.path.basename(sys.argv[1]))[0] + '.png'
    write_sheet(read_font(sys.argv[1]), out)
    print(out)


if __name__ == '__main__':
    main()
