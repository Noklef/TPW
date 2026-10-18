# The engine textures' small methods (src/game/gfx/texture: 0x590a00, 0x590c40, 0x590cb0, 0x590cf0,
# 0x590f80-0x590fb0, 0x590d40, 0x590da0, 0x590eb0)
# against the originals: sizes around the powers of two (zero and negative too), flags with and
# without 0x30, 0x300, 0x30000 and 0x4000000, driver kinds 1 and others; the constructor most
# derived or not. Lock / LockFlipped: the pixels or a locked object (a recorder vtable, its stack record logged as a
# hash), flipped rows of widths 2-17 and heights around them, formats 0-3.
# usage: texture.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=WrapMask,Level,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, RAM, blob, log_region, put, vtable  # noqa: E402

B = Bench([(0x590a00, 0x590a70), (0x590c40, 0x590ca8), (0x590cb0, 0x590d3e), (0x590f80, 0x590fcb), (0x590d40, 0x590f7e), (0x67a830, 0x67a860)])
CALLS = {'Ctor': 0x590a00, 'FormatBits': 0x590c40, 'WrapMask': 0x590cb0, 'Level': 0x590cf0, 'ClearCacheFlags': 0x590f80, 'SetKeep': 0x590f90,
         'ClearKeep': 0x590fa0, 'UsesDriverFilter': 0x590fb0, 'Lock': 0x590d40, 'LockFlipped': 0x590da0, 'SetSize': 0x590eb0}
RETURNS = ('Ctor', 'FormatBits', 'Level', 'UsesDriverFilter', 'Lock', 'LockFlipped')
PIXELS, OBJ = RAM + 0x1000, RAM + 0x800


def size(r):
    p = 1 << r.randrange(12)
    if r.random() < 0.4:
        return r.choice([8, 9, 16, 17, 32, 33, 64, 65, 128, 129])
    return r.choice([p, p - 1, p + 1, 0, -p, 0x8000, r.getrandbits(16)]) & 0xffff


def lock(r, op):
    texture = bytearray(r.bytes(0x60))
    flags = r.getrandbits(32) & ~0x10040000 | r.choice([0, 0x10000000]) | (0x40000 if op == 'LockFlipped' and r.random() < 0.85 else r.choice([0, 0x40000]))
    put(texture, 8, 'I', flags)
    w = r.choice([2, 3, 4, 5, 8, 16, 17, r.randrange(2, 18)])
    h = r.choice([0, 1, 2, 3, w, w - 1, w + 1, 2 * w, r.randrange(0, 2 * w + 1), r.choice([-2, -1, 0x8001])])
    put(texture, 0x10, 'H', w)
    put(texture, 0x12, 'H', h & 0xffff)
    log = log_region()
    regions = {RAM: texture, PIXELS: r.bytes(w * w * 4), 0x8bd554: bytes([r.randrange(4), 0, 0, 0]), LOG: log}
    if flags & 0x10000000:
        put(texture, 0x40, 'I', OBJ)
        regions[OBJ] = bytearray(4) + r.bytes(4)
        put(regions[OBJ], 0, 'I', OBJ + 0x100)
        regions[OBJ + 0x100] = vtable({2: 1}, 8)
        blob(log, 2, 0, 0xf)
        regions[0x879a68] = bytearray(4)
        put(regions[0x879a68], 0, 'I', PIXELS)
    else:
        put(texture, 0x40, 'I', PIXELS)
    return Call(CALLS[op], [RAM], regions, conv='t', ret='a')


def case(r):
    op = r.choice(OPS)
    if op in ('Lock', 'LockFlipped'):
        return lock(r, op)
    texture = bytearray(r.bytes(0x60))
    put(texture, 8, 'I', r.choice([0, 0x30, 0x37, 0x10, 0x7000, 0x10000, 0x30000, r.getrandbits(32)]) | r.choice([0, 0x100, 0x200, 0x300, 0x4000000]))
    put(texture, 0x14, 'H', size(r))
    put(texture, 0x16, 'H', size(r))
    regions = {RAM: texture, 0x8bd520: r.bytes(4) + bytes([r.choice([0, 1, 4, 0xff])]) + r.bytes(3)}
    if r.random() < 0.3:
        regions[0x8bd520] = r.bytes(4) + bytes([1, 0, 0, 0])
    if op == 'SetSize':   # (any high halves; the float +0x30 about 0..1.5 mostly, flags 0x4000 and 0x80000 often)
        if r.random() < 0.8:
            put(texture, 0x30, 'f', r.choice([r.uniform(0, 1.5), 0.0, 1.5, -2.0]))
        put(texture, 8, 'I', r.getrandbits(32) & ~0x84000 | r.choice([0, 0x4000]) | r.choice([0, 0x80000]))
        return Call(CALLS[op], [RAM, size(r) | r.getrandbits(16) << 16, size(r) | r.getrandbits(16) << 16], regions, conv='t', ret='v')
    args = [RAM, r.choice([0, 1, r.getrandbits(32)])] if op == 'Ctor' else [RAM]
    return Call(CALLS[op], args, regions, conv='t', ret='a' if op in RETURNS else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
