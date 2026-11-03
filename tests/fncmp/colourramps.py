# The five colour ramps (src/game/gfx/colourramps: ColourRamps_Build 0x46be50) against the original, with Palette_Nearest as it is, and the object's six pointers to
# separate places or one (their copies in order) or into the object itself (a copy then read by a later one). The palette (see palette()) holds the ramps' colours first.
# usage: colourramps.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x46be50, 0x46c00f), (0x5f73a0, 0x5f76b7)])
OBJECT, TARGETS = RAM, RAM + 0x200
POINTERS = (0x34, 0x6c, 0xa4, 0xdc, 0x114, 0x118)


def palette(r):
    """The ramps' 46 colours shuffled into the first entries (Palette_Nearest stops at an exact one: a case costs about a twentieth of
    50 searches of 256 entries, and each colour's entry says which it asked for); the rest random. Now and then all random."""
    data = bytearray(r.bytes(0x400))
    if r.random() < 0.005:
        return data
    levels = [i * 0xff // 9 for i in range(10)]
    colours = sorted({c for v in levels for c in ((v, 0, 0), (0, v, 0), (0, 0, v), (v, v, 0), (v, v, v))})
    r.shuffle(colours)
    for k, colour in enumerate(colours):
        data[4 * k:4 * k + 3] = bytes(colour)
    return data


def case(r):
    obj = bytearray(r.bytes(0x120))
    how = r.randrange(3)
    for k, at in enumerate(POINTERS):
        to = [TARGETS + 8 * k, TARGETS, OBJECT + r.choice([0xe0, 0x10d, 0xc1, 0x89, 0x51, 0x19, 0x108, 0xf0])][how]
        put(obj, at, 'I', to)
    return Call(0x46be50, [OBJECT], {OBJECT: obj, TARGETS: r.bytes(0x40), 0xfa71f8: palette(r)}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
