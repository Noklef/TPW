# A track piece's spline handles (src/game/coaster/handles: 0x431660) against the original, with its Vec3_NormalizeXZ and the CRT (its
# _isnan, acos). A piece, a prev, a prev's prev and a next on a line now and then straight or folded back; the prev, the next and the
# coaster's first by flags 0x80000 / 0x40000 / 0x100000 or the links dealt empty; each type's info none, railed or not; the info's limit
# fit for either comparison. Now and then a NaN.
# usage: trackhandles.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x431660, 0x4321c0), (0x4387a0, 0x438810), (0x67a400, 0x690000)])
RULES, COASTER, LIST, PIECES = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x1000
TYPES, INFOS = RAM + 0x800, RAM + 0x100
PIECE, TYPE, INFO = 0x120, 0x20, 0xb8
NANS = [0x7fc00000, 0xffc00000, 0x7fa00000]


def piece_at(k):
    return PIECES + PIECE * k


def case(r):
    nan = r.deal([0, 0, 0, 0, 0, 0.03])
    shape = r.deal(['any'] * 4 + ['straight', 'folded', 'level', 'same'])
    pieces = bytearray(r.bytes(5 * PIECE))
    types = bytearray(r.bytes(5 * TYPE))
    infos = bytearray(r.bytes(5 * INFO))
    x, y, z = r.uniform(-30, 30), r.uniform(0, 10), r.uniform(-30, 30)
    dx, dy, dz = r.uniform(-4, 4), r.uniform(-2, 2), r.uniform(-4, 4)
    places = {}
    places[0] = (x, y, z)
    if shape == 'any':
        places[1] = (x + r.uniform(-4, 4), y + r.uniform(-2, 2), z + r.uniform(-4, 4))
        places[3] = (x + r.uniform(-4, 4), y + r.uniform(-2, 2), z + r.uniform(-4, 4))
    elif shape == 'straight':
        places[1], places[3] = (x + dx, y + dy, z + dz), (x + 2 * dx, y + 2 * dy, z + 2 * dz)
    elif shape == 'folded':
        places[1], places[3] = (x + dx, y + dy, z + dz), (x - dx, y - dy, z - dz)
    elif shape == 'level':
        places[1], places[3] = (x + dx, y, z + dz), (x + r.uniform(-4, 4), y, z + r.uniform(-4, 4))
    else:
        places[1], places[3] = (x, y, z), (x + dx, y + dy, z + dz)
    places[2] = (places[1][0] + r.uniform(-3, 3), places[1][1] + r.uniform(-1, 1), places[1][2] + r.uniform(-3, 3))
    places[4] = (x + r.uniform(-4, 4), y + r.uniform(-2, 2), z + r.uniform(-4, 4))

    for k in range(5):
        at = k * PIECE
        for j in range(3):
            put(pieces, at + 0x4c + 4 * j, 'I', r.choice(NANS) if r.random() < nan else fb(places[k][j]))
        put(pieces, at + 0x64, 'II', piece_at(r.choice([3, 4])), piece_at(r.choice([1, 2])))
        put(pieces, at + 0x100, 'I', TYPES + k * TYPE)
        put(types, k * TYPE + 0x18, 'I', r.deal([0, INFOS + k * INFO, INFOS + k * INFO, INFOS + k * INFO]))
        put(infos, k * INFO, 'I', r.getrandbits(32) & ~0x4000 | r.deal([0, 0x4000]))
        put(infos, k * INFO + 0xb0, 'f', r.choice([-0.5, 0.0, 0.5, 0.95, -1.5, r.uniform(0, 60)]))

    put(pieces, 0x64, 'II', piece_at(3) if r.random() < 0.85 else 0, piece_at(1) if r.random() < 0.85 else 0)
    put(pieces, PIECE + 0x68, 'I', piece_at(2) if r.random() < 0.8 else 0)
    put(pieces, 8, 'I', r.getrandbits(32) & ~0x1c00c0 | r.deal([0, 0, 0, 0, 0, 0x80000, 0x40000, 0x40000, 0x100000, 0xc0000]) | r.deal([0, 0x80]) | r.deal([0, 0, 0x40]))

    rules = bytearray(r.bytes(0x40))
    put(rules, 0, 'I', r.getrandbits(32) & ~8 | r.deal([0, 8, 8]))
    put(rules, 8, 'ff', r.uniform(0.1, 2), r.uniform(0.5, 6))
    coaster = bytearray(r.bytes(0x40))
    put(coaster, 0x18, 'I', LIST)
    lst = bytearray(r.bytes(0x70))
    put(lst, 0x64, 'I', r.deal([piece_at(4), piece_at(4), piece_at(3), 0]))

    regions = {RULES: rules, COASTER: coaster, LIST: lst, PIECES: pieces, TYPES: types, INFOS: infos}
    return Call(0x431660, [PIECES, RULES, COASTER], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
