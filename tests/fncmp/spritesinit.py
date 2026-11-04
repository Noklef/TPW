# The sprites set up (src/game/sprite/loading: 0x540900 Sprites_Setup) against the original: Sprites_LoadAll (cdecl k 10), TrackedArray_Alloc (k 11), Grid_AddObject (k 12) and Renderer_Add (k 13) recorders. The mode's byte 0 or not (its upper bytes any); the map's size small, about 4s, 0, negative or any; the cells' vtables' offsets.
# usage: spritesinit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, log_region, put, set_return  # noqa: E402

LOAD, ALLOC, GRID, ADD = 10, 11, 12, 13
B = Bench([(0x540900, 0x540aa9)], hooks={0x541d90: _recorder(LOAD, 1, cdecl=True), 0x58a4f0: _recorder(ALLOC, 1), 0x576620: _recorder(GRID, 6), 0x576410: _recorder(ADD, 2)})
OWNER, MAP, VTABLES, ALLOCS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400


def size(r):
    return r.choice([0, 1, 3, 4, 5, 8, 128, 256, 255, 1024, -1, -4, -5, r.getrandbits(31), -r.getrandbits(31)])


def case(r):
    log = log_region()
    vtables = bytearray(r.bytes(0x88))
    cells = bytearray(r.bytes(0x240))
    for i in range(16):
        put(vtables, 8 * i + 4, 'I', r.choice([0, 4, 8, 0xc, r.getrandbits(32)]))
        put(cells, 0x24 * i, 'I', VTABLES + 8 * i)
    put(vtables, 0x84, 'I', r.choice([0, 4, r.getrandbits(32)]))
    answers(log, ALLOC, ALLOCS)
    set_return(log, LOAD, r.choice([0, 1]))
    set_return(log, ADD, r.getrandbits(32))
    regions = {LOG: log, OWNER: put(bytearray(r.bytes(0x70)), 0x6c, 'I', MAP), MAP: put(bytearray(r.bytes(0x20)), 0x18, 'ii', size(r), size(r)), VTABLES: vtables, ALLOCS: r.bytes(0xa0), 0x7a0854: struct.pack('<I', OWNER),
               0x8751f0: cells, 0x875528: r.bytes(0x40), 0x876420: put(bytearray(r.bytes(8)), 0, 'I', VTABLES + 0x80), 0x8bcbc8: r.bytes(4), 0x763f80: r.bytes(8), 0x877348: r.bytes(0xc), 0x876b24: r.bytes(4), 0x8bd4f0: r.bytes(8), 0x8768fc: r.bytes(4),
               0x876894: r.bytes(8), 0x8762b4: r.bytes(4), 0x87614c: r.bytes(4), 0x87692c: r.bytes(4), 0x87a6d0: r.bytes(4)}
    mode = r.getrandbits(32) & r.choice([0xffffff00, 0xffffffff, 0xff, 0])
    return Call(0x540900, [mode, r.getrandbits(32)], regions, conv='c', ret='b')


if __name__ == '__main__':
    B.main(case)
