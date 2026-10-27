# The weight map builder of src/game/terrain/terrain_blend (Lnd_BuildWeightMap 0x55e780) against the original: control maps of random texels, tile sizes 4 * 2^n and odd ones
# (which the original treats with shift 0), tile positions and channels.
# The weight maps' exit handlers (0x55e620 / 0x55e680 / 0x55e6e0: W1 0x8798a8, W3 0x879398, W2 0x8798b8): any dwords, the block +0xc
# 0, a heap block or any.
# usage: lndweight.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Build,Free1,Free3,Free2)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x55e780, 0x55eaa0), (0x55e620, 0x55e64f), (0x55e680, 0x55e6af), (0x55e6e0, 0x55e70f), (0x67acb0, 0x67acb1)])
FREES = {'Free1': 0x55e620, 'Free3': 0x55e680, 'Free2': 0x55e6e0}
OPS = [k for k in os.environ.get('OPS', 'Build,Free1,Free3,Free2').split(',') if k]
CONTROL, CDATA, OUT, ODATA = RAM, RAM + 0x100, RAM + 0x40, RAM + 0x10000


def case(r):
    op = r.deal(OPS)
    if op in FREES:
        maps = bytearray(r.bytes(0x30))
        for off in (0xc, 0x1c, 0x2c):   # (the blocks of W3, then W1 and W2)
            put(maps, off, 'I', r.choice([0, HEAP + 0x100, r.getrandbits(32)]))
        return Call(FREES[op], [], {HEAP: heap(), 0x879398: bytes(maps[:0x10]), 0x8798a8: bytes(maps[0x10:])})
    size = r.choice([4, 8, 8, 16, 16, 32, 64, 12, 20, 6, 0, 3, 16, 32])
    x, y, channel = r.randrange(2), r.randrange(2), r.randrange(4)
    pitch = 128
    control = bytearray(r.bytes(0x10))
    put(control, 0, 'III', pitch, 40, 4)
    put(control, 0xc, 'I', CDATA)
    out = bytearray(0x10)
    put(out, 0, 'III', size, size, 1)
    put(out, 0xc, 'I', ODATA)
    n = max(size >> 2, 1)
    texels = bytearray(r.bytes(pitch * 40))
    if r.random() < 0.3:  # smooth gradients and flats
        for i in range(len(texels)):
            texels[i] = r.choice([0, 255, texels[i // 4 * 4] if i >= 4 else 0, texels[i]])
    regions = {CONTROL: control, OUT: out, CDATA: texels, ODATA: r.bytes(max(size * size, 16) * 3 + 0x400)}
    return Call(0x55e780, [CONTROL, x, y, channel, OUT], regions, ret='b')


if __name__ == '__main__':
    B.main(case)
