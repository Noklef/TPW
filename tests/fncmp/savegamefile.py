# A saved game's file (src/game/world/savegamefile: 0x414920, 0x414d40, 0x415140) against the originals: every callee a recorder but Stream_Write and, from the load, 0x415140; the local streams' opens set their status and handle (fill_this()),
# the handle now and then null; the local streams logged by their vtable, those passed on by vtable and handle; each step good in most cases so that the end is reached, one failing else. A file's read and write and the stream's write are vtable slots 1, 2, 3.
# usage: savegamefile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, echo, fill, fill_this, log_fold, log_region, mask_this, recorder, set_return, strings, this_first, vtable  # noqa: E402

SAVE, LOAD, AFTER = 0x414920, 0x414d40, 0x415140
READ, WRITE, SWRITE = 1, 2, 3
CTOR, DTOR, CLOSE, SEEK, HEADW, BODYW, HEADR, BODYR, SIZE, OPENMEM, OPENPATH = 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
UNPACK, UNPACKED, COMPRESS, OPEN, SCLOSE, DELETE, ALLOC, FREE = 24, 25, 26, 27, 28, 29, 30, 31
TH = {0x5f7180: (CTOR, 0), 0x5f71e0: (DTOR, 0), 0x5f71f0: (CLOSE, 0), 0x5f7210: (SEEK, 2), 0x415f50: (HEADW, 2), 0x4164c0: (BODYW, 2), 0x416240: (HEADR, 3), 0x415270: (BODYR, 2), 0x639100: (SIZE, 0), 0x5f72c0: (OPENMEM, 2), 0x5f7ed0: (OPENPATH, 3),
      0x402e60: (16, 0), 0x4d8330: (17, 0), 0x5191e0: (18, 0), 0x515fb0: (19, 0), 0x515f30: (20, 0), 0x4815d0: (21, 0), 0x519120: (22, 0), 0x4d83a0: (23, 0), 0x4d3e00: (27, 0), 0x402e80: (28, 0), 0x4815e0: (29, 0)}
CD = {0x5f8100: (UNPACK, 3), 0x5f8120: (UNPACKED, 1), 0x5f8050: (COMPRESS, 4), 0x5f5ad0: (OPEN, 4), 0x5f5b80: (SCLOSE, 2), 0x5f5d60: (DELETE, 2), 0x67acc0: (ALLOC, 1), 0x67acb0: (FREE, 1), 0x52f8d0: (8, 2), 0x457e10: (9, 0), 0x42a190: (10, 0), 0x5445d0: (11, 0),
      0x437150: (15, 0), 0x5584b0: (16, 0), 0x5996d0: (17, 1), 0x5508c0: (18, 0), 0x550930: (19, 0), 0x55e3b0: (20, 0), 0x450d10: (21, 0), 0x473440: (22, 0), 0x468500: (23, 0), 0x51c1c0: (1, 1), 0x51fd80: (2, 1), 0x475f40: (3, 0)}
