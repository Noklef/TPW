# A visitor pays and visits an object against the original.
#   Takings (0x4e16b0 Object_TakePayment, object.cpp's): object_checks.py's Pay with definitions of +0x4ac, +0x164 and +0x158 about their cases and a selected slot of
#   every id they name;
#   Pay (0x4fe1a0 Visitor_PayObject), Visit (0x4fd970 Visitor_VisitObject, person.cpp's): object_checks.py's Use with +0xe8 and +0x164 of the definitions, the park
#   analyser's rings sized 1, 2 or 0x30, the two records, the byte +0x1f1 and the float +0x20c. Thoughts_Show either stops at [0x790ab0] or at the mode 1 with force 0
#   (thoughtshow.py compares what follows).
# usage: objectvisit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import object_checks as oc  # noqa: E402
from fncmp import Bench, RAM, put  # noqa: E402

B = Bench(oc.RANGES + [(0x4fd970, 0x4fdc60), (0x4fe1a0, 0x4fe1e0), (0x4e1690, 0x4e18f0), (0x4d0190, 0x4d01f0), (0x4e1e00, 0x4e1e30), (0x50be80, 0x50c075)])
PARK = RAM + 0x40000


def amount_regions(r, regions):
    """the park record's two sums and the analyser's rings"""
    rings = bytearray(r.bytes(0x500))
    for k in range(0x14):
        size = r.choice([1, 2, 0x30])
        put(rings, k * 0x40 + 0x38, 'i', size)
        put(rings, k * 0x40 + 0x34, 'i', r.randrange(size))
    regions.update({PARK + 0x20130: r.bytes(4), PARK + 0x20380: r.bytes(4), PARK + 0x216fc: rings})


def takings(r):
    c = oc.pay(r, {0x4ac: [0, 1, 1, 1, 1, 2, 3], 0x164: [0, 1, 2, 3, 4, 5, 6, 7], 0x158: [0, 1, 2, 3, 4, 5]}, (0xc, 0xd, 1, 2, 3, 4, 5, 6, 7, 8))
    amount_regions(r, c.regions)
    # mostly the selected slot the id the object's definition names, or the id's neighbour
    regions = c.regions
    obj, items = regions[oc.OBJ], regions[oc.ITEMS]
    ident = struct.unpack_from('<H', obj, 0xe)[0]
    at = next((i * oc.ITEM for i in range(oc.NITEMS) if struct.unpack_from('<H', items, i * oc.ITEM)[0] == ident), 0)
    kind, by164, by158 = struct.unpack_from('<I', items, at + 0x4ac)[0], *struct.unpack_from('<I', items, at + 0x164), *struct.unpack_from('<I', items, at + 0x158)
    want = {1: 5, 3: 8, 5: 7, 6: 6}.get(by164, {1: 2, 2: 1, 3: 3, 4: 4}.get(by158, 0xc)) if kind == 1 else 0xd
    slots = regions[RAM + 0x4000]
    index = struct.unpack_from('<i', slots, 0x3cc)[0]
    if 0 <= index < 20 and r.random() < 0.7:
        put(slots, 0xc + 48 * index, 'I', want if r.random() < 0.8 else want + 1)
    c.address = 0x4e16b0
    return c


def visit_env(r):
    c = oc.use(r)
    regions = c.regions
    items = bytearray(regions[oc.ITEMS])
    for i in range(oc.NITEMS):
        put(items, i * oc.ITEM + 0xe8, 'i', r.choice([0, 1, 5, 20, 100, -5, 0x7fffffff, -0x80000000]))
        put(items, i * oc.ITEM + 0x164, 'I', r.choice([0, 1, 2, 3, 4, 5, 6, 7]))
    regions[oc.ITEMS] = items
    amount_regions(r, regions)
    visitor = bytearray(regions[oc.USE_VISITOR])
    visitor[0x1f1] = r.choice([0, 1, 1, 1, 7])
    put(visitor, 0x20c, 'I', oc.use_stat(r))
    put(visitor, 0x1b8, 'I', oc.use_stat(r))
    # the sprite's script there and the list ready: Sprite_PlaySound's position is an uninitialised stack word otherwise
    scripts = bytearray(regions[0x7b49f0])
    table = bytearray(regions[oc.USE_TABLE])
    nslots = max(2, struct.unpack_from('<i', scripts, 4)[0])
    table += bytes(4 * nslots - len(table))
    put(scripts, 4, 'i', nslots)
    put(scripts, 0x24, 'I', 1)
    slot = r.randrange(1, nslots)
    put(table, 4 * slot, 'I', oc.USE_SCRIPTS + 0x120 * r.randrange(3))
    put(visitor, 0xc, 'I', slot)
    regions.update({0x7b49f0: scripts, oc.USE_TABLE: table})
    regions[oc.USE_VISITOR] = visitor
    quiet = r.choice([2, 4, 0x10, 0x16, 0])
    regions[0x790ab0] = bytes([quiet])
    regions[0xfb34aa] = b'\x01'
    if quiet == 0:
        regions[0xfb3b7c] = struct.pack('<I', 1)
    return c


def pay_object(r):
    c = visit_env(r)
    c.address = 0x4fe1a0
    return c


def visit(r):
    c = visit_env(r)
    c.address = 0x4fd970
    return c


CASES = {'Takings': takings, 'Pay': pay_object, 'Visit': visit}
OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]


def case(r):
    return CASES[r.deal(OPS)](r)


if __name__ == '__main__':
    B.main(case)
