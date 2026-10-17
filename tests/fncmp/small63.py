# The thing constructor of src/game/core/small63 (0x50afe0) against the original: things.py's table of slots (free or taken) as the world, a cell array whose chain heads are empty
# (the cell's first thing is set), cell ids over the first rows and kinds of every byte value.
# usage: small63.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import things  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d91f0, 0x4d9290), (0x50afe0, 0x50b090), (0x516270, 0x516320), (0x50b350, 0x50b360), (0x5da3c0, 0x5da3c1)])
CELLS, THING = RAM + 0x6000, RAM + 0x5000


def case(r):
    things.OPS = ['Add']
    c = things.case(r)
    regions = dict(c.regions)
    cells = bytearray(r.bytes(0x44 * 0x41))
    for k in range(0x41):
        put(cells, 0x44 * k + 0x24, 'H', 0)
    k = r.choice([3, 7])  # the free slot (id k), else the handle is a random word and the cell chain write faults in both
    regions[0x801bbc] = struct.pack('<I', things.SLOTS + 20 * k)
    regions[things.SLOTS + 20 * k] = struct.pack('<IIIII', 0, k, 0, 0, 0)
    regions.update({CELLS: cells, 0x8023a0: struct.pack('<I', CELLS), 0x80239c: struct.pack('<I', RAM), THING: r.bytes(0x20)})
    cell = r.choice([1, 2, 3, 0x40, 0x41, 0x20, r.randrange(1, 0x41)]) | r.choice([0, 0, 0x10000])
    return Call(0x50afe0, [THING, cell, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
