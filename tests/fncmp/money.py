# The money record's changes (src/game/world/money: 0x4d0190, 0x4d01f0, 0x4d0750, 0x4d0850) and its total (0x4d09f0) against the originals: any record bytes, amounts of either
# sign, the flag set or not, the game's thing found through its handle and the table. The time in the red (0x4d0260): the balance copy
# either side of 0, the time it went below about the world's ticks; the world's calendar clock's rate. The loan owing least (0x4d09a0):
# eight loans' terms, rates, taken flags and months paid. A loan's amount owed and their sum (0x4d0970, 0x4d0a00): the same, the loan
# 0-7 mostly (8-13 now and then: the record's later bytes).
# usage: money.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Credit,Debit,Take,Repay,Total,InRed,Smallest,Owed,TotalOwed)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Credit': (0x4d0190, 0x60), 'Debit': (0x4d01f0, 0x70), 'Take': (0x4d0750, 0x100), 'Repay': (0x4d0850, 0x150), 'Total': (0x4d09f0, 7), 'InRed': (0x4d0260, 0x68),
         'Smallest': (0x4d09a0, 0x46), 'Owed': (0x4d0970, 0x2d), 'TotalOwed': (0x4d0a00, 0x2d)}
RANGES = [(a, a + n) for (a, n) in CALLS.values()] + [(0x519510, 0x519550), (0x5da3c0, 0x5da3c1), (0x4f88b0, 0x4f88f6), (0x67c320, 0x67c3ca),
                                                      (0x67c7f0, 0x67c824)]
B = Bench(RANGES)
GAME, THING, TIME, WORLD = RAM + 0x40000, RAM + 0x80000, RAM + 0xc0000, RAM + 0x100000   # (WORLD: the time in the red's)
AMOUNTS = [0, 1, 5, 100, 999999, 1000000, 1000001, 0x7fffffff, -1, -100, -0x80000000, 0x40000000]


def record(r):
    """A money record's bytes: the balance (+0xc), the flag +0x114 and the balance copy +0x11c of the sorts the code tells apart."""
    money = bytearray(r.bytes(0x200))
    put(money, 0xc, 'i', r.choice([0, 1, 50, 99, 100, 101, -1, 0x7fffffff, r.getrandbits(32) - (1 << 31)]))
    put(money, 0x114, 'I', r.choice([0, 1, 1, 1, 0x1234]))
    put(money, 0x11c, 'i', r.choice([-1, -1, 0, 5, -0x80000000, 0x7fffffff]))
    return money


def case(r):
    name = r.deal(OPS)
    address, _n = CALLS[name]
    money = record(r)
    handle = r.choice([1, 2, 3])
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * handle, 'I', THING)
    regions = {RAM: money,
               0x7cf83c: struct.pack('<I', GAME - 0x1da720), GAME: struct.pack('<H', handle), 0x7cfb90: slots,
               0x80239c: struct.pack('<I', TIME - 0x1da70c), TIME: r.bytes(4),
               THING + 0x1fc90: r.bytes(4), THING + 0x1f5a0: r.bytes(4)}
    if name == 'InRed':
        ticks = r.choice([0, 4, r.randrange(1 << 20), r.getrandbits(32)])
        put(money, 0x120, 'I', r.choice([ticks, ticks - 1 & 0xffffffff, ticks + 1 & 0xffffffff, 0, r.getrandbits(32), ticks - r.randrange(1 << 16) & 0xffffffff]))
        clock = bytearray(r.bytes(0x20))
        put(clock, 0x1c, 'I', r.choice([15000, 1, 0, r.getrandbits(32)]))
        regions.update({0x80239c: struct.pack('<I', WORLD), WORLD + 0x2a0: clock, WORLD + 0x1da70c: struct.pack('<I', ticks), RAM + 0x400: r.bytes(8)})
        return Call(address, [RAM, RAM + 0x400], regions, conv='t', ret='a')
    if name == 'Total':
        return Call(address, [RAM], regions, conv='t', ret='a')
    if name in ('Smallest', 'Owed', 'TotalOwed'):  # the eight loans' terms, rates, taken flags and months paid (entries of 0x20 from +0x20)
        for i in range(8):
            put(money, 0x20 + 0x20 * i, 'IIII', r.choice([0, 3, 5, 10, 100, 0xffffffff]), r.choice([0, 1, 2, 5, 0x10000]),
                r.choice([0, 0, 1, RAM]), r.choice([0, 1, 3, 5, 0xffffffff]))
        if name == 'Owed':
            return Call(address, [RAM, r.choice(list(range(8)) * 3 + [8, 12, 13])], regions, conv='t', ret='a')
        return Call(address, [RAM], regions, conv='t', ret='a')
    if name in ('Take', 'Repay'):  # loans: records of 0x20 bytes whose fields +0x18 (amount), +0x24 (rate), +0x28 (taken), +0x2c (months paid) and whose first dword (the term of the previous) matter
        for k in range(4):
            put(money, 0x20 * k, 'I', r.choice([0, 1, 12, 24, 36, 100]))
            put(money, 0x20 * k + 0x18, 'I', r.choice([0, 1000, 5000, 999999, 1000001, 0x7fffffff, r.getrandbits(32)]))
            put(money, 0x20 * k + 0x24, 'I', r.choice([0, 1, 2, 3, 10, 110]))
            put(money, 0x20 * k + 0x28, 'I', r.choice([0, 0, 1, 1, 5]))
            put(money, 0x20 * k + 0x2c, 'I', r.choice([0, 1, 5, 12, 100]))
        if name == 'Repay':
            put(money, 0xc, 'I', r.choice([0, 100, 1000, 5000, 100000, 0x7fffffff, 0xffffffff, r.getrandbits(32)]))
        return Call(address, [RAM, r.randrange(3)], regions, conv='t', ret='a')
    return Call(address, [RAM, r.choice(AMOUNTS + [r.getrandbits(32)])], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
