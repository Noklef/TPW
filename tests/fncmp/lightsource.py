# A light source's destruction (src/game/render/lightsource: 0x458400) against the original: lights of every kind (0-3 and others) with
# parts at +4 and +0xc or none (each a virtual base at an offset 0, 8 or 0x20 whose slot 0 is a recorder), a renderer (0x87a6d0) whose
# tag 0x70 list holds nodes keyed by those parts or not and whose free list takes the removed nodes, the ambient colour 0x879dd8, and
# MessageBoxA (m3d2.py's import recorder) for kind 0. Main (0x458580) destroys the light 0x7a0b40 points at.
# Colour (0x458890, or half the time 0x458350 on the part with an alpha): a part at an offset of 0 to 0x40 given channels about 0..1, beyond, huge, NaN or any bits.
# Scaled (0x4587e0): the same part from a light {object, colour} whose colour (three floats such channels) is scaled first.
# usage: lightsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Destroy,Main,Colour,Scaled)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, RAM, fb, import_table, log_region, put, recorder, set_return  # noqa: E402
import m3d2  # noqa: E402

B = Bench([(0x458400, 0x4584bc), (0x46f100, 0x46f11f), (0x576540, 0x5765b8), (0x458580, 0x45858f), (0x458890, 0x458909), (0x458350, 0x4583cf), (0x5836b0, 0x5836c2), (0x4587e0, 0x45888f)])
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
    op = r.deal(OPS)
    if op == 'Scaled':   # Colour's part from {object, colour}; a scale about 0..1, beyond, huge or not a number
        d = r.choice([0, 8, 0x20, 0x40])
        obj = put(bytearray(r.bytes(0xa0)), 0, 'I', TABLES)
        nan = lambda: r.choice([0x7fc00000, 0xffc00000, 0x7fc00000, 0xffc00000,   # (ties: only the sign differs, the operand order shows)
                                r.choice([0x7fc00000, 0x7f800000, 0xff800000]) | r.getrandbits(22) | 1])  # noqa: E731  (quiet or signalling, either sign)
        chans = [fb(r.choice([0.0, 1.0, 0.5, r.random(), r.random(), r.uniform(-1, 2), 1e10, -1e10, 1e-30, float('nan')])) if r.random() < 0.8 else
                 nan() if r.random() < 0.5 else r.getrandbits(32) for _ in range(3)]
        regions.update({TABLES: put(bytearray(r.bytes(0x10)), 8, 'i', d), PARTS: obj,
                        RAM + 0x1800: put(put(bytearray(0x20), 0, 'I', PARTS), 4, 'I', RAM + 0x1810)})
        put(regions[RAM + 0x1800], 0x10, '3I', *chans)
        scale = fb(r.choice([1.0, 0.5, r.random(), 2.0, 255.0, 1e30, -1.0, 0.0, float('nan')])) if r.random() < 0.8 else nan() if r.random() < 0.5 else r.getrandbits(32)
        return Call(0x4587e0, [RAM + 0x1800, scale], regions, ret='v')
    if op == 'Colour':   # a part at an offset of the object (its vtable's +8); channels about 0..1, beyond, huge or not numbers
        d = r.choice([0, 8, 0x20, 0x40])
        obj = put(bytearray(r.bytes(0xa0)), 0, 'I', TABLES)
        regions.update({TABLES: put(bytearray(r.bytes(0x10)), 8, 'i', d), PARTS: obj, RAM + 0x1800: put(bytearray(4), 0, 'I', PARTS)})
        chans = [fb(r.choice([0.0, 1.0, 0.5, r.random(), r.random(), r.uniform(-1, 2), 1e10, -1e10, float('nan')])) if r.random() < 0.9 else r.getrandbits(32)
                 for _ in range(3)]
        if r.random() < 0.5:   # (0x458350: the part itself, alpha first)
            return Call(0x458350, [PARTS + d, fb(r.choice([0.0, 1.0, 0.5, r.random(), 2.0, float('nan')]))] + chans, regions, conv='t', ret='v')
        return Call(0x458890, [RAM + 0x1800] + chans, regions, ret='v')
    if op == 'Main':
        regions[0x7a0b40] = put(bytearray(4), 0, 'I', LIGHT)
        return Call(0x458580, [], regions, ret='v')
    return Call(0x458400, [LIGHT], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Destroy,Main,Colour,Scaled').split(',') if k]


if __name__ == '__main__':
    B.main(case)
