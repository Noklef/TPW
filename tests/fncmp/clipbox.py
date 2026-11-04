# A box's clip codes (src/game/render/clipcodes: 0x561a30) against the original, the projection a recorder that gives eight points: all outside one plane or not, the least z with NaNs among them.
# usage: clipbox.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fb, fill, log_region, strings, vtable  # noqa: E402

PROJECT = 1
B = Bench([(0x561a30, 0x561bb6)])
SELF, BOX, MATRIX, PROJECTION, VT = RAM, RAM + 0x40, RAM + 0x80, RAM + 0xc0, RAM + 0x100
OPS = ['mixed', 'mixed', 'mixed', 'outside', 'nan', 'flat', 'huge']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    low = [r.uniform(-300, 300) for _ in range(3)]
    size = [0.0] * 3 if op == 'flat' else [r.uniform(0, 1e30) for _ in range(3)] if op == 'huge' else [r.uniform(-20, 200) for _ in range(3)]
    box = [fb(v) for v in low + [a + b for a, b in zip(low, size)]]
    if op == 'nan' and r.random() < 0.5:
        box[r.randrange(0, 6)] = 0x7fc00000
    points = []
    for _ in range(8):
        points += [fb(r.uniform(-5, 5)), fb(r.uniform(-5, 5)), r.choice([fb(r.uniform(-5, 5)), fb(r.uniform(0, 5)), fb(0.0)]), fb(r.uniform(-5, 5))]
    if op == 'nan':
        points[4 * r.randrange(0, 8) + 2] = r.choice([0x7fc00000, 0xffc00000])
    data = struct.pack('<32I', *points)
    fill(log, PROJECT, 2, data[:0x40], which=0)
    fill(log, PROJECT, 2, data[0x40:], which=1, offset=0x40)
    strings(log, PROJECT, 0, 4)
    blob(log, PROJECT, 3, 0xffffff)
    table = bytearray(r.bytes(0x80))
    if op == 'outside':
        table = bytearray(b | 0x10 for b in table)
    regions = {LOG: log, SELF: r.bytes(8), BOX: struct.pack('<6I', *box), PROJECTION: struct.pack('<I', VT), VT: vtable({}, cdecl={PROJECT: 8}), 0x87b1d8: struct.pack('<I', PROJECTION), 0x879970: table}
    return Call(0x561a30, [SELF, BOX, MATRIX], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
