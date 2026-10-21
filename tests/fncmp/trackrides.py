# The track rides' module helpers (src/game/world/trackrides: 0x543490, 0x54bbb0, 0x549b50,
# 0x549d30, 0x5452a0, 0x54ab40) against the originals: writes and reads that fit or run short, pools absent, full or
# with a free record anywhere, handles good, stale, of other rides or 0, with the module loaded
# or not and links absent or flagged either way; a track section added (0x54b2f0): see section(). A ride's object thing found (0x54bbe0):
# thingenv's park, some objects (and others) given the ride's handle at +0x28.
# usage: trackrides.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Write,Read,Free,Linked,Field5c,PopNode)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, put, record_stream  # noqa: E402
import thingenv  # noqa: E402

B = Bench([(0x5452a0, 0x545307), (0x54ab40, 0x54abc3), (0x543490, 0x5434d0), (0x54bbb0, 0x54bbd6), (0x549b50, 0x549b71), (0x549d30, 0x549da6),
           (0x5f5e20, 0x5f5e50), (0x5f5e50, 0x5f5e80), (0x544a10, 0x544b50), (0x544840, 0x544a10), (0x545890, 0x546280), (0x54b130, 0x54b290), (0x544e50, 0x544f90), (0x545180, 0x5452a0), (0x545310, 0x545450), (0x54aa80, 0x54ab40), (0x51feb0, 0x51ff10), (0x544160, 0x5441f0), (0x5441f0, 0x544290), (0x544c00, 0x544c80), (0x545100, 0x545180), (0x54abd0, 0x54ac70), (0x5da3c0, 0x5da3c1),
           (0x54b2f0, 0x54bb71), (0x54bbe0, 0x54bc9c), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360)])
STREAM, SINK, BUF, POOL, LINKS, GLOBALS = RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x1000, RAM + 0x800, 0x877b58
SETUPREG, SYSTEMS_AT = 0x816d40, 0x80cefc
CALLS = ['Write', 'Read', 'Free', 'Linked', 'Field5c', 'PopNode', 'Worn', 'Dur', 'PushNode', 'ReadN', 'ReadD', 'Close', 'Fixed', 'Perf', 'Count', 'AddCar', 'Open', 'Create', 'Section', 'FindThing']


def find_thing(r):
    regions, handles, _things = thingenv.park(r, count=r.choice([0, 1, 2, 3, 4, 6]), kinds=(3, 3, 3, 1, 2, 0), defs=False)
    ride = r.choice([0, 1, 0x105, 0xff, r.getrandbits(32)])
    pool = regions[thingenv.POOL]
    for i in range(len(handles)):
        put(pool, thingenv.THING * i + 0x28, 'I', ride if r.random() < 0.4 else r.choice([ride ^ 1, ride ^ 0x100, ride ^ 0x10000, ride ^ 0x80000000, 0, r.getrandbits(32)]))
    regions[RAM] = r.bytes(4)
    return Call(0x54bbe0, [RAM, ride], regions, ret='a')


