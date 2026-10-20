# The surface wrapper's DirectDraw calls (src/game/gfx/surface: 0x5f8f80-0x5f963e, cdecl) against the originals: two wrappers (the destination
# and the source) whose IDirectDrawSurfaces are COM recorders, the source's on slots 32 up so the log tells them apart. GetSurfaceDesc fills
# its description (bit counts 8, 16, 24, 32 or random, masks of the game's formats or any, small sizes; the description pointer, the stack,
# logged as 0); Blt / BltFast succeed, fail or find the surface lost (IsLost and Restore answering either way), their source rectangle given or
# all of the source, Blt's destination rectangle too, with the rectangles and DDBLTFX on the stack hashed (one a case, the others logged as 0);
# SetColorKey's DDCOLORKEY hashed. Lock fills its description, fails, or finds the surface lost and tries again; DDLOCK_NOSYSLOCK
# (0xfa76b4) set or not. Attach wraps a surface (the wrapper's old bytes random), SetDirectDraw stores 0xfa76b0, GetSurface adds a reference.
# usage: surface.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetDirectDraw,... to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob, fill, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x5f8f80, 0x5f9080), (0x5f9160, 0x5f9650), (0x61af20, 0x61af50), (0x61b090, 0x61b0f0), (0x61b130, 0x61b1f0), (0x5da3c0, 0x5da3c1)])
CALLS = {'SetDirectDraw': 0x5f8f80, 'Attach': 0x5f8f90, 'Restore': 0x5f9040, 'BltFast': 0x5f9160, 'Blt': 0x5f92e0, 'Lock': 0x5f9550,
         'Unlock': 0x5f9600, 'GetSurface': 0x5f9620}
DST, SRC, SURFD, SURFS, VTD, VTS, OUT, DRECT, SRECT, BITS = (RAM, RAM + 0x60, RAM + 0xc0, RAM + 0xd0, RAM + 0x100, RAM + 0x200, RAM + 0x300,
                                                            RAM + 0x310, RAM + 0x320, RAM + 0x330)
ADDREF, BLT, BLTFAST, DESC, ISLOST, LOCK, RESTORE, SETKEY, UNLOCK = 1, 5, 7, 22, 24, 25, 27, 29, 32
LOST = 0x887601c2
ARGS = {ADDREF: 1, 2: 1, BLT: 6, BLTFAST: 6, DESC: 2, ISLOST: 1, LOCK: 5, RESTORE: 1, SETKEY: 3, UNLOCK: 2}
FORMATS = [(8, 0, 0, 0, 0), (16, 0xf800, 0x7e0, 0x1f, 0), (16, 0x7c00, 0x3e0, 0x1f, 0x8000), (24, 0xff0000, 0xff00, 0xff, 0),
           (32, 0xff0000, 0xff00, 0xff, 0xff000000), (32, 0xff, 0xff00, 0xff0000, 0)]


def vt(base):
    """A surface's vtable: slot k a COM-style recorder base + k (the source's on 32 up; its Unlock never called)."""
    return struct.pack('<33I', *[recorder((base + k) & 63, ARGS.get(k, 0), True) for k in range(33)])


def desc(r):
    d = bytearray(r.bytes(0x6c))
    f = r.choice(FORMATS) if r.random() < 0.85 else (r.choice([8, 16, 24, 32, r.getrandbits(32)]),) + tuple(r.getrandbits(32) for _ in range(4))
    put(d, 8, '3i', r.randrange(0, 600), r.randrange(0, 800), r.choice([640, 1280, 2560, r.getrandbits(31)]))
    put(d, 0x24, 'I', BITS)
    put(d, 0x54, '5I', *f)
    return bytes(d)


def surface(r, surf):
    s = bytearray(r.bytes(0x50))
    put(s, 0x24, 'I', surf)
    put(s, 0x14, 'B', r.getrandbits(8))
    return s


def case(r):
    op = r.deal(OPS)
    log = log_region()
    for k in (DESC, DESC + 32):
        strings(log, k, 0, 1 << 1)   # (the description: the stack)
    blob(log, SETKEY + 32, 2, 3)     # (the source's DDCOLORKEY: the stack)
    fill(log, DESC, 1, desc(r), 0)
    fill(log, DESC + 32, 1, desc(r), 1)
    for k in (ISLOST, ISLOST + 32):
        set_return(log, k, r.choice([0, LOST, LOST, 1]))
    for k in (RESTORE, RESTORE + 32):
        set_return(log, k, r.choice([0, 0, 1, r.getrandbits(32)]))
    regions = {DST: surface(r, SURFD), SRC: surface(r, SURFS), SURFD: struct.pack('<I', VTD), SURFS: struct.pack('<I', VTS), VTD: vt(0), VTS: vt(32),
               OUT: r.bytes(4), DRECT: r.bytes(0x10), SRECT: r.bytes(0x10), BITS: r.bytes(4), LOG: log,
               0xfa76b0: struct.pack('<II', r.getrandbits(32), r.choice([0, 0, 1, 0x100]))}
    address = CALLS[op]
    if op == 'SetDirectDraw':
        return Call(address, [r.getrandbits(32)], regions, conv='c', ret='v')
    if op == 'Attach':
        return Call(address, [OUT, SURFD, DST], regions, conv='c', ret='a')
    if op == 'Restore':
        return Call(address, [DST], regions, conv='c', ret='a')
    if op == 'GetSurface':
        if r.random() < 0.2:
            put(regions[DST], 0x24, 'I', 0)
        return Call(address, [DST, r.choice([0, 1, r.getrandbits(32)])], regions, conv='c', ret='a')
    if op == 'Unlock':
        set_return(log, UNLOCK, r.choice([0, LOST, r.getrandbits(32)]))
        return Call(address, [DST], regions, conv='c', ret='a')
    if op == 'Lock':
        strings(log, LOCK, 0, 1 << 2)
        fill(log, LOCK, 2, desc(r), 0)
        set_return(log, LOCK, r.choice([0, 0, LOST, 1, 0x80004005]))
        return Call(address, [OUT, DST, r.choice([BITS, 0])], regions, conv='c', ret='a')
    src_rect = r.choice([SRECT, 0])
    flags = r.getrandbits(32) if r.random() < 0.2 else r.choice([0, 1, 0x10, 0x11, 0x8000, 0x800, 0x8800, 0x1000000, 0x1008800])
    if op == 'BltFast':
        set_return(log, BLTFAST, r.choice([0, 0, LOST, 1]))
        if not src_rect:
            blob(log, BLTFAST, 4, 0xf)
        return Call(address, [OUT, DST, r.getrandbits(32), r.getrandbits(32), SRC, src_rect, flags], regions, conv='c', ret='a')
    set_return(log, BLT, r.choice([0, 0, LOST, 1]))
    dst_rect = r.choice([DRECT, 0])
    stack = [i for i, rect in ((1, dst_rect), (3, src_rect)) if not rect] + [5]   # (the arguments on the stack: one hashed, the others 0)
    hashed = r.choice(stack)
    blob(log, BLT, hashed, 0xf if hashed < 5 else (1 << 25) - 1)
    strings(log, BLT, 0, sum(1 << i for i in stack if i != hashed))
    return Call(address, [OUT, DST, dst_rect, SRC, src_rect, flags], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
