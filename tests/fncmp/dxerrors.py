# The DirectDraw / Direct3D result messages (src/game/render/dxerrors: 0x56a770) against the original: every code of the DirectDraw facility from
# 0x88760000 to 0x887602ff, the renderer's own 0xb0753ffe-0xb075401b, the system codes it names (and their neighbours), dealt so that a run of 900
# cases or more meets each, with random codes among them.
# usage: dxerrors.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x56a770, 0x56ae60)])
SYSTEM = [0, 1, 0xffffffff, 0x80000000, 0x80004000, 0x80004001, 0x80004002, 0x80004004, 0x80004005, 0x80004006, 0x8007000d, 0x8007000e, 0x8007000f,
          0x80070056, 0x80070057, 0x80070058]
CODES = [0x88760000 + k for k in range(0x300)] + [(0xb0754000 + k) & 0xffffffff for k in range(-2, 0x1c)] + SYSTEM + ['random'] * 64


def case(r):
    code = r.deal(CODES)
    return Call(0x56a770, [r.getrandbits(32) if code == 'random' else code], {}, ret='a')


if __name__ == '__main__':
    B.main(case)
