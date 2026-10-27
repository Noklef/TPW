# A staff kind's advice test (src/game/world/staff: StaffAdvice_KindLow 0x509360) against the original: the dword +0x64 set now and then;
# the kind 0-4 mostly (others now and then); the world's staff chains (heads +0x1da73c, +0x1da73e, +0x1da740, +0x1da744, +0x1da742 of
# [0x80239c]; each member's next +0x210) of 0-8 members, their floats +0x1f8 and +0x1fc around 15 mostly (fractions, negatives, past 255,
# huge, infinite or NaN now and then); for kind 0 agent C's unported sums 0x4c8180 / 0x4c8150 thiscall recorders (k 16 / 17, driver.cpp's
# kStaffAdviceEnv) answering small counts, 0 or any, on the record of the world's handle +0x1da720.
# Its user (Update: StaffAdvice_Update 0x508f70) with the same surroundings, the world being funnytime.py's (its ticks) and the game
# clock ([[0x7cf6ec]] + 0x2a0) at a rate of 4 (whole seconds) mostly, so its elapsed time is about 24 30-day months (or any); the kind's
# state +0x24 + 12 * kind 0-5 or any and its flag; the message centre [0x788d3c] made, the posts (agent B's unported 0x40fb10) a recorder
# (k 21) hashing the message's 16 bytes.
# usage: staffadvice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=KindLow,Update)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, set_return  # noqa: E402
sys.path.insert(0, HERE)
import funnytime  # noqa: E402

B = Bench([(0x509360, 0x50996d), (0x519510, 0x51954d), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860), (0x508f70, 0x509336), (0x40f6f0, 0x40f710)]
          + funnytime.RANGES, hooks={0x4c8180: _recorder(16, 1), 0x4c8150: _recorder(17, 1), 0x40fb10: _recorder(21, 1)})
SELF, WORLD, THINGS, RECORD = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x1800
HEADS = (0x1c, 0x1e, 0x20, 0x24, 0x22)   # (the world's +0x1da73c... from WORLD = its +0x1da720)
SLOTS = 40


def value(r):
    if r.random() < 0.85:
        return r.choice([r.uniform(0, 30), float(r.randrange(31)), r.uniform(10, 20), r.uniform(-5, 300)])
    return r.choice([256.0 + r.randrange(20), -1.5, 1e12, -1e12, float('inf'), float('nan'), 255.99, 14.999])


MONTHS24 = 24 * 30 * 86400   # (seconds)
CLOCK, CLOCK_AT = RAM + 0x200, RAM + 0x300   # (Update's; its world block at funnytime's TICKS + 0x14, past the ticks)


def chains(r, world_at, log):
    """The world block (its +0x1da720) at world_at, its record and chains, and the sums' answers: the regions."""
    world = bytearray(r.bytes(0x30))
    table = bytearray(r.bytes(20 * SLOTS))
    things = bytearray(r.bytes(0x400 * (SLOTS - 1)))
    rec = r.randrange(1, SLOTS)
    put(world, 0, 'H', rec)
    put(table, 20 * rec, 'I', RECORD)
    free = [h for h in range(1, SLOTS) if h != rec]
    r.shuffle(free)
    for head in HEADS:
        n = r.choice([0, 1, 3, 4, 4, 5, 6, 8]) if r.random() < 0.9 else 0
        chain, free = free[:n], free[n:]
        for i, h in enumerate(chain):
            t = 0x400 * (h - 1)
            put(table, 20 * h, 'I', THINGS + t)
            put(things, t + 0x1f8, 'ff', value(r), value(r))
            put(things, t + 0x210, 'H', chain[i + 1] if i + 1 < len(chain) else 0)
        put(world, head, 'H', chain[0] if chain else 0)
    for k in (16, 17):
        set_return(log, k, r.choice([0, 1, 2, 5, 10, 20, 100, 1000, -3, r.getrandbits(32)]))
    return {world_at: world, 0x7cfb90: table, THINGS: things, RECORD: r.bytes(0x10)}


def update(r, self, kind):
    regions = {}
    log = funnytime.clock_regions(r, regions)   # ([0x80239c] = funnytime.WORLD, its ticks)
    regions.update(chains(r, funnytime.TICKS + 0x14, log))
    clock = bytearray(r.bytes(0x40))
    ticks = MONTHS24 + r.choice([-1, 0, 1, 0, -86400, 86400, r.randrange(-MONTHS24, MONTHS24)])
    if r.random() < 0.8:
        put(clock, 0x1c, 'I', 4)
        put(regions[funnytime.TICKS], 0, 'I', ticks & 0xffffffff)
    else:
        put(clock, 0x1c, 'I', funnytime.u32(r))
    for k in (0, 1, 2, 3, 4, 5):   # (the states and flags)
        put(self, 0x24 + 12 * k, 'II', r.choice([0, 1, 2, 3, 4, 5, 1, 1, r.getrandbits(32)]), r.choice([0, 1, r.getrandbits(32)]))
    if r.random() < 0.5:
        put(self, 0x64, 'I', 0)
    blob_bytes(log, 21, 0, 0xffff)
    regions.update({SELF: self, CLOCK: clock, CLOCK_AT: struct.pack('<I', CLOCK - 0x2a0), 0x7cf6ec: struct.pack('<I', CLOCK_AT),
                    0x788d3c: struct.pack('<I', RAM + 0x380), LOG: log})
    return Call(0x508f70, [SELF, kind if kind < 6 else r.randrange(5)], regions, conv='t')


def case(r):
    self = put(bytearray(r.bytes(0x80)), 0x64, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)]))
    kind = r.choice([0, 0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)])
    if r.deal(OPS) == 'Update':
        return update(r, self, kind)
    log = log_region()
    regions = chains(r, WORLD, log)
    regions.update({SELF: self, LOG: log, 0x80239c: struct.pack('<I', WORLD - 0x1da720)})
    return Call(0x509360, [SELF, kind], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'KindLow,Update').split(',') if k]


if __name__ == '__main__':
    B.main(case)
