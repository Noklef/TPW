# The map cell handles' lookups and the memory ranges (src/game/world/mapcell: 0x4d0a70-0x4d0b10)
# and the map's bounds check, RE cells and fixed positions (0x4d8300-0x4d8710) against the
# originals: handles 0, 1 and wild under any gMapCells, ranges set at any index, coordinates
# about 0 and 128, positions of any value for A and B.
# usage: mapcell.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d0a70, 0x4d0a86), (0x4d0a90, 0x4d0ab1), (0x4d0ac0, 0x4d0ae1), (0x4d0af0, 0x4d0b06),
           (0x4d0b10, 0x4d0b27), (0x4d8300, 0x4d8328), (0x4d8410, 0x4d8434), (0x4d8610, 0x4d8750)])
POS = [0x4d8610, 0x4d8650, 0x4d8690, 0x4d86d0, 0x4d8710]


def coord(r):
    return r.choice([0, 1, 0x7f, 0x80, 0x81, -1, r.randrange(-3, 0x84), r.getrandbits(32) - (1 << 31)]) & 0xffffffff


def case(r):
    mem = r.bytes(0x200)
    op = r.randrange(7)
    if op == 4:
        return Call(0x4d8300, [coord(r), coord(r)], {}, ret='a')
    if op == 5:
        return Call(0x4d8410, [r.getrandbits(32), r.choice([0, 1, 0x4000, r.getrandbits(32)])], {}, conv='t', ret='a')
    if op == 6:
        level = r.bytes(0x60)
        for k in range(0, 0x60, 4):
            if r.random() < 0.7:
                put(level, k, 'I', coord(r))
        return Call(r.choice(POS), [r.getrandbits(32), RAM + 0x100, r.choice([0, 1, 2, r.getrandbits(32)])],
                    {RAM: mem, 0x785590: level}, conv='t', ret='a')
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
