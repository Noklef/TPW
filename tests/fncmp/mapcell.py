# The map cell handles' lookups and the memory ranges (src/game/world/mapcell: 0x4d0a70-0x4d0b10)
# against the originals: handles 0, 1 and wild under any gMapCells, ranges set at any index.
# usage: mapcell.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d0a70, 0x4d0a86), (0x4d0a90, 0x4d0ab1), (0x4d0ac0, 0x4d0ae1), (0x4d0af0, 0x4d0b06),
           (0x4d0b10, 0x4d0b27)])


def case(r):
    mem = r.bytes(0x200)
    op = r.randrange(4)
    if op == 0:
        put(mem, 0x100, 'H', r.choice([0, 1, 2, 0x4000, 0xffff, r.getrandbits(16)]))
        cells = put(bytearray(4), 0, 'I', r.getrandbits(32))
        return Call(r.choice([0x4d0af0, 0x4d0b10]), [RAM + 0x100], {RAM: mem, 0x8023a0: cells}, conv='t', ret='a')
    if op == 1:
        return Call(0x4d0a70, [RAM + 0x40, r.randrange(-8, 0x18) & 0xffffffff, r.getrandbits(32), r.getrandbits(32),
                               r.getrandbits(32)], {RAM: mem}, conv='t')
    return Call(r.choice([0x4d0a90, 0x4d0ac0]), [r.getrandbits(32), r.choice([0x44, 0x28, r.getrandbits(32)])],
                {0x7cdb30: r.bytes(0x30)})


if __name__ == '__main__':
    B.main(case)
