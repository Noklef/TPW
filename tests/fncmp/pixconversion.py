# The pixel conversion (src/game/gfx/pixconv: 0x630880-0x630c96, thiscall) against the originals: one that holds a converter of any kind (or none)
# whose vtable is recorders (+0 the deleting destructor, +4 forwards, +8 backwards), or no converter; Setup from two formats (pixconv.py's, the
# game's RGB888 / ARGB8888 ones, or paletted), a palette or none, flags any, the new converter allocated into the heap region (its allocation
# failing now and then) and made by the originals' own converter code (the game's two formats mapped at 0xfaa268 / 0xfaa280, so a source of either
# with a 16-bit target reaches the 16-bit converter); the base converter's destructors (flags any, freed into the heap region).
# Gap: the index converter's inverse map is never built here (its table allocation fails, or it already has a table built from this palette).
# usage: pixconversion.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,Setup,Row,Rect,BaseDel,BaseDel2,BaseDtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pixconv import FIXED, fmt, pack  # noqa: E402

B = Bench([(0x630880, 0x630c97), (0x639230, 0x63b2c0), (0x642860, 0x6428a5), (0x5f5660, 0x5f568c), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1)])
CONV, OLD, VT, TARGET, SOURCE, PAL, TABLE, SRC, DST = RAM, RAM + 0x100, RAM + 0x600, RAM + 0x700, RAM + 0x720, RAM + 0x800, RAM + 0xc00, RAM + 0x1000, RAM + 0x1800
COUNT = 0xfaa534   # (the counted allocations' count)
FORMATS = 0xfaa268   # the game's RGB888 and ARGB8888 formats (0xfaa280)
CALLS = {'Ctor': 0x630880, 'Dtor': 0x630c30, 'Setup': 0x630890, 'Row': 0x630b70, 'Rect': 0x630ba0, 'BaseDel': 0x630c50, 'BaseDel2': 0x630c70,
         'BaseDtor': 0x630c90}


def format_(r):
    """A pixel format (masks, bit count, paletted): pixconv.py's, the game's two the 16-bit converter knows, or paletted."""
    k = r.random()
    if k < 0.25:
        return FIXED[r.choice(list(FIXED))]
    if k < 0.45:
        return (r.getrandbits(32) if r.random() < 0.2 else 0, 0, 0, 0, 8, r.choice([1, 1, r.getrandbits(32)]))
    return fmt(r)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    old = bytearray(r.bytes(0x440))
    put(old, 0, 'I', VT)
    kind = r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])
    conv = bytearray(r.bytes(0x10))
    put(conv, 0, 'I', OLD if r.random() < 0.9 else 0)
    put(conv, 0xc, 'I', kind)
    regions = {CONV: conv, OLD: old, VT: vtable({0: 1, 1: 3, 2: 3}, size=4), LOG: log, PAL: r.bytes(0x400), COUNT: r.bytes(4)}
    address = CALLS[op]
    if op == 'Setup':
        target, source = format_(r), format_(r)
        if source in FIXED.values() and r.random() < 0.6:   # (the 16-bit converter's targets)
            target = fmt(r, 16)
        palette = PAL if r.random() < 0.85 else 0
        regions[TARGET], regions[SOURCE] = pack(target), pack(source)
        regions[FORMATS] = pack(FIXED[FORMATS]) + pack(FIXED[FORMATS + 0x18])
        fail = r.choice([0, 0, 0, 0, 1])
        if not source[5] and target[5] and palette:   # (the index converter: never building its map)
            if kind == 3 and r.random() < 0.5:
                put(old, 0x404, '2I', PAL, TABLE)
                fail = 0
            elif kind == 3:
                put(old, 0x408, 'I', 0)
                fail = 1
            else:
                fail = r.choice([1, 2])
        regions[HEAP] = heap(fail=fail)
        regions.update(global_heap())
        return Call(address, [CONV, TARGET, SOURCE, palette, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='t')
    if op == 'Row':
        return Call(address, [CONV, DST, SRC, r.choice([0, 1, 7, 640, r.getrandbits(32)])], regions, conv='t')
    if op == 'Rect':
        src = r.choice([SRC, DST])
        return Call(address, [CONV, r.choice([0, DST, SRC]), src, r.choice([0, 64, -64 & 0xffffffff, r.getrandbits(32)]),
                              r.choice([0, 64, -64 & 0xffffffff, r.getrandbits(32)]), r.choice([0, 1, 2, 5]), r.getrandbits(32)], regions, conv='t')
    if op in ('BaseDel', 'BaseDel2'):
        regions[HEAP] = heap()
        return Call(address, [OLD, r.choice([0, 1, 1, 2, 3, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op == 'BaseDtor':
        return Call(address, [OLD], regions, conv='t')
    return Call(address, [CONV], regions, conv='t', ret='a' if op == 'Ctor' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
