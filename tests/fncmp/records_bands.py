# The record copy with text, band match and byte permutation of src/game/core/records_bands against the originals: records with short texts, band tables of signed
# bytes around the limits, flag words with every combination of the permutation bits. Sort (the bands object's quicksort, 0x56f2f0): 0-40
# dwords (few distinct, sorted, reversed or random) between 8 guard dwords each side, the whole range or a part (lo past hi too).
# Collect (0x56f380, which ends in the sort): 0-6 rows (from -1 on) of 0-8 columns (a negative now and then) over a map 8-16 wide whose
# cells are skipped (bit 1) or name one of 8 texture sets (some none), the override box (flag 4 of [0x8bd550]) around them or not, the
# textures' ids few and their flag 0x1000 set or not.
# usage: records_bands.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Copy,Bands,Perm,Sort,Collect)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Copy': (0x489350, 't', 'a', 0x70), 'Bands': (0x56ee90, 't', 'a', 0x80), 'Perm': (0x56f4f0, 's', 'v', 0x80),
         'Sort': (0x56f2f0, 't', 'v', 0x81), 'Collect': (0x56f380, 't', 'v', 0x170)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x575f90, 0x575f9d)])
MAP, SETS, TEXSETS, ARRAYS, TEXTURES, IDS, COUNT, OUT = (RAM + k for k in (0x400, 0x600, 0x700, 0x800, 0x900, 0xc00, 0xd00, 0xe00))
CELLS = RAM + 0x1000


def collect(r):
    obj = bytearray(r.bytes(0x400))
    lo = r.choice([-1, 0, 0, 1, 2])
    put(obj, 0, '2I', MAP, SETS)
    put(obj, 0x128, '2i', lo, lo + r.choice([-1, 0, 1, 2, 4, 6]))
    for row in range(-1, 8):
        start = r.choice([0, 0, 1, 2, 3, 8, 0xff])
        obj[0x130 + row] = start
        obj[0x1b0 + row] = r.choice([0, 1, 2, 4, 6, 8, 0xff]) if r.random() < 0.3 else min(8, (start + r.randrange(6)) & 0xff)
    x0, y0 = r.choice([-1, 0, 1, 2, 5]), r.choice([-1, 0, 1, 2])
    put(obj, 0x234, '4i', x0, y0, x0 + r.choice([-1, 0, 1, 3, 8]), y0 + r.choice([-1, 0, 1, 2, 6]))
    textures = [TEXTURES + 0x20 * k for k in range(8)]
    for k in range(0x48):
        put(obj, 0x248 + 4 * k, 'I', r.choice(textures + [0]))
    width = r.choice([8, 9, 16])
    mp = put(bytearray(r.bytes(0x30)), 0x18, 'I', width)
    put(mp, 0x2c, 'I', CELLS)
    cells = bytearray(r.bytes(0x400))
    for k in range(0x100):
        cells[4 * k] = r.getrandbits(8) & r.choice([0xfe, 0xfe, 0xff])
        put(cells, 4 * k + 2, 'H', r.randrange(8))
    sets, texsets, arrays = bytearray(r.bytes(0x40)), bytearray(r.bytes(0x80)), bytearray(r.bytes(0x20))
    for k in range(8):
        put(sets, 8 * k + 4, 'I', r.choice([TEXSETS + 0x10 * k] * 3 + [0]))
        put(texsets, 0x10 * k + 8, 'I', ARRAYS + 4 * k)
        put(arrays, 4 * k, 'I', r.choice(textures + [0]))
    tex = bytearray(r.bytes(0x100))
    pool = r.choice([[1, 2, 3], [0, 7, 0x80000000, 0xffffffff], [5]])
    for k in range(8):
        put(tex, 0x20 * k + 8, 'I', r.getrandbits(32) ^ r.choice([0, 0x1000]))
        put(tex, 0x20 * k + 0x1c, 'I', r.choice(pool) if r.random() < 0.9 else r.getrandbits(32))
    regions = {RAM: obj, MAP: mp, SETS: sets, TEXSETS: texsets, ARRAYS: arrays, TEXTURES: tex, IDS: r.bytes(0x100), COUNT: r.bytes(8),
               OUT: r.bytes(0x100), CELLS: cells, 0x8bd550: r.bytes(1)}
    return Call(0x56f380, [RAM, IDS, COUNT, OUT], regions, conv='t')
SRC, DST = RAM, RAM + 0x200


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    if name == 'Collect':
        return collect(r)
    if name == 'Copy':
        src = bytearray(r.bytes(0x80))
        text = bytes(r.choice(b'abc XYZ\x80\xff') for _ in range(r.choice([0, 1, 2, 5, 9, 20]))) + b'\0'
        src[0x24:0x24 + len(text)] = text
        put(src, 0x20, 'I', r.choice([SRC + 0x24, SRC + 0x30, 0, r.getrandbits(32)]))
        return Call(address, [DST, SRC], {SRC: src, DST: r.bytes(0x80)}, conv=conv, ret=ret)
    if name == 'Bands':
        obj = bytearray(r.bytes(0x240))
        put(obj, 0x128, 'ii', r.choice([-1, 0, 1, 2, 5, 100]), r.choice([-1, 0, 1, 3, 5, 8, 200]))
        for i in range(0x130, 0x1b0):
            obj[i] = r.choice([0x80, 0xff, 0, 1, 2, 5, 0x7f, r.getrandbits(8)])
        for i in range(0x1b0, 0x230):
            obj[i] = r.choice([0x80, 0xff, 0, 1, 2, 5, 0x7f, r.getrandbits(8)])
        packed = r.choice([0, 1, 5, 20, 100]) | r.choice([0, 1, 2, 3, 20]) << 8 | r.choice([0, 1, 5, 0x7f, 0x80, 0xff]) << 16 | r.choice([0, 1, 5, 0x7f, 0x80, 0xff]) << 24
        return Call(address, [RAM, packed], {RAM: obj}, conv=conv, ret=ret)
    if name == 'Sort':
        n = r.choice([0, 1, 2, 3, 5, 8, 13, 20, 40])
        pool = r.choice([[1, 2, 3], [0, 0x80000000, 0xffffffff, 7], None])
        values = [r.choice(pool) if pool else r.getrandbits(32) for _ in range(n)]
        shape = r.choice(['any', 'any', 'up', 'down'])
        if shape != 'any':
            values.sort(reverse=shape == 'down')
        data = bytearray(r.bytes(0x20)) + b''.join(v.to_bytes(4, 'little') for v in values) + bytearray(r.bytes(0x20))
        lo, hi = 0, n - 1
        if n > 2 and r.random() < 0.3:
            lo, hi = r.randrange(n), r.randrange(n)
        return Call(address, [RAM, RAM + 0x420, lo & 0xffffffff, hi & 0xffffffff], {RAM: r.bytes(0x240), RAM + 0x400: data}, conv=conv,
                    ret=ret)
    flags = r.choice([0, 8, 0x10, 0x20, 0x40, 0x48, 0x50, 0x60, 0x78, 0x7f, 0x37, 0xffff, r.getrandbits(16)])
    return Call(address, [SRC, SRC + 0x20], {SRC: r.bytes(8), SRC + 0x20: flags.to_bytes(2, 'little') + bytes(2)}, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
