# A run-length image's scaled draw (src/game/gfx/rleimage: 0x564790) against the original: images of 1 to 40 x 1 to 20 pixels whose rows are runs and
# literal spans of random palette indices (zero-length runs too; each row's length byte right), palettes of random u16 / u32 entries by the format
# (0x8bd554 indexing pixel sizes 2, 4 and 3 at 0x701158), and destinations the same size or narrower, wider, shorter and taller, with a pitch that
# fits the row, exceeds it or is 0.
# usage: rleimage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x564790, 0x564a45), (0x5da3c0, 0x5da3c1)])
OBJ, CODES, PALETTE, DEST, OUT = RAM, RAM + 0x100, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000


def row(r, width):
    out = bytearray()
    x = 0
    while x < width:
        n = min(r.choice([1, 1, 2, 3, 5, 8]), width - x) if r.random() < 0.97 else r.choice([1, 2, 4])  # (sometimes past the width)
        if r.random() < 0.5:
            out += struct.pack('<b', n) + bytes(r.randrange(256) for _ in range(n))
        else:
            if r.random() < 0.05:
                out += bytes([0, r.randrange(256)])  # a run of none
            out += struct.pack('<b', -n) + bytes([r.randrange(256)])
        x += n
    return bytes([len(out)]) + bytes(out) if len(out) < 0x80 else row(r, width)  # (a length byte over 0x7f skips back, sic: not made)


def case(r):
    w, h = r.choice([1, 2, 4, 8, r.randrange(1, 41)]), r.choice([1, 2, 3, 6, r.randrange(1, 21)])
    codes = b''.join(row(r, w) for _ in range(h)) + r.bytes(0x40)
    mode = r.randrange(4)
    size = [2, 4, 3, 2][mode]
    dw = r.choice([w, w, max(1, w // 2), w * 2, r.randrange(1, 2 * w + 1)])
    dh = r.choice([h, h, max(1, h // 2), h * 2, r.randrange(1, 2 * h + 1)])
    pitch = r.choice([size * dw, size * dw, size * dw + r.choice([2, 4, 8]), 0])
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x20 - 0x14, '4I', CODES, PALETTE, w, h)
    dest = put(bytearray(r.bytes(0x20)), 0, '4I', OUT, dw, dh, pitch)
    regions = {OBJ: obj, CODES: codes, PALETTE: r.bytes(0x400), DEST: dest, OUT: r.bytes((h + 2) * max(pitch, size * dw) + 0x40),
               0x701158: struct.pack('<4I', 2, 4, 3, 2), 0x8bd554: struct.pack('<I', mode)}
    return Call(0x564790, [OBJ + 0x20, DEST], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
