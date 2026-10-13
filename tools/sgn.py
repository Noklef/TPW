"""Theme Park World .sgn sign styles: how a park's name is painted onto a sign texture.

Rides and park gates carry sign1.tga / sign2.tga placeholder textures. Sign_Apply 0x467a80
splits the name into two lines (at the space nearest the middle), loads <theme>/<style>.sgn
with SignStyle_Load 0x5ec3a0 and draws the text with GDI and the TrueType font named here.

Layout (little-endian):
  u32  version (100 or 101)
  u32  ?  (0 in every file)
  u8   has background image
  u32  line 1 effect, u32 line 2 effect   (0 none; 1 or 2 add an effect block below)
  line style x 2:
    char font name[64]           e.g. "Gargamel Smurf AOE"
    char font file[260]          the .ttf in data/fonts
    i32  x, i32 y                where the line sits on the texture
    LOGFONTA (0x3c bytes)        height, width, escapement, orientation, weight, italic,
                                 underline, strikeout, charset, precision, clipping,
                                 quality, pitch, face name[32]
    11 x u32                     layout values: i32, 8 floats, 2 i32 (meaning not traced yet)
  effect block (line 1, then line 2, each only if its effect is non-zero):
    u8 colour[4], u32, f32, u32, u32
  array x 2                      fill for each line's letters: a 16 x 128 x 4 vertical gradient strip
  background (only if the flag is set):
    version 100: array (raw pixels)
    version 101: u32 w, h, bpp, then the encoder's own stream (WaveletCodec_Encode 0x591ab0):
      bpp 1: f32 q, u32 size, stream
      bpp 3: f32 qY, qCb, qCr, u32 size, stream
      bpp 4: f32 qY, qCb, qCr, qA, u32 colour size, u32 alpha size, colour stream, alpha stream
    The streams are BILZ (zlib) and decode like .wct v0x12 (rows [all low, all high]) with
    the float quantisers used as the steps directly (0x592730 colour, 0x592e60 grey).
  array = u32 w, h, bpp, then w*h*bpp bytes

Usage:
  python -I tools/sgn.py <file.sgn | archive.wad | folder> [out_dir]   print, and write PNGs
"""
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wct2png  # noqa: E402
from wadtool import read_wad  # noqa: E402

LOGFONT = ('height', 'width', 'escapement', 'orientation', 'weight', 'italic', 'underline',
           'strikeout', 'charset', 'out_precision', 'clip_precision', 'quality', 'pitch')


class Reader:
    def __init__(self, data):
        self.data, self.pos = data, 0

    def take(self, n):
        self.pos += n
        if self.pos > len(self.data):
            raise ValueError('truncated')
        return self.data[self.pos - n:self.pos]

    def unpack(self, fmt):
        return struct.unpack('<' + fmt, self.take(struct.calcsize('<' + fmt)))

    def text(self, n):
        return self.take(n).split(b'\0')[0].decode('latin-1')

    def array(self):
        w, h, bpp = self.unpack('3i')
        return {'width': w, 'height': h, 'bpp': bpp, 'pixels': self.take(w * h * bpp)}


def parse_sgn(data):
    r = Reader(data)
    version, unknown, has_bg, fx1, fx2 = r.unpack('iiBii')
    if version < 100:
        raise ValueError('unsupported .sgn version %d' % version)
    lines = []
    for _ in range(2):
        line = {'font': r.text(64), 'file': r.text(260)}
        line['x'], line['y'] = r.unpack('2i')
        lf = r.unpack('5i8B')
        line['logfont'] = dict(zip(LOGFONT, lf))
        line['logfont']['face'] = r.text(32)
        line['layout'] = r.unpack('i8f2i')
        lines.append(line)
    for line, fx in zip(lines, (fx1, fx2)):
        line['effect'] = fx
        if fx:
            colour = r.take(4)
            line['effect_data'] = (colour.hex(),) + r.unpack('ifii')
    fills = [r.array(), r.array()]
    background = None
    if has_bg:
        if version == 100:
            background = r.array()
        else:
            w, h, bpp = r.unpack('3i')
            if bpp == 1:
                q = r.unpack('f')
                streams = [r.take(r.unpack('I')[0])]
            elif bpp == 3:
                q = r.unpack('3f')
                streams = [r.take(r.unpack('I')[0])]
            elif bpp == 4:
                q = r.unpack('4f')
                sizes = r.unpack('2I')
                streams = [r.take(n) for n in sizes]
            else:
                raise ValueError('background bpp %d' % bpp)
            background = {'width': w, 'height': h, 'bpp': bpp, 'q': q, 'streams': streams}
    if r.pos != len(data):
        raise ValueError('%d bytes left over' % (len(data) - r.pos))
    return {'version': version, 'unknown': unknown, 'lines': lines, 'fills': fills, 'background': background}


