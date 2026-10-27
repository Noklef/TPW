# The array holder clear and the eleven dword read of src/game/core/teardown_images (0x5eb090, 0x5eb1a0) against the originals: four array headers with or without blocks
# (Array3D_Free frees them first, so the second free is normally a no-op), and a stream whose slot 2 is a recorder. Ctor (0x5ebfc0): the sign
# renderer's constructor over any bytes, its wavelet codec made as wavelet.py's CREATE (construct()'s regions; few cases: a codec is slow).
# usage: teardown_images.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,Read,Crop,Tiles,Tear,Ctor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wavelet  # noqa: E402

CALLS = {'Clear': 0x5eb090, 'Read': 0x5eb1a0, 'Crop': 0x5ea3f0, 'Tiles': 0x55f210, 'Tear': 0x5ec220, 'Ctor': 0x5ebfc0}
B = Bench([(0x5eb090, 0x5eb180), (0x5eb1a0, 0x5eb280), (0x5d3860, 0x5d38b0), (0x67acb0, 0x67acb1), (0x5c4f60, 0x5c4f90), (0x5ea3f0, 0x5ea520), (0x5e7840, 0x5e78b0), (0x55f210, 0x55f4b0), (0x55f1e0, 0x55f210), (0x5ec220, 0x5ec3a0),
           (0x5ebfc0, 0x5ec20e), (0x5ec210, 0x5ec220), (0x5ebef0, 0x5ebf0a), (0x5eb180, 0x5eb195),
           # (wavelet.py's: the codec's creation and construction)
           (0x5916c0, 0x59196b), (0x591ab0, 0x59205a), (0x592150, 0x593ab0), (0x595920, 0x5985c0), (0x5985c0, 0x598629), (0x598630, 0x598790),
           (0x5f8050, 0x5f8120), (0x5839e0, 0x583a00)])
VT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    obj = bytearray(r.bytes(0x80))
    if name == 'Clear':
        for off in (0x2c, 0x3c, 0x4c, 0x5c):
            put(obj, off, 'III', r.choice([0, 4, 8]), r.choice([0, 3]), r.choice([0, 4]))
            put(obj, off + 0xc, 'I', r.choice([0, HEAP + 0x100 + off, HEAP + 0x200]))
        return Call(CALLS[name], [RAM], {RAM: obj, HEAP: heap()}, conv='t', ret='v')
    if name == 'Ctor':
        c = wavelet.construct(r)
        c.regions[HEAP] = heap(0xb0000, fail=r.choice([0] * 6 + [1, 2]))
        c.regions[RAM + 0x10000] = r.bytes(0x460)
        return Call(CALLS[name], [RAM + 0x10000], c.regions, conv='t', ret='a')
    if name == 'Tear':
        big = bytearray(r.bytes(0x500))
        put(big, 0x14, 'I', RAM + 0x600)
        for group in (0x3c4, 0x358):
            for off in (0x2c, 0x3c, 0x4c, 0x5c):
                put(big, group + off, 'III', r.choice([0, 4, 8]), r.choice([0, 3]), r.choice([0, 4]))
                put(big, group + off + 0xc, 'I', r.choice([0, HEAP + 0x100 + group + off, HEAP + 0x2000 + off]))
        for off in (0x18, 0x28):
            put(big, off + 0xc, 'I', r.choice([0, HEAP + 0x300 + off, HEAP + 0x3000 + off]))
        sub = bytearray(r.bytes(0x20))
        put(sub, 0, 'I', VT)
        return Call(CALLS[name], [RAM], {RAM: big, RAM + 0x600: sub, VT: vtable({6: 0}), LOG: log_region(), HEAP: heap()}, conv='t', ret='v')
    if name == 'Tiles':
        big = bytearray(r.bytes(0x100))
        for off in range(8, 0xd8, 0x10):
            put(big, off, 'III', r.choice([0, 4, 8]), r.choice([0, 3]), r.choice([0, 4]))
            put(big, off + 0xc, 'I', r.choice([0, HEAP + 0x100 + off, HEAP + 0x1000 + off]))
        return Call(CALLS[name], [RAM], {RAM: big, HEAP: heap()}, conv='t', ret='v')
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
