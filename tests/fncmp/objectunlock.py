# An object definition's availability (src/game/world/objectunlock: 0x4d4b90) against the original: globalsave.py's count() set-up for
# the count left (the instance [0xf7c58c] made: its info), the object definitions 0x7890a0 (1-3 of 0x100 bytes, the cache hit or not,
# the id now and then none of them: a crash in both), the definition's +0xc4 about the count or any (signed both ways).
# usage: objectunlock.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=IsUnlocked)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import globalsave  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402

CALLS = {'IsUnlocked': 0x4d4b90}
B = Bench([(0x5af7f0, 0x5af80b), (0x5afc30, 0x5afc5a), (0x5b09a0, 0x5b09b4), (0x5b0ca0, 0x5b0cc5), (0x5b0940, 0x5b0996), (0x67b0d0, 0x67b10f),
           (0x5aef50, 0x5af0dc), (0x409410, 0x40947a), (0x5b3110, 0x5b3130), (0x5b2fd0, 0x5b2ff0), (0x5b1680, 0x5b16a6),
           (0x5af4a0, 0x5af52a), (0x5af530, 0x5af5cf), (0x5af5d0, 0x5af674), (0x5af680, 0x5af734), (0x5b11c0, 0x5b12d3), (0x5b2480, 0x5b2530),
           (0x5b11a0, 0x5b11b9), (0x4d4b90, 0x4d4bbb), (0x412e90, 0x412eea)],
          hooks={0x691de0: globalsave.lock, 0x691ea0: lambda e: None})   # (globalsave.py's ranges and STL lock stand-ins)
DEFS = RAM + 0x8000


def case(r):
    op = r.deal(OPS)
    c = globalsave.count(r, 'CountLeft')
    regions = dict(c.regions)
    regions[0xf7c58c] = struct.pack('<I', globalsave.INFO)
    table = bytearray(r.bytes(0x740))
    n = r.randrange(1, 4)
    ids = r.sample(range(0x10000), n)
    for i in range(n):
        put(table, 4 * i, 'I', DEFS + 0x100 * i)
        d = put(bytearray(r.bytes(0x100)), 0, 'H', ids[i])
        put(d, 0xc4, 'i', r.choice([0, 1, 2, 3, 4, 6, 8, -1, r.getrandbits(31), -r.getrandbits(31)]))
        regions[DEFS + 0x100 * i] = d
    put(table, 0x528, 'i', n)
    k = r.randrange(n)
    put(table, 0x730, 'HHI', r.choice([ids[k], r.getrandbits(16)]), 0, DEFS + 0x100 * k)
    regions[0x7890a0] = table
    want = ids[k] if r.random() < 0.97 else r.getrandbits(16)
    return Call(CALLS[op], [want | r.choice([0, r.getrandbits(16)]) << 16], regions, conv='s', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
