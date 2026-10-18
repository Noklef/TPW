# The mouse modes' constructors, destructors and empty methods (src/game/world/mousemode:
# 0x46c450-0x46d000) and the statics and frame step before them (0x46c070-0x46c100) against
# the originals: each on a random object (the destructors' frees through heap()); and TileAt, the pointer's tile from a position
# (0x45d560) on a map's grid of random origins, cell sizes and dimensions.
# usage: mousemode.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import struct

from fncmp import HEAP, Bench, Call, RAM, fb, heap, put  # noqa: E402

B = Bench([(0x46c450, 0x46c453), (0x46c460, 0x46c480), (0x46c580, 0x46c5c6), (0x46c650, 0x46c653),
           (0x46c6a0, 0x46c6b0), (0x46c6d0, 0x46c702), (0x46c710, 0x46c72e), (0x46cbc0, 0x46cbe1),
           (0x46ced0, 0x46ced7), (0x46cfc0, 0x46cfd0), (0x46cff0, 0x46d000), (0x46c070, 0x46c10e),
           (0x45d560, 0x45d60e), (0x4d8300, 0x4d8328)])
# op: (address, how many arguments after self)
CALLS = {'Nop4': (0x46c450, 4), 'Nop2': (0x46c650, 2), 'Dtor': (0x46ced0, 0), 'DelDtor': (0x46c460, 1),
         'DerivedDelDtor': (0x46c710, 1), 'Tile': (0x46c580, 1), 'TileKind': (0x46c5a0, 2),
         'OnlineBase': (0x46c6a0, 0), 'PlaceStaff': (0x46c6d0, 4), 'StaffDrop': (0x46cbc0, 2),
         'Aa0': (0x46cfc0, 0), 'Ad0': (0x46cff0, 0), 'NewFrame': (0x46c100, 0)}
STATICS = {'One': 0x46c070, 'Epsilon': 0x46c090, 'Tile': 0x46c0b0}  # 0x7b05a0-0x7b05d3


def case(r):
    op = r.choice(OPS)
    if op == 'TileAt':   # a map with a grid (or not): origins and cell sizes whole or odd (0, NaN), dims about 128 (0, 1 or huge)
        def num(*usual):
            return fb(r.choice(list(usual) * 3 + [0.0, -10.0, r.uniform(-50, 50), 1e10, float('nan')]))
        grid = bytearray(r.bytes(0x20))
        struct.pack_into('<4I', grid, 8, num(0.0, 0.0, 5.0), num(0.0, 0.0, -5.0), num(10.0, 10.0, 2.5), num(10.0, 10.0, 4.0))
        struct.pack_into('<2I', grid, 0x18, *(r.choice([128, 128, 128, 100, 1, 0, 200, r.getrandbits(32)]) for _ in range(2)))
        mp = put(bytearray(r.bytes(0x70)), 0x6c, 'I', r.choice([RAM + 0x200] * 9 + [0]))
        pos = [fb(r.choice([r.uniform(-30, 1400), r.uniform(-30, 1400), r.uniform(0, 1280), 0.0, 5.0, -5.0, 1279.5, 2e3, -3e9, 1e20,
                            float('nan')])) for _ in range(3)]
        dims = [r.choice([128, 128, 128, 100, 1, 0, -1, 200]) for _ in range(2)]
        regions = {0x7a0854: struct.pack('<I', r.choice([RAM + 0x100] * 9 + [0])), RAM + 0x100: mp, RAM + 0x200: grid,
                   RAM + 0x300: struct.pack('<3I', *pos), 0x7a0314: struct.pack('<i', dims[0]), 0x7a0374: struct.pack('<i', dims[1]),
                   0x7b05a0: bytearray(r.bytes(0x34))}
        return Call(0x45d560, [RAM + 0x300], regions, ret='v')
    if op in STATICS:
        return Call(STATICS[op], [], {0x7b05a0: bytearray(r.bytes(0x34))})
    at, n = CALLS[op]
    args = [r.choice([0, 1, 2, 3, r.getrandbits(32)]) for _ in range(n)]
    regions = {RAM: bytearray(r.bytes(0x40)), HEAP: heap(0x100)}
    ret = 'v' if op in ('Nop4', 'Nop2', 'Dtor', 'NewFrame') else 'a'
    return Call(at, [RAM] + args, regions, conv='t', ret=ret)


OPS_ALL = list(CALLS) + list(STATICS) + ['TileAt'] * 3
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