HOOKS = {a: _recorder(k, n) for a, (k, n) in TH.items()}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()})
B = Bench([(0x414920, 0x414d30), (0x414d40, 0x41513f), (0x415140, 0x415261), (0x5f5e50, 0x5f5e80), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
SELF, NAME, FILE, MEMORY, START, VT, ALLOCS, BLOCKS, FONTS, FONT_VT, FONT_NAMES, READS = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x90, RAM + 0xa0, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x1000, RAM + 0x1100, RAM + 0x1180, RAM + 0x2000


def table(*values):
    return struct.pack('<40I', *[v & 0xffffffff for v in values] + [0] * (40 - len(values)))


def status(r):
    return struct.pack('<i', r.choice([0, 0, 0, 0, 0, 1, 1, -1, -0x7fffffff]) if r.randrange(4) else 0)


def font_regions(r):
    count = r.choice([0, 1, 2, 3])
    fonts = bytearray(r.bytes(0x14 * 4))
    for i in range(4):
        struct.pack_into('<I', fonts, 0x14 * i, FONT_VT)
        struct.pack_into('<I', fonts, 0x14 * i + 8, FONT_NAMES + 8 * i)
    return {FONTS: fonts, FONT_VT: vtable({SWRITE: 1}), FONT_NAMES: r.bytes(0x20), 0x789c1c: struct.pack('<2I', FONTS, FONTS + 0x14 * count), 0x6fd058: struct.pack('<I', recorder(40, 1, True)), 0x879070: r.bytes(4),
            0x78d8e0: struct.pack('<I', r.choice([0, 1, r.getrandbits(32)]))}


def case(r):
    log = log_region()
    log_fold(log)
    strings(log, DELETE, 0, 1)
    regions = {LOG: log, 0x7cf83c: r.bytes(4)}
    regions.update(font_regions(r))
    op = r.choice([SAVE, SAVE, LOAD, LOAD, LOAD, AFTER])
    if op == AFTER:
        return Call(AFTER, [r.getrandbits(32)], regions, conv='t', ret='v')
    mask_this(log, CTOR)
    for k in DTOR, CLOSE, SEEK, SIZE, OPENMEM, OPENPATH:
        this_first(log, k)
    for k in OPEN, OPENMEM, OPENPATH:
        echo(log, k, 0)
    strings(log, OPEN, 0, 3)
    strings(log, OPENMEM, 0, 1)
    strings(log, OPENPATH, 0, 1)
    strings(log, SCLOSE, 0, 1)
    for k in BODYW, HEADR, BODYR:
        blob(log, k, 0, 3)
    fill(log, OPEN, 0, status(r), which=0)
    fill(log, OPEN, 1, struct.pack('<I', START), which=1)
    fill(log, OPENMEM, 0, status(r), which=2)
    fill(log, OPENPATH, 0, status(r), which=3)
    fill_this(log, OPENMEM, struct.pack('<I', r.choice([MEMORY, MEMORY, MEMORY, MEMORY, MEMORY, 0])), 4, which=4)
    fill_this(log, OPENPATH, struct.pack('<I', r.choice([FILE, FILE, FILE, FILE, FILE, 0])), 4, which=5)
    size = r.choice([0x10, 0x100, r.getrandbits(32)])
    position = r.choice([0, 4, 0x20, size])
    set_return(log, SIZE, size)
    set_return(log, SEEK, position)
    for k in HEADW, BODYW, HEADR, BODYR:
        set_return(log, k, r.choice([1, 1, 1, 1, r.getrandbits(32) | 1, 0]))
    set_return(log, SWRITE, r.choice([4, 0, -1]))
    length = r.choice([0x20, 0x40, 0xffffffff, r.getrandbits(32)])
    set_return(log, COMPRESS, length)
    set_return(log, UNPACKED, r.choice([0x40, 0x40, 0x40, r.getrandbits(32), 0]))
    regions.update({SELF: r.bytes(0x10), NAME: r.bytes(8), FILE: struct.pack('<I', VT), MEMORY: struct.pack('<I', VT), START: struct.pack('<I', VT), VT: vtable({READ: 2, WRITE: 2, SWRITE: 2}), BLOCKS: r.bytes(0x400),
                    ALLOCS: table(*[BLOCKS + 0x100 * i if r.randrange(8) else 0 for i in range(3)])})
    answers(log, ALLOC, ALLOCS)
    if op == SAVE:
        set_return(log, WRITE, r.choice([length, length, length, r.getrandbits(32)]))
        set_return(log, READ, r.getrandbits(32))
    else:
        left = (size - position) & 0xffffffff
        regions[READS] = table(r.choice([4, 4, 4, 4, 4, 3]), r.choice([left, left, left, left, r.getrandbits(32)]))
        answers(log, READ, READS)
        set_return(log, WRITE, r.choice([0, 1, 0x40, 0x40, -1]))
        fill(log, READ, 0, struct.pack('<i', r.choice([0, 5, 0x1f4, 0x1f5, -1])), which=6)
        strings(log, READ, 0, 1)   # (the magic: a stack slot)
    mode = r.choice([0, 1, 1, 2, 2, 3, r.getrandbits(32)])
    return Call(op, [SELF, NAME, r.getrandbits(32), mode], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
