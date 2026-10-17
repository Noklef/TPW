# The map step test of src/game/world/mapcell (0x4d8750 MapCell_StepBlocked) against the original, with the callees run: a step from (x, y) about the
# map's edges in each direction (and past 3) in modes 0-3; the two cells (behind [0x7cf6ec]) of types about the branches, their neighbour bits and
# direction; the target's 0x28-byte entry ([0x8023a0]) and its parent ([0x7cf83c]) or low-flag entry; a ride cell's parent's things (0x7cfb90), one of
# them (kind 3) a ride whose state is ridestate.py's (Ride_IsBusy) and whose definition (thingenv's) forbids riding or not.
# usage: cellstep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridestate  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import DEFS, ITEM, ITEMS, NITEMS, SLOTS, park  # noqa: E402

B = Bench([(0x4d8750, 0x4d8b29), (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510),
           (0x412e90, 0x412f00), (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0),
           (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1)])  # (ridestate.py's for Ride_IsBusy)
POINTER, WORLD, ENTRIES, PARENTS, FILLERS = RAM + 0x10000, RAM + 0x100000, RAM + 0x400000, RAM + 0x600000, RAM + 0x20000
TYPES = [0, 1, 2, 3, 4, 5, 7, 9, 0xa, 0x1e, 0xb, 0xd, 0x10, 0x12, 0x19, 0xc, 0x11, 0x15]


class Pages:
    """Sparse memory of random 0x1000-byte pages (cells and entries anywhere, overlapping as they fall)."""

    def __init__(self, r):
        self.r, self.pages = r, {}

    def put(self, address, fmt, *values):
        data = struct.pack('<' + fmt, *values)
        for i, b in enumerate(data):
            a = address + i
            page = self.pages.setdefault(a & ~0xfff, bytearray(self.r.bytes(0x1000)))
            page[a & 0xfff] = b

    def touch(self, address, size):
        for a in range(address & ~0xfff, address + size, 0x1000):
            self.pages.setdefault(a, bytearray(self.r.bytes(0x1000)))


def cell_type(r, mode):
    return r.choice(TYPES + [9, 9, 3, 1, 1, 0] * (2 if mode == 2 else 1) + [r.getrandbits(32)])


def case(r):
    mode = r.choice([0, 1, 1, 2, 2, 2, 3, -1])
    d = r.choice([0, 1, 2, 3] * 4 + [4, 0xff])
    x = r.choice([0, 1, 0x7e, 0x7f, 0x80, 0xff, r.randrange(0x80), r.randrange(0x80)])
    y = r.choice([0, 1, 0x7e, 0x7f, r.randrange(0x80), r.randrange(0x80)])
    xa = x | (r.getrandbits(24) << 8 if r.random() < 0.3 else 0)
    ya = y | (r.getrandbits(24) << 8 if r.random() < 0.2 else 0)
    da = d | (r.getrandbits(24) << 8 if r.random() < 0.1 else 0)
    base = x + (y << 7)
    target = {0: base - 0x7f, 1: base + 2, 2: base + 0x81, 3: base}.get(d, xa) & 0xffff
    source = (base + 1) & 0xffff
    mem = Pages(r)
    cells = {}
    for h in (source, target):
        a = WORLD + 0x294 + h * 0x44
        mem.touch(a, 0x44)
        cells[h] = a
    for h in (source, target):  # (the same cell for direction 3 past x 0 is not a case: x 0 is the edge)
        a = cells[h]
        mem.put(a + 8, 'I', cell_type(r, mode))
        mem.put(a + 0xc, 'BB', r.choice([0, 0x10, 0x40, 1, 4, 0x55, 0xff, r.getrandbits(8)]), r.choice([0, 4, 0x10, 0x40, 1, r.getrandbits(8)]))
        mem.put(a + 0xe, 'H', r.choice([0, 0x400, 1, 0xf, r.getrandbits(16)]))
        mem.put(a + 2, 'B', r.choice([2, 0, 1]))
        mem.put(a + 0x22, 'B', r.choice([0, 0x10, 0x40, 1, 4, r.getrandbits(8)]))
    # the target's entry, and its parent's (types 0xc / 0x11) or the low-flag entry its handle names
    to = cells[target]
    handle = r.choice([1, 2, 0x10, r.getrandbits(16)])
    mem.put(to, 'H', handle)
    entry = ENTRIES + 0x10ffd8 + handle * 0x28
    mem.put(entry + 8, 'I', r.choice([0xb, 0xd, 0x10, 0x12, 0x19, 0xc, 0x11, 0xc, 0x11, 0, 1]))
    owner = r.choice([0, 1, 2, r.getrandbits(16)])
    mem.put(entry + 0x10, 'H', owner)
    mem.put(PARENTS + 0x1102b0 + owner * 0x28 + 8, 'I', r.choice([0xb, 0xd, 0x10, 0x12, 0x19, 0, 3]))
    for h in (handle, owner):
        mem.put(WORLD + 0x1102b0 + h * 0x28 + 0xe, 'B', r.choice([0, 0, 1, 8, 0x10, 0xf0]))
    # a ride cell's parent: its things (handles 1-11 into 0x7cfb90), one of them (at times) the ride object of ridestate.py's case
    ride = ridestate.case(r)
    regions = dict(ride.regions)
    obj = bytearray(regions[ridestate.OBJ])
    defs, _, _ = park(r, count=0, defs=True)
    items = bytearray(defs[ITEMS])
    n = struct.unpack_from('<i', defs[DEFS], 0x528)[0]
    for i in range(NITEMS):
        put(items, i * ITEM + 0x118, 'I', r.choice([0, 0, 1, 5]))
    ids = [struct.unpack_from('<H', items, ITEM * i)[0] for i in range(n)]
    handles = r.sample(range(1, 12), r.choice([0, 1, 1, 2, 3]))
    slots = bytearray(r.bytes(20 * 12))
    fillers = bytearray(r.bytes(0x40 * 3))
    ride_at = r.randrange(len(handles) + 1) if handles else -1
    for i, h in enumerate(handles):
        nxt = handles[i + 1] if i + 1 < len(handles) else 0
        if i == ride_at:
            put(slots, 20 * h, 'I', ridestate.OBJ)
            obj[2] = 3
            put(obj, 0xa, 'H', nxt)
            put(obj, 0xe, 'H', r.choice(ids))
            put(obj, 0x20, 'I', r.randrange(4))
        else:
            put(slots, 20 * h, 'I', FILLERS + 0x40 * i)
            fillers[0x40 * i + 2] = r.choice([0, 1, 2, 0xff])
            put(fillers, 0x40 * i + 0xa, 'H', nxt)
    parent = r.choice([target, source, 0x10, 0x101, r.getrandbits(16)])
    mem.put(to + 0x10, 'H', parent)
    mem.put(WORLD + 0x294 + parent * 0x44 + 0x24, 'H', handles[0] if handles else 0)
    regions.update(mem.pages)
    regions.update({ridestate.OBJ: obj, FILLERS: fillers, SLOTS: slots, DEFS: defs[DEFS], ITEMS: items,
                    0x7cf6ec: struct.pack('<I', POINTER), POINTER: struct.pack('<I', WORLD), 0x8023a0: struct.pack('<I', ENTRIES),
                    0x7cf83c: struct.pack('<I', PARENTS), 0x81b4cc: bytes([r.choice([0, 0, 1])])})
    return Call(0x4d8750, [xa, ya, da, mode & 0xffffffff], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
