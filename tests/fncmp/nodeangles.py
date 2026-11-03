# The turn to each coaster node near a place (src/game/coaster/nodeangles: 0x438ab0) against the original, with its Angle_Lerp,
# Vec3_AngleY and the CRT (its acos). 2-12 nodes on a wandering line with 6 more before and 6 after them; a position anywhere on them,
# now and then past the end or just below 0 (index -1, reading the node before); distances that want 1-7 rows either way; bit 8 of the
# nodes dealt; a degenerate car vector, a repeated node and NaNs now and then.
# usage: nodeangles.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x438ab0, 0x43929c), (0x43e480, 0x43e51c), (0x46f220, 0x46f22c), (0x67a400, 0x690000)], sines=4096)
PATH, CAR, PPARAMS, CPARAMS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300
OUT1, OUT2, OUT3, COUNT, NODES = RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800
NODE, BEFORE, SPARE = 0x38, 6, 6
NANS = [0x7fc00000, 0xffc00000, 0x7fa00000]


def num(r, lo, hi, nan):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def case(r):
    nan = r.deal([0, 0, 0, 0, 0.05])
    count = r.randrange(2, 13)
    nodes = bytearray(r.bytes((BEFORE + count + SPARE) * NODE))
    x, y, z, yaw = r.uniform(-50, 50), r.uniform(0, 20), r.uniform(-50, 50), r.uniform(-3.2, 3.2)
    bits = r.deal([0, 0, 8, 8, 'mixed'])
    for k in range(BEFORE + count + SPARE):
        at = k * NODE
        if r.random() > 0.05:
            x, y, z = x + r.uniform(-3, 3), y + r.uniform(-1, 1), z + r.uniform(-3, 3)
            yaw += r.uniform(-0.6, 0.6)
        put(nodes, at, 'I', r.getrandbits(32) & ~8 | (r.choice([0, 8]) if bits == 'mixed' else bits))
        put(nodes, at + 4, 'IIII', num(r, x, x, nan), num(r, y, y, nan), num(r, z, z, nan), num(r, yaw, yaw, nan))

    where = r.deal(['on'] * 8 + ['past', 'below'])
    if where == 'on':
        position = r.uniform(0, count - 1)
    elif where == 'past':
        position = r.uniform(count - 1, count + 1)
    else:
        position = r.uniform(-1.9, 0)
    if r.random() < 0.1:
        position = float(int(position))

    path = bytearray(r.bytes(0x100))
    put(path, 4, 'I', PPARAMS)
    put(path, 0x9c, 'I', NODES + BEFORE * NODE)
    put(path, 0xa4, 'I', count)
    spacing = r.uniform(0.5, 4)
    put(path, 0xac, 'f', spacing)
    params = bytearray(r.bytes(0x60))
    put(params, 0x58, 'II', num(r, -4, 4, nan), num(r, -4, 4, nan))

    car = bytearray(r.bytes(0x40))
    put(car, 4, 'I', CPARAMS)
    how = r.deal(['any'] * 6 + ['flat', 'zero'])
    if how == 'any':
        put(car, 0x10, 'III', num(r, -1, 1, nan), num(r, -1, 1, nan), num(r, -1, 1, nan))
    elif how == 'flat':
        put(car, 0x10, 'fff', r.uniform(-1, 1), 0.0, r.uniform(-1, 1))
    else:
        put(car, 0x10, 'III', 0, r.getrandbits(32) & 0x3fffffff, 0)
    cparams = bytearray(r.bytes(0x40))
    put(cparams, 0x24, 'ff', r.uniform(0, 6) * spacing, r.uniform(0, 6) * spacing)

    regions = {PATH: path, CAR: car, PPARAMS: params, CPARAMS: cparams, NODES: nodes, OUT1: bytes(0x100), OUT2: bytes(0x100), OUT3: bytes(0x100), COUNT: bytes(4)}
    return Call(0x438ab0, [PATH, CAR, OUT1, OUT2, OUT3, r.getrandbits(32), fb(position), COUNT], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
