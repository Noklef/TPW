# The two blends of src/game/core/pixel_stamps (0x5e7d70, 0x5e7f60) against the originals: sources of 0 to 4 x 0 to 3 pixels of 1, 2, 3 or 4 bytes, masks, destinations of 8 x 8 pixels
# of 4 bytes (or other sizes for the row step) with random bytes, positions inside.
# usage: pixel_stamps.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Stamp,Tint)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Stamp': 0x5e7d70, 'Tint': 0x5e7f60}
B = Bench([(0x5e7d70, 0x5e7f00), (0x5e7f60, 0x5e80d0)])
SRC, DST, SDATA, DDATA = RAM, RAM + 0x20, RAM + 0x1000, RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    sw, sh = r.randrange(0, 5), r.randrange(0, 4)
    bpp = r.choice([1, 2, 3, 4]) if name == 'Stamp' else 1
    src = bytearray(r.bytes(0x10))
    put(src, 0, 'IIII', sw, sh, bpp, SDATA if name == 'Stamp' or r.random() < 0.9 else 0)
    dw, dh, dbpp = r.randrange(max(sw, 1), 9), 8, r.choice([4, 4, 4, 3, 5])
    dst = bytearray(r.bytes(0x10))
    put(dst, 0, 'IIII', dw, dh, dbpp, DDATA)
    sdata = bytearray(r.bytes(sw * sh * 4 + 0x20))
    for i in range(len(sdata)):
        if r.random() < 0.3:
            sdata[i] = r.choice([0, 1, 0x7f, 0x80, 0xfe, 0xff])
    regions = {SRC: src, DST: dst, SDATA: sdata, DDATA: r.bytes(dw * dh * 5 + 0x80)}
    if name == 'Stamp':
        x, y = r.randrange(0, max(dw - sw, 0) + 1), r.randrange(0, 4)
        return Call(CALLS[name], [SRC, DST, x, y], regions, conv='t', ret='v')
    return Call(CALLS[name], [SRC, DST] + [r.choice([0, 1, 0x7f, 0x80, 0xff, r.getrandbits(32)]) for _ in range(4)], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
