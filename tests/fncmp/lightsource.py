# A light source's destruction (src/game/render/lightsource: 0x458400) against the original: lights of every kind (0-3 and others) with
# parts at +4 and +0xc or none (each a virtual base at an offset 0, 8 or 0x20 whose slot 0 is a recorder), a renderer (0x87a6d0) whose
# tag 0x70 list holds nodes keyed by those parts or not and whose free list takes the removed nodes, the ambient colour 0x879dd8, and
# MessageBoxA (m3d2.py's import recorder) for kind 0. Main (0x458580) destroys the light 0x7a0b40 points at.
# Colour (0x458890, or half the time 0x458350 on the part with an alpha): a part at an offset of 0 to 0x40 given channels about 0..1, beyond, huge, NaN or any bits.
# Scaled (0x4587e0): the same part from a light {object, colour} whose colour (three floats such channels) is scaled first.
# The light part's constructor 0x458310, its vector 0x4583d0 and its flag 0x10000000 (0x458910 / 0x458940) over any object bytes.
# Create (0x457e70): a light of every kind and colour, its block from the GlobalAlloc heap and its part from Mem_Alloc (the part failing
# now and then), a point and a direction, the renderer's tag 0x70 list empty or not (the entries' slot 8 answering 1) and its free list
# with nodes or none. MainCreate (0x4584d0): the directional light from the direction at 0x7a0b30 (normalised first: any length, none,
# huge) and the colour 0x7a0b3c, kept at 0x7a0b40. MainUpdate (0x458590): that light's direction normalised again and its colour and
# the ambient colour (0x7a0b44) set again. Slot (0x5d92e0): see slot().
# usage: lightsource.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Destroy,Main,Colour,Scaled,Create,MainCreate,MainUpdate,CtorVirtual,SetVec,MeshClear,MeshSet,Slot)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, Bench, Call, RAM, fb, global_heap, heap, import_table, log_region, put, recorder, set_return, vtable  # noqa: E402
import m3d2  # noqa: E402

B = Bench([(0x458400, 0x4584bc), (0x46f100, 0x46f11f), (0x576540, 0x5765b8), (0x458580, 0x45858f), (0x458890, 0x458909), (0x458350, 0x4583cf), (0x5836b0, 0x5836c2), (0x4587e0, 0x45888f),
           (0x458310, 0x45834d), (0x4583d0, 0x4583f6), (0x458910, 0x458935), (0x458940, 0x458965), (0x457e70, 0x458300), (0x458590, 0x4587a7), (0x45b9d0, 0x45ba15),
           (0x5d92e0, 0x5d9679), (0x67acc0, 0x67acc1),
           (0x576410, 0x576550), (0x576270, 0x576320), (0x42a030, 0x42a03b), (0x42a0d0, 0x42a0d8), (0x5d1410, 0x5d1416), (0x4584d0, 0x458573)])
LIGHT, PARTS, TABLES, RENDERER, NODES = RAM, RAM + 0x100, RAM + 0x800, RAM + 0x1000, RAM + 0x1100


