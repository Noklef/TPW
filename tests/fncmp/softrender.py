# The software renderer's buffer clear (src/game/render/softrender: 0x573ca0) against the
# original: sizes 0 to 0x200, whole dwords and not.
# usage: softrender.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x573ca0, 0x573cc0)])


def case(r):
    mem = bytearray(r.bytes(0x320))
    put(mem, 8, 'I', RAM + 0x20 + r.randrange(4))
    put(mem, 0xc, 'I', r.choice([0, 1, 3, 4, 5, 0x100, 0x2ff, r.randrange(0x300)]))
    return Call(0x573ca0, [RAM], {RAM: mem}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
