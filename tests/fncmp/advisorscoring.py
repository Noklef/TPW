# The advisor scoring's staff criteria (src/game/core/advisorscoring) against the originals: Low (0x59d620-0x59d7a0) and High
# (0x59f630-0x59f7f0). thingenv.py's park of staff mostly (kinds 4-8 and others, between none and six things; their byte +3 0 mostly, the
# dwords +0x1e4 the averages sum small); the scoring's bounds about the count and the average (or anything); the game table entry's dwords
# +0xc..+0x1c about 0 (slot 0 of the thing table, which [[0x7cf6ec]]'s +0x1da718 word mostly names; now and then one of the park's things).
# usage: advisorscoring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Low4,High7,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, SLOTS, THING, park  # noqa: E402

ENTRY_AT = {5: 0xc, 4: 0x10, 6: 0x14, 7: 0x18, 8: 0x1c}
# name: (address, family, kind, the scoring's bound)
CALLS = {'Low4': (0x59d620, 'low', 4, 0x194), 'Low5': (0x59d680, 'low', 5, 0x19c), 'Low7': (0x59d6e0, 'low', 7, 0x1a4),
         'Low6': (0x59d740, 'low', 6, 0x1ac), 'Low8': (0x59d7a0, 'low', 8, 0x1b4),
         'High7': (0x59f630, 'high', 7, 0x444), 'High8': (0x59f6a0, 'high', 8, 0x450), 'High5': (0x59f710, 'high', 5, 0x45c),
         'High6': (0x59f780, 'high', 6, 0x468), 'High4': (0x59f7f0, 'high', 4, 0x474)}
SIZE = {'low': 0x5d, 'high': 0x61}
B = Bench([(a, a + SIZE[f]) for (a, f, _k, _b) in CALLS.values()] +
          [(0x4c9960, 0x4c9a20), (0x4c9c00, 0x4c9cb0), (0x519450, 0x519490), (0x519510, 0x519550), (0x516130, 0x516160), (0x516160, 0x5161b0),
           (0x50b350, 0x50b360), (0x4e0440, 0x4e0450), (0x4e2710, 0x4e2740), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1),
           (0x41a960, 0x41a970), (0x5161b0, 0x516200), (0x41a990, 0x41a9a7)])
SCORING, ENTRY, GAMEPTR = RAM + 0x40000, RAM + 0x41000, RAM + 0x41100
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def i32(v):
    return v & 0xffffffff


def case(r):
    name = r.deal(OPS)
    address, family, kind, bound = CALLS[name]
    regions, handles, things = park(r, kinds=(4, 5, 6, 7, 8) * 3 + (kind,) * 3 + (3, 1, 0), defs=False)
    pool = regions[POOL]
    count = total = n = 0
    for i in range(len(things)):
        put(pool, THING * i + 0x1e4, 'I', r.choice([0, 1, 2, 3, 4, 5]) if r.random() < 0.9 else r.getrandbits(32))
        if pool[THING * i + 2] == kind:
            n += 1
            total += struct.unpack_from('<i', pool, THING * i + 0x1e4)[0]
            count += pool[THING * i + 3] == 0
    average = int(total * 100 / (5 * n)) if n else 0
    # The game object G = RAM for both getters ([0x7cf83c] is RAM already): its words +0x1da718 (Get1da718) and +0x1da720 (Get1da720).
    window = bytearray(r.bytes(0x10))
    put(window, 0, 'H', 0 if r.random() < 0.85 or not handles else r.choice(handles))
    regions[RAM + 0x1da718] = window
    regions[0x7cf6ec] = struct.pack('<I', GAMEPTR)
    regions[GAMEPTR] = struct.pack('<I', RAM)
    slots = bytearray(regions[SLOTS])
    put(slots, 0, 'I', ENTRY)
    regions[SLOTS] = slots
    entry = bytearray(r.bytes(0x20))
    for k, off in ENTRY_AT.items():
        put(entry, off, 'I', i32(r.choice([0, 0, -1, 1, 2, r.getrandbits(32)]) if k != kind else
                                 r.choice([0, 0, 1, 1, 2, -1, -0x80000000, r.getrandbits(32)])))
    regions[ENTRY] = entry
    s = bytearray(r.bytes(0x480))
    big = r.getrandbits(32)
    if family == 'low':
        put(s, bound, 'I', i32(r.choice([average, average + 1, average - 1, average, 0, 100, -1, big])))
    else:
        put(s, bound, 'I', i32(r.choice([count - 1, count - 1, count, count - 2, 0, -1, big])))
        put(s, bound + 4, 'I', i32(r.choice([average - 1, average - 1, average, average + 1, 0, -1, big])))
    regions[SCORING] = s
    return Call(address, [SCORING], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
