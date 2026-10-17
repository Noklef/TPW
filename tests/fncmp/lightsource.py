# A light source's destruction (src/game/render/lightsource: 0x458400) against the original: lights of every kind (0-3 and others) with
# parts at +4 and +0xc or none (each a virtual base at an offset 0, 8 or 0x20 whose slot 0 is a recorder), a renderer (0x87a6d0) whose
# tag 0x70 list holds nodes keyed by those parts or not and whose free list takes the removed nodes, the ambient colour 0x879dd8, and
# MessageBoxA (m3d2.py's import recorder) for kind 0. Main (0x458580) destroys the light 0x7a0b40 points at.
# usage: lightsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Destroy,Main)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, import_table, log_region, put, recorder, set_return  # noqa: E402
import m3d2  # noqa: E402

B = Bench([(0x458400, 0x4584bc), (0x46f100, 0x46f11f), (0x576540, 0x5765b8), (0x458580, 0x45858f)])
LIGHT, PARTS, TABLES, RENDERER, NODES = RAM, RAM + 0x100, RAM + 0x800, RAM + 0x1000, RAM + 0x1100


def case(r):
    log = log_region()
    set_return(log, m3d2.K_BOX, r.choice([1, 2]))
    light = bytearray(r.bytes(0x10))
    put(light, 0, 'I', r.choice([0, 1, 1, 2, 2, 3, 3, 4, 0xffffffff, r.getrandbits(32)]))
    parts = bytearray(r.bytes(0x200))
    tables = bytearray(0x20)
    bases = []
    for k, off in enumerate((4, 0xc)):
        if r.random() < 0.2:
            put(light, off, 'I', 0)
            continue
        d = r.choice([0, 8, 0x20])
        put(tables, 0x10 * k, 'II', recorder(0, 1), d)
        put(parts, 0x100 * k, 'I', TABLES + 0x10 * k)
        put(parts, 0x100 * k + d, 'I', TABLES + 0x10 * k)
        put(light, off, 'I', PARTS + 0x100 * k)
        bases.append(PARTS + 0x100 * k + d)
    # the renderer: the tag 0x70 list (+0x2c) of nodes {key, next}, the free list (+4) and the count (+0xc)
    renderer = bytearray(r.bytes(0x40))
    nodes = bytearray(r.bytes(0x80))
    head = 0
    for i in range(r.choice([0, 1, 2, 3, 4])):
        put(nodes, 8 * i, 'II', r.choice(bases + [r.getrandbits(32), 0]) if bases else r.getrandbits(32), head)
        head = NODES + 8 * i
    put(renderer, 0x2c, 'I', head)
    put(renderer, 4, 'I', r.choice([0, NODES + 0x40]))
    put(renderer, 0xc, 'I', r.randrange(8))
    regions = {LOG: log, IMPORT_TABLE: import_table(log, m3d2.IMPORTS), LIGHT: light, PARTS: parts, TABLES: tables, RENDERER: renderer,
               NODES: nodes, 0x87a6d0: put(bytearray(4), 0, 'I', RENDERER), 0x8bd508: r.bytes(8), 0x879dd8: r.bytes(0x10)}
    if r.deal(OPS) == 'Main':
        regions[0x7a0b40] = put(bytearray(4), 0, 'I', LIGHT)
        return Call(0x458580, [], regions, ret='v')
    return Call(0x458400, [LIGHT], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Destroy,Main').split(',') if k]


if __name__ == '__main__':
    B.main(case)
