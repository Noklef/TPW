# The fixed-point transform and the scaled share of src/game/core/small41 (0x50f310, 0x503980) against the originals: matrices and points of every sign and size; the share
# over thingscan.py's chain of things (the game object's handle, the 12-byte table the sums are taken from) with the two global factors and the table entries random.
# usage: small41.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Matrix,Scaled)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import thingscan  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

CALLS = {'Matrix': 0x50f310, 'Scaled': 0x503980}
B = Bench([(0x50f310, 0x50f3b0), (0x503980, 0x5039f0), (0x4c9e60, 0x4c9ed0), (0x4c9ed0, 0x4c9ef0), (0x519510, 0x519550), (0x5da3c0, 0x5da3c1)])
TABLE = RAM + 0x40000


def big(r):
    return r.choice([0, 1, -1, 0x10000, -0x10000, 0x7fffffff, -0x80000000, r.randrange(-0x20000, 0x20000), r.getrandbits(32) - (1 << 31)]) & 0xffffffff


def case(r):
    name = r.choice(OPS)
    if name == 'Matrix':
        m = bytearray(r.bytes(0x30))
        for off in (0x20, 0x24, 0x28, 0x2c):
            put(m, off, 'I', big(r))
        pts = struct.pack('<II', big(r), big(r))
        return Call(CALLS[name], [RAM, RAM + 0x100, RAM + 0x200], {RAM: m, RAM + 0x100: bytes(16), RAM + 0x200: pts}, conv='t', ret='v')
    thingscan.OPS = ['Sum']
    c = thingscan.case(r)
    regions = dict(c.regions)
    regions[0x7852f8] = struct.pack('<I', r.choice([0, 1, 2, 7, 100, r.randrange(1, 1000), r.getrandbits(32)]))
    regions[0x78548c] = struct.pack('<I', r.choice([0, 1, 3, 100, r.randrange(1, 1000), r.getrandbits(32)]))
    regions[TABLE] = r.bytes(0x400)
    regions.setdefault(RAM + 0x1f000, r.bytes(0x1000))
    return Call(CALLS[name], [TABLE, -0x141 + r.randrange(0x30)], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
