# The array holder clear and the eleven dword read of src/game/core/small52 (0x5eb090, 0x5eb1a0) against the originals: four array headers with or without blocks
# (Array3D_Free frees them first, so the second free is normally a no-op), and a stream whose slot 2 is a recorder.
# usage: small52.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Read)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

CALLS = {'Clear': 0x5eb090, 'Read': 0x5eb1a0, 'Crop': 0x5ea3f0}
B = Bench([(0x5eb090, 0x5eb180), (0x5eb1a0, 0x5eb280), (0x5d3860, 0x5d38b0), (0x67acb0, 0x67acb1), (0x5c4f60, 0x5c4f90), (0x5ea3f0, 0x5ea520), (0x5e7840, 0x5e78b0)])
VT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    obj = bytearray(r.bytes(0x80))
    if name == 'Clear':
        for off in (0x2c, 0x3c, 0x4c, 0x5c):
            put(obj, off, 'III', r.choice([0, 4, 8]), r.choice([0, 3]), r.choice([0, 4]))
            put(obj, off + 0xc, 'I', r.choice([0, HEAP + 0x100 + off, HEAP + 0x200]))
        return Call(CALLS[name], [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')
    if name == 'Crop':
        sw, sh, sb = r.randrange(0, 6), r.randrange(0, 5), r.choice([1, 1, 2, 3, 4])
        dst = bytearray(r.bytes(0x10))
        src = bytearray(r.bytes(0x10))
        put(src, 0, 'IIII', sw, sh, sb, RAM + 0x1000)
        put(dst, 0, 'IIII', r.randrange(0, 12), r.randrange(0, 12), r.choice([1, 2, 4]), HEAP + 0x1000)
        x, y = r.randrange(-7, 8), r.randrange(-6, 7)
        return Call(CALLS[name], [RAM, RAM + 0x20, x & 0xffffffff, y & 0xffffffff], {RAM: dst, RAM + 0x20: src, RAM + 0x1000: r.bytes(0x400), HEAP: heap()}, ret='b')
    stream = bytearray(r.bytes(0x20))
    put(stream, 0, 'I', VT)
    return Call(CALLS[name], [RAM, RAM + 0x400], {RAM: obj, RAM + 0x400: stream, VT: vtable({2: 2}), LOG: log_region()}, conv='t', ret='b')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
