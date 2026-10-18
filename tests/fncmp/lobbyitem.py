# The lobby class 0x702af0's constructor (src/game/frontend/lobbyscene: 0x5d9db0) against the original: counts around the clamps of
# its share (count * the percentage at 0x7858dc / 100, at least 1 and at most the count, the product wrapping now and then), names of up to
# 12 chars or none for its two strings (the game's String class, gamestring.py's ranges), two 12-byte triples and two dwords; the list's
# allocation failing now and then (fncmp's heap); the registry at [0xf83784] a recorder (slot 6).
# usage: lobbyitem.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x5d9db0, 0x5d9ed0), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
OBJ, NAMES, TRIPLES, REG, RVT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400


def name(r):
    return bytes(r.choice(b'abAz') for _ in range(r.randrange(13))) + b'\0'


def case(r):
    count = r.choice([0, 1, 2, 3, 5, 10, 99, 100, 101, 1000, -1, -100, 0x7fffffff, r.getrandbits(32)])
    pct = r.choice([0, 1, 10, 50, 99, 100, 101, 200, -1, -50, 0x7fffffff, r.getrandbits(32)])
    names = bytearray(0x40)
    a, b = name(r), name(r)
    names[0:len(a)] = a
    names[0x20:0x20 + len(b)] = b
    regions = {OBJ: r.bytes(0x60), NAMES: names, TRIPLES: r.bytes(0x18), HEAP: heap(0x400, r.choice([0, 0, 0, 1])), LOG: log_region(),
               0x7858dc: struct.pack('<I', pct & 0xffffffff), 0xf83784: struct.pack('<I', REG), REG: struct.pack('<I', RVT), RVT: vtable({6: 1})}
    first = r.choice([TRIPLES, TRIPLES + 0xc, TRIPLES + 4])
    second = r.choice([TRIPLES, TRIPLES + 0xc, TRIPLES + 8])
    return Call(0x5d9db0, [OBJ, count & 0xffffffff, NAMES, NAMES + 0x20, first, second, r.getrandbits(32), r.getrandbits(32)], regions,
                conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
