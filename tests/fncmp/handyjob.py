# A handyman's next job (0x4d7880 Handyman_FindJob, src/game/world/staff) against the original: movingperson.py's nearest() set-up (navpath.py's
# person, so the destination it is sent to by cell works): up to 7 objects in the world's list (the head +0x1da746) about the person's
# cell, flag bits 0 and 1 of +0x32 any, tracking nobody, the person or another (+0x5e, the stamp +0x60 about the world's tick), +0x64
# set or not, the condition float +0x44 about 25 (Object_InPoorCondition), their entrance cells (+0x36) mostly the person's; the person's
# pay grade +0x1e4 0-3 mostly (the radii 0x7853f0) and handle +0 one of the objects' trackers or not.
# usage: handyjob.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import navpath  # noqa: E402
import objectblocks  # noqa: E402
import wctimage  # noqa: E402
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
from movingperson import cell_xy  # noqa: E402

B = Bench(wctimage.merged(navpath.RANGES + [
    (0x4d0af0, 0x4d0b20), (0x4d7880, 0x4d7a80), (0x4d8300, 0x4d8340), (0x4d8750, 0x4d8b50), (0x4d8c50, 0x4d8c70), (0x4d8ca0, 0x4d8ce0),
    (0x4d96f0, 0x4d97f0), (0x4d99c0, 0x4d99f0), (0x4de670, 0x4de7f0), (0x4e01f0, 0x4e0300), (0x4e0390, 0x4e03f0), (0x4fa530, 0x4fa600),
    (0x4fa770, 0x4fa880), (0x506580, 0x5065e0), (0x50b350, 0x50b370), (0x50cd80, 0x50cdc0), (0x50f8e0, 0x50fd50), (0x510100, 0x510160),
    (0x5108a0, 0x511f00), (0x522770, 0x5227a0), (0x522850, 0x522870), (0x536300, 0x536360), (0x536380, 0x536460), (0x67bb90, 0x67bd98),
    (0x5da3c0, 0x5da3c1)]))
POOL, OUT = RAM + 0x8000, RAM + 0x7000


def case(r):
    cid = r.randrange(1, 0x4001)
    c = navpath.person_case(r, cell_xy(cid))
    regions = dict(c.regions)
    person = bytearray(regions[RAM])
    if len(person) < 0x200:
        person += bytearray(r.bytes(0x200 - len(person)))
    n = r.choice([0, 1, 2, 3, 5, 7])
    handles = r.sample(range(1, 13), n)
    mine = r.choice(handles + [r.randrange(1, 0x10000)] if handles else [r.randrange(1, 0x10000)])
    put(person, 0, 'H', mine)
    put(person, 0x1e4, 'I', r.choice([0, 1, 2, 3, 0, 1, 2, 3, 4, 0xffffffff]))
    regions[RAM] = person
    now = r.choice([0, 100, 5000, 0xfffffff0, r.getrandbits(32)])
    table = bytearray(r.bytes(20 * 13))
    pool = bytearray(r.bytes(0x80 * max(n, 1)))
    for i, h in enumerate(handles):
        put(table, 20 * h, 'I', POOL + 0x80 * i)
        at = 0x80 * i
        put(pool, at, 'H', h)
        put(pool, at + 0xc, 'H', handles[i + 1] if i + 1 < n else 0)
        pool[at + 0x32] = r.choice([1, 1, 3, 3, 0, 2, r.getrandbits(8)])
        pool[at + 5] = (person[5] + r.randrange(-6, 7)) & 0xff if r.random() < 0.9 else r.getrandbits(8)
        pool[at + 7] = (person[7] + r.randrange(-6, 7)) & 0xff if r.random() < 0.9 else r.getrandbits(8)
        put(pool, at + 0x36, 'H', r.choice([cid] * 18 + [0, r.randrange(1, 0x4001)]))
        put(pool, at + 0x44, 'I', fb(r.choice([0.0, 10.0, 24.0, 24.9, 25.0, 26.0, 100.0, -1.0, 255.5, 256.0, r.uniform(0, 100)])))
        put(pool, at + 0x5e, 'H', r.choice([0, 0, mine, mine, r.choice(handles), r.getrandbits(16)]))
        put(pool, at + 0x60, 'I', (now - r.choice([0, 1, 0x63, 0x64, 0x65, 1000, r.getrandbits(32)])) & 0xffffffff)
        put(pool, at + 0x64, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x7cfb90] = table
    regions[POOL] = pool
    world = bytearray(r.bytes(0x40))   # (the tick +0x1da70c, the list's head +0x1da746)
    put(world, 0, 'I', now)
    put(world, 0x3a, 'H', handles[0] if n else 0)
    regions[objectblocks.MAP + 0x1da70c] = world
    regions[OUT] = r.bytes(4)
    return Call(0x4d7880, [RAM, OUT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
