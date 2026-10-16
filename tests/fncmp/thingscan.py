# The thing scans of src/game/world/thingscan against the originals: a consistent thing table (slots in use linked newest first, the things
# behind them with a kind, flags and the fields the scans read), between none and six things.
# usage: thingscan.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=C1,C2,Max)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, convention, size)
CALLS = {'C1': (0x4c9f90, 'c', 0xa0), 'C2': (0x4ca0c0, 'c', 0xa0), 'Max': (0x4ca820, 's', 0xa0)}
B = Bench([(a, a + n) for (a, _c, n) in CALLS.values()] + [(0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x4e0440, 0x4e0450),
                                                          (0x5da3c0, 0x5da3c1)])
SLOTS = 0x7cfb90
POOL = RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    address, conv, _n = CALLS[name]
    count = r.choice([0, 1, 2, 3, 4, 6])
    handles = r.sample(range(1, 12), count)  # newest first
    slots = bytearray(r.bytes(20 * 12))
    things = bytearray(r.bytes(0x100 * 6))
    for i, h in enumerate(handles):
        nxt = SLOTS + 20 * handles[i + 1] if i + 1 < count else 0
        prv = SLOTS + 20 * handles[i - 1] if i else 0
        put(slots, 20 * h, 'IIIII', POOL + 0x100 * i, h, nxt, prv, 0)
        t = 0x100 * i
        put(things, t, 'H', h)
        things[t + 2] = r.choice([3, 3, 3, 1, 2, 0xff, 0])
        things[t + 0x32] = r.choice([0, 1, 2, 3, 0xff, r.getrandbits(8)])
        put(things, t + 0x68, 'I', r.choice([0, 0, 0, 1, 5]))
        put(things, t + 0xe, 'H', r.choice([0, 1, 2, 3, 0x10001 & 0xffff]))
        things[t + 0x50] = r.choice([0, 1, 5, 0x7f, 0x80, 0xff, r.getrandbits(8)])
    regions = {SLOTS: slots, POOL: things, 0x7cf56c: struct.pack('<I', SLOTS + 20 * handles[0] if count else 0),
               0x80239c: struct.pack('<I', RAM)}
    args = [r.choice([0, 1, 2, 3, 0x10001, 0xffff, r.getrandbits(32)])] if name == 'Max' else []
    return Call(address, args, regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