def case(r):
    op = r.choice(OPS)
    if op == 'FindThing':
        return find_thing(r)
    globals_ = bytearray(r.bytes(0x40))
    if op in ('Perf', 'Count', 'AddCar', 'Fixed'):
        rides = bytearray(r.bytes(0x100 * 0xd0))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        kind = r.choice([-14, -11, -6, -3, -1, 0, 1, 5, -2, -13, -15]) if op == 'Fixed' else r.choice([0, 1, 5, -7, 0x1234])
        put(rides, index * 0xd0, 'i', kind)
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.9 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        handle = r.choice([index | (kind << 8) & 0xffffffff] * 5 + [index | (kind ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 1]) & 0xffffffff
        regions = {GLOBALS: globals_, POOL: rides}
        if op == 'Perf':
            return Call(0x545180, [handle, r.choice([-5, 0, 1, 50, 99, 100, 101, 1000, -0x80000000, r.getrandbits(32)]) & 0xffffffff], regions, ret='v')
        if op == 'AddCar':
            put(rides, index * 0xd0 + 0x50, 'I', r.choice([0, 1, 7]))
            put(rides, index * 0xd0 + 0xc4, 'I', r.choice([LINKS + 0x20, 0, r.getrandbits(32)]))
            put(globals_, 0x34, 'I', r.choice([LINKS, LINKS, 0]))
            regions[LINKS] = r.bytes(0x40)
            return Call(0x54aa80, [handle, r.getrandbits(32)], regions, ret='a')
        if op == 'Count':  # a chain of pieces (kind in the word at +4, a value at +8, the next at +0x24)
            n = r.choice([0, 1, 2, 3, 5])
            pieces = bytearray(r.bytes(0x40 * 6))
            for k in range(n):
                put(pieces, 0x40 * k + 4, 'H', r.choice([8, 9, 10, 11, 11, 12, 13, 13, 14, 0, 0xffff, 0x100b]))
                put(pieces, 0x40 * k + 8, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
                put(pieces, 0x40 * k + 0x24, 'I', RAM + 0x8000 + 0x40 * (k + 1) if k + 1 < n else 0)
            put(rides, index * 0xd0 + 0xbc, 'I', RAM + 0x8000 if n else 0)
            regions[RAM + 0x8000] = pieces
            outs = [RAM + 0x600 + 4 * k if r.random() < 0.8 else 0 for k in range(4)]
            regions[RAM + 0x600] = r.bytes(0x10)
            return Call(0x545310, [handle] + outs, regions, ret='a')
        # Fixed: pool records of the ride with particle system handles (valid ones among them), flags 0x4000 and no animator
        put(rides, index * 0xd0 + 0x54, 'I', r.choice([0, 1, 2, 2, 2, 3]))
        recs = bytearray(r.bytes(8 * 0xac))
        systems = bytearray(r.bytes(0x78 * 0x140))
        used = 0
        for k in range(8):
            used += recs[k * 0xac] & 1
            put(recs, k * 0xac + 0x9c, 'I', POOL + index * 0xd0 if r.random() < 0.7 else r.getrandbits(32))
            put(recs, k * 0xac + 8, 'I', 0)
            slot, serial = r.randrange(0x78), r.getrandbits(16)
            put(systems, slot * 0x140 + 0xa, 'H', serial)
            put(recs, k * 0xac + 0x2c, 'I', r.choice([serial << 16 | slot, serial << 16 | slot, 0xffffffff, r.getrandbits(32)]))
        regions.update({SETUPREG: bytes([r.choice([0, 1, 1, 1])]) + bytes(0xf), SYSTEMS_AT: systems, RAM + 0x30000: recs})
        put(globals_, 0x10, 'I', RAM + 0x30000 if r.random() < 0.9 else 0)
        put(globals_, 0x14, 'I', r.choice([0, 1, used, max(used - 1, 0), 2]))
        return Call(0x544e50, [handle], regions, ret='v')
    if op == 'Open':  # a ride of a kind with a chain of 1 to 4 pieces (flag 4 on one, flag 2 off on the first, so the searches end)
        rides = bytearray(r.bytes(0x100 * 0xd0))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        kind = r.choice([-13, -12, -8, -5, -10, -9, -7, -4, -14, -11, -6, -3, -2, -1, 0, 5])
        put(rides, index * 0xd0, 'i', kind)
        put(rides, index * 0xd0 + 0x50, 'I', r.choice([0, 0, 0, 1, 7]))
        n = r.choice([1, 2, 3, 4])
        pieces = bytearray(r.bytes(0x40 * 5))
        for k in range(n):
            pieces[0x40 * k] = r.getrandbits(8) & ~4 & 0xff
            put(pieces, 0x40 * k + 0x14, 'I', RAM + 0x8000 + 0x40 * (k + 1) if k + 1 < n else 0)
        pieces[0x40 * r.randrange(n)] |= 4
        pieces[0] &= ~2 & 0xff
        put(rides, index * 0xd0 + 0xa8, 'I', RAM + 0x8000)
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.9 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        handle = r.choice([index | (kind << 8) & 0xffffffff] * 5 + [index | (kind ^ 1) << 8 & 0xffffffff, r.getrandbits(32)]) & 0xffffffff
        return Call(0x544840, [handle], {GLOBALS: globals_, POOL: rides, RAM + 0x8000: pieces}, ret='v')
    if op == 'Section':
        return section(r, globals_)
    if op == 'Create':  # the ride pool (a handle for a record, or the first free one), the collision node and buoy pools with the free ones anywhere
        rides = bytearray(r.bytes(0x100 * 0xd0))
        free = r.choice([0, 1, 5, 0x3f, 2, 3, 7, 0x20, -1])
        for k in range(0x40):
            if k != free:
                rides[k * 0xd0] |= 1
            else:
                rides[k * 0xd0:k * 0xd0 + 4] = bytes(4)
        kind = r.choice(range(-14, 0))
        slot = r.randrange(0x40)
        handle = r.choice([0, 0, slot | (kind << 8) & 0xffffffff])
        colls = bytearray(r.bytes(0x800 * 0x20))
        buoys = bytearray(r.bytes(0x400 * 0x1c))
        for pool, stride, count in ((colls, 0x20, 0x800), (buoys, 0x1c, 0x400)):
            first = r.choice([0, 1, 2, 5, 0x3f, count - 1, count - 20, count - 40, 10, 100, -1])
            for k in range(count):
                if k < first or first < 0:
                    pool[k * stride] |= 1
                elif k == first:
                    pool[k * stride] &= 0xfe
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.93 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        put(globals_, 0xc, 'I', r.getrandbits(8))
        put(globals_, 0x18, 'I', RAM + 0x40000)
        put(globals_, 0x20, 'I', RAM + 0x60000)
        put(globals_, 0x1c, 'I', r.getrandbits(8))
        put(globals_, 0x24, 'I', r.getrandbits(8))
        trig = {0x877458: bytearray(r.bytes(0x10)), 0x877658: bytearray(r.bytes(0x10)), 0x877a58: bytearray(r.bytes(0x10))}
        for g in trig:
            put(trig[g], 0, 'i', r.choice([0, 256, 181, -181, 255, -256, r.randrange(-300, 300), r.getrandbits(32) - 0x80000000]))
        regions = {GLOBALS: globals_, POOL: rides, RAM + 0x40000: colls, RAM + 0x60000: buoys, **trig}
        return Call(0x545890, [kind & 0xffffffff, r.randrange(-0x20000, 0x20000) & 0xffffffff, r.randrange(-0x20000, 0x20000) & 0xffffffff,
                               r.getrandbits(3) | r.choice([0, 0, 0x10]), handle & 0xffffffff, r.getrandbits(32)], regions, ret='a')
    if op == 'Close':  # a ride of one of the kinds (the first dword, also the serial of its handle) with a list at +0xc8 / +0xc4 and pool records pointing at it
        rides = bytearray(r.bytes(0x100 * 0xd0))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        kind = r.choice([-14, -11, -6, -3, -1, 0, 1, 5, -2, -13, -15])
        put(rides, index * 0xd0, 'i', kind)
        put(rides, index * 0xd0 + 0x50, 'I', r.choice([0, 1, 5]))
        nodes = bytearray(r.bytes(0x100))
        n = r.choice([0, 1, 2, 3])
        for k in range(n):
            put(nodes, 0x20 * k + 0x10, 'I', LINKS + 0x20 * (k + 1) if k + 1 < n else 0)
        put(rides, index * 0xd0 + 0xc8, 'I', LINKS if n else 0)
        put(rides, index * 0xd0 + 0xc4, 'I', r.choice([0, LINKS + 0x80, LINKS + 0xa0]))
        recs = bytearray(r.bytes(8 * 0xac))
        used = 0
        for k in range(8):
            used += recs[k * 0xac] & 1
            put(recs, k * 0xac + 0x9c, 'I', POOL + index * 0xd0 if r.random() < 0.6 else r.getrandbits(32))
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.9 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        put(globals_, 0x10, 'I', RAM + 0x30000 if r.random() < 0.9 else 0)
        put(globals_, 0x14, 'I', r.choice([0, 1, used, max(used - 1, 0), 2]))
        handle = r.choice([index | (kind << 8) & 0xffffffff] * 5 + [index | (kind ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 1])
        return Call(0x544a10, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: nodes, RAM + 0x30000: recs}, ret='v')
    if op in ('ReadN', 'ReadD'):  # a chunk: its stream (+4), the position (+8) and the end (+0x10); the stream holds `left` more bytes
        count = r.choice([0, 1, 3, 4, 5, 12, r.randrange(0x40)])
        left = r.choice([count, count, 0, 2, r.randrange(0x40), 0x100])
        chunk = bytearray(r.bytes(0x20))
        pos = r.choice([0, 1, 100])
        put(chunk, 4, 'II', STREAM, pos)
        put(chunk, 0x10, 'I', pos + r.choice([0, 1, 3, 4, 5, count, count + 3, 0x40]))
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), BUF: bytearray(r.bytes(0x100)), RAM: chunk}
        if op == 'ReadD':
            return Call(0x5441f0, [RAM], regions, ret='a')
        return Call(0x544160, [RAM, BUF, count], regions, ret='a')
    if op in ('Write', 'Read'):
        count = r.choice([0, 1, 4, 12, r.randrange(0x40)])
        left = r.choice([count, count, 0, r.randrange(0x40), 0x100])
        regions = {STREAM: record_stream(STREAM, SINK, left), SINK: r.bytes(0x100), BUF: r.bytes(0x100)}
        if op == 'Write':
            regions[RAM] = put(bytearray(r.bytes(0x20)), 4, 'I', STREAM)
            return Call(0x543490, [RAM, BUF, count], regions, ret='a')
        regions[RAM] = r.bytes(0x20)
        return Call(0x54bbb0, [RAM, STREAM, BUF, count, r.choice([RAM + 8, 0])], regions, ret='a')
    if op == 'Free':
        pool = bytearray(r.bytes(0x100 * 0xac))
        for i in range(0x100):
            pool[i * 0xac] |= 1
        if r.random() < 0.8:
            for _ in range(r.randrange(1, 4)):
                pool[r.choice([0, 1, 0xff, r.randrange(0x100)]) * 0xac] &= 0xfe
        put(globals_, 0x10, 'I', r.choice([POOL, POOL, 0]))
        return Call(0x549b50, [], {GLOBALS: globals_, POOL: pool}, ret='a')
    rides = bytearray(r.bytes(0x100 * 0xd0))
    if op in ('Worn', 'Dur', 'PushNode'):  # the handle of a ride (or a stale one, or 0 with the module loaded); the free node list at 0x877b8c
        nodes = bytearray(r.bytes(0x40))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        serial = r.choice([0, 1, r.getrandbits(24)])
        put(rides, index * 0xd0, 'I', serial)
        put(rides, index * 0xd0 + 0xc8, 'I', r.choice([LINKS + 0x20, 0, r.getrandbits(32)]))
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.85 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        put(globals_, 0x34, 'I', r.choice([LINKS, LINKS, 0]))
        handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 1])
        if op == 'Worn':
            return Call(0x544c00, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides}, ret='v')
        return Call(0x545100 if op == 'Dur' else 0x54abd0, [handle & 0xffffffff, r.getrandbits(32)], {GLOBALS: globals_, POOL: rides, LINKS: nodes}, ret='v')
    if op == 'PopNode':
        put(globals_, 0x34, 'I', r.getrandbits(32))
        nodes = bytearray(r.bytes(0x40))
        index = r.choice([0, 1, 0xff, r.randrange(0x100)])
        serial = r.choice([0, 1, r.getrandbits(24)])
        put(rides, index * 0xd0, 'I', serial)
        put(rides, index * 0xd0 + 0xc8, 'I', r.choice([LINKS, LINKS, 0]))
        put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.85 else r.getrandbits(32))
        put(globals_, 8, 'I', POOL)
        handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32), 0])
        return Call(0x54ab40, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: nodes}, ret='a')
    links = bytearray(r.bytes(0x40))
    put(links, 0, 'I', r.choice([0x400000, 0x400001, 0, r.getrandbits(32)]))
    index = r.choice([0, 1, 0xff, r.randrange(0x100)])
    serial = r.choice([0, 1, r.getrandbits(24), r.getrandbits(32)])
    put(rides, index * 0xd0, 'I', serial)
    put(rides, index * 0xd0 + 0x58, 'I', r.choice([LINKS, LINKS, 0]))
    loaded = r.random() < 0.8
    put(globals_, 0, 'I', 0x4a454647 if loaded else r.getrandbits(32))
    put(globals_, 8, 'I', POOL)
    handle = r.choice([index | (serial << 8) & 0xffffffff] * 4 + [index | (serial ^ 1) << 8 & 0xffffffff, r.getrandbits(32)])
    if not loaded and handle == 0:
        handle = 1  # the original reads address 0x58
    if op == 'Field5c':
        return Call(0x5452a0, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: links}, ret='a')
    return Call(0x549d30, [handle & 0xffffffff], {GLOBALS: globals_, POOL: rides, LINKS: links}, ret='a')


