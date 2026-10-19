# The surface that owns its DirectDraw surface (src/game/gfx/surfbuf: 0x6306c0-0x6307bc, thiscall) against the originals, with Surface_Create
# (0x5f9070), Surface_Release (0x5f9130) and Surface_InitHeader (0x5f9000) under it: the IDirectDraw at [0xfa76b0] and the surface it makes are
# COM recorders (CreateSurface 6, its DDSURFACEDESC on the stack logged as a hash of the five dwords it sets, the new surface filled through its out argument; Release 2; GetSurfaceDesc 22, its
# description either hashed as the code sets it or filled with a bit count of 8, 16, 24, 32 or random). Created and borrowed set or not; the creation
# fails now and then; the deleting destructor's flags any, the buffer freed into the heap region.
# usage: surfbuf.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,DeletingDtor,Dtor,Create,Release,SurfCreate,SurfRelease,InitHeader)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, blob, fill, heap, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x6306c0, 0x6307bd), (0x5fa680, 0x5fa6b0), (0x5f9000, 0x5f9031), (0x5f9070, 0x5f912b), (0x5f9130, 0x5f9151), (0x67acb0, 0x67acb1)])
BUF, OUT, SIZE = RAM, RAM + 0x60, RAM + 0x70   # (the buffer, the result, {width, height})
SURF, VTS, DD, VTD = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
CREATE, RELEASE, DESC = 6, 2, 22   # (the COM slots: IDirectDraw::CreateSurface, IDirectDrawSurface::Release, GetSurfaceDesc)
CALLS = {'Ctor': (0x6306c0, 'a'), 'DeletingDtor': (0x630700, 'a'), 'Dtor': (0x630720, 'v'), 'Create': (0x630730, 'a'), 'Release': (0x630790, 'v'),
         'SurfCreate': (0x5f9070, 'a'), 'SurfRelease': (0x5f9130, 'a'), 'InitHeader': (0x5f9000, 'a')}


def case(r):
    op = r.deal(OPS)
    log = log_region()
    blob(log, CREATE, 1, 0xf | 1 << 26)   # (the size, flags, height, width and caps: the original leaves the rest stack garbage)
    fill(log, CREATE, 2, struct.pack('<I', SURF), 0)
    if r.random() < 0.15:
        set_return(log, CREATE, r.choice([1, 0x88760064, r.getrandbits(32)]))
    if r.random() < 0.5:
        desc = bytearray(r.bytes(0x6c))
        put(desc, 0x54, 'I', r.choice([8, 16, 24, 32, r.getrandbits(32)]))
        fill(log, DESC, 1, desc, 1)
    else:
        blob(log, DESC, 1, (1 << 27) - 1)
    buf = bytearray(r.bytes(0x100))
    put(buf, 0x28, 'I', SURF)
    put(buf, 0x54, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    put(buf, 0x58, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(buf, 0x70, '2I', r.choice([1, 64, 640, r.getrandbits(32)]), r.choice([1, 32, 480, r.getrandbits(32)]))
    regions = {BUF: buf, SURF: struct.pack('<I', VTS) + r.bytes(0xc), VTS: vtable({}, com={RELEASE: 1, DESC: 2}), DD: struct.pack('<I', VTD) + r.bytes(0xc),
               VTD: vtable({}, com={CREATE: 4}), 0xfa76b0: struct.pack('<I', DD), LOG: log}
    address, ret = CALLS[op]
    caps = r.choice([0, 0x800, 0x4000, r.getrandbits(32)])
    if op == 'DeletingDtor':
        regions[HEAP] = heap()
        return Call(address, [BUF, r.choice([0, 1, 1, 2, 3, 0xffffffff, r.getrandbits(32)])], regions, conv='t', ret=ret)
    if op == 'Create':
        return Call(address, [BUF, OUT, SIZE, caps], regions, conv='t', ret=ret)
    if op == 'SurfCreate':   # (cdecl: &out, width, height, caps, the surface (the buffer's, at + 4))
        return Call(address, [OUT, *struct.unpack_from('<2I', buf, 0x70), caps, BUF + 4], regions, ret=ret)
    if op == 'InitHeader':   # (cdecl: the surface, width, height, bit count, its + 0x1c)
        return Call(address, [BUF + 4, r.getrandbits(32), r.getrandbits(32), r.choice([8, 8, 16, 32, r.getrandbits(32)]), r.getrandbits(32)], regions, ret=ret)
    if op == 'SurfRelease':
        return Call(address, [OUT, BUF + 4], regions, ret=ret)
    return Call(address, [BUF], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
