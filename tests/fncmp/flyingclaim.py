# The flying rides' ride claims (src/game/world/flyingrides: 0x55e3b0) against the original: 0-3 records in the table at 0x8791fc (99
# entries; the rest empty), each with its thing (the slot at +0x58 in gThingSlots, the id at +0xe of the thing) found in the definitions set
# 0x7890a0 by its one-entry cache or by the search, and 20 slots of which some are in use (+0x80 set); each used slot's ride (+0xc0) is
# claimed again (Ride_Claim, key 0x101, no description) with the definition's +0x4bc as the type, over coaster.py's claim environment.
# usage: flyingclaim.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import coaster  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x55e3b0, 0x55e470), (0x412e90, 0x412eea), (0x463ab0, 0x463c7a), (0x436820, 0x4368f0), (0x5da3c0, 0x5da3d0)])
RECORDS, THINGS, DEF = RAM + 0x10000, RAM + 0x18000, RAM + 0x19000   # (clear of coaster.py's regions, RAM to RAM + 0xb000)
TABLE, SLOTS = 0x8791fc, 0x7cfb90


def case(r):
    regions = dict(coaster.claim(r, 'Claim').regions)
    m = struct.unpack('<I', regions[0x7a439c])[0]
    rides = [i for i, v in enumerate(struct.unpack('<%dI' % (len(regions[0x7a4610]) // 4), regions[0x7a4610])) if v]
    ids = [r.choice([1, 2, 0x8001]) for _ in range(3)]
    defs = bytearray(regions[0x7890a0])
    if r.random() < 0.5:   # the cache holds the id
        put(defs, 0x730, 'HxxI', ids[0], DEF)
    else:                  # found by the search (or not: the first id only)
        put(defs, 0x528, 'I', 1)
        put(defs, 0, 'I', DEF)
        put(defs, 0x730, 'HxxI', 0xffff, 0)
    regions[0x7890a0] = defs
    type_ = r.randrange(m + 1)
    regions[DEF] = put(put(bytearray(r.bytes(0x4c0)), 0, 'H', ids[0]), 0x4bc, 'I', type_)
    types = struct.unpack('<%dI' % (m + 1), regions[0x7ae6a8])
    cars = bytearray(regions[coaster.CARS])
    for k in range(len(cars) // 0x100):   # some rides of the definition's type with the claims' key 0x101 (or one off it)
        if r.random() < 0.4:
            put(cars, 0x100 * k, 'I', types[type_])
            put(cars, 0x100 * k + 0xe4, 'I', r.choice([0x101, 0x101, 0x100]))
    regions[coaster.CARS] = cars
    table = bytearray(99 * 4)
    records = bytearray(0x1240 * 3)
    things = bytearray(r.bytes(0x60))
    for k in range(3):
        put(things, 0x20 * k + 0xe, 'H', ids[0] if r.random() < 0.9 else ids[k])
    regions[THINGS] = things
    regions[SLOTS] = b''.join(struct.pack('<I16x', THINGS + 0x20 * k) for k in range(3))
    for k in range(r.choice([0, 1, 1, 2, 3])):
        put(table, 4 * r.choice([0, 0, 98, r.randrange(99)]), 'I', RECORDS + 0x1240 * k)   # (the ends often: indices 1 and 99)
        rec = bytearray(r.bytes(0x1240))
        put(rec, 0x58, 'H', r.randrange(3))
        for s in range(20):
            put(rec, 0x5c + 0xe4 * s + 0x80, 'I', r.choice([0, 0, 0, 1, 2, 0x100, 0x10000, r.getrandbits(32)]))
            put(rec, 0x5c + 0xe4 * s + 0xc0, 'I', r.choice([0] + rides) if rides else 0)
        records[0x1240 * k:0x1240 * (k + 1)] = rec
    regions[TABLE] = table
    regions[RECORDS] = records
    return Call(0x55e3b0, [], regions)


if __name__ == '__main__':
    B.main(case)