def part_case(r, op):   # 0x458310 (CtorVirtual), 0x4583d0 (SetVec), 0x458910 / 0x458940 (MeshClear / MeshSet) over any object bytes
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, 0x8bd500: r.bytes(0x20), 0x8bcc40: r.bytes(8)}
    if op == 'CtorVirtual':
        put(obj, 0, 'I', RAM + 0x300)  # the vtable it starts with (when not most derived): +4 the virtual base offset
        put(obj, 0x304, 'I', r.choice([0x60, 0x60, 0x64, 0x70, 0x80, 0x68]))
        return Call(0x458310, [RAM, r.choice([0, 1, 1])], regions, conv='t', ret='a')
    if op == 'SetVec':
        return Call(0x4583d0, [RAM, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='a')
    put(obj, 0x100, 'I', RAM + 0x200)  # the light: -> the object, whose vtable (+8) gives the part's offset
    put(obj, 0x200, 'I', RAM + 0x300)
    put(obj, 0x308, 'I', r.choice([0x20, 0x40, 0x10, 0x60]))
    return Call(0x458910 if op == 'MeshClear' else 0x458940, [RAM + 0x100], regions)


def case(r):
    op = r.deal(OPS)
    if op in PART_OPS:
        return part_case(r, op)
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
    if op in ('Create', 'MainCreate'):
        return create(r, regions, renderer, nodes, op == 'MainCreate')
    if op == 'MainUpdate':   # 0x458590 (any flag, mostly set): the direction as MainCreate's, its colour and the ambient 0x7a0b44 any;
        # the light's part (+4) a virtual base at an offset 0-0x40; the numerator 0x74d0d8 mostly 1
        d = r.choice([0, 8, 0x20, 0x40])
        v = [r.choice([0.0, 1.0, -1.0, r.uniform(-2, 2), r.uniform(-1e3, 1e3)]) for _ in range(3)]
        if r.random() < 0.05:
            v = r.choice([[0.0, 0.0, 0.0], [1e30, 1e30, 0.0], [1e-30, 0.0, 0.0], [float('nan'), 1.0, 0.0]])
        block = put(bytearray(r.bytes(0x18)), 0, '3f', *v)
        put(block, 0xc, 'I', r.choice([0, 0xffffffff, 0xff808080, r.getrandbits(32), r.getrandbits(32)]))
        put(block, 0x10, 'I', LIGHT)
        regions.update({0x7a0b30: block, LIGHT: put(light, 4, 'I', PARTS), PARTS: put(bytearray(r.bytes(0xa0)), 0, 'I', TABLES),
                        TABLES: put(bytearray(r.bytes(0x10)), 8, 'i', d), 0x8bcc40: r.bytes(8),
                        0x74d0d8: put(bytearray(4), 0, 'f', r.choice([1.0, 1.0, 1.0, 0.5, 2.0, r.uniform(-3, 3)]))})
        return Call(0x458590, [r.choice([0, 1, 1, 1, 2, 0x80000000])], regions, ret='v')
    if op == 'Slot':
        return slot(r, regions, renderer, nodes, bases)
    if op == 'Main':
        regions[0x7a0b40] = put(bytearray(4), 0, 'I', LIGHT)
        return Call(0x458580, [], regions, ret='v')
    return Call(0x458400, [LIGHT], regions, ret='v')


def create(r, regions, renderer, nodes, main=False):   # 0x457e70 (kind, colour, point, direction) or 0x4584d0
    iat = bytearray(regions[IMPORT_TABLE])
    for at, v in global_heap().items():   # (GlobalAlloc / GlobalFree in the import table m3d2's recorders fill)
        iat[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = v
    regions[IMPORT_TABLE] = iat
    regions[HEAP] = heap(0x1000, r.choice([0] * 12 + [2]))
    regions[0x8bcc40] = r.bytes(8)
    entries = r.choice([0, 0, 1, 2])   # the tag 0x70 list (+0x2c): nodes {object, next} whose objects' slot 8 is a recorder
    head = 0
    for i in range(entries):
        put(nodes, 8 * i, 'II', PARTS + 0x10 * i, head)
        head = NODES + 8 * i
    put(renderer, 0x2c, 'I', head)
    free = r.choice([0, 1, 2, 2])   # the free list (+4)
    for i in range(free):
        put(nodes, 0x40 + 8 * i, 'II', r.getrandbits(32), NODES + 0x40 + 8 * (i + 1) if i + 1 < free else 0)
    put(renderer, 4, 'I', NODES + 0x40 if free else 0)
    parts = bytearray(r.bytes(0x100))
    for i in range(2):
        put(parts, 0x10 * i, 'I', TABLES)
    regions.update({PARTS: parts, TABLES: vtable({2: 0}), RENDERER: renderer, NODES: nodes,
                    RAM + 0x1800: r.bytes(0x18)})
    kind = r.choice([0, 1, 1, 2, 2, 2, 3, 3, 3, 4, r.getrandbits(32)])
    colour = r.choice([0, 0xffffffff, 0xff808080, r.getrandbits(32), r.getrandbits(32)])
    if main:
        v = [r.choice([0.0, 1.0, -1.0, r.uniform(-2, 2), r.uniform(-1e3, 1e3)]) for _ in range(3)]
        if r.random() < 0.05:
            v = r.choice([[0.0, 0.0, 0.0], [1e30, 1e30, 0.0], [1e-30, 0.0, 0.0]])
        regions[0x7a0b30] = put(put(bytearray(r.bytes(0x14)), 0, '3f', *v), 0xc, 'I', colour)
        return Call(0x4584d0, [], regions, ret='v')
    return Call(0x457e70, [kind, colour, RAM + 0x1800, RAM + 0x180c], regions, ret='a')


SLOT = RAM + 0x1900


def slot(r, regions, renderer, nodes, bases):
    """0x5d92e0 (thiscall on a slot; on, colour, direction, value): off (the slot's light (+0x20) one of Destroy's parts, or none), on with a
    light (a part at a base offset 0-0x40; a colour and a direction each given or not: directions as MainUpdate's) or on without one (create()'s
    renderer and heap, its Mem_Alloc failing now and then: a crash in both)."""
    s = bytearray(r.bytes(0x28))
    on = r.choice([0, 1, 1, 1, 0x80000000])
    v = [r.choice([0.0, 1.0, -1.0, r.uniform(-2, 2), r.uniform(-1e3, 1e3)]) for _ in range(3)]
    if r.random() < 0.05:
        v = r.choice([[0.0, 0.0, 0.0], [1e30, 1e30, 0.0], [1e-30, 0.0, 0.0], [float('nan'), 1.0, 0.0], [3e38, 3e38, 0.0]])   # (the last's length past a float)
    block = put(put(bytearray(r.bytes(0x18)), 0, '3f', *v), 0xc, 'I', r.choice([0, 0xffffffff, 0xff808080, r.getrandbits(32)]))
    colour = r.choice([RAM + 0x180c, RAM + 0x180c, 0])
    direction = r.choice([RAM + 0x1800, RAM + 0x1800, 0])
    if not on:
        light = regions[LIGHT]
        put(s, 0x20, 'I', r.choice([struct.unpack_from('<I', light, 4)[0], struct.unpack_from('<I', light, 0xc)[0], 0]))
    elif r.random() < 0.5:
        d = r.choice([0, 8, 0x20, 0x40])
        put(s, 0x20, 'I', PARTS)
        regions.update({PARTS: put(bytearray(r.bytes(0xa0)), 0, 'I', TABLES), TABLES: put(bytearray(r.bytes(0x10)), 4, 'i', d)})
    else:
        c = create(r, regions, renderer, nodes)
        regions = dict(c.regions)
        regions[HEAP] = heap(0x1000, r.choice([0] * 20 + [1]))
        put(s, 0x20, 'I', 0)
    regions[SLOT] = s
    regions[RAM + 0x1800] = block
    return Call(0x5d92e0, [SLOT, on, colour, direction, r.getrandbits(32)], regions, conv='t', ret='v')


PART_OPS = ['CtorVirtual', 'SetVec', 'MeshClear', 'MeshSet']
OPS = [k for k in os.environ.get('OPS', ','.join(['Destroy', 'Main', 'Colour', 'Scaled', 'Create', 'MainCreate', 'MainUpdate', 'Slot'] + PART_OPS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
