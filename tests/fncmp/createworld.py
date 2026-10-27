# The world made (src/game/core/namedobjects: NamedObjects_CreateWorld 0x407d80) against the original, with all it calls: Mem_Alloc
# (the heap region, 0x1da748 bytes and a little more; the allocation failing now and then), World_Ctor (funnytime.py's World: its clock's
# imports as Init's) and World_Attach (things.py's World: the 0x2800-slot thing table any bytes). The world pointer 0x7cf83c any before.
# Heavy (about 5 s a case in the emulator): run it with a small count (40 cases a mode).
# usage: createworld.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, fill, heap, put, set_return  # noqa: E402
import funnytime  # noqa: E402
from thingenv import SLOTS  # noqa: E402

RANGES = sorted(set(funnytime.RANGES + funnytime.WORLD_RANGES + [(0x407d80, 0x407dfa), (0x515660, 0x5156a0), (0x515f30, 0x515fb0),
                                                                 (0x4d0a70, 0x4d0a90), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)]))
B = Bench(RANGES)
K = funnytime.K


def case(r):
    regions = {}
    log = funnytime.clock_regions(r, regions)
    st = bytearray(r.bytes(16))
    st[r.choice([1, 2, 3, 3, 3])] = 0   # (the string the SYSTEMTIME is hashed as ends within its year and month)
    fill(log, K['GetLocalTime'], 0, bytes(st), 1)
    fill(log, K['SystemTimeToFileTime'], 1, r.bytes(8), 0)
    if r.random() < 0.7:
        set_return(log, K['SystemTimeToFileTime'], r.choice([0, 1, 1, r.getrandbits(32)]))
    if r.random() < 0.3:
        put(log, 0xb10 + 4, 'I', r.choice([0x80000000, 0x7fffffff, 0xffffffff]))
    regions[0x80239c] = bytes(regions[0x80239c][:4]) + r.bytes(4)   # (World_Attach's world and its +0x2d8)
    regions.update({HEAP: heap(0x1da748 + 0x40, fail=1 if r.random() < 0.2 else 0), SLOTS: r.bytes(0x2800 * 20),
                    0x7cf56c: r.bytes(4), 0x801bbc: r.bytes(4), 0x7cdb30: r.bytes(0x28), 0x7cf83c: r.bytes(4)})
    return Call(0x407d80, [0x786b68], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
