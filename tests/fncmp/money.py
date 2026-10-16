# The money record's changes (src/game/world/money: 0x4d0190, 0x4d01f0) against the originals: any record bytes, amounts of either
# sign, the flag set or not, the game's thing found through its handle and the table.
# usage: money.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Credit,Debit)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Credit': (0x4d0190, 0x60), 'Debit': (0x4d01f0, 0x70)}
B = Bench([(a, a + n) for (a, n) in CALLS.values()] + [(0x519510, 0x519550), (0x5da3c0, 0x5da3c1)])
GAME, THING, TIME = RAM + 0x40000, RAM + 0x80000, RAM + 0xc0000
AMOUNTS = [0, 1, 5, 100, 999999, 1000000, 1000001, 0x7fffffff, -1, -100, -0x80000000, 0x40000000]


def case(r):
    name = r.choice(OPS)
    address, _n = CALLS[name]
    money = bytearray(r.bytes(0x200))
    put(money, 0xc, 'i', r.choice([0, 1, 50, 99, 100, 101, -1, 0x7fffffff, r.getrandbits(32) - (1 << 31)]))
    put(money, 0x114, 'I', r.choice([0, 1, 1, 1, 0x1234]))
    put(money, 0x11c, 'i', r.choice([-1, -1, 0, 5, -0x80000000, 0x7fffffff]))
    handle = r.choice([1, 2, 3])
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * handle, 'I', THING)
    regions = {RAM: money,
               0x7cf83c: struct.pack('<I', GAME - 0x1da720), GAME: struct.pack('<H', handle), 0x7cfb90: slots,
               0x80239c: struct.pack('<I', TIME - 0x1da70c), TIME: r.bytes(4),
               THING + 0x1fc90: r.bytes(4), THING + 0x1f5a0: r.bytes(4)}
    return Call(address, [RAM, r.choice(AMOUNTS + [r.getrandbits(32)])], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
