# A ride's linked sound played (src/game/core/soundplay: 0x51eeb0) against the original: ridelist.py's records (0-4 of the ids 1-8, each
# linking one of them or 99; their arrays' elements the sounds, 0 now and then), the id one of them or not and the index 0-5 (now and
# then past the array); soundplay.py's sound globals and manager (its command logged as a hash of its 7 dwords), the owner, category and
# position any.
# usage: ridesound.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Play)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Play': 0x51eeb0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(0x51eeb0, 0x51eeec), (0x55a3e0, 0x55a470), (0x55a070, 0x55a0b0), (0x51bfc0, 0x51c048), (0x5da3c0, 0x5da3c1)])
NODES, ARRAYS, MANAGER, VT = RAM + 0x1000, RAM + 0x4000, RAM + 0x100, RAM + 0x200


def coord(r):
    return r.choice([0, 1, -1, r.randrange(-2000, 2000), r.getrandbits(32) - (1 << 31)]) & 0xffffffff


def case(r):
    r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 4, 4, 4, 4])
    ids = r.sample(range(1, 9), count)
    nodes, arrays = bytearray(r.bytes(0x100 * 4)), bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + ids + [99]))
        put(nodes, o + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([2, 4, 6, 6, 6, 6, 0, -1]))
        for k in range(6):
            put(arrays, 0x40 * i + 4 * k, 'I', r.choice([0, 1, 0x1d, 0x123, r.getrandbits(32)]))
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    if r.random() < 0.85:
        set_return(log, 2, r.getrandbits(32))
    regions = {NODES: nodes, ARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1, 1])),
               0x8791b0: struct.pack('<I', NODES if count else 0), LOG: log, 0x802bc0: g,
               MANAGER: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({2: 2}, 4)}
    ident = r.choice(ids + ids + [0, 99]) if ids else r.choice([0, 1, 5])
    index = r.choice([0, 1, 2, 3, 4, 5, 5, -1, 0x7fffffff])
    return Call(CALLS['Play'], [ident, r.getrandbits(32), r.getrandbits(32), index & 0xffffffff, coord(r), coord(r), coord(r)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
