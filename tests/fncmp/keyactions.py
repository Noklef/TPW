# The key maps' action handlers (src/game/core/keyactions) against the originals, each called as Actions_FireUp calls it: the turns on
# 0x81d7a4 (about 0, 90, 270, 360, negative and wild; the global recorder as recorded_actions.py sets it up), the game clock 0x785970's rate
# (about the bounds, and junk), the money record of the world as challenges.py lays it out, the dword 0x787d1c (0, 1 or any), the render
# view's flags (any).
# usage: keyactions.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Turn90,... to pick)
import math
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import recorded_actions  # noqa: E402
from gameclock import rate_double  # noqa: E402
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x403be0, 0x403f00), (0x4032d0, 0x4033a0), (0x40c120, 0x40c190), (0x40c3b0, 0x40c400), (0x40c450, 0x40c4c0), (0x40c830, 0x40c880),
           (0x4d0190, 0x4d0200), (0x519510, 0x519560), (0x5195d0, 0x519620), (0x52f1b0, 0x52f210), (0x550510, 0x550530), (0x5da3c0, 0x5da3c1)])
CALLS = {'Turn90': 0x40c120, 'Turn270': 0x40c150, 'ClockSlower': 0x40c3b0, 'ClockFaster': 0x40c3c0, 'ClockNormal': 0x40c3d0, 'Credit': 0x40c450,
         'Toggle': 0x40c480, 'ViewFill': 0x40c830}
MONEY, PARK, WORLD = RAM + 0x1000, RAM + 0x2000, RAM + 0x80000


def case(r):
    op = r.deal(OPS)
    regions = {}
    if op.startswith('Turn'):
        regions = recorded_actions.recorder(r)
        regions[0x81b130] = bytearray(r.bytes(0xc))
        angle = r.choice([0, 1, 90, 180, 270, 359, 360, 450, -90, -1, -360, 0x7fffffff, 0x7fffff00, r.randrange(360), r.getrandbits(32)])
        regions[0x81d7a0] = put(bytearray(r.bytes(0xc)), 4, 'I', angle & 0xffffffff)
        if r.random() < 0.4:   # (0x81b134 the new angle: recorded only because forced)
            turned = ((angle + (90 if op == 'Turn90' else 270) + (1 << 31)) % (1 << 32)) - (1 << 31)
            put(regions[0x81b130], 4, 'I', int(math.fmod(turned, 360)) & 0xffffffff)
    elif op.startswith('Clock'):
        clock = bytearray(r.bytes(0x60))
        clock[0x20:0x28] = rate_double(r) if r.random() < 0.4 else struct.pack('<d', r.choice([0.2, 0.25, 0.3125, 1.6, 2.0, 2.5, r.uniform(0.15, 2.7)]))
        regions[0x785970] = clock
    elif op == 'Credit':   # (the world's handles: the park +0x1da720, the money record +0x1da726, through the table 0x7cfb90)
        world = bytearray(r.bytes(0x40))
        put(world, 0x20, 'H', 2)
        put(world, 0x26, 'H', 1)
        slots = bytearray(r.bytes(20 * 4))
        put(slots, 20 * 1, 'I', MONEY)
        put(slots, 20 * 2, 'I', PARK)
        money = bytearray(r.bytes(0x200))
        put(money, 0xc, 'i', r.choice([0, -10000, 0x7fffffff - 5000, r.randrange(-10 ** 6, 10 ** 6)]))
        regions.update({WORLD + 0x1da700: world, 0x7cfb90: slots, MONEY: money, PARK + 0x1fc90: r.bytes(4), 0x80239c: struct.pack('<I', WORLD),
                        0x7cf83c: struct.pack('<I', WORLD)})
    elif op == 'Toggle':
        regions[0x787d10] = put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 1, 1, 2, 0xffffffff, r.getrandbits(32)]))
        regions[0x878120] = r.bytes(0x10)
    else:
        regions[0x8bcbc0] = r.bytes(0x10)
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
