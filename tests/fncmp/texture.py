# The engine textures' small methods (src/game/gfx/texture: 0x590cb0, 0x590cf0, 0x590f80-0x590fb0)
# against the originals: sizes around the powers of two (zero and negative too), flags with and
# without 0x30 and 0x30000, driver kinds 1 and others.
# usage: texture.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=WrapMask,Level,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x590cb0, 0x590d3e), (0x590f80, 0x590fcb)])
CALLS = {'WrapMask': 0x590cb0, 'Level': 0x590cf0, 'ClearCacheFlags': 0x590f80, 'SetKeep': 0x590f90,
         'ClearKeep': 0x590fa0, 'UsesDriverFilter': 0x590fb0}
RETURNS = ('Level', 'UsesDriverFilter')


def size(r):
    p = 1 << r.randrange(12)
    if r.random() < 0.4:
        return r.choice([8, 9, 16, 17, 32, 33, 64, 65, 128, 129])
    return r.choice([p, p - 1, p + 1, 0, -p, 0x8000, r.getrandbits(16)]) & 0xffff


def case(r):
    op = r.choice(OPS)
    texture = bytearray(r.bytes(0x48))
    put(texture, 8, 'I', r.choice([0, 0x30, 0x37, 0x10, 0x7000, 0x10000, 0x30000, r.getrandbits(32)]))
    put(texture, 0x14, 'H', size(r))
    put(texture, 0x16, 'H', size(r))
    regions = {RAM: texture, 0x8bd520: r.bytes(4) + bytes([r.choice([0, 1, 4, 0xff])]) + r.bytes(3)}
    if r.random() < 0.3:
        regions[0x8bd520] = r.bytes(4) + bytes([1, 0, 0, 0])
    return Call(CALLS[op], [RAM], regions, conv='t', ret='a' if op in RETURNS else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
