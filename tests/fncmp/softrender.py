# The software renderer's buffer clear and texture memory (src/game/render/softrender: 0x573ca0,
# 0x573e30) against the originals: sizes 0 to 0x200, whole dwords and not; memory sizes around
# the clamps, negative and not a multiple of 4, with and without the option.
# usage: softrender.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Memory)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x573ca0, 0x573cc0), (0x573e30, 0x573e61)])
CALLS = {'Clear': 0x573ca0, 'Memory': 0x573e30}


def case(r):
    op = r.choice(OPS)
    if op == 'Memory':
        phys = r.choice([0, 3, -3, -5, 0x2000000, 0x2000003, 0x1fffffc, 0x6000000, 0x6000004, 0x5ffffff, 0x7fffffff, 0x80000000, r.getrandbits(32)])
        option = r.choice([0, 0, 0x100000, r.getrandbits(32)])
        return Call(0x573e30, [RAM], {0x8bd520: struct.pack('<I', phys & 0xffffffff), 0x8c1620: struct.pack('<I', option), RAM: bytes(4)}, conv='t', ret='a')
    mem = bytearray(r.bytes(0x320))
    put(mem, 8, 'I', RAM + 0x20 + r.randrange(4))
    put(mem, 0xc, 'I', r.choice([0, 1, 3, 4, 5, 0x100, 0x2ff, r.randrange(0x300)]))
    return Call(0x573ca0, [RAM], {RAM: mem}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
