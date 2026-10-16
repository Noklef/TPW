# The staff base class's tiring, rest check and name (src/game/world/staff: 0x506680-0x506860)
# against the originals: grades in and beyond 0-6 (negative, wild), energy and happiness
# about 0 and 100, just above a drain (so that its rounding shows), NaN and infinite.
# usage: staff.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, f32, put  # noqa: E402

B = Bench([(0x506680, 0x5066a0), (0x5066a0, 0x50675b), (0x506760, 0x50682b), (0x506830, 0x506867), (0x67b0d0, 0x67b10f)])
STAFF = RAM + 0x100
RATES = [0.012, 0.005, f32(0.025), 0.01]
# 6 - grade where the product rounded to a double and then to 24 bits is off by one (0.012,
# 0.005, 0.01): the x87 rounds the exact product once.
TIES = [1123533375, 1975276750, -885032875, 626506425, 1258784050, -634244325, -1261247650, 1417863850,
        -960464450]


def level(r, k):
    """a happiness or energy: about 0 or 100, a drain's worth above them, or anything."""
    pick = r.randrange(5)
    if pick == 0:
        return fb(r.choice([0.0, 100.0, -0.0]) + r.choice([0, 1e-6, -1e-6, 1e-3, -1e-3]))
    if pick == 1:
        return fb(k * r.choice(RATES) + r.choice([0.0, 100.0]) + r.uniform(-1e-4, 1e-4) * r.choice([0, 1, 1e-3]))
    if pick == 2:
        return fb(r.uniform(-10, 110))
    return r.f(r.choice([1, 100, 1e6, 1e9]))


def case(r):
    grade = r.choice([r.randrange(7), r.randrange(-50, 60), r.randrange(-1 << 31, 1 << 31), 6 - r.choice(TIES)])
    if r.wild and r.random() < 0.3:
        grade = r.getrandbits(32) - (1 << 31)
    k = 6 - grade
    k = (k + (1 << 31)) % (1 << 32) - (1 << 31)
    mem = r.bytes(0x400)
    put(mem, 0x100 + 0x1e4, 'i', grade)
    put(mem, 0x100 + 0x1f8, 'I', level(r, k))
    put(mem, 0x100 + 0x1fc, 'I', level(r, k))
    regions = {RAM: mem}
    op = r.randrange(6)
    if op == 0:
        regions[0x785324] = bytearray(put(bytearray(4), 0, 'i', r.choice([r.randrange(-5, 300), r.getrandbits(32) - (1 << 31)])))
        return Call(0x506680, [STAFF], regions, conv='t', ret='a')
    if op == 1:
        return Call(0x506830, [STAFF, RAM + 0x20 + r.randrange(0x40)], regions, conv='t')
    if op == 2:
        return Call(0x506860, [STAFF], regions, conv='t', ret='a')
    return Call(r.choice([0x5066a0, 0x506760]), [STAFF], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
