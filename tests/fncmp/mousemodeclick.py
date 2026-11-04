# What two mouse modes do with the tile under the pointer (src/game/world/mousemodeclick: 0x46cc40, 0x46cdc0, 0x46d000, 0x46d0d0) against the originals, all callees recorders answering in turn: the tile's id any, the tests either way,
# the allocations failing now and then (the modes' fields by the blocks' regions), the holder made or missing, the guard made or not, online or not. The ids passed as words logged as words: their upper half is a stale stack slot.
# usage: mousemodeclick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, fill, log_region, set_return, strings  # noqa: E402

CURSOR, FLAG, FLAG2, OBJECT, IS_PART, BUSY, SET_MODE, HOLDER_CTOR, MADE, WINDOWED, OPEN = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
Q1, Q2, Q3, Q4, Q5, PLACER, PLACE = 12, 13, 14, 15, 16, 17, 18
ALLOC, ATEXIT, PREVIEW, FIND, WALK, KILL = 19, 20, 21, 22, 23, 24
TH = {0x4814a0: (CURSOR, 1), 0x535db0: (FLAG, 1), 0x5363a0: (FLAG2, 0), 0x4d0af0: (OBJECT, 0), 0x53ad90: (IS_PART, 0), 0x53ad60: (BUSY, 0), 0x46c350: (SET_MODE, 1), 0x46d550: (HOLDER_CTOR, 0), 0x550ca0: (MADE, 0), 0x4dd4e0: (WINDOWED, 0),
      0x4e15b0: (OPEN, 1), 0x536450: (Q1, 0), 0x5363f0: (Q2, 0), 0x536390: (Q3, 0), 0x536310: (Q4, 0), 0x536320: (Q5, 0), 0x519450: (PLACER, 0), 0x509af0: (PLACE, 1)}
CD = {0x67acc0: (ALLOC, 1), 0x67a7d0: (ATEXIT, 1), 0x5234d0: (PREVIEW, 3), 0x527e80: (FIND, 3), 0x42ae70: (WALK, 0), 0x475550: (KILL, 1)}
HOOKS = {a: _recorder(k, n) for a, (k, n) in TH.items()}
HOOKS.update({a: _recorder(k, n, cdecl=True) for a, (k, n) in CD.items()})
B = Bench([(0x46cc40, 0x46cdb9), (0x46cdc0, 0x46ce74), (0x46d000, 0x46d0ce), (0x46d0d0, 0x46d548), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
SELF, FLAGS, ALLOCS, THING, PLACE_OBJ, BLOCKS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x1000, RAM + 0x2000
OBJ = RAM + 0x3000


def dwords(*values):
    return struct.pack('<40I', *[v & 0xffffffff for v in values] + [0] * (40 - len(values)))


def case(r):
    log = log_region()
    op = r.choice(['Click', 'Cancel', 'Hover', 'ModeClick', 'ModeClick'])
    tile_id = r.choice([0, 1, 2, 0x80, 0x81, 0x100, 0xffff, r.getrandbits(16)]) if op != 'Click' else r.getrandbits(16)
    level = r.choice([r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), -(0x294 + tile_id * 0x44)]) & 0xffffffff
    found = r.choice([0, 1, 3, 5])
    records = bytearray(r.bytes(0x14 * 6))
    for i in range(6):
        struct.pack_into('<I', records, 0x14 * i, r.choice([PLACE_OBJ, THING + 0x40 * i]) if op == 'Cancel' else THING + 0x40 * i)
    blocks = [BLOCKS + 0x10 * i for i in range(4)]
    allocs = [r.choice([b, b, b, 0]) for b in blocks]
    regions = {LOG: log, SELF: r.bytes(0x10), 0x7b05cc: struct.pack('<H', tile_id), 0x7cf83c: struct.pack('<I', level), 0x7b05e8: struct.pack('<I', r.choice([0, 0, r.getrandbits(32)])), 0x7b05e0: r.bytes(4), 0x7b05d4: r.bytes(4),
               0xfb3518: bytes([r.choice([0, 1, 0xfe, 0xff])]) + r.bytes(3), 0xfb3b7c: struct.pack('<I', r.choice([0, 1, 1, 2])), 0x7cfb90: records, BLOCKS: r.bytes(0x40), THING: r.bytes(0x600), PLACE_OBJ: r.bytes(0x100), OBJ: r.bytes(0x20),
               FLAGS: dwords(r.choice([0, 1]), r.choice([0, 1])), ALLOCS: dwords(*allocs)}
    answers(log, FLAG, FLAGS)
    answers(log, ALLOC, ALLOCS)
    for k in FLAG2, IS_PART, BUSY, Q1, Q2, Q3, Q4, Q5:
        set_return(log, k, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, OBJECT, OBJ)
    set_return(log, HOLDER_CTOR, r.getrandbits(32))
    set_return(log, WINDOWED, r.choice([THING, THING + 0x40, THING + 0x80]))
    set_return(log, PLACER, PLACE_OBJ)
    struct.pack_into('<I', log, LOG_BYTES + 4 * PLACE, 1 << 16)
    strings(log, FIND, 0, 1)
    fill(log, FIND, 0, struct.pack('<H', found), which=0)
    address = {'Click': 0x46cc40, 'Cancel': 0x46cdc0, 'Hover': 0x46d000, 'ModeClick': 0x46d0d0}[op]
    args = {'Click': 2, 'Cancel': 0, 'Hover': 4, 'ModeClick': 2}[op]
    if op == 'Cancel':
        block = bytearray(regions[PLACE_OBJ])
        struct.pack_into('<H', block, 0x60, r.choice([0, found, found]))
        regions[PLACE_OBJ] = block
        me = bytearray(regions[SELF])
        struct.pack_into('<I', me, 8, r.choice([0, r.getrandbits(32)]))
        regions[SELF] = me
    return Call(address, [SELF] + [r.getrandbits(32) for _ in range(args)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
