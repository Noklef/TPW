# The objects with the most walk-aways (src/game/world/thingscan: 0x4caae0 of a category, 0x4cb300 of categories 1, 5 and 6) against the
# originals: thingenv.py's park (objects mostly, between none and six things) with the definitions' category +0x164 mostly the ones asked
# for, the objects' customer and walk-away histories (CircBufU32s of 30 at +0x1a8 and +0x230: cursor, size, wrapped byte about their
# bounds, the entries small, zero or large enough for the 24-bit sums to round) and their ages (funnytime.py's clock: days about the 30
# they need); the output word random beforehand. The analyser they are called on is not read (any pointer).
# usage: walkaways.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Of,Of156)
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
           (0x67c7f0, 0x67c840), (0x5da3c0, 0x5da3c1)])
CALLS = {'Of': 0x4caae0, 'Of156': 0x4cb300}
OUT, CLOCK = RAM + 0x1000, RAM + 0x28000
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
    if r.random() < 0.1:   # (no customers: the sum 0, skipped)
        for k in range(30):
            put(buf, at + 4 + 4 * k, 'I', 0)


def case(r):
    name = r.deal(OPS)
    regions, handles, things = park(r, count=r.choice([0, 1, 2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 3, 3, 1, 2, 0))
    category = r.choice([0, 1, 2, 3, 5, 6, 7, 0x10, r.getrandbits(32)])
    items = regions[ITEMS]
    for i in range(NITEMS):
        wanted = [category] if name == 'Of' else [1, 5, 6]
        put(items, ITEM * i + 0x164, 'I', r.choice(wanted) if r.random() < 0.8 else r.choice([category, 1, 5, 6, 0, 2, 4, 7, r.getrandbits(32)]))
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
    regions[OUT] = r.bytes(4)
    analyser = r.getrandbits(32)
    if name == 'Of':
        return Call(CALLS[name], [analyser, category, OUT], regions, conv='t', ret='a')
    return Call(CALLS[name], [analyser, OUT], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
