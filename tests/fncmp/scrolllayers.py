# The renderer's scrolling texture layers (src/game/render/scrolllayers: 0x585f10) against the original, the texture's and the cache's calls recorders: each early way out, the time step usual, at the ends of its range, past them and a NaN, the flag that stops it, positions about -1 and 1.
# usage: scrolllayers.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, LOG, _recorder, log_region, put  # noqa: E402

KEEP, TOUCH = 1, 2
B = Bench([(0x585f10, 0x5860c6), (0x67a830, 0x67a860)], hooks={0x590f90: _recorder(KEEP, 0), 0x58c9f0: _recorder(TOUCH, 2)})
SELF, WORLD = RAM, RAM + 0x4000
OPS = ['usual', 'usual', 'usual', 'edge', 'no count', 'no first', 'no second', 'kind 4', 'flag', 'stopped']


def case(r):
    op = r.deal(OPS)
    self = bytearray(r.bytes(0x2ed0))
    count = 0 if op == 'no count' else r.choice([1, 2, 3, 4, 4, 5])
    put(self, 0x2ec8, 'I', count)
    put(self, 0x1fd0, 'I', 0 if op == 'no first' else r.getrandbits(32) | 1)
    put(self, 0x1fd8, 'I', 0 if op == 'no second' else r.getrandbits(32) | 1)
    put(self, 0x21dc, 'I', r.choice([0, 1, 3, 16]))
    for i in range(count):
        at = 0x1fa0 + 0xc * i
        put(self, at - 0x30, 'f', r.uniform(-1, 1))
        put(self, at - 0x28, 'f', r.uniform(-1, 1))
        put(self, at, 'f', r.choice([r.uniform(-1.2, 1.2), r.uniform(-30, 30), 1.0, -1.0, 0.0, float('nan')]))
        put(self, at + 8, 'f', r.choice([r.uniform(-1.2, 1.2), r.uniform(-30, 30), 1.0, -1.0, 0.0, float('nan')]))
    step = r.uniform(-1.5, 1.5) if op == 'edge' else r.uniform(0, 0.5)
    if op == 'edge':
        step = r.choice([step, 1.0, -1.0, 1.0000001, -1.0000001, float('nan'), float('inf')])
    flags = r.getrandbits(32) & r.choice([0x3ffffffb, 0x3ffffffb, 0xfffffffb])
    regions = {LOG: log_region(), SELF: self, WORLD + 0x9d98: struct.pack('<f', step), 0x8bcbac: struct.pack('<I', WORLD), 0x8c4b44: r.bytes(4), 0x8bd524: struct.pack('<I', 4 if op == 'kind 4' else r.choice([1, 1, 0, 2])),
               0x8bcbc8: struct.pack('<I', r.getrandbits(31) | (0x80000000 if op == 'flag' else 0)), 0x8bd508: struct.pack('<I', flags | (4 if op == 'stopped' else 0))}
    return Call(0x585f10, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
