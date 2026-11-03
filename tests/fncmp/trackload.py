# The track rides' load (src/game/world/trackrides: TrackRides_Load 0x543560) against the original: a memory stream of chunks (versions 2-9, now and then cut short or unknown) with rides remade under their handles,
# sections, cars, nodes and two rides already there with courses; the real Create / Section / Claim / lookups underneath, their pools and tables mapped.
# usage: trackload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, global_heap, heap, memory_stream, put  # noqa: E402

B = Bench([(0x543560, 0x544119 + 8), (0x543400, 0x543415), (0x544160, 0x544290), (0x544290, 0x5442d1), (0x5f5bb0, 0x5f5bb9), (0x5f5bf0, 0x5f5c04), (0x5f5e20, 0x5f5e50), (0x5da3c0, 0x5da3c1),
           (0x545890, 0x546280), (0x54b130, 0x54b290), (0x545180, 0x5452a0), (0x54aa80, 0x54ab40), (0x54abd0, 0x54ac70), (0x54b2f0, 0x54bb71), (0x549b50, 0x549b71), (0x54bbb0, 0x54bbd6),
           (0x5493f0, 0x5494c6), (0x463ab0, 0x463c80), (0x412e90, 0x412eea), (0x44b220, 0x44b267), (0x67b410, 0x67b440), (0x67a830, 0x67a860), (0x51feb0, 0x51ff10)])
STREAM, DATA, RIDES, CARS, COLLS, BUOYS, SECTIONS, FREE, COURSE, HOLDERS, DEFS, TYPES = (RAM, RAM + 0x100, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x6000, RAM + 0x8000, RAM + 0xa000,
                                                                                       RAM + 0xa200, RAM + 0xa400, RAM + 0xb000, RAM + 0xc400)
GLOBALS, IDS = 0x877b58, [0x11, 0x22, 0x33]
KINDS = [-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -1]


def dwords(*v):
    return struct.pack('<%dI' % len(v), *(x & 0xffffffff for x in v))


def chunk(r, version, body=b''):
    cut = r.random() < 0.04
    count = 12 + (r.randrange(len(body) + 1) if cut else len(body))
    return dwords(version, count, 12 + len(body)) + body


def holder_regions(r):
    """Ride instances 0-5 (0x7a4610): flags without 0x40000; +8 -> its +4 -> a model with 3 parts of keys 1-3; +0xe4 the key mostly, so that a search claims some."""
    table, blob = bytearray(), bytearray(r.bytes(6 * 0x200))
    for i in range(6):
        at, o = HOLDERS + i * 0x200, i * 0x200
        table += struct.pack('<I', at)
        put(blob, o + 4, 'I', r.getrandbits(32) & ~0x40000 & ~(0x10000 if r.random() < 0.6 else 0))
        put(blob, o + 8, 'I', at + 0x180)
        put(blob, o + 0x184, 'I', at + 0x100)
        put(blob, o + 0x100 + 0x48, 'H', 3)
        put(blob, o + 0x100 + 0x7c, 'I', RAM + 0xd000)
        put(blob, o + 0xe4, 'I', 0x101 if r.random() < 0.7 else r.getrandbits(32))
    return table, blob


