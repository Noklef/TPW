# The path drag's held cell (src/game/world/pathdrag: 0x52c150, 0x52c1c0) against the originals. Begin: the column 0-127 and row 0-3 mostly
# (the map id's cell, at the world [0x7cf83c] + 0x294 + id * 0x44, its handle small: its entry at the cells [0x8023a0] + 0x10ffd8 + handle *
# 0x28), the flag 0 or not; 0x5367a0 (agent B's, unported) a thiscall recorder (k 28, driver.cpp's kPathDragEnv). Finish: the held column
# and row any or -1, the held byte 0 or not; 0x536100 (agent B's, unported) a cdecl recorder (k 29) whose three calls write a column and a
# row (0-127, 0-3) through its out arguments (logged as 0), the last pair's entry ([0x7cf83c] + 0x1102b4 + id * 0x28) given the saved value.
# usage: pathdrag.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Begin,Finish)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x52c150, 0x52c1b7), (0x52c1c0, 0x52c296), (0x4d0af0, 0x4d0b10)],
          hooks={0x5367a0: _recorder(28, 2), 0x536100: _recorder(29, 5, cdecl=True)})
CALLS = {'Begin': 0x52c150, 'Finish': 0x52c1c0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WORLD, CELLS = RAM + 0x100000, RAM + 0x10000


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def case(r):
    op = r.deal(OPS)
    log = log_region()
    g = bytearray(r.bytes(0x10))   # (0x818c50: the held byte +4, the saved value +8)
    g[4] = r.choice([0, 1, 1, 1, r.getrandbits(8)])
    regions = {LOG: log, 0x7cf83c: struct.pack('<I', WORLD), 0x818c50: g}
    if op == 'Begin':
        x, y = r.randrange(128), r.choice([0, 1, 2, 3])
        handle = r.randrange(8)
        regions[WORLD + 0x294 + ident(x, y) * 0x44] = put(bytearray(r.bytes(0x44)), 0, 'H', handle)
        regions[0x8023a0] = struct.pack('<I', CELLS)
        regions[CELLS + 0x10ffd8] = r.bytes(0x28 * 8)
        regions[0x763a00] = r.bytes(8)
        return Call(0x52c150, [x, y, r.choice([0, 0, 1, r.getrandbits(32)])], regions)
    held = r.random() < 0.85
    x, y = (r.randrange(128), r.randrange(4)) if held else (r.choice([0xffffffff, r.randrange(128)]), r.choice([0xffffffff, r.randrange(4)]))
    regions[0x763a00] = struct.pack('<2I', x, y)
    xs, ys = [r.randrange(128) for _ in range(3)], [r.randrange(4) for _ in range(3)]
    strings(log, 29, 0, out=0x18)
    fill(log, 29, 3, struct.pack('<3I', *xs), which=0, records=3)
    fill(log, 29, 4, struct.pack('<3I', *ys), which=1, records=3)
    if r.random() < 0.3:
        set_return(log, 29, r.getrandbits(32))
    regions[WORLD + 0x1102b4 + ident(xs[2], ys[2]) * 0x28] = r.bytes(4)
    return Call(0x52c1c0, [], regions)


if __name__ == '__main__':
    B.main(case)
