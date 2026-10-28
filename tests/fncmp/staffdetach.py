# The staff kinds' deletes (src/game/world/staff: Mechanic_Detach 0x4d9fe0, Handyman_Detach 0x4d6c90, Entertainer_Detach 0x4d4620; things:
# ThingKinds9To11_Detach 0x4c9040) against the originals, in thingdetach.py's surroundings of Staff_Detach (its park, the thing's map cell
# chain, the message centre, the person's scripts), the thing's column 0-127 and row 0-119. The kind's list in the game object ([0x80239c]:
# heads +0x1da73e / +0x1da73c / +0x1da740, the things' +0x210 words) of some of the park's things, the thing in it mostly. Mechanic /
# Handyman: the object handle (+0x218 / +0x21a) 0 or one of the park's (now and then a free one, whose slot has no thing), that object's
# tracked handle +0x5e (0, the staff member's or another's) and its stamp +0x60 about the game object's time +0x1da70c less 100. Entertainer:
# the map id it last recorded (+0xcc) near its cell, anywhere on the map or 0; spread record 0 of [0x8023a0] (radius 0-3, five values small,
# extreme or any) and the cells around that id. Kinds9: kinds 9-0xe, 0x10 and 0x11.
# usage: staffdetach.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mechanic,Handyman,Entertainer,Kinds9)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import thingdetach as td  # noqa: E402
from fncmp import RAM, Bench, Call, _recorder, put  # noqa: E402
from thingenv import SLOTS, THING  # noqa: E402

B = Bench(td.RANGES + [(0x4d9fe0, 0x4da0b4), (0x4d6c90, 0x4d6cee), (0x4d4620, 0x4d465b), (0x4c9040, 0x4c9046), (0x519690, 0x5197d0),
                       (0x519800, 0x519940), (0x519970, 0x519ab0), (0x4e0210, 0x4e02f0), (0x506580, 0x5065e0), (0x4f9460, 0x4f9490),
                       (0x4d8460, 0x4d8480), (0x4d8480, 0x4d8620)],
          hooks={0x463c80: _recorder(31, 1, cdecl=True), 0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
# name: (address, kind, list head)
CALLS = {'Mechanic': (0x4d9fe0, 4, 0x1da73e), 'Handyman': (0x4d6c90, 5, 0x1da73c), 'Entertainer': (0x4d4620, 6, 0x1da740), 'Kinds9': (0x4c9040, None, None)}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WORLD = RAM + 0x62000   # the game object's +0x1da708..+0x1da748
VALUES = [0, 1, -1, 100, -100, 0x7fff, -0x8000]


def spread(r, regions, base, cell):
    """Spread record 0 of the map at base and the cells around the map id cell."""
    source = bytearray(r.bytes(12))
    radius = r.choice([0, 1, 2, 3, 3])
    source[0xa] = radius
    struct.pack_into('<5h', source, 0, *[r.choice(VALUES + [r.randrange(-300, 300)]) for _ in range(5)])
    regions[base + 0x1d9104] = source
    at = (cell - 1) & 0xffff
    cx, cy = at & 0x7f, at >> 7
    xmin, xmax = max(cx - radius, 0), min(cx + radius, 0x7f)
    for y in range(max(cy - radius, 0), min(cy + radius, 0x7f) + 1):
        if xmin <= xmax:
            regions[base + 0x1b1104 + ((y << 7) + xmin) * 10] = r.bytes((xmax - xmin + 1) * 10)


def case(r):
    op = r.deal(OPS)
    address, kind, head = CALLS[op]
    td.OPS = ['Staff']
    c = td.case(r)
    thing, regions = c.args[0], c.regions
    pool, slots = bytearray(regions[td.POOL]), regions[SLOTS]
    t = thing - td.POOL
    things = {}
    for h in range(1, 12):
        ptr = struct.unpack_from('<I', slots, 20 * h)[0]
        if td.POOL <= ptr < td.POOL + 6 * THING:
            things[h] = ptr - td.POOL
    handles = sorted(things)
    free = [h for h in range(1, 12) if h not in things]
    pool[t + 2] = kind or r.choice([9, 0xa, 0xb, 0xc, 0xd, 0xe, 0x10, 0x11])
    col, row = r.randrange(128), r.randrange(120)
    pool[t + 5], pool[t + 7] = col, row
    ident = (row << 7) + col + 1
    base = (td.CELL - ident * 0x44 + 0x44) & 0xffffffff
    regions[0x8023a0] = struct.pack('<I', base)
    world = bytearray(r.bytes(0x40))
    time = r.choice([0, 50, 100, 101, 1000, r.getrandbits(32)])
    put(world, 4, 'I', time)
    regions[WORLD] = world
    regions[0x80239c] = struct.pack('<I', WORLD - 0x1da708)
    if head:
        mine = struct.unpack_from('<H', pool, t)[0]
        order = r.sample(handles, r.randrange(0, len(handles) + 1))
        if mine not in order and r.random() < 0.85:
            order.insert(r.randrange(len(order) + 1), mine)
        for k, h in enumerate(order):
            put(pool, things[h] + 0x210, 'H', order[k + 1] if k + 1 < len(order) else 0)
        put(world, head - 0x1da708, 'H', order[0] if order else 0)
    if op in ('Mechanic', 'Handyman'):
        obj = r.choice([0, 0] + handles * 3 + (free[:1] if r.random() < 0.3 else []))
        put(pool, t + (0x218 if op == 'Mechanic' else 0x21a), 'H', obj)
        if obj in things:
            o = things[obj]
            put(pool, o + 0x5e, 'H', r.choice([0, struct.unpack_from('<H', pool, t)[0], r.choice(handles)]))
            put(pool, o + 0x60, 'I', r.choice([(time - 100 + d) & 0xffffffff for d in (-5, -1, 0, 1, 5)] + [r.getrandbits(32)]))
            if op == 'Mechanic':   # (a person the object tracks names it back, or not)
                for h in handles:
                    if r.random() < 0.5:
                        put(pool, things[h] + r.choice([0x218, 0x21a]), 'H', r.choice([obj, r.getrandbits(16)]))
                    pool[things[h] + 2] = r.choice([pool[things[h] + 2], 4, 5])
                pool[t + 2] = kind
    if op == 'Entertainer':
        cell = r.choice([ident, ident + r.choice([1, -1, 0x80, -0x80, 0x81]), (r.randrange(120) << 7) + r.randrange(128) + 1, 0])
        cell = cell if 0 <= cell < 0x3c00 + 0x81 else ident
        put(pool, t + 0xcc, 'H', cell)
        spread(r, regions, base, cell)
    regions[td.POOL] = pool
    return Call(address, [thing], regions, conv='t', ret='a' if op == 'Kinds9' else 'v')


if __name__ == '__main__':
    B.main(case)