def build(r):
    regions = {}
    # The definitions: three ids, types 1-3 at +0x4bc on.
    defs = bytearray(r.bytes(3 * 0x600))
    for i, ident in enumerate(IDS):
        put(defs, i * 0x600, 'H', ident)
        for k in range(8):
            put(defs, i * 0x600 + 0x4bc + 4 * k, 'I', r.randrange(1, 4))
    defs_at = bytearray(r.bytes(0x738))
    put(defs_at, 0, '3I', DEFS, DEFS + 0x600, DEFS + 0xc00)
    put(defs_at, 0x528, 'I', 3)
    put(defs_at, 0x730, 'H', r.choice(IDS + [0x77]))
    put(defs_at, 0x734, 'I', DEFS)
    types = bytearray(r.bytes(4 * 0x100))
    table7 = bytearray(0x20)
    for t in range(1, 4):
        put(table7, t * 4, 'I', TYPES + t * 0x100)
        put(types, t * 0x100 + 0xf8, 'I', r.choice(IDS))
    hold_table, hold = holder_regions(r)
    parts = bytearray(r.bytes(3 * 0x14))
    for k in range(3):
        put(parts, k * 0x14 + 4, 'I', k + 1)
        put(parts, k * 0x14, 'I', r.choice([0x80, 0x80, 0x100, r.getrandbits(32)]))
    regions.update({0x7890a0: defs_at, DEFS: defs, TYPES: types, 0x7ae6a8: table7, 0x7a4610: hold_table, HOLDERS: hold, RAM + 0xd000: parts,
                    0x7a3ebc: struct.pack('<I', 3), 0x7a4084: struct.pack('<I', 2), 0x7a439c: struct.pack('<I', 3)})
    # The pools. Rides 4 and 5 are there already, each with a course: a first piece without flag 2, one with flag 4.
    rides = bytearray(r.bytes(6 * 0xd0))
    for i in range(6):
        put(rides, i * 0xd0, 'I', 0)
    named = []
    course = bytearray(r.bytes(0x200))
    points = []
    for i, base in ((4, 0), (5, 0x100)):
        kind = r.choice([-13, -12, -10, -9, -8, -7, -5, -4, -3, -1])
        put(rides, i * 0xd0, 'i', kind)
        put(rides, i * 0xd0 + 0x10, 'I', r.choice(IDS))
        put(rides, i * 0xd0 + 0x50, 'I', r.choice([0, 1, 2]))
        put(rides, i * 0xd0 + 0xa8, 'I', COURSE + base)
        put(rides, i * 0xd0 + 0xcc, 'I', 0)
        put(rides, i * 0xd0 + 0xbc, 'I', 0)
        n = r.choice([1, 2, 3])
        for k in range(n):
            at = base + k * 0x20
            course[at] = r.getrandbits(8) & ~4 & 0xff
            put(course, at + 0x14, 'I', COURSE + at + 0x20 if k + 1 < n else 0)
            points.append((i | kind << 8, at))
        course[base + r.randrange(n) * 0x20] |= 4
        course[base] &= ~2 & 0xff
        named.append(i | kind << 8)
    # Collision nodes: a few used (areas round small points), the rest free.
    colls = bytearray(0x100 * 0x20)
    used = 0
    for k in range(6):
        if r.random() < 0.6:
            put(colls, k * 0x20, '5I', 1 | r.choice([0, 0, 0x20, 0x40, 0x20]), r.randrange(0, 0x2000), r.randrange(0, 0x2000), r.randrange(0x200, 0x2000), r.randrange(0x200, 0x2000))
            used += 1
        else:
            colls[k * 0x20 + 1] = r.getrandbits(8)
    nodes = bytearray(r.bytes(12 * 0x14))
    for k in range(12):
        put(nodes, k * 0x14 + 0x10, 'I', FREE + (k + 1) * 0x14 if k < 11 else 0)
    block = bytearray(r.bytes(0x40))
    put(block, 0, 'I', 0x4a454647 if r.random() < 0.97 else r.getrandbits(32))
    put(block, 8, 'I', RIDES)
    put(block, 0xc, 'I', 2)
    put(block, 0x10, 'I', CARS if r.random() < 0.97 else 0)
    put(block, 0x14, 'I', 0)
    put(block, 0x18, 'I', COLLS)
    put(block, 0x1c, 'I', used)
    put(block, 0x20, 'I', BUOYS)
    put(block, 0x24, 'I', 0)
    put(block, 0x28, 'I', SECTIONS)
    put(block, 0x2c, 'I', 0)
    put(block, 0x34, 'I', FREE)
    regions.update({GLOBALS: block, RIDES: rides, COLLS: colls, COURSE: course, FREE: nodes, CARS: bytes(10 * 0xac), BUOYS: bytes(0x100 * 0x1c), SECTIONS: bytes(0x40 * 0x28)})
    for g in (0x877458, 0x877658, 0x877a58):
        regions[g] = put(bytearray(r.bytes(0x10)), 0, 'i', r.choice([0, 256, 181, -181, 255, -256]))
    # The chunks.
    chunks = []
    if r.random() < 0.2:
        chunks.append(chunk(r, 2, r.bytes(r.randrange(20))))
    for index in r.sample(range(4), r.choice([0, 1, 2, 2, 3])):
        kind = r.choice(KINDS)
        handle = index | kind << 8
        if r.random() < 0.04:
            handle = 0
        chunks.append(chunk(r, 3, dwords(handle, r.randrange(0, 0x4000), r.randrange(0, 0x4000), r.getrandbits(3), r.choice(IDS), r.choice([0, 50, 100, 101, -1]),
                                         r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([0, 1, 2]))))
        if handle:
            named.append(handle & 0xffffffff)
    def name():
        return r.choice(named) if r.random() < 0.9 else named[0] ^ 0x100
    for _ in range(r.choice([0, 0, 1, 2])):
        chunks.append(chunk(r, 4, dwords(name(), r.choice(range(1, 14)) | r.choice([0, 0x10000]), r.randrange(-4, 4) * 0x1800, r.randrange(-4, 4) * 0x1800)))
    latest = set()
    for _ in range(r.choice([0, 1, 2, 3, 4])):
        handle = name()
        car = bytearray(r.bytes(0xac))
        put(car, 0, 'I', r.getrandbits(32) & (0x2002000 | 0x400000 | 4 | 1) if r.random() < 0.8 else 1)
        put(car, 4, 'I', r.randrange(8))
        put(car, 8, 'I', r.randrange(6))
        put(car, 0xc, 'I', r.randrange(6))
        put(car, 0x34, 'II', r.randrange(0, 0x3000), r.randrange(0, 0x3000))
        if r.random() < 0.1:
            put(car, 0xa0, 'I', r.choice([0x7fc00000, 0x7f800001, 0xffc00000, 0x7f800000, 0xff800000]))
        else:
            put(car, 0xa0, 'f', r.choice([0.0, 1.5, -2.0]))
        stop = r.choice([0, 0, 1, 1, 2])
        at = r.choice(points) if points else None
        if stop == 1:
            cs = (r.getrandbits(32), r.getrandbits(32))
            put(course, at[1] + 8, '2I', *cs)
            tail = dwords(at[0], *cs)
        elif stop == 2:
            tail = dwords(at[0], r.getrandbits(32), r.getrandbits(32))
        else:
            tail = dwords(0, r.getrandbits(32), r.getrandbits(32))
        chunks.append(chunk(r, 5, dwords(handle) + bytes(car) + dwords(r.randrange(0, 0x3000), r.randrange(0, 0x3000)) + tail))
        latest.add(handle)
    for handle in latest:
        for _ in range(r.choice([0, 1, 2])):
            if len(chunks) < 30:
                chunks.append(chunk(r, 9, dwords(handle, r.getrandbits(32), r.choice([1, 2, 3, 4]))))
    for _ in range(r.choice([0, 1, 2])):
        chunks.append(chunk(r, 7, dwords(name(), r.getrandbits(32))))
    for _ in range(r.choice([0, 1, 2])):
        chunks.append(chunk(r, 8, dwords(name(), r.getrandbits(32))))
    for handle in named:
        if r.random() < 0.6:
            chunks.append(chunk(r, 6, dwords(handle)))
    if r.random() < 0.06:
        chunks.insert(r.randrange(len(chunks) + 1), chunk(r, r.choice([0, 1, 10, r.getrandbits(32)]), r.bytes(4)))
    if not chunks:
        chunks.append(chunk(r, 2))
    body = b''.join(chunks)
    root = dwords(1 if r.random() < 0.96 else r.choice([0, 2]), 12 + len(body), 12 + len(body))
    data = root + body
    if len(data) > 0x1e00:
        raise ValueError('stream too long')
    regions.update({STREAM: memory_stream(STREAM, DATA, len(data)), DATA: data, HEAP: heap(size=0x800, fail=r.choice([0] * 24 + [2, 3])), **global_heap()})
    return regions


def case(r):
    return Call(0x543560, [STREAM], build(r), ret='a')


if __name__ == '__main__':
    B.main(case)
