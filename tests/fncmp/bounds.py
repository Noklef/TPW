# A model's bounds put into its root's frame (src/game/model/bounds: 0x46ed80) against the
# original: the corners copied from the source (often the model itself) and transformed by the
# root's matrix (often affine), NaNs and all.
# usage: bounds.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x46ed80, 0x46eeaa)])
MODEL, SOURCE, ROOT = RAM, RAM + 0x100, RAM + 0x200


def case(r):
    ram = bytearray(r.bytes(0x300))
    scale = r.choice([1, 2, 100, 1000, None])
    r.floats(ram, range(0x80, 0x98, 4), scale)
    r.floats(ram, range(0x180, 0x198, 4), scale)
    r.floats(ram, range(0x210, 0x250, 4), r.choice([1, 2, 100, None]))
    if r.random() < 0.5:  # affine: 0, 0, 0, 1 in the last column
        for o in (0x21c, 0x22c, 0x23c):
            put(ram, o, 'f', 0.0)
        put(ram, 0x24c, 'f', 1.0)
    put(ram, 0x78, 'I', r.choice([ROOT, ROOT, ROOT, MODEL + 0x70]))  # or a matrix over the corners
    source = r.choice([MODEL, MODEL, SOURCE, MODEL + 4])
    return Call(0x46ed80, [source, MODEL], {RAM: ram})


if __name__ == '__main__':
    B.main(case)
