# The objects with the most / fewest walk-aways (src/game/world/thingscan: 0x4caae0 / 0x4cadf0 of a category, 0x4cb300 / 0x4cb640 of
# categories 1, 5 and 6, 0x4cbba0 / 0x4cc090 of the definitions with 2 at +0x4ac) and the happiest / unhappiest (0x4cb100 / 0x4cb200 of
# a category, 0x4cb980 / 0x4cba90 of 1, 5 and 6, 0x4cbec0 the happiest with 2 at +0x4ac) against the originals: thingenv.py's park
# (objects mostly, between none and six things) with the definitions' category +0x164 and dword +0x4ac mostly the ones asked for, the
# objects' customer, walk-away and happiness histories (CircBufU32s of 30 at +0x1a8, +0x230 and +0x340: cursor, size, wrapped byte about
# their bounds, the entries small, zero or large enough for the 24-bit sums to round; the happiness mostly small signed values, ties
# among them) and their ages (funnytime.py's clock: days about the 30 they need); the
# output word random beforehand. The analyser they are called on is not read (any pointer). The coasters with the most flagged cars
# (0x4cc810, MostCars) and the tallest (0x4cc920, Tallest): the same park, its things' definitions of kind 3 mostly (+0x9c), each thing's
# coaster link (+0x20; mostly one of up to six at [0x790fe0], chained by +0x18, ids +0x10) with cars +0x108 small, tied or past 2^31 and a
# height +0x110 whole, fractional (a later 5.5 against a 5.7 cut to 5), negative, past 2^32 or NaN; the definition asked for 0, a thing's
# or another.
# usage: walkaways.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Of,Of156,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x495cf0, 0x495d50), (0x4caae0, 0x4cae00), (0x4cb300, 0x4cb650), (0x4dd4e0, 0x4dd510), (0x4dd670, 0x4dd6e0),
           (0x4f8690, 0x4f8740), (0x50b350, 0x50b370), (0x516130, 0x5161c0), (0x5fc530, 0x5fc580), (0x5fc660, 0x5fc750), (0x67c320, 0x67c3e0),
           (0x67c7f0, 0x67c840), (0x5da3c0, 0x5da3c1), (0x4cadf0, 0x4cb100), (0x4cb640, 0x4cb980), (0x4cbba0, 0x4cbec0), (0x4cc090, 0x4cc3b0),
           (0x4cb100, 0x4cb300), (0x4cb980, 0x4cbba0), (0x4cbec0, 0x4cc090), (0x4e1e30, 0x4e1f30),
           (0x4cc810, 0x4cc914), (0x4cc920, 0x4cca3e), (0x43e0e0, 0x43e120), (0x4418f0, 0x441910), (0x436860, 0x436880), (0x4e2b50, 0x4e2b7b),
           (0x67a830, 0x67a860)])
CALLS = {'Of': 0x4caae0, 'Of156': 0x4cb300, 'FewestOf': 0x4cadf0, 'FewestOf156': 0x4cb640, 'MostDef4ac2': 0x4cbba0, 'FewestDef4ac2': 0x4cc090,
         'HappiestOf': 0x4cb100, 'UnhappiestOf': 0x4cb200, 'HappiestOf156': 0x4cb980, 'UnhappiestOf156': 0x4cba90, 'HappiestDef4ac2': 0x4cbec0,
         'MostCars': 0x4cc810, 'Tallest': 0x4cc920}
OUT, CLOCK, LINKS = RAM + 0x1000, RAM + 0x28000, RAM + 0x40000
DAY = funnytime.DAY


def ring(r, buf, at):
    """A CircBufU32 of 30 at buf + at: entries from +4, cursor +0x7c, size +0x80, wrapped +0x84."""
    big = r.random() < 0.2
    for k in range(30):
        value = r.choice([0, 1, 3, 7, 20, 100, r.randrange(1000)]) if not big else r.choice([r.getrandbits(32), 0xffffffff, 0x1000001, r.getrandbits(26)])
        put(buf, at + 4 + 4 * k, 'I', 0 if r.random() < 0.05 else value)
    size = r.choice([30, 30, 30, 0, 1, 29, 31, 40, r.randrange(31)])
    put(buf, at + 0x7c, 'i', r.choice([0, 1, 5, 28, 29, size - 1, r.randrange(30), -1, 30]))
    put(buf, at + 0x80, 'i', size)
    buf[at + 0x84] = r.choice([0, 1, 1, r.getrandbits(8)])
    few = r.random()
    if few < 0.2:   # (no customers: the sum 0, skipped; or one or two: a percentage past 2^31 against many walk-aways, negative)
        for k in range(30):
            put(buf, at + 4 + 4 * k, 'I', 0)
        if few < 0.1:
            put(buf, at + 4 + 4 * r.randrange(30), 'I', r.choice([1, 2]))