def pool(r, data, stride, count):
    """The pool's records in use up to some index (none, a few, all), its first free one there."""
    first = r.choice([0, 1, 2, 5, 0x3f, count - 1, count - 20, 10, 100, -1, -1 if count == 0x400 else 0])
    for k in range(count):
        if k < first or first < 0:
            data[k * stride] |= 1
        elif k == first:
            data[k * stride] &= 0xfe


def section(r, globals_):
    """0x54b2f0: a ride (its kind one of the spinning ones now and then, its +8 / +0xc close or any) named by the handle (or not); its chain
    of 0-3 sections (taken from the section pool, one maybe at the point), its buoy chain of 0-2; the section, collision node and buoy pools
    with the first free one anywhere (or full, or absent); the piece's kind 1-13 mostly, 0x10000 now and then."""
    rides = bytearray(r.bytes(0x40 * 0xd0))
    index = r.choice([0, 1, 0x3f, r.randrange(0x40)])
    kind = r.choice([-5, -8, -12, -13, -1, -14, 0, r.getrandbits(32) - (1 << 31)])
    ride = index * 0xd0
    put(rides, ride, 'i', kind)
    base = r.choice([0, r.randrange(-0x800, 0x800), r.getrandbits(32) - (1 << 31)])
    put(rides, ride + 8, 'II', base & 0xffffffff, (base + r.choice([0, 0x200, -0x200, r.randrange(-0x1000, 0x1000), r.getrandbits(32)])) & 0xffffffff)
    if r.random() < 0.1:
        put(rides, ride + 8, 'II', r.getrandbits(32), r.getrandbits(32))
    x, y = (r.choice([r.randrange(-8, 8) * 0x1800, r.randrange(-0x20000, 0x20000), r.getrandbits(32) - (1 << 31)]) for _ in range(2))
    SECTIONS, COLLS, BUOYS = RAM + 0x70000, RAM + 0x40000, RAM + 0x60000
    sections = bytearray(r.bytes(0x400 * 0x28))
    colls = bytearray(r.bytes(0x800 * 0x20))
    buoys = bytearray(r.bytes(0x400 * 0x1c))
    pool(r, sections, 0x28, 0x400)
    pool(r, colls, 0x20, 0x800)
    pool(r, buoys, 0x1c, 0x400)
    chain = r.sample(range(0x400), r.choice([0, 0, 1, 2, 3]))
    for n, k in enumerate(chain):
        sections[k * 0x28] |= 1
        near = r.random() < 0.8
        put(sections, k * 0x28 + 0x14, 'II', *((v + (r.choice([0x1800, -0x1800, 0, 1]) if near else 0)) & 0xffffffff for v in (x, y)))
        put(sections, k * 0x28 + 0x24, 'I', SECTIONS + chain[n + 1] * 0x28 if n + 1 < len(chain) else 0)
    put(rides, ride + 0xbc, 'I', SECTIONS + chain[0] * 0x28 if chain else 0)
    tied = r.sample(range(0x400), r.choice([0, 1, 2]))
    for n, k in enumerate(tied):
        buoys[k * 0x1c] |= 1
        put(buoys, k * 0x1c + 0x14, 'I', BUOYS + tied[n + 1] * 0x1c if n + 1 < len(tied) else 0)
    put(rides, ride + 0xa8, 'I', BUOYS + tied[0] * 0x1c if tied else 0)
    put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.93 else r.getrandbits(32))
    put(globals_, 8, 'I', POOL)
    put(globals_, 0x18, 'I', COLLS if r.random() < 0.97 else 0)
    put(globals_, 0x20, 'I', BUOYS if r.random() < 0.97 else 0)
    put(globals_, 0x28, 'I', SECTIONS if r.random() < 0.95 else 0)
    for at in (0x1c, 0x24, 0x2c):
        put(globals_, at, 'I', r.getrandbits(8))
    handle = r.choice([index | (kind << 8) & 0xffffffff] * 6 + [index | ((kind ^ 1) << 8) & 0xffffffff, r.getrandbits(32)])
    piece = r.choice(range(1, 14)) | r.choice([0, 0, 0x10000, r.getrandbits(16) << 16])
    if r.random() < 0.05:
        piece = r.choice([0, 14, 0xffff]) | 0x10000
    regions = {GLOBALS: globals_, POOL: rides, SECTIONS: sections, COLLS: colls, BUOYS: buoys}
    return Call(0x54b2f0, [handle & 0xffffffff, x & 0xffffffff, y & 0xffffffff, piece] + [r.getrandbits(32) for _ in range(3)], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
