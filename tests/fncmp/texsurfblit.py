# Class B texture surfaces' scaled draw (src/game/gfx/texsurface: 0x58c750, and its thunk 0x58c930, core/thunks) against the originals:
# source sizes 1-32 (pitch at or past the width), sizes drawn 1-24 (a few 0, where the IDIV faults in both, or negative), at (0-8, 0-8) of
# a destination its pitch at or past x + the width; flipped (0x40000) or not; flag 0x10000000: the texture's object asked first (a
# recorder, its stack record logged as a hash) answering 0 or not, then locked by Texture_Lock (0x590d40) through the same object.
# usage: texsurfblit.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Blit,Thunk)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x58c750, 0x58c89d), (0x58c930, 0x58c938), (0x590d40, 0x590d96)])
CALLS = {'Blit': 0x58c750, 'Thunk': 0x58c930}
SUB, VBT, TEX, OBJ, SRC, DST, OFF, ADJ = RAM + 0x100, RAM + 0x40, RAM + 0x400, RAM + 0x600, RAM + 0x2000, RAM + 0x8000, 0x20, 0x30


def size(r, lo, hi):
    if r.random() < 0.03:
        return r.choice([0, -1, -2])
    return r.choice([lo, hi, r.randrange(lo, hi + 1), r.randrange(lo, hi + 1)])


def case(r):
    op = r.deal(OPS)
    log = log_region()
    sw, sh, dw, dh = size(r, 1, 32), size(r, 1, 32), size(r, 1, 24), size(r, 1, 24)
    x, y = r.randrange(9), r.randrange(9)
    spitch = max(sw, 0) + r.choice([0, 0, 1, 3])
    dpitch = x + max(dw, 0) + r.choice([0, 0, 1, 5])
    flags = r.getrandbits(32) & ~0x10040000 | r.choice([0, 0x40000]) | (0x10000000 if r.random() < 0.25 else 0)
    texture = bytearray(r.bytes(0x60))
    put(texture, 8, 'I', flags)
    put(texture, 0x10, 'hhhh', sw, sh, dw, dh)
    put(texture, 0x34, 'h', spitch)
    put(texture, 0x40, 'I', SRC)
    area = bytearray(r.bytes(0x200))   # (RAM: the vbtable at +0x40, the sub-object at +0x100)
    put(area, VBT - RAM + 4, 'I', OFF)
    put(area, SUB - RAM - 0x10, 'II', VBT, DST)
    put(area, SUB - RAM + OFF, 'I', dpitch)
    put(area, SUB - RAM + ADJ - 4, 'I', ADJ)
    regions = {RAM: area, TEX: texture, LOG: log, SRC: r.bytes(spitch * max(sh, 1) * 4 + 0x20),
               DST: r.bytes(dpitch * (y + max(dh, 1)) * 4 + 0x20), 0x8bd554: struct.pack('<I', r.choice([2, 3]))}
    if flags & 0x10000000:
        put(texture, 0x40, 'I', OBJ)
        regions[OBJ] = struct.pack('<I', OBJ + 0x10) + r.bytes(0xc) + vtable({1: 2, 2: 1}, 8)
        blob(log, 1, 0, 0xf)
        blob(log, 2, 0, 0xf)
        if r.random() < 0.7:
            set_return(log, 1, 0)
        regions[0x879a68] = struct.pack('<I', SRC)
    this = SUB + ADJ if op == 'Thunk' else SUB
    return Call(CALLS[op], [this, TEX, x, y], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
