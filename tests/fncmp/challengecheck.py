# Whether a challenge is met (src/game/world/challenges: 0x4d1660 Challenge_Met) against the original, a kind (+0) of every one the table tells apart, or beyond it: the
# value given (kinds that compare it, or divide it by +4, never 0) or the world's count of each kind's own: thingscan.py's park for the plain cell counts and the greatest
# byte or average, walkaways.py's coaster links for the most flagged cars, ridesearch.py's track rides for the best rides (their handle written to +0x2e), objectcontrols.py's
# table for the control records; the threshold +8 about the values, the skip +0x1c set and the flag's byte clear now and then, the flag's other bytes any (they go on with the
# word +0xc to a call). The game object [[0x7cf6ec]] is looked up for each count in the original and its record ignored by the callee.
# usage: challengecheck.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectcontrols  # noqa: E402
import ridesearch  # noqa: E402
import walkaways  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x4d1660, 0x4d19f0), (0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x4c9130, 0x4c91d0), (0x4c9530, 0x4c95d0), (0x4c95d0, 0x4c9670), (0x4ca820, 0x4ca8c0),
                  (0x4c9c00, 0x4c9cb0), (0x4cc4b0, 0x4cc5d0), (0x4cc5d0, 0x4cc6f0), (0x4cc810, 0x4cc914), (0x4ca8c0, 0x4ca8e0), (0x516130, 0x5161c0), (0x50b350, 0x50b370),
                  (0x4e0440, 0x4e0450), (0x4e2710, 0x4e2770), (0x4dd4e0, 0x4dd510), (0x412e60, 0x412f00), (0x41a960, 0x41a970), (0x4e2b50, 0x4e2c10), (0x545310, 0x545450),
                  (0x43e0e0, 0x43e120), (0x4418f0, 0x441910), (0x436860, 0x436880), (0x4d3d10, 0x4d3ec9), (0x4d8c50, 0x4d8c93), (0x522770, 0x5227a0), (0x522850, 0x522870), (0x5161b0, 0x516200), (0x4fa990, 0x4fa9fa),
                  (0x536300, 0x53644c), (0x41a990, 0x41a9a7), (0x506eb0, 0x506ecc), (0x481f80, 0x481fa0), (0x481fc0, 0x482030), (0x4cd300, 0x4cd331)]))
CHALLENGE, VALUE, GAME_PTR, HANDLE = RAM + 0xf0000, RAM + 0xf0100, RAM + 0xf0200, RAM + 0xf0300
VALUES = [0, 1, 2, 3, 5, 10, 50, 100, -1, -5, 0x7fffffff]
# the table's kinds: what each kind reads
WORLDS = {**{k: 'value' for k in (1, 2, 3, 4, 5, 6, 7, 8, 0xb, 0xc, 0xd, 0x10, 0x11, 0x14, 0x16)}, 9: 'plain', 0xa: 'plain', 0xe: 'cars', 0xf: 'rides', 0x12: 'cars', 0x13: 'cars',
          0x15: 'rides', 0x18: 'value', 0x19: 'value', 0x1b: 'value', 0x1c: 'park', 0x1d: 'staff', 0x1e: 'controls', 0x1f: 'controls', 0x20: 'controls', 0x21: 'controls',
          0: 'value', 0x17: 'value', 0x1a: 'value', 0x22: 'value', 0xffffffff: 'value'}
DECK = list(WORLDS) + [0x1d] * 5 + [0x13] * 4 + [0x12] * 2 + [0xe] * 2
ridesearch.OPS = ['Kind11']


def world(r, name):
    if name == 'plain':
        return park(r, defs=False, cells=True)[0], r.getrandbits(16)
    if name == 'park':
        regions, _handles, _things = park(r, defs=False)
        ids = [struct.unpack_from('<H', regions[POOL], THING * i + 0xe)[0] for i in range(len(_things))] or [0]
        return regions, r.choice(ids + [r.getrandbits(16)])
    if name == 'staff':
        return park(r, defs=False, kinds=(4, 5, 6, 7, 8, 3, 1, 0))[0], r.getrandbits(16)
    if name == 'cars':
        regions, _handles, things = park(r, count=r.choice([0, 1, 2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 3, 3, 1, 2, 0))
        c = walkaways.coasters(r, 'MostCars', regions, things)
        links = c.regions[walkaways.LINKS]
        for k in range(len(links) // 0x200):  # (counts about 3, kind 0x13's)
            put(links, 0x200 * k + 0x108, 'I', r.choice([0, 1, 2, 3, 4]))
        return c.regions, c.args[1] & 0xffff
    if name == 'rides':
        c = ridesearch.case(r)
        return c.regions, r.choice([0, 0, 1, 2, 3, r.getrandbits(16)])
    if name == 'controls':
        c = objectcontrols.find_case(r)
        regions = dict(c.regions)
        regions[0x80239c] = struct.pack('<I', RAM - objectcontrols.GAME)
        put(regions[RAM], 0x12c0, 'i', 6)
        ids = r.sample([1, 2, 3, 4, 5, 9], 6)  # (all found: one not found faults in both, reading a null record)
        for k in range(6):
            put(regions[RAM], 0x20 * k, 'H', ids[k])
            put(regions[RAM], 0x20 * k + 0x18, 'i', r.choice([-1, 0, 0, 1, 7]))
        put(regions[RAM], 0x12c4, 'H', r.choice(ids))
        put(regions[RAM], 0x12c8, 'I', RAM + 0x20 * r.randrange(6))
        return regions, ids[0], ids[1]
    return {}, r.getrandbits(16)


def case(r):
    kind = r.deal(DECK)
    regions, word, *other = world(r, WORLDS[kind])
    regions = dict(regions)
    challenge = bytearray(r.bytes(0x40))
    put(challenge, 0, 'I', kind)
    divisor = r.choice([1, 1, 2, 3, 5, 10, -1, 0x10000])
    put(challenge, 4, 'i', divisor)
    put(challenge, 8, 'i', r.choice(VALUES))
    put(challenge, 0xc, 'H', word)
    put(challenge, 0xe, 'H', other[0] if other else r.choice([word, 1, 2, 3, r.getrandbits(16)]))
    put(challenge, 0x10, 'I', r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
    challenge[0x1c] = r.choice([0, 0, 0, 1, 0x80])
    value = r.choice(VALUES + [r.getrandbits(31), -r.getrandbits(31)])
    flag = r.choice([0, 1, 1, 0x100, r.getrandbits(32) & ~0xff, r.getrandbits(32)])
    regions.setdefault(0x7cfb90, r.bytes(0x50))
    regions.update({CHALLENGE: challenge, VALUE: struct.pack('<i', value), GAME_PTR: struct.pack('<I', HANDLE - 0x1da720), HANDLE: struct.pack('<H', r.randrange(4)) + r.bytes(2),
                    0x7cf6ec: struct.pack('<I', GAME_PTR)})
    return Call(0x4d1660, [CHALLENGE, VALUE, flag], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
