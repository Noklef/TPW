# The screen of static's surface (src/game/gfx/tvstatic: 0x53e650, 0x53e920, 0x53e930) against the original. Create / Init: the allocation
# failing now and then (fncmp's heap); the IDirectDraw at [[0x879a24]] a COM recorder whose CreateSurface (slot 6) logs the description by
# its set dwords (fncmp.blob(): the rest is stack garbage) and the out slot as 0, writes a surface (a recorder whose GetSurfaceDesc fills in
# a size and depth for Surface_Attach), null or nothing through it, and answers 0, a failure or its hash. Free: the Surface at 0x874fd0
# (or none) with its DirectDraw surface (AddRef and Release recorders, or none).
# usage: tvstatic.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create,Init,Free)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, blob, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x53e650, 0x53e759), (0x53e920, 0x53e92b), (0x53e930, 0x53e955), (0x5fa680, 0x5fa697), (0x5f8f90, 0x5f9050),
           (0x5f9620, 0x5f963f)])
HOLDER, DD, DVT, SURF, SVT, OBJ = RAM, RAM + 0x10, RAM + 0x20, RAM + 0x100, RAM + 0x110, RAM + 0x200
GLOBAL = 0x874fd0


def create(r):
    log = log_region()
    blob(log, 6, 1, 0xf | 1 << 0x1a)   # (the size, flags, height, width and caps)
    strings(log, 6, 0, 4)
    if r.random() < 0.85:
        fill(log, 6, 2, struct.pack('<I', r.choice([SURF, SURF, SURF, 0])))
    result = r.choice([0, 0, 0, 1, 0x88760233, None])
    if result is not None:
        set_return(log, 6, result)
    strings(log, 22, 0, 2)
    desc = bytearray(r.bytes(0x6c))
    put(desc, 4, '3I', 0x100f, r.choice([240, 480, 0, r.getrandbits(32)]), r.choice([320, 640, 1, r.getrandbits(32)]))
    put(desc, 0x54, 'I', r.choice([8, 8, 16, 16, 32, r.getrandbits(32)]))
    fill(log, 22, 1, bytes(desc), 1)
    return {HEAP: heap(0x400, r.choice([0, 0, 0, 1])), LOG: log, 0x879a24: struct.pack('<I', HOLDER), HOLDER: struct.pack('<I', DD),
            DD: struct.pack('<I', DVT) + r.bytes(0xc), DVT: vtable({}, 24, com={6: 4}),
            SURF: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({}, 24, com={22: 2}), GLOBAL: r.bytes(4)}


def free(r):
    obj = bytearray(r.bytes(0x50))
    put(obj, 0x24, 'I', r.choice([SURF, SURF, SURF, 0]))
    return {HEAP: heap(0x400), LOG: log_region(), OBJ: obj, GLOBAL: struct.pack('<I', r.choice([OBJ] * 9 + [0])),
            SURF: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({}, 24, com={1: 1, 2: 1})}


def case(r):
    op = r.deal(OPS)
    if op == 'Create':
        return Call(0x53e650, [], create(r), ret='a')
    if op == 'Init':
        return Call(0x53e920, [], create(r))
    return Call(0x53e930, [], free(r))


OPS = [k for k in os.environ.get('OPS', 'Create,Init,Free').split(',') if k]


if __name__ == '__main__':
    B.main(case)
