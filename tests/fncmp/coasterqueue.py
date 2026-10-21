# The coaster queue's requeue of waiting peeps (0x4379a0 Coaster_RequeueWaitingPeeps, src/game/coaster/queue) against the original: the
# coaster list at 0x790fe0 (one or two coasters, the id asked for one of theirs mostly, else none), thingenv.py's park of 0-6 things (kind 1
# the peeps Things_FindNextOfCategory 0x40 walks), the peeps' +0x1dc the coaster thing's handle mostly and +0x220 state 0x10 mostly; the
# coaster's ride record always found (Rides_FindById: with none the original compares against uninitialised stack slots), its elements 0
# and 1 peep handles now and then (or fewer elements); 0-2 trains of 0-3 cars, their rider lists 0-3 dwords whose low words are peep
# handles now and then; the queue and boarding rings (coaster.py's way) full or not.
# usage: coasterqueue.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

B = Bench([(0x412e90, 0x412f00), (0x4379a0, 0x437b70), (0x43b130, 0x43b200), (0x481f80, 0x481fb0), (0x481fc0, 0x482050), (0x4824d0, 0x482db0),
           (0x4cd300, 0x4cd350), (0x4dd4e0, 0x4dd510), (0x4e2710, 0x4e2750), (0x50b350, 0x50b370), (0x516130, 0x5161c0), (0x55a070, 0x55a0c0),
           (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1)])
COAST, TRAINS, CARS, LISTS, PIECES, RECORDS = RAM + 0x10000, RAM + 0x11000, RAM + 0x11400, RAM + 0x12000, RAM + 0x13000, RAM + 0x14000


def ring(r, c, off, buf):
    """A ring at c+off over buf: capacity 1..8, pointers on its slots, small counts (coaster.py's)."""
    cap = r.randrange(1, 9)
    slot = lambda: buf + 4 * r.choice([r.randrange(cap), r.randrange(cap), cap - 1, 0])  # noqa: E731
    head = slot()
    put(c, off, 'IIIIIIII', cap, buf, r.randrange(-2, 10) & 0xffffffff, r.randrange(0, cap + 1), r.choice([0, 0, r.randrange(4)]),
        head, slot(), r.choice([0, head, slot()]))


def case(r):
    regions, handles, things = park(r, kinds=(1, 1, 1, 1, 3, 2, 0), defs=False)
    pool = regions[POOL]
    owner = r.choice(handles) if handles and r.random() < 0.8 else r.randrange(1, 12)
    who = lambda: r.choice(handles) if handles and r.random() < 0.6 else r.getrandbits(16)  # noqa: E731
    for i in range(len(handles)):
        put(pool, THING * i + 0x1dc, 'H', owner if r.random() < 0.8 else r.choice([0, owner ^ 1, r.getrandbits(16)]))
        put(pool, THING * i + 0x220, 'I', 0x10 if r.random() < 0.8 else r.choice([0, 0xf, 0x11, 0x110, r.getrandbits(32)]))
    n = r.choice([1, 2])
    ids = r.sample(range(0, 40), n)
    coasters = bytearray(r.bytes(0x200 * n))
    for k in range(n):
        c = 0x200 * k
        put(coasters, c + 0x10, 'i', ids[k])
        put(coasters, c + 0x18, 'I', COAST + 0x200 * (k + 1) if k + 1 < n else 0)
    k = r.randrange(n)   # (the coaster asked for, mostly)
    id_ = ids[k] if r.random() < 0.9 else r.choice([-1, 41, 1000])
    c = 0x200 * k
    regions[RAM + 0x15000] = struct.pack('<H', owner) + r.bytes(0x1e)   # (the coaster's thing)
    put(coasters, c + 0x24, 'i', 7)
    put(coasters, c + 0x28, 'I', RAM + 0x15000)
    ring(r, coasters, c + 0xb0, PIECES + 0x20)
    ring(r, coasters, c + 0xd0, PIECES + 0x60)
    trains = r.choice([0, 1, 1, 2])
    put(coasters, c + 0xf4, 'I', trains)
    put(coasters, c + 0x128, 'I', TRAINS)
    train_bytes, car_bytes, list_bytes = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x400)), bytearray(r.bytes(0x200))
    for t in range(2):
        put(train_bytes, 0x80 * t + 0xc, 'I', r.choice([0, 1, 2, 3]))
        put(train_bytes, 0x80 * t + 0x28, 'I', CARS + 0x200 * t)
        for j in range(3):
            car = 0x200 * t + 0x60 * j
            at = 0x40 * (3 * t + j)
            put(car_bytes, car + 0x28, 'IIII', LISTS + at, LISTS + at + 0x20, r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3]))
            for e in range(8):
                put(list_bytes, at + 4 * e, 'I', who() | r.getrandbits(16) << 16)
    regions.update({COAST: coasters, TRAINS: train_bytes, CARS: car_bytes, LISTS: list_bytes, PIECES: r.bytes(0x100),
                    0x790fe0: struct.pack('<I', COAST)})
    record = bytearray(r.bytes(0x100))   # (one record before it with another id now and then)
    put(record, 8, 'i', 7)
    put(record, 0x1c, 'I', RECORDS + 0x200)
    put(record, 0x8c, 'i', r.choice([2, 2, 2, 1, 0]))
    other = bytearray(r.bytes(0x100))
    put(other, 0, 'I', RECORDS + 0x100)
    put(other, 8, 'i', 8)
    put(record, 0, 'I', 0)
    before = r.random() < 0.3
    regions[RECORDS] = (other if before else r.bytes(0x100)) + record + struct.pack('<2I', who(), who()) + r.bytes(8)
    regions[0x8791a0] = struct.pack('<I', 1) + r.bytes(0xc) + struct.pack('<I', RECORDS if before else RECORDS + 0x100)
    return Call(0x4379a0, [id_], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
