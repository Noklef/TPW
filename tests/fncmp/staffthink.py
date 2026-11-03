# A staff member's step by its state (src/game/world/staffthink: Guard_Step 0x4d6410, Handyman_Step 0x4d73c0, Entertainer_Step 0x4d4810, Mechanic_Step 0x4da490) against the originals. Every call a recorder (their own benches have them; driver.cpp's kStaffThinkEnv): SpriteScript_GetPosition 2 (the place filled), Sound_PlayAt 3,
# the rest as B's hooks below. The tick about the waits, the walk's answer 0 and 2 or not, the handle +0x21a a small one.
# usage: staffthink.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

T = {0x516330: (10, 0), 0x4faa00: (11, 2), 0x4fa2a0: (12, 0), 0x5066a0: (13, 0), 0x506760: (14, 0), 0x505fe0: (15, 0), 0x5061d0: (16, 0), 0x5056e0: (17, 0), 0x506a40: (18, 0), 0x4f9490: (19, 0), 0x4d7100: (20, 0), 0x4d7580: (21, 0), 0x4d76a0: (22, 0),
     0x4d7790: (23, 0), 0x4dfd80: (24, 0), 0x4df390: (25, 0), 0x4217f0: (26, 1), 0x4d7330: (27, 1), 0x4d65d0: (28, 1), 0x4d6790: (29, 0), 0x4d6a50: (30, 0), 0x4d8690: (31, 2), 0x4fa530: (9, 1), 0x4d46d0: (8, 0), 0x4da5b0: (7, 0), 0x4da740: (6, 0), 0x4da830: (5, 0)}
B = Bench([(0x4d63d0, 0x4d640b), (0x4d73c0, 0x4d7546), (0x4d6410, 0x4d6593), (0x4d4810, 0x4d4977), (0x4da490, 0x4da567), (0x5da3c0, 0x5da3c1)],
          hooks={**{a: _recorder(k, n) for a, (k, n) in T.items()}, 0x4754e0: _recorder(2, 5, cdecl=True), 0x51bfc0: _recorder(3, 6, cdecl=True)})
SELF, WORD, WORLD = RAM, RAM + 0x400, RAM + 0x80000
STATES = {0x4d63d0: [0],
          0x4d6410: [0, 0, 0, 1, 1, 1, 2, 3, 0x10, 0x12, 0x13, 0x13, 0x13, 4, 0xf, 0x11, 0x14],
          0x4d73c0: [0, 0, 1, 1, 2, 3, 4, 7, 8, 9, 0xa, 0xb, 0xb, 0xb, 0xc],
          0x4d4810: [0, 0, 1, 1, 2, 3, 4, 0xd, 0xe, 0xe, 0xe, 0xf],
          0x4da490: [0, 0, 1, 1, 2, 3, 4, 0xb, 0xc, 0xd, 0xe]}


def case(r):
    fn = r.choice(list(STATES))
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 9, 1 << 16)   # (the cell a word: the upper half the register's)
    self = bytearray(r.bytes(0x300))
    level = r.randrange(4)
    put(self, 0x19c, 'I', r.choice(STATES[fn] + [r.getrandbits(32)] * (1 if r.random() < 0.1 else 0)))
    put(self, 0x1e4, 'I', level)
    tick = r.getrandbits(31)
    wait = r.randrange(1 << 20)
    since = tick - wait + r.choice([-1, 0, 1, r.randrange(-1000, 1000)])   # (the wait, as the sum wraps with +0x200 or +0x214)
    put(self, 0x200, 'I', since & 0xffffffff)
    put(self, 0x214, 'I', since & 0xffffffff)
    put(self, 0x21a, 'H', r.randrange(16))
    tables = bytearray(r.bytes(0x120))
    put(tables, level << 4, 'I', wait)
    put(tables, 0x68 + (level << 4), 'I', wait)
    put(tables, 0xbc + level * 12, 'I', wait)
    world = bytearray(r.bytes(0x10))
    put(world, 0xc, 'I', tick & ~r.choice([0, 3, 3, 1]))
    set_return(log, 10, r.choice([0, 0x10, 0xf0, 1, 0xf, r.getrandbits(32)]))
    set_return(log, 12, r.choice([0, 0, 2, 2, 1, 3, r.getrandbits(32)]))
    set_return(log, 18, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, 19, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 31, WORD)
    strings(log, 31, 0, out=1)
    set_return(log, 9, r.choice([0, 1, r.getrandbits(32)]))
    strings(log, 2, 0, out=0b11110)
    for i in range(3):
        fill(log, 2, i + 1, struct.pack('<f', r.choice([0.0, 1.5, -1.5, 1000.25, r.uniform(-30000, 30000)])), which=i)
    regions = {LOG: log, SELF: self, WORD: r.bytes(4), WORLD + 0x1da700: world, 0x785330: tables, 0x7cfb90: r.bytes(20 * 16), 0x80239c: struct.pack('<I', WORLD), 0x8023a0: r.bytes(4), 0x803a30: r.bytes(4)}
    return Call(fn, [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
