# Screen_Init (src/game/render/screen: 0x449ec0) and the unused static initialisers 0x44a3d0 /
# 0x44a3f0 (render/mode) against the original: the search path copy, the screen size from the
# option flags, Resolution.sam or 0x78d8dc, its centre, then Frame_ResetBounds and
# Quickload_Init (GlobalMem_Alloc from global_heap()). Screen_Shutdown calls the unhooked sine
# table builder by address, which the driver cannot.
# usage: screen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x449ec0, 0x44a1d0), (0x44a3d0, 0x44a403), (0x44e570, 0x44e5ad), (0x45b9d0, 0x45ba15),
           (0x461790, 0x4617d1)])
TEXT = RAM
OPTION_BITS = (1, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x20000000, 0x40000000, 0x80000000)


def word(v):
    return bytearray(struct.pack('<I', v & 0xffffffff))


def case(r):
    op = r.choice(OPS)
    if op == 'One':
        return Call(0x44a3d0, [], {0x79d9c4: bytearray(r.bytes(4))})
    if op == 'Epsilon':
        return Call(0x44a3f0, [], {0x79d9cc: bytearray(r.bytes(4))})
    n = r.choice([0, 1, 5, r.randrange(1, 300)])
    text = bytearray(bytes(r.randrange(1, 256) for _ in range(n)) + b'\0' + r.bytes(15))
    length = r.choice([n, n, n, r.randrange(n + 1), r.randrange(0x400)])
    options = r.getrandbits(32) & ~sum(OPTION_BITS)
    for bit in OPTION_BITS:
        if r.random() < 0.25:
            options |= bit
    width = r.choice([0x280, r.getrandbits(32), r.randrange(1 << 25), 0xffffffff, 0x80000001])
    height = r.choice([0x1e0, r.getrandbits(32), r.randrange(1 << 25)])
    regions = {
        TEXT: text,
        0x79cb4c: bytearray(r.bytes(4)), 0x79cb6c: bytearray(struct.pack('<II', length, TEXT)),
        0x79d25c: bytearray(r.bytes(4)), 0x79d73c: bytearray(r.bytes(4)),
        0x78d968: word(r.choice([1, 1, 0, 2, r.getrandbits(32)])),
        0x7858ec: word(r.choice([r.randrange(-1, 9), r.randrange(-1, 9), 0, r.getrandbits(32)])),
        0x78d8dc: word(r.choice([0, 1, 2, r.randrange(-1, 4), r.getrandbits(32)])),
        0x7a1a8c: word(options), 0x7a1968: word(width), 0x7a1930: word(height), 0x7a1a14: bytearray(r.bytes(4)),
        0x7a0c9c: bytearray(r.bytes(4)), 0x7a162c: bytearray(r.bytes(4)), 0x7a192c: bytearray(r.bytes(4)),
        0x7a194c: bytearray(r.bytes(4)), 0x7b0600: bytearray(r.bytes(0x18)), 0x7a3d3c: bytearray(r.bytes(4)),
        0x7a45dc: bytearray(r.bytes(4)), 0x7a3e5c: bytearray(r.bytes(4)), HEAP: heap(0x1000), **global_heap()}
    return Call(0x449ec0, [], regions)


OPS_ALL = ['Init'] * 8 + ['One', 'Epsilon']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
