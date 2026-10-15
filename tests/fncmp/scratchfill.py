# The view's node vertices put in the world (src/game/model/scratch: 0x450f20) against the
# original: a main node and big and small parts with parent chains (ending at the root or
# not), flagged or not, at each view angle, main only or all.
# Also 0x450ea0, which makes a view current and fills for it.
# usage: scratchfill.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Fill,Begin to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x450ea0, 0x450f14), (0x450f20, 0x45119e)])
VIEW, MODEL, ROOT, HOLDER, INST, FLAGS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400
BIG, SMALL, MAIN, VERTS, OUT = RAM + 0x1000, RAM + 0x1400, RAM + 0x1900, RAM + 0x2000, RAM + 0x3400


def case(r):
    ram = bytearray(r.bytes(0x4000))
    nodes = [MAIN] + [BIG + 0xa0 * i for i in range(6)] + [SMALL + 0x58 * i for i in range(12)]
    scale = r.choice([1, 100, None])
    for j, node in enumerate(nodes):
        o = node - RAM
        parent = r.choice([0, ROOT, ROOT] + nodes[:j] * (j > 0))
        put(ram, o + 4, 'I', parent)
        r.floats(ram, range(o + 0x40, o + 0x4c, 4), scale)
        put(ram, o + 0x58, 'H', r.choice([0, 1, 4, 9]))
        put(ram, o + 0x60, 'I', VERTS + 0x100 * j)
    r.floats(ram, range(VERTS - RAM, VERTS - RAM + 0x1300, 4), scale)
    put(ram, ROOT - RAM + 4, 'I', 0)
    r.floats(ram, range(ROOT - RAM + 0x40, ROOT - RAM + 0x4c, 4), scale)
    put(ram, 4, 'I', MODEL)
    put(ram, 0x10, 'i', r.choice([0, 90, 180, 270, 45, -90, 360]))
    put(ram, 0x28, 'I', HOLDER)
    put(ram, HOLDER - RAM + 4, 'I', INST)
    put(ram, 0x2c, 'I', r.choice([MAIN, MAIN, MAIN, 0]))
    m = MODEL - RAM
    put(ram, m + 0x44, 'HHH', r.randrange(7), r.randrange(5), r.choice([0, 1, 3, 5]))
    put(ram, m + 0x70, 'II', BIG, SMALL)
    put(ram, m + 0x78, 'II', ROOT, FLAGS)
    for i in range(5):
        put(ram, FLAGS - RAM + 0x14 * i, 'B', r.getrandbits(8) | r.choice([2, 2, 0]))
    regions = {RAM: ram, 0x79f7e0: struct.pack('<I', OUT)}
    if r.choice(OPS) == 'Fill':
        return Call(0x450f20, [VIEW, r.choice([0, 0, 1, 7])], regions)
    # Begin: the holder flagged 8 or not (or none), the model's +0x30 bit 0x80, +0xa0 0 or not
    put(ram, HOLDER - RAM, 'B', r.getrandbits(8) | r.choice([8, 0, 0]))
    put(ram, 0x28, 'I', r.choice([HOLDER, HOLDER, 0]))
    put(ram, m + 0x30, 'B', r.getrandbits(8) & r.choice([0x7f, 0x7f, 0xff]))
    put(ram, m + 0xa0, 'I', r.choice([0, 0x80000000, 0x7fc00000, r.f()]))
    regions.update({0x79fb18: bytearray(r.bytes(4)), 0x79fb10: bytearray(r.bytes(4))})
    return Call(0x450ea0, [VIEW], regions)


OPS = [k for k in os.environ.get('OPS', 'Fill,Begin').split(',') if k]


if __name__ == '__main__':
    B.main(case)