def coasters(r, name, regions, things):
    """MostCars / Tallest (the module comment)"""
    n = r.randrange(1, 7)
    ids = r.sample([1, 2, 3, 4, 5, 6, 7, 0x10001, r.getrandbits(32)], n)
    links = bytearray(r.bytes(0x200 * n))
    for k in range(n):
        put(links, 0x200 * k + 0x10, 'I', ids[k])
        put(links, 0x200 * k + 0x18, 'I', LINKS + 0x200 * (k + 1) if k + 1 < n else 0)
        put(links, 0x200 * k + 0x108, 'I', r.choice([0, 1, 2, 3, 3, 5, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)]))
        put(links, 0x200 * k + 0x110, 'f', r.choice([0.0, 5.0, 5.5, 5.7, 10.0, 10.0, 12.25, r.uniform(0, 200), r.uniform(0, 200), -1.0, -5.5,
                                                     float('nan'), 3e9, 5e9, 1e20]))
    regions[LINKS] = links
    regions[0x790fe0] = struct.pack('<I', LINKS)
    pool = regions[POOL]
    for i in range(len(things)):
        put(pool, THING * i + 0x20, 'I', r.choice(ids) if r.random() < 0.97 else r.getrandbits(32))
    mine = [struct.unpack_from('<H', pool, THING * i + 0xe)[0] for i in range(len(things))] or [0]
    want = r.choice([0, 0, r.choice(mine), r.choice(mine), r.choice(mine) | r.getrandbits(16) << 16, r.getrandbits(16)])
    regions[OUT] = r.bytes(4)
    return Call(CALLS[name], [OUT, want], regions, conv='s', ret='a')


def case(r):
    name = r.deal(OPS)
    regions, handles, things = park(r, count=r.choice([0, 1, 2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 3, 3, 1, 2, 0))
    category = r.choice([0, 1, 2, 3, 5, 6, 7, 0x10, r.getrandbits(32)])
    items = regions[ITEMS]
    for i in range(NITEMS):
        wanted = [category] if name.endswith('Of') else [1, 5, 6]
        put(items, ITEM * i + 0x164, 'I', r.choice(wanted) if r.random() < 0.8 else r.choice([category, 1, 5, 6, 0, 2, 4, 7, r.getrandbits(32)]))
        put(items, ITEM * i + 0x4ac, 'I', 2 if r.random() < 0.8 else r.choice([0, 1, 3, 0x102, r.getrandbits(32)]))
    # the clock (as visitoroption.py's) and each object's date: days about 30
    funnytime.clock_regions(r, regions)
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', funnytime.u32(r))
    world = CLOCK - 0x2a0
    regions[CLOCK] = clock
    regions[0x80239c] = struct.pack('<I', world)
    regions[world + 0x1da70c] = bytearray(regions.pop(funnytime.TICKS)) + bytearray(r.bytes(0x10))
    rate, tick = struct.unpack_from('<I', clock, 0x1c)[0], struct.unpack_from('<I', regions[world + 0x1da70c])[0]
    product = rate * tick
    seconds = product // 4 if product < 1 << 63 else -((2 ** 64 - product) // 4)
    now = struct.unpack_from('<Q', clock)[0] + seconds * 10000000
    pool = regions[POOL]
    for i in range(len(things)):
        days = r.choice([31, 31, 40, 100, 100, 365, 30, 29, 0, r.randrange(400)])
        age = days * DAY + r.choice([0, 1, DAY - 1, r.randrange(DAY)]) if r.random() < 0.95 else r.getrandbits(64)
        put(pool, THING * i + 0x18, 'Q', (now - age) % 2 ** 64)
        ring(r, pool, THING * i + 0x1a8)
        ring(r, pool, THING * i + 0x230)
        ring(r, pool, THING * i + 0x340)
        if r.random() < 0.7:
            for k in range(30):
                put(pool, THING * i + 0x344 + 4 * k, 'i', r.choice([0, 1, -1, 5, -5, 20, -50, 100]))
    if name in ('MostCars', 'Tallest'):
        return coasters(r, name, regions, things)
    regions[OUT] = r.bytes(4)
    analyser = r.getrandbits(32)
    if name.endswith('Of'):
        return Call(CALLS[name], [analyser, category, OUT], regions, conv='t', ret='a')
    return Call(CALLS[name], [analyser, OUT], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
