# The dynamic face models' release (src/game/render/dynfaces: 0x53c810) against the original: the flag at 0x874fcc set or not, and of the three model pointers
# (0x871f64, 0x871ee4, 0x871ed8) one an object whose animator at +0x54 is the holder bench's disposable one (tests/fncmp/holder.py animator_env()), the others
# null (or all null); the dword at 0x763c78 random.
# usage: facemodels.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
from fncmp import Bench, Call, put  # noqa: E402

B = Bench(holder.RANGES + [(0x53c810, 0x53c8b1), (0x424ed0, 0x424ee1)], hooks=holder.HOOKS)
OBJECT = holder.IANIM - 0x54


def case(r):
    regions = holder.animator_env(r)
    regions[OBJECT] = r.bytes(0x54) + bytes(regions.pop(holder.IANIM))
    live = r.choice([0, 1, 2, 3])
    models = bytearray(r.bytes(0x90))   # 0x871ed8 to 0x871f68
    for k, at in enumerate((0x871f64, 0x871ee4, 0x871ed8)):
        put(models, at - 0x871ed8, 'I', OBJECT if k == live else 0)
    regions.update({0x871ed8: models, 0x874fcc: put(bytearray(4), 0, 'I', r.choice([1, 1, 1, 0, 0x80000000])), 0x763c78: r.bytes(4)})
    return Call(0x53c810, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
