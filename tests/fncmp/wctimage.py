# The WCT image's deleting destructor (src/game/gfx/wctimage: 0x565000 on the part at +0x2c) and the wavelet codec's shutdown
# (src/game/gfx/wavelet: 0x564b60) against the originals: virtual base offsets 0x2c and others, blocks at +0x20 / +0x24 tracked, untracked or
# none (the tracked blocks' table as in trackedglobal.py), flags with and without bit 0; the shutdown with or without its block (0x879a68)
# and codec (0x879a64, a recorder whose slot 6 is logged); the image's constructor (0x564ba0: most derived or not, virtual base offsets 0x2c
# and others). Decode / DecodeInto (0x564e20 / 0x564ef0, also through their thunks 0x565090 / 0x565080): kind bytes around 0x12, the
# header words any (small often), formats 0-3, the codec a recorder whose slots 1 and 2 are logged; the surface's sizes and pitch any
# (small often), flipped or not. Read (0x564d00): a recorder stream, the header at 0x879a38 random with floats about 0-100 (a few huge or
# NaN) and block sizes small, GlobalAlloc failing for the first or second block now and then.
# usage: wctimage.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Dtor,Shutdown,Ctor,Decode,DecodeInto,Read)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x565000, 0x565070), (0x564b60, 0x564b94), (0x579b80, 0x579c13), (0x564ba0, 0x564bf0), (0x564e20, 0x564ee0), (0x564ef0, 0x564fe2),
           (0x565080, 0x5650a0), (0x564d00, 0x564e12), (0x579b40, 0x579b80), (0x5c4f60, 0x5c4f7c), (0x67a830, 0x67a860)])
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
OBJ, VBT, CODEC, CVT, SURFACE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
BLOCKS = [HEAP + 0x20, HEAP + 0x40, HEAP + 0x3c]


def tracked(r, regions):
    n = r.choice([0, 1, 2, 3, 5])
    table = bytearray(8 * 8)
    for k in range(8):
        b = r.choice(BLOCKS + [r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', b, r.choice([b, b, b + 4]) & 0xffffffff)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', n)


def block(r):
    return r.choice([0] + BLOCKS + [r.getrandbits(32) | 0x80000000])


def dtor(r):
    regions = {HEAP: heap(), **global_heap()}
    tracked(r, regions)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0, 'I', VBT)
    put(obj, 0x20, '2I', block(r), block(r))
    regions[VBT] = put(bytearray(r.bytes(8)), 4, 'I', r.choice([0x2c, 0x2c, 0x2c, 0x30, 0x34, 0x38, 4, 0x24]))
    image = HEAP + 0x100 if r.random() < 0.5 else OBJ
    if image == OBJ:
        regions[OBJ] = obj
    else:   # (inside the heap region: a freed image shows in its hash)
        regions[HEAP][0x100:0x140] = obj
    flags = r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])
    return Call(0x565000, [image + 0x2c, flags], regions, conv='t', ret='a')


def shutdown(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region(), CODEC: struct.pack('<I', CVT), CVT: vtable({6: 0})}
    tracked(r, regions)
    regions[0x879a64] = struct.pack('<2I', r.choice([CODEC, CODEC, 0]), r.choice([0] + BLOCKS + BLOCKS))
    return Call(0x564b60, [], regions, ret='a')


def ctor(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', RAM + 0x300)  # the vtable it starts with (when not most derived): +4 the virtual base offset
    put(obj, 0x304, 'I', r.choice([0x2c, 0x2c, 0x30, 0x34]))
    return Call(0x564ba0, [RAM, r.choice([0, 1, 1])], {RAM: obj}, conv='t', ret='a')


THUNKS = {0x564e20: 0x565090, 0x564ef0: 0x565080}


def decode(r, address):
    obj = bytearray(r.bytes(0x100))
    obj[4] = r.choice([0x12, 0x13, 0x11, 0x14, 0, 0xff, r.randrange(256)])
    for at in range(8, 0x14, 2):
        put(obj, at, 'H', r.choice([0, 1, 2, 7, 0x40, 0xffff, r.getrandbits(16)]))
    regions = {LOG: log_region(), CODEC: struct.pack('<I', CVT), CVT: vtable({1: 14, 2: 11}), 0x879a64: struct.pack('<I', CODEC),
               0x8bd554: bytes([r.randrange(4), 0, 0, 0]), SURFACE: r.bytes(0x10)}
    if address == 0x564ef0:
        regions[SURFACE] = put(bytearray(r.bytes(0x10)), 4, '3I', *(r.choice([0, 1, 2, 0x40, 0x100, r.getrandbits(32)]) for _ in range(3)))
        args = [SURFACE, r.choice([0, 1, 1, r.getrandbits(32)])]
    else:
        args = [SURFACE]
    self = OBJ + 0x2c
    if r.random() < 0.3:   # through the thunk: this the virtual base, the dword before it its offset from the part
        address, self = THUNKS[address], OBJ + 0x80
        put(obj, 0x7c, 'I', 0x80 - 0x2c)
    regions[OBJ] = obj
    return Call(address, [self] + args, regions, conv='t', ret='a')


def read(r):
    regions = {HEAP: heap(fail=r.choice([0] * 6 + [1, 2])), **global_heap(), LOG: log_region(), CODEC: struct.pack('<I', CVT), CVT: vtable({2: 2})}
    tracked(r, regions)
    header = bytearray(r.bytes(0x2c))
    for at in range(0x14, 0x24, 4):
        put(header, at, 'f', r.choice([r.uniform(0, 100), float(r.randrange(0, 40)), 0.5, 1.0, 0.25, -3.0, 1e20, float('nan')]))
    put(header, 0x24, '2I', *(r.choice([0, 1, 0x10, 0x100, r.randrange(0x400)]) for _ in range(2)))
    regions.update({OBJ: bytearray(r.bytes(0x40)), 0x879a38: header})
    return Call(0x564d00, [OBJ, CODEC], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Dtor,Shutdown,Ctor,Decode,DecodeInto,Read').split(',') if k]


def case(r):
    op = r.deal(OPS)
    if op in ('Decode', 'DecodeInto'):
        return decode(r, 0x564e20 if op == 'Decode' else 0x564ef0)
    return {'Dtor': dtor, 'Shutdown': shutdown, 'Ctor': ctor, 'Read': read}[op](r)


if __name__ == '__main__':
    B.main(case)
