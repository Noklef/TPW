# The park analyser's periodic sample (src/game/world/analyser: ParkAnalyser_Sample 0x4c7720) against the original: thingenv.py's park of
# 0-6 things (staff, kind 1 and objects on map cells; the definitions' +0x4ac mostly the values Things_FacilityScore counts by, the kind 1
# things' floats +0x19c 0..100 mostly) in the world at RAM (its ticks +0x1da70c, +0x1da710 mostly 0, the handle +0x1da726 of the tracked
# source: slot 0's (a source of its own) or one of the things), [[0x7cf6ec]] that world. The analyser's +0x1f000 on mapped (no lower field
# is read): its sixteen CircBufU32s of 144 (sizes 144 mostly, else 0-2 or 72; cursors -2 up to the size; their accumulators any), the
# CircBufU32 of 50 at +0x21600 summed (size 0-50, cursor within it), the kept counts of CountKind1OnPlainCell / CountDef4ac0 fresh or not
# (thingscan.py's), the park score's value fresh and worked out always (a scan would read the whole map), the total +0x21c08 any.
# HandleMessage (0x4c7370): Sample's park and a message (slot 0 a recorder answering its kind: 0xc, 0x11, 0x13 and 0x17 mostly, else
# about them or any); 0x11's id (+8) a definition's mostly (else any: none found, a fault in both), the definitions' +0x4ac 0-5 and the
# u16 rings there (+0x1f068 + 0x14 * that: cursors -1 up to the size, sizes 0-3 or 5) sane; 0x13's +8 and 0x17's +0xc 3 or 2 mostly
# (else about them, upper bits set now and then); the world's clock (+0x2a0) and the imports funnytime.py's (as walkaways.py's).
# usage: analysersample.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Sample,HandleMessage)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
from fncmp import RAM, Bench, Call, put, set_return, vtable  # noqa: E402
from thingenv import DEFS, ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

CALLS = {'Sample': 0x4c7720, 'HandleMessage': 0x4c7370}
B = Bench([(0x412e90, 0x412f00), (0x41a960, 0x41a980), (0x41a990, 0x41a9c0), (0x481f80, 0x481fb0), (0x4c7370, 0x4c74b0), (0x4c7720, 0x4c7aa0), (0x4c7bb0, 0x4c7cd0),
           (0x4c7db0, 0x4c8160), (0x4c9960, 0x4c9a20), (0x4ca8e0, 0x4ca9c0), (0x4cceb0, 0x4cd350), (0x4ce290, 0x4ce2e0), (0x4cef30, 0x4cf000),
           (0x4d0af0, 0x4d0b20), (0x4e0380, 0x4e03a0), (0x4f8690, 0x4f8740), (0x4fa990, 0x4faa10), (0x50b350, 0x50b370), (0x516130, 0x5161c0), (0x5195d0, 0x519620),
           (0x51a280, 0x51a2a0), (0x535db0, 0x535de0), (0x536300, 0x536370), (0x536390, 0x5363b0), (0x5363f0, 0x536450), (0x53ad30, 0x53ad70),
           (0x53ad90, 0x53adc0), (0x5d60c0, 0x5d60e0), (0x5fc530, 0x5fc560), (0x5fc660, 0x5fc750), (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840), (0x6ad810, 0x6ad830), (0x5da3c0, 0x5da3c1)])   # (tools/ranges.py 4c7370)
A = RAM + 0x60000           # the analyser (from +0x1f000 to +0x21c40 mapped)
LOW, HIGH = 0x1f000, 0x21c40
RINGS = [0x1f100 + 0x250 * i for i in range(16)]   # (+4 values, +0x244 cursor, +0x248 size, +0x24c wrapped)
SOURCE, WORLD_AT = RAM + 0x50100, RAM + 0x50000
MESSAGE, MVT = RAM + 0x50200, RAM + 0x50300   # (HandleMessage)


