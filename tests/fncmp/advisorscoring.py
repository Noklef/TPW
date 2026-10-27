# The advisor scoring's staff criteria (src/game/core/advisorscoring) against the originals: AtLimit / Full (0x59efa0-0x59f1e0), Low
# (0x59d620-0x59d7a0) and High (0x59f630-0x59f7f0). thingenv.py's park of staff mostly (kinds 4-8 and others, between none and six things;
# their byte +3 0 mostly, the dwords +0x1e4 the averages sum small); the hiring limits (0x7852e8-0x7852f8) about the kind's count; the
# scoring's bounds about the count, the limit and the average (or anything); the game table entry's dwords +0xc..+0x1c about 0 (slot 0 of the
# thing table, which [[0x7cf6ec]]'s +0x1da718 word mostly names; now and then one of the park's things).
# usage: advisorscoring.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=At5,Full5,Low4,High7,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, SLOTS, THING, park  # noqa: E402

LIMITS = {4: 0x7852ec, 5: 0x7852e8, 6: 0x7852f0, 7: 0x7852f4, 8: 0x7852f8}
ENTRY_AT = {5: 0xc, 4: 0x10, 6: 0x14, 7: 0x18, 8: 0x1c}
# name: (address, family, kind, the scoring's bound, the limit's kind (At8: 7's, sic))
CALLS = {'At5': (0x59efa0, 'at', 5, 0x38c, 5), 'At6': (0x59f020, 'at', 6, 0x398, 6), 'At4': (0x59f0a0, 'at', 4, 0x3a4, 4),
         'At7': (0x59f120, 'at', 7, 0x3b0, 7), 'At8': (0x59f1a0, 'at', 8, 0x3bc, 7),
         'Full5': (0x59efe0, 'full', 5, 0x38c, 5), 'Full6': (0x59f060, 'full', 6, 0x398, 6), 'Full4': (0x59f0e0, 'full', 4, 0x3a4, 4),
         'Full7': (0x59f160, 'full', 7, 0x3b0, 7), 'Full8': (0x59f1e0, 'full', 8, 0x3bc, 8),
         'Low4': (0x59d620, 'low', 4, 0x194, 4), 'Low5': (0x59d680, 'low', 5, 0x19c, 5), 'Low7': (0x59d6e0, 'low', 7, 0x1a4, 7),
         'Low6': (0x59d740, 'low', 6, 0x1ac, 6), 'Low8': (0x59d7a0, 'low', 8, 0x1b4, 8),
         'High7': (0x59f630, 'high', 7, 0x444, 7), 'High8': (0x59f6a0, 'high', 8, 0x450, 8), 'High5': (0x59f710, 'high', 5, 0x45c, 5),
         'High6': (0x59f780, 'high', 6, 0x468, 6), 'High4': (0x59f7f0, 'high', 4, 0x474, 4)}
SIZE = {'at': 0x38, 'full': 0x38, 'low': 0x5d, 'high': 0x61}
B = Bench([(a, a + SIZE[f]) for (a, f, _k, _b, _l) in CALLS.values()] +
          [(0x4c9960, 0x4c9a20), (0x4c9c00, 0x4c9cb0), (0x519450, 0x519490), (0x519510, 0x519550), (0x516130, 0x516160), (0x516160, 0x5161b0),
           (0x50b350, 0x50b360), (0x4e0440, 0x4e0450), (0x4e2710, 0x4e2740), (0x4dd4e0, 0x4dd500), (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1),
           (0x41a960, 0x41a970), (0x5161b0, 0x516200), (0x41a990, 0x41a9a7)])
SCORING, ENTRY, GAMEPTR = RAM + 0x40000, RAM + 0x41000, RAM + 0x41100
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def i32(v):
    return v & 0xffffffff


def case(r):
    name = r.deal(OPS)
    address, family, kind, bound, lkind = CALLS[name]
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
    limits = bytearray(r.bytes(0x14))
    for k, at in LIMITS.items():
        put(limits, at - 0x7852e8, 'I', i32(r.choice([count, count, count + 1, count - 1, 0, 1, 2, -1, r.getrandbits(32)])))
    regions[0x7852e8] = limits
    limit = struct.unpack_from('<i', limits, LIMITS[lkind] - 0x7852e8)[0]
    s = bytearray(r.bytes(0x480))
    big = r.getrandbits(32)
    if family == 'at':
        first = 0x3b0 if name == 'At8' else bound
        put(s, first, 'I', i32(r.choice([limit, limit, limit, count, big])))
        if first != bound:
            put(s, bound, 'I', i32(r.choice([count, count + 1, count - 1, limit, 0, big])))
        elif r.random() < 0.3:
            put(s, bound, 'I', i32(r.choice([count, count + 1, count - 1])))
            put(limits, LIMITS[lkind] - 0x7852e8, 'I', i32(struct.unpack_from('<i', s, bound)[0]))
    elif family == 'full':
        put(s, bound, 'I', i32(r.choice([limit, limit + 1, limit - 1, count, 0, big])))
    elif family == 'low':
        put(s, bound, 'I', i32(r.choice([average, average + 1, average - 1, average, 0, 100, -1, big])))
    else:
        put(s, bound, 'I', i32(r.choice([count - 1, count - 1, count, count - 2, 0, -1, big])))
        put(s, bound + 4, 'I', i32(r.choice([average - 1, average - 1, average, average + 1, 0, -1, big])))
    regions[SCORING] = s
    return Call(address, [SCORING], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
