# The cell steps of src/game/world/mapcell (Map_StepCell 0x4d96f0, Map_StepCellInDirection 0x4d97e0) against the originals: cell ids over the map and its edges, offsets
# and scales either side of zero, and the eight direction pairs (small or large values) with other direction values.
# usage: mapstep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Step,Dir)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Dir28': 0x50c3e0, 'Step28': 0x50c320, 'Step': 0x4d96f0, 'Dir': 0x4d97e0, 'Spread': 0x4d8480, 'SpreadAdd': 0x4d8440, 'SpreadRemove': 0x4d8460, 'Init': 0x4d7bb0, 'Facing': 0x4de670}
B = Bench([(0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1), (0x4d8440, 0x4d8480), (0x4d8480, 0x4d8620), (0x4d7bb0, 0x4d7ea0), (0x4de670, 0x4de7e0), (0x50c320, 0x50c3e0), (0x50c3e0, 0x50c540), (0x536300, 0x53644c), (0x522850, 0x522860)])
CELL = RAM


def case(r):
    name = r.choice(OPS)
    if name == 'Facing':
        world = RAM + 0x10000
        cells = bytearray(r.bytes(0x44 * 0x200))
        for idx in range(0x200):
            struct.pack_into('<H', cells, 0x44 * idx, idx)
            struct.pack_into('<I', cells, 0x44 * idx + 8, r.choice([3, 3, 9, 0, 1, 3]))
            cells[0x44 * idx + 0xd] = r.choice([0x10, 1, 0x40, 4, 0x10, 1, 0x40, 4, 0x80, 2])
        cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x82, 0x100, 0x101, r.randrange(1, 0x100)])
        return Call(CALLS[name], [RAM + 0x2000, cell], {0x7cf83c: struct.pack('<I', world), 0x80239c: struct.pack('<I', world), world + 0x294: cells, RAM + 0x2000: r.bytes(8)}, ret='a')
    if name == 'Init':
        world = RAM + 0x10000
        return Call(CALLS[name], [world], {world: r.bytes(0x1d9180), 0x785610: r.bytes(0xc0)}, conv='t', ret='v')
    if name in ('Spread', 'SpreadAdd', 'SpreadRemove'):
        world = RAM + 0x10000
        records = bytearray(r.bytes(12 * 4))
        for k in range(4):
            records[12 * k + 0xa] = r.choice([0, 1, 2, 3, 3])
            struct.pack_into('<5h', records, 12 * k, *[r.choice([0, 1, -1, 100, -100, 0x7fff, -0x8000, r.randrange(-300, 300)]) for _ in range(5)])
        re_cells = bytearray(r.bytes(10 * 0x302))
        cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x100, 0x14a, r.randrange(1, 0x100), 0xfe])
        regions = {world + 0x1d9104: records, world + 0x1b1104: re_cells}
        return Call(CALLS[name], [world] + ([r.choice([0, 1, 2])] if name == 'Spread' else []) + [r.randrange(4), cell], regions, conv='t', ret='v')
    cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x4000, 0x3f81, 0x2000, r.randrange(1, 0x4001), 0, 0x4001, r.getrandbits(16)])
    regions = {CELL: struct.pack('<I', cell), 0x80239c: struct.pack('<I', RAM + 0x1000 + 4 * r.randrange(0x100))}
    small = lambda: r.choice([0, 1, -1, 2, -2, 3, 127, -127, 128, r.randrange(-5, 6), r.getrandbits(32) - (1 << 31)]) & 0xffffffff  # noqa: E731
    if name in ('Step', 'Step28'):
        return Call(CALLS[name], [CELL, small(), small(), r.choice([1, 1, 0, 2, 3, -1 & 0xffffffff, small()])], regions, conv='t', ret='a')
    pairs = bytearray(0x40)
    scale = r.choice([1, 1, 2, 0, 3, -1 & 0xffffffff])
    for i in range(0, 0x40, 4):
        struct.pack_into('<I', pairs, i, r.choice([0, 1, 0xffffffff, 2, 0xfffffffe, 0x7f, r.getrandbits(8)]))
    regions[0x7cdba0] = pairs
    return Call(CALLS[name], [CELL, r.choice([1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0, 3, 0x100, 0x101, 0xff, r.getrandbits(32)])] + ([scale] if name == 'Dir28' else []), regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(k for k in CALLS if k != 'Init')).split(',') if k]

if __name__ == '__main__':
    B.main(case)
