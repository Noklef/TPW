# The object being placed made (src/game/world/placeobject: PlaceObject_Create 0x529e10) against the original. The definitions
# (gObjectDefs at 0x7890a0, 1-3 of them, the cache hit now and then) with the placed one's id at 0x818c38; its ride kind +0xa0 0, -1 to
# -14 or any, its offsets +0xa4-+0xc0 and pair +0x4b0 small or any, its +0x288 2 or not; the cell 0x818c3c any word (0 now and then);
# the angle 0, 0x5a, 0xb4, 0x10e or another. With a ride kind the track rides' module (0x877b58) loaded mostly, as trackrides.py's
# Create has it (the ride pool with a free record anywhere, the collision node and buoy pools). The heap (Mem_Alloc) failing now and
# then; agent C's unported object constructor 0x4db090 a thiscall recorder (k 11 of 4, driver.cpp's kPlaceObjectEnv; the cell and
# the id logged as words, the constructor reading only those) answering an object (or 0). The flag 0x8187f8 and the thing at
# [0x8186ac] (its handle); 0x7c2654.
# usage: placeobject.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _recorder, heap, log_region, put, set_return  # noqa: E402

B = Bench([(0x40f330, 0x40f360), (0x40f9e0, 0x40fa50), (0x40fb10, 0x40fc90), (0x412e90, 0x412f00), (0x413020, 0x413150), (0x454550, 0x454680),
           (0x4815b0, 0x4815d0), (0x48c940, 0x48c960), (0x4c7680, 0x4c76e0), (0x4d01f0, 0x4d0270), (0x4d3d10, 0x4d3d90), (0x4d4ad0, 0x4d4ba0),
           (0x4d8440, 0x4d8470), (0x4d9cc0, 0x4d9d80), (0x4dcf90, 0x4dd0b0), (0x4dd500, 0x4dd680), (0x4dd7f0, 0x4dd930),
           (0x4e0de0, 0x4e0e00), (0x4e0e60, 0x4e1200), (0x4f8690, 0x4f8740), (0x50afe0, 0x50b090), (0x50b350, 0x50b370), (0x516270, 0x516320),
           (0x519510, 0x519560), (0x5195d0, 0x519620), (0x519d80, 0x519dd0), (0x527930, 0x527950), (0x529e10, 0x52a060), (0x545180, 0x5452b0),
           (0x545890, 0x546290), (0x54b130, 0x54b180), (0x54b1e0, 0x54b220), (0x55a070, 0x55a110), (0x55a300, 0x55a340), (0x67acf0, 0x67ad6f),
           (0x67ae00, 0x67af10), (0x5da3c0, 0x5da3c1)],
          hooks={0x4db090: _recorder(11, 4)})
DEFS, ITEMS, ITEM, OBJECT, THING = 0x7890a0, RAM + 0x1000, 0x500, RAM + 0x3000, RAM + 0x3100
RIDES, COLLS, BUOYS, GLOBALS = RAM + 0x10000, RAM + 0x20000, RAM + 0x30000, 0x877b58


def small(r):
    return r.choice([0, 1, 2, 3, -1, -2, -3, 5, -5, r.getrandbits(32) - (1 << 31)])


def rides(r, regions):
    """The track rides' module as trackrides.py's Create has it: loaded mostly; the ride pool (0x40 records of 0xd0 from RIDES) with a free
    record anywhere (or none), the collision node and buoy pools with their first free record anywhere, the trig tables' first entries."""
    globals_ = bytearray(r.bytes(0x40))
    pool = bytearray(r.bytes(0x100 * 0xd0))
    free = r.choice([0, 1, 5, 0x3f, 2, 3, 7, 0x20, -1])
    for k in range(0x40):
        if k != free:
            pool[k * 0xd0] |= 1
        else:
            pool[k * 0xd0:k * 0xd0 + 4] = bytes(4)
    colls = bytearray(r.bytes(0x800 * 0x20))
    buoys = bytearray(r.bytes(0x400 * 0x1c))
    for p, stride, count in ((colls, 0x20, 0x800), (buoys, 0x1c, 0x400)):
        first = r.choice([0, 1, 2, 5, 0x3f, count - 1, count - 20, count - 40, 10, 100, -1])
        for k in range(count):
            if k < first or first < 0:
                p[k * stride] |= 1
            elif k == first:
                p[k * stride] &= 0xfe
    put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.93 else r.getrandbits(32))
    put(globals_, 8, 'I', RIDES)
    put(globals_, 0xc, 'I', r.getrandbits(8))
    put(globals_, 0x18, 'I', COLLS)
    put(globals_, 0x20, 'I', BUOYS)
    put(globals_, 0x1c, 'I', r.getrandbits(8))
    put(globals_, 0x24, 'I', r.getrandbits(8))
    for g in (0x877458, 0x877658, 0x877a58):
        regions[g] = put(bytearray(r.bytes(0x10)), 0, 'i', r.choice([0, 256, 181, -181, 255, -256, r.randrange(-300, 300)]))
    regions.update({GLOBALS: globals_, RIDES: pool, COLLS: colls, BUOYS: buoys})


def case(r):
    log = log_region()
    regions = {HEAP: heap(fail=r.choice([0] * 9 + [1])), LOG: log}
    n = r.randrange(1, 4)
    ids = r.sample(range(1, 0x10000), n)
    k = r.randrange(n)
    dtab = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(ITEM * n))
    for i in range(n):
        put(dtab, 4 * i, 'I', ITEMS + ITEM * i)
        put(items, ITEM * i, 'H', ids[i])
    put(dtab, 0x528, 'i', n)
    put(dtab, 0x730, 'HxxI', ids[k] if r.random() < 0.3 else ids[k] ^ r.randrange(1, 0x10000), ITEMS + ITEM * k)
    d = ITEM * k
    kind = r.choice([0, 0, 0, r.randrange(-14, 0), r.randrange(-14, 0), r.randrange(-14, 0), r.getrandbits(32) - (1 << 31)])
    put(items, d + 0xa0, 'i', kind)
    for off in range(0xa4, 0xc4, 4):
        put(items, d + off, 'i', small(r))
    put(items, d + 0x4b0, 'ii', small(r), small(r))
    put(items, d + 0x288, 'I', r.choice([2, 0, 1, r.getrandbits(32)]))
    regions.update({DEFS: dtab, ITEMS: items})
    if -14 <= kind < 0:
        rides(r, regions)
    regions[0x818c38] = put(bytearray(r.bytes(4)), 0, 'H', ids[k])
    regions[0x818c3c] = put(bytearray(r.bytes(4)), 0, 'H', r.choice([0, 1, 0x81, 0x4000, 0x3fff, r.getrandbits(14), r.getrandbits(16)]))
    regions[0x8187f8] = struct.pack('<I', r.choice([0, 1, 1, r.getrandbits(32)]))
    regions[0x8186ac] = struct.pack('<I', THING)
    regions[THING] = r.bytes(0x10)
    regions[0x7c2654] = r.bytes(4)
    regions[OBJECT] = r.bytes(0x40)
    set_return(log, 11, r.choice([OBJECT, OBJECT, OBJECT, 0]))
    struct.pack_into('<I', log, LOG_BYTES + 4 * 11, 3 << 16)   # (the cell and the id: words)
    angle = r.choice([0, 0x5a, 0xb4, 0x10e, 0, 0x5a, 0xb4, 0x10e, 0x2d, 360, -90, r.getrandbits(32)])
    return Call(0x529e10, [angle & 0xffffffff], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
