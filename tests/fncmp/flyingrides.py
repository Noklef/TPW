# The flying rides' accessors (src/game/world/flyingrides: 0x55a570, 0x55d610-0x55de30) against
# the originals: indices in and out of 1-99, empty entries, slots in state 1 or not with steps
# either side of 0, and the ride part a step releases there, absent or not.
# usage: flyingrides.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=FirstRide,In,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x55a570, 0x55a592), (0x55d610, 0x55d716), (0x55d720, 0x55d82f), (0x55d950, 0x55d9a8),
           (0x55d9b0, 0x55d9e1), (0x55d9f0, 0x55da61), (0x55dcf0, 0x55dd20), (0x55dd20, 0x55dd3f),
           (0x55dd40, 0x55dd74), (0x55dd80, 0x55ddc3), (0x55ddd0, 0x55ddf0), (0x55ddf0, 0x55de10),
           (0x55de10, 0x55de30), (0x55de30, 0x55de65), (0x44b220, 0x44b267), (0x44b4c0, 0x44b505),
           (0x475550, 0x47558b)])
TABLE, RIDES, SCRIPTS = 0x8791f8, 0x7a4610, 0x7b49f0
HOLDER, H8, MODEL, PARTS, PS, PSE, REC = (RAM + o for o in (0, 0x40, 0x100, 0x200, 0x300, 0x400, 0x1000))
CALLS = {'FirstRide': 0x55a570, 'In': 0x55d610, 'Out': 0x55d720, 'Cap': 0x55d950, 'Tenth': 0x55d9b0,
         'On': 0x55d9f0, 'Set4': 0x55dcf0, 'Get3c': 0x55dd20, 'Ready': 0x55dd40, 'Take3c': 0x55dd80,
         'Set44': 0x55ddd0, 'Set20': 0x55ddf0, 'Set24': 0x55de10, 'Busy': 0x55de30}
RETURNS = {'FirstRide', 'In', 'Out', 'Get3c', 'Ready', 'Take3c', 'Busy'}


def small(r):
    return r.choice([0, 1, 2, 3, -1, r.randrange(-5, 6), r.getrandbits(32) - (1 << 31)])


def record(r, op):
    rec = bytearray(r.bytes(0x122c))
    for off in (0, 4, 0xc, 0x34, 0x38, 0x3c, 0x48):
        put(rec, off, 'i', r.choice([0, 1]) if op in ('Take3c', 'Busy') and r.random() < 0.6 else small(r))
    if r.random() < 0.3:
        put(rec, 0x34, 'I', 3)
    first = r.choice([0, 1, 5, 19, r.randrange(20)])
    for i in range(20):
        s = 0x5c + i * 0xe4
        if op in ('In', 'Out', 'On', 'Ready') or r.random() < 0.3:
            state = 1 if i == first else r.choice([0, 0, 2, 1 if r.random() < 0.2 else 3])
            put(rec, s + 0x80, 'I', state)
            step = r.choice([0, 0, 1, 2, 11, 12, -1, -2, -11, -12]) if i >= first or r.random() < 0.3 else 0
            put(rec, s + 0x8c, 'i', step)
        put(rec, s + 0x84, 'i', r.choice([0, 1, 4, 0x12c, -3, r.getrandbits(32) - (1 << 31)]))
        put(rec, s + 0xb8, 'I', r.choice([0, 1, r.getrandbits(32)]))
        put(rec, s + 0xc0, 'I', r.choice([0, 1, 2, 3]))
        put(rec, s + 0xd8, 'i', r.choice([0, 1, 3, r.randrange(-8, 9)]))
        put(rec, s + 0xdc, 'i', r.choice([0, 1, 3, 4, 7, -3]))
    return rec


def case(r):
    op = r.choice(OPS)
    index = r.choice([1, 2, 99, r.randrange(1, 100)] * 4 + [0, 100, -1, r.getrandbits(32) - (1 << 31)])
    table = bytearray(r.bytes(0x194))  # and the entry past the end, at 100
    if 0 < index <= 100:
        put(table, index * 4, 'I', REC if r.random() < 0.9 else 0)
    rides = bytearray(r.bytes(0x10))
    for k in range(1, 4):
        put(rides, k * 4, 'I', HOLDER)
    holder = bytearray(r.bytes(0x40))
    put(holder, 8, 'I', H8)
    h8 = bytearray(r.bytes(0x40))
    put(h8, 4, 'I', MODEL)
    put(h8, 0x28, 'I', PS)
    model = bytearray(r.bytes(0x100))
    n = r.choice([0, 1, 4, 8])
    put(model, 0x48, 'H', n)
    put(model, 0x7c, 'I', PARTS)
    parts = bytearray(r.bytes(0xa0))
    for i in range(8):
        put(parts, i * 0x14, '2I', r.choice([0x80, 0x81, 1, 0, 0x100, r.getrandbits(32)]), r.choice([1, 2, 3, 4, 5, 8]))
    ps = bytearray(r.bytes(0x10))
    put(ps, 4, 'I', PSE)
    put(ps, 8, 'I', r.choice([1, 2, 5]))
    scripts = bytearray(r.bytes(0x30))
    put(scripts, 0x24, 'I', 0)
    regions = {TABLE: table, RIDES: rides, SCRIPTS: scripts, HOLDER: holder, H8: h8, MODEL: model,
               PARTS: parts, PS: ps, PSE: r.bytes(0xa0), REC: record(r, op)}
    args = [index & 0xffffffff]
    if op not in RETURNS:
        args.append(r.choice([0, 1, 9, 10, 64, 63, -64, -65, -10, 0x12c * 64, r.getrandbits(32)]) & 0xffffffff)
    return Call(CALLS[op], args, regions, ret='a' if op in RETURNS else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
