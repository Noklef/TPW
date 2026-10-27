# A staff kind's advice test (src/game/world/staff: StaffAdvice_KindLow 0x509360) against the original: the dword +0x64 set now and then;
# the kind 0-4 mostly (others now and then); the world's staff chains (heads +0x1da73c, +0x1da73e, +0x1da740, +0x1da744, +0x1da742 of
# [0x80239c]; each member's next +0x210) of 0-8 members, their floats +0x1f8 and +0x1fc around 15 mostly (fractions, negatives, past 255,
# huge, infinite or NaN now and then); for kind 0 agent C's unported sums 0x4c8180 / 0x4c8150 thiscall recorders (k 16 / 17, driver.cpp's
# kStaffAdviceEnv) answering small counts, 0 or any, on the record of the world's handle +0x1da720.
# usage: staffadvice.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x509360, 0x50996d), (0x519510, 0x51954d), (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)],
          hooks={0x4c8180: _recorder(16, 1), 0x4c8150: _recorder(17, 1)})
SELF, WORLD, THINGS, RECORD = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x1800
HEADS = (0x1c, 0x1e, 0x20, 0x24, 0x22)   # (the world's +0x1da73c... from WORLD = its +0x1da720)
SLOTS = 40


def value(r):
    if r.random() < 0.85:
        return r.choice([r.uniform(0, 30), float(r.randrange(31)), r.uniform(10, 20), r.uniform(-5, 300)])
    return r.choice([256.0 + r.randrange(20), -1.5, 1e12, -1e12, float('inf'), float('nan'), 255.99, 14.999])


def case(r):
    self = put(bytearray(r.bytes(0x80)), 0x64, 'I', r.choice([0] * 6 + [1, r.getrandbits(32)]))
    kind = r.choice([0, 0, 1, 2, 3, 4] * 3 + [5, 0xffffffff, r.getrandbits(32)])
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
    log = log_region()
    for k in (16, 17):
        set_return(log, k, r.choice([0, 1, 2, 5, 10, 20, 100, 1000, -3, r.getrandbits(32)]))
    regions = {SELF: self, WORLD: world, 0x7cfb90: table, THINGS: things, RECORD: r.bytes(0x10), LOG: log,
               0x80239c: struct.pack('<I', WORLD - 0x1da720)}
    return Call(0x509360, [SELF, kind], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
