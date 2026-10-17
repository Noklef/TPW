# The coaster types' teardown (src/game/coaster/setup: 0x430ad0) against the original: a table (0x790b08, count 0x790fe8) of 0 to 2 types,
# each with its three pieces (the first, or others, at +0x2fc / +0x300 / +0x304), blocks at +0x14, +0x78, +0x88 (records of 0x18 bytes, one
# picked by +0x8c), +0x90 to +0x98 and +0x308 (some null), c (+0x1c) with 3 to 6 entries (the static count 0x74ce10 is 3) and b (+0x18)
# holder.py's animator (its disposal run on that fixture; a second type shares it). Frees go to the GlobalFree import (global_heap).
# usage: coastertypes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402
import holder  # noqa: E402

B = Bench(sorted(set(holder.RANGES + [(0x430ad0, 0x430c95), (0x424ed0, 0x424ef0)])), hooks=holder.HOOKS)
TYPES, TABLE, COUNT = RAM + 0x40000, 0x790b08, 0x790fe8


def block(r):
    return r.choice([0, 0x5000 + 0x10 * r.randrange(16), r.getrandbits(32) | 1])


def case(r):
    regions = holder.animator_env(r)
    regions.update(global_heap())
    regions.setdefault(HEAP, heap())
    n = r.choice([0, 1, 1, 1, 1, 2])
    table = bytearray(r.bytes(0x10))
    for k in range(n):
        at = TYPES + 0x800 * k
        put(table, 4 * k, 'I', at)
        t = bytearray(r.bytes(0x310))
        pieces = [at + 0x400 + 0x30 * j for j in range(3)]
        put(t, 0x2fc, 'III', r.choice(pieces), pieces[0], r.choice(pieces))
        put(t, 0x14, 'III', block(r), holder.IANIM, at + 0x500)
        put(t, 0x78, 'I', block(r))
        put(t, 0x88, 'II', at + 0x600, r.randrange(4))
        put(t, 0x90, 'III', block(r), block(r), block(r))
        put(t, 0x308, 'I', block(r))
        regions[at] = bytes(t)
        regions[at + 0x400] = b''.join(put(bytearray(r.bytes(0x30)), 0x2c, 'I', block(r)) for _ in range(3))
        entries = r.choice([3, 3, 4, 5, 6])
        c = put(bytearray(r.bytes(0x20)), 4, 'H', entries)
        put(c, 0x10, 'I', at + 0x540)
        regions[at + 0x500] = bytes(c)
        regions[at + 0x540] = b''.join(put(bytearray(r.bytes(0x10)), 4, 'I', block(r)) for _ in range(6))
        regions[at + 0x600] = b''.join(put(bytearray(r.bytes(0x18)), 0xc, 'I', block(r)) for _ in range(4))
    regions[TABLE] = bytes(table)
    regions[COUNT] = struct.pack('<I', n)
    return Call(0x430ad0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