def decode_background(bg):
    """RGBA bytes of a version 101 background."""
    w, h, bpp, q = bg['width'], bg['height'], bg['bpp'], bg['q']
    size = max(8, 1 << (max(w, h) - 1).bit_length())
    planes = wct2png.Coeffs(wct2png.bilz_decompress(bg['streams'][0]))
    clamp = lambda v: 0 if v < 0 else 255 if v > 255 else int(round(v))
    if bpp == 1:
        g = wct2png.decode_plane(planes, size, q[0], False)
        return b''.join(bytes((clamp(g[y][x]),) * 3 + (255,)) for y in range(h) for x in range(w))
    Y = wct2png.decode_plane(planes, size, q[0], False)
    U = wct2png.decode_plane(planes, size // 2, q[1], False)
    V = wct2png.decode_plane(planes, size // 2, q[2], False)
    A = None
    if bpp == 4:
        A = wct2png.decode_plane(wct2png.Coeffs(wct2png.bilz_decompress(bg['streams'][1])), size, q[3], False)
    out = bytearray()
    for y in range(h):
        for x in range(w):
            yy, cb, cr = Y[y][x], U[y // 2][x // 2], V[y // 2][x // 2]
            out += bytes((clamp(yy + 1.402 * cr), clamp(yy - 0.344136 * cb - 0.714136 * cr),
                          clamp(yy + 1.772 * cb), clamp(A[y][x]) if A else 255))
    return bytes(out)


def raw_rgba(arr):
    px, bpp = arr['pixels'], arr['bpp']
    if bpp == 1:
        return b''.join(bytes((v, v, v, 255)) for v in px)
    if bpp == 3:
        return b''.join(bytes((px[i + 2], px[i + 1], px[i], 255)) for i in range(0, len(px), 3))
    return b''.join(bytes((px[i + 2], px[i + 1], px[i], px[i + 3])) for i in range(0, len(px), 4))


def sgn_files(path):
    if os.path.isdir(path):
        for root, _, files in os.walk(path):
            for f in sorted(files):
                if f.lower().endswith(('.wad', '.sgn')):
                    yield from sgn_files(os.path.join(root, f))
    elif path.lower().endswith('.wad'):
        for name, data, _ in read_wad(path):
            if name.lower().endswith('.sgn'):
                yield '%s:%s' % (path, name), name, data
    else:
        yield path, os.path.basename(path), open(path, 'rb').read()


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[2] if len(sys.argv) > 2 else None
    count = 0
    for label, name, data in sgn_files(sys.argv[1]):
        s = parse_sgn(data)
        count += 1
        bg = s['background']
        print('%s: v%d, background %s' % (label, s['version'],
                                         '%dx%dx%d' % (bg['width'], bg['height'], bg['bpp']) if bg else 'none'))
        for i, line in enumerate(s['lines']):
            lf = line['logfont']
            print('  line %d: %s (%s) at %d,%d height %d weight %d effect %d' %
                  (i + 1, line['font'], line['file'], line['x'], line['y'], lf['height'], lf['weight'], line['effect']))
        if out:
            os.makedirs(out, exist_ok=True)
            stem = os.path.join(out, os.path.splitext(name)[0])
            if bg:
                rgba = raw_rgba(bg) if 'pixels' in bg else decode_background(bg)
                wct2png.write_png(stem + '.png', bg['width'], bg['height'], rgba)
            for i, fill in enumerate(s['fills']):
                wct2png.write_png('%s_fill%d.png' % (stem, i + 1), fill['width'], fill['height'], raw_rgba(fill))
    print('%d sign styles' % count)


if __name__ == '__main__':
    main()
