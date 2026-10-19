# The mesh draws' add (src/game/render/meshdraws: 0x581cd0) against the original: the list's count 0 to past 0x1000, or -1 (only the
# entry it would fill laid out: the full check is signed, so -1 fills the entry before the list), the renderer at 0x87a6d0 with its +0x30, the mesh (+0 flags with 0x1000000 or not, +0x58 vertex count 0 to any
# word, +0x60 / +0x64 / +0x9c pointers) and flags any; the draw buffer at 0x8bcbc4 with a scratch area that fits the vertices or not
# (DrawBuffer_Alloc runs); the transformer at 0x87b1d8 with a cdecl recorder in slot 2 logging the seven arguments; the engine's error
# list at 0x879ad0 (its count 0 to 0x40) for the full list.
# usage: meshdraws.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402

B = Bench([(0x581cd0, 0x581da0), (0x582640, 0x582670), (0x56af20, 0x56af9c)])
LIST, MESH, REND, BUF, XF, VT, SCRATCH, ERRORS = RAM + 0x40, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x8000, 0x879ad0


def case(r):
    log = log_region()
    count = r.choice([0, 1, 2, 7, 0xffe, 0xfff, 0x1000, 0x1001, 0x7fffffff, -1, r.randrange(0x1000)])  # (-1: the entry before the list, sic)
    mesh = bytearray(r.bytes(0xa0))
    put(mesh, 0, 'I', r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0))
    put(mesh, 0x58, 'H', r.choice([0, 0, 1, 3, 0x10, 0x100, 0x1000, 0xffff, r.getrandbits(16)]))
    buf = bytearray(r.bytes(0x60))
    put(buf, 0x5c, 'I', SCRATCH)
    put(buf, 0x3c, 'I', SCRATCH + r.choice([0, 0x10, 0x100, 0x1000, 0x10000, 0x100000]))
    errors = bytearray(r.bytes(0x204))
    put(errors, 0x200, 'i', r.choice([0, 1, 0x3f, 0x40, r.randrange(0x41)]))
    regions = {LOG: log, MESH: mesh, REND: r.bytes(0x34), BUF: buf, ERRORS: errors,
               XF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 4, cdecl={2: 7}),
               0x87a6d0: struct.pack('<I', REND), 0x8bcbc4: struct.pack('<I', BUF), 0x87b1d8: struct.pack('<I', XF),
               0x8bb624: r.bytes(4), 0x8bd508: r.bytes(4)}
    if count == -1:  # (the entry before the list ends with the count itself: one region)
        regions[LIST - 0x10] = r.bytes(0x10) + struct.pack('<i', count)
    else:
        regions[LIST] = struct.pack('<i', count)
        if 0 <= count < 0x1000:
            regions[LIST + 4 + count * 0x14] = r.bytes(0x14)
    args = [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), MESH, r.getrandbits(32)]
    return Call(0x581cd0, [LIST] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