def ring(r, buf, base, n, cursor_at):
    size = r.choice([n] * 6 + [0, 1, 2, n // 2])
    cursor = r.choice([-1, 0, 1, size - 2, size - 1, size, r.randrange(-2, size + 1)]) if n == 144 else \
        r.choice([0, size - 1, r.randrange(size)] if size else [0])
    put(buf, base + cursor_at, 'iiB', max(cursor, -2), size, r.choice([0, 1, 1]))


def case(r):
    op = r.deal(OPS)
    regions, handles, things = park(r, kinds=(4, 5, 6, 1, 1, 1, 1, 3, 3, 3, 0), cells=True)
    pool = bytearray(regions[POOL])
    for i in range(len(things)):
        f = r.choice([0.0, 1.0, 50.0, 99.9, 100.0, 0.5, r.uniform(0, 100)]) if r.random() < 0.85 else \
            r.choice([-1.0, 255.9, 256.0, 300.0, float('nan'), 1e10])
        put(pool, THING * i + 0x19c, 'f', f)
    regions[POOL] = pool
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, ITEM * i + 0x4ac, 'I', r.choice([0, 1, 2, 3, 3, 5]))
    regions[ITEMS] = items
    now = r.getrandbits(32)
    world = bytearray(r.bytes(0x40))   # (the world at RAM: +0x1da70c on)
    put(world, 0, 'II', now, r.choice([0, 0, 0, 1, r.getrandbits(32)]))
    put(world, 0x1a, 'H', r.choice([0, 0] + handles) if handles else 0)
    regions[RAM + 0x1da70c] = world
    regions[WORLD_AT] = struct.pack('<I', RAM)
    regions[0x7cf6ec] = struct.pack('<I', WORLD_AT)
    slots = bytearray(regions[0x7cfb90])
    put(slots, 0, 'I', SOURCE)
    regions[0x7cfb90] = slots
    regions[SOURCE] = r.bytes(0x20)
    a = bytearray(r.bytes(HIGH - LOW))
    for base in RINGS:
        ring(r, a, base - LOW, 144, 0x244)
    ring(r, a, 0x21600 - LOW, 50, 0xcc)
    for at in (0x1f0b8, 0x1f0c4):   # (CountKind1OnPlainCell / CountDef4ac0: stamp, count, kept)
        put(a, at - LOW, 'I', (now - r.choice([0, 0, 1, 3, 4, 8, 400])) & 0xffffffff)
        a[at + 8 - LOW] = r.choice([0, 1, 1, 1])
    put(a, 0x1f0e8 - LOW, 'I', now)   # (the park score: fresh, worked out)
    a[0x1f0f0 - LOW] = r.choice([1, 1, 0x40])
    if op == 'HandleMessage':
        return message(r, regions, a, now)
    regions[A + LOW] = a
    return Call(CALLS[op], [A], regions, conv='t', ret='v')


def message(r, regions, a, now):
    """HandleMessage (0x4c7370): see above."""
    log = funnytime.clock_regions(r, regions)
    del regions[funnytime.TICKS]
    regions[0x80239c] = struct.pack('<I', RAM)   # (the world at RAM again: its ticks +0x1da70c the case's)
    clock = bytearray(r.bytes(0x40))
    put(clock, 0x1c, 'I', funnytime.u32(r))
    regions[RAM + 0x2a0] = clock
    kind = r.choice([0xc, 0x11, 0x11, 0x11, 0x13, 0x13, 0x17, 0x17]) if r.random() < 0.85 else \
        r.choice([0xb, 0xd, 0x10, 0x12, 0x14, 0x16, 0x18, 0, 0x11 + 0x100, r.getrandbits(32)])
    set_return(log, 0, kind)
    regions[MVT] = vtable({0: 0})
    msg = bytearray(r.bytes(0x10))
    put(msg, 0, 'I', MVT)
    if kind == 0x11:
        n = struct.unpack_from('<i', regions[DEFS], 0x528)[0]
        items = regions[ITEMS]
        for i in range(NITEMS):
            put(items, ITEM * i + 0x4ac, 'I', r.choice([0, 1, 2, 3, 4, 5]))
        put(msg, 8, 'H', struct.unpack_from('<H', items, ITEM * r.randrange(n))[0] if r.random() < 0.95 else r.getrandbits(16))
        for k in range(6):
            size = r.choice([3, 3, 3, 1, 2, 0, 5])
            cursor = r.choice([-1, 0, size - 1, size, r.randrange(-1, size + 1)])
            put(a, 0x1f068 + 0x14 * k + 8 - LOW, 'iiB', max(cursor, -1), size, r.choice([0, 1]))
    elif kind in (0x13, 0x17):
        v = r.choice([2, 3, 3, 3]) if r.random() < 0.8 else r.choice([0, 1, 4, 0x102, 0x103, 0x10003, r.getrandbits(32)])
        put(msg, 8 if kind == 0x13 else 0xc, 'I', v)
    regions[MESSAGE] = msg
    regions[A + LOW] = a
    return Call(CALLS['HandleMessage'], [A, MESSAGE], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
