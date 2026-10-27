# Two staff constructors (src/game/world/staff: 0x504a90 (cell, kind), 0x504b10 (cell, kind, +0x1e4, unused)) against the originals: the moving
# person's constructor's environment (thing_ctors.py's Sprite: the thing table and cells, the made message centre, the world's random state) and
# the common tail's (staffinit.py's: the calendar clock and its imports, the park the world's +0x1da720 names), the world the tail's (the random
# state put just before its ticks), the staff member 0x240 random bytes.
# usage: staffctor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Plain,Valued)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import funnytime  # noqa: E402
import staffinit  # noqa: E402
import thing_ctors  # noqa: E402
from fncmp import Bench, Call  # noqa: E402


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x40f950, 0x40f980), (0x40f9e0, 0x40fa50), (0x40fe60, 0x40ff90), (0x4107b0, 0x410ad0), (0x410b20, 0x410bf0), (0x415e00, 0x415e80),
                        (0x4c76d0, 0x4c7730), (0x4d3ed0, 0x4d3ef0), (0x4d91f0, 0x4d9290), (0x4f8690, 0x4f8740), (0x4f8940, 0x4f8ae0), (0x504a90, 0x504c80),
                        (0x50afe0, 0x50b090), (0x50b350, 0x50b370), (0x50ba80, 0x50bac0), (0x50ffe0, 0x510110), (0x510160, 0x510190), (0x516270, 0x516320),
                        (0x516330, 0x516380), (0x519510, 0x519560), (0x5e7720, 0x5e7750), (0x5fc530, 0x5fc560), (0x5fc660, 0x5fc750), (0x67acf0, 0x67ada0),
                        (0x67b050, 0x67b0d0), (0x67c320, 0x67c3e0), (0x67c7f0, 0x67c840), (0x5da3c0, 0x5da3c1)]
                       + [(0x4d91f0, 0x4d9290), (0x502d20, 0x502d70), (0x50afe0, 0x50b090), (0x50ba80, 0x50bac0), (0x50f1f0, 0x50f200), (0x50ffe0, 0x5100f0),
                          (0x516270, 0x516320), (0x5d1750, 0x5d1751), (0x4f8940, 0x4f8ad0), (0x4d3ed0, 0x4d3ee0), (0x5100f0, 0x510100), (0x510160, 0x510180),
                          (0x516330, 0x516370)]   # (thing_ctors.py's for the moving person)
                       + funnytime.RANGES + thing_ctors.CENTRE_RANGES),
          hooks=thing_ctors.CENTRE_HOOKS)
CALLS = {'Plain': 0x504a90, 'Valued': 0x504b10}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    thing_ctors.OPS = ['Sprite']
    sprite = thing_ctors.case(r)
    tail = staffinit.case(r)
    regions = dict(sprite.regions)
    regions.update(tail.regions)
    del regions[staffinit.STAFF]
    ticks = regions.pop(staffinit.WORLD + 0x1da70c)
    regions[staffinit.WORLD + 0x1da708] = r.bytes(4) + bytes(ticks)
    regions[thing_ctors.OBJ] = r.bytes(0x240)
    table = bytearray(regions[0x7cfb90])
    for k in [k for k in tail.regions if 0x7cfb90 < k < 0x7cfb90 + len(table)]:   # (the park's slot inside the thing table: written into it)
        entry = regions.pop(k)
        table[k - 0x7cfb90:k - 0x7cfb90 + len(entry)] = entry
    regions[0x7cfb90] = table
    cell = sprite.args[1]
    args = [thing_ctors.OBJ, cell, r.getrandbits(32)]
    if op == 'Valued':
        args += [r.choice([0, 1, 2, 5, 0xffffffff, r.getrandbits(32)]), r.getrandbits(32)]
    return Call(CALLS[op], args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
