# The bar mesh (src/game/gfx/barmesh: 0x477c00, 0x477cc0, 0x477d20) against the original: a
# vertex set, and the top edge's vertices and texture coordinates at the bar's level, with the
# vertex / texture-coordinate arrays apart from the bar or overlapping it (level, pointers).
# usage: barmesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x477c00, 0x477c30), (0x477cc0, 0x477d19), (0x477d20, 0x477d70)])
BAR, ARR = RAM, RAM + 0x1000
CALLS = ('Vertex', 'TopVerts', 'TopTex')


def case(r):
    op = r.choice(OPS)
    if op == 'Vertex':  # vertex i of ARR (blocks of 0x30)
        i = r.choice([r.randrange(8), r.randrange(0x40)])
        return Call(0x477c00, [ARR, i, r.f(), r.f(), r.f()], {ARR: r.bytes(0x400)})
    bar = bytearray(r.bytes(0x200))
    struct.pack_into('<I', bar, 0x120, r.f())
    for off in (0x7c, 0x84):  # apart, or inside the bar (over the level or the pointers)
        struct.pack_into('<I', bar, off, r.choice([ARR, ARR, BAR + 4 * r.randrange(0x60)]))
    regs = {BAR: bar, ARR: r.bytes(0x100)}
    return Call(0x477cc0 if op == 'TopVerts' else 0x477d20, [BAR], regs, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
