# World_Save / World_Load (src/game/world/worldsave) against the originals. The world is 0x1da748 bytes in the game; only its 25 fields are read, so a
# region at +0x1da700 stands for it. Save: the mouse mode as focus.py's modes_case (a recorder object or none, the holder set or not), a list of 0-8 slots
# from [0x7cf56c] (now and then none: a fault in both), the things' kind bytes 0-20, a sink with room or short by a little. Load: a stream built as Save
# writes it: a dword, the fields (+0x1da738 4 half the time), the first handle and for each thing its next's and its kind (all of them; 13's big block
# seldom, 0 and 20 past the table). Cut at a random place or a serializer answering non-zero in a third of the cases; about one in ten runs to the end
# with the whole table as a region (200 KB), for the free list. The serializers and Things_InitTable (it writes the whole table) are recorders.
# usage: worldsave.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, memory_stream, put, set_return, vtable  # noqa: E402

CALLS = {'save': 0x516c80, 'load': 0x5179c0}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
SERIALIZERS = {0x4d3aa0: 13, 0x4d7a70: 14, 0x4d7ea0: 15, 0x4fb530: 16, 0x4db7d0: 17, 0x4da110: 18, 0x4d6d60: 19, 0x4d4460: 20, 0x4d6000: 21, 0x502760: 22,
               0x508bb0: 23, 0x4da960: 24, 0x599f60: 25, 0x509bc0: 26, 0x4c5d70: 27, 0x502dc0: 28, 0x512010: 29, 0x4cf920: 30, 0x4d2320: 31}
KIND_K = {1: 16, 3: 17, 4: 18, 5: 19, 6: 20, 7: 21, 8: 22, 9: 23, 10: 24, 11: 25, 12: 26, 13: 27, 14: 28, 15: 29, 16: 30, 17: 26, 19: 31}
SIZES = {1: 0x22c, 3: 0x450, 4: 0x220, 5: 0x220, 6: 0x218, 7: 0x218, 8: 0x218, 9: 0x68, 10: 0xe, 11: 0x16f0, 12: 0x80e, 13: 0x21c50, 14: 0x146c, 15: 0x68, 16: 0x128,
         17: 0xc, 19: 0x3e8}
KINDS = [k for k in range(0, 21) if k != 13]
FIELD_SIZES = [2, 2, 2, 2, 2, 4, 2, 2, 4, 4, 2, 2, 4, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2]   # (worldsave.cpp's table; the 19th is +0x1da738)
FIELDS_BYTES = sum(FIELD_SIZES)

MODES = [(0x40cb40, 0x40cb90), (0x46c350, 0x46c460), (0x46c6a0, 0x46c6c0), (0x46cff0, 0x46d010), (0x550ca0, 0x550d90), (0x5da3c0, 0x5da3c1), (0x46ce80, 0x46ceca),
         (0x46cfb0, 0x46cfc0), (0x40f3a0, 0x40f3a6), (0x40f440, 0x40f446), (0x5d1410, 0x5d1416), (0x46d590, 0x46d59d), (0x46c710, 0x46c72e), (0x46ced0, 0x46ced7),
         (0x5d1750, 0x5d1751), (0x67a740, 0x67a800), (0x46d550, 0x46d5a0), (0x4814a0, 0x481500)]
CTORS = [(0x401000, 0x401040), (0x4c5d10, 0x4c5d80), (0x4cdb60, 0x4cdb90), (0x4ce9e0, 0x4cea10), (0x4d0f70, 0x4d0fa0), (0x504750, 0x504770), (0x50b080, 0x50b0a0),
         (0x50ba80, 0x50bac0), (0x50c620, 0x50c650), (0x50ffe0, 0x5100f0), (0x513290, 0x5132b0), (0x518e00, 0x518eb0), (0x51ad30, 0x51ad50), (0x51ade0, 0x51aec0),
         (0x5408e0, 0x540910), (0x5c5520, 0x5c5540), (0x67acf0, 0x67ada0), (0x67b050, 0x67b0d0)]


EXTRA = [(0x50f1f0, 0x50f210)]


def merged(ranges):
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged([(0x516c80, 0x51796b), (0x5179c0, 0x518db2), (0x416e10, 0x416e30), (0x5f5bb0, 0x5f5bd0), (0x5f5e20, 0x5f5e90)] + MODES + CTORS + EXTRA),
          hooks={0x515f30: _recorder(1, 0, cdecl=True), **{addr: _recorder(k, 4) for addr, k in SERIALIZERS.items()}})
WORLD = RAM + 0x100000
FIELDS = WORLD + 0x1da700
MODE, MODE_VT, HOLDER = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200
STREAM, THINGS, DATA = RAM + 0x1000, RAM + 0x2000, RAM + 0x4000
LIST_HEAD, TABLE, TABLE_END, FREE_HEAD = 0x7cf56c, 0x7cfb90, 0x801bc0, 0x801bbc
SLOT = 20


def handles(r, n):
    h = r.sample(range(1, 40), n)
    if h and r.random() < 0.1:
        h[0] = 0   # (slot 0 as the first)
    return h


def save_case(r, regions, log):
    regions[HEAP] = heap(0x1000, r.choice([0] * 8 + [1, 2]))
    regions[MODE_VT] = vtable({0: 1})
    regions[MODE] = put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT)
    set_return(log, 9, r.choice([9, 9, 9, 1, 7, 0, r.getrandbits(32)]))
    regions[0x7b05d4] = put(put(bytearray(r.bytes(0x18)), 4, 'I', r.choice([MODE, MODE, 0])), 0x14, 'I', r.choice([0, HOLDER]))
    regions[0xfb3518] = bytes([r.choice([1, 3, 0xff])])
    regions[0xfb3b7c] = struct.pack('<I', 1)
    n = r.randint(1, 8)
    hs = handles(r, n)
    table = bytearray(r.bytes(SLOT * 41))
    things = bytearray(r.bytes(0x10 * n))
    for i, h in enumerate(hs):
        kind = r.choice(KINDS + [13])
        things[0x10 * i + 2] = kind
        at = SLOT * h
        put(table, at, 'I', THINGS + 0x10 * i)
        put(table, at + 4, 'I', r.getrandbits(32))
        put(table, at + 8, 'I', TABLE + SLOT * hs[i + 1] if i + 1 < n else 0)
    regions[TABLE] = table
    regions[THINGS] = things
    regions[LIST_HEAD] = struct.pack('<I', 0 if r.random() < 0.03 else TABLE + SLOT * hs[0])
    room = 4 + FIELDS_BYTES + 4 + 8 * n
    if r.random() < 0.2:
        room = r.randrange(room)
    elif r.random() < 0.25:
        k = r.choice([13, 14, 15] + [KIND_K[things[0x10 * i + 2]] for i in range(n) if things[0x10 * i + 2] in KIND_K])
        set_return(log, k, r.choice([1, 0xffffffff, r.getrandbits(32) | 1]))
    regions[STREAM] = memory_stream(STREAM, DATA, room)
    regions[DATA] = bytearray(r.bytes(room + 0x10))


def load_case(r, regions, log):
    finish = r.random() < 0.1
    n = r.randint(0, 8)
    hs = handles(r, n + 1)
    kinds = [r.choice(KINDS) for _ in range(n + 1)]
    if r.random() < 0.05:
        kinds[r.randrange(n + 1)] = 13
    data = bytearray(r.bytes(4 + FIELDS_BYTES))
    if r.random() < 0.5:
        put(data, 4 + sum(FIELD_SIZES[:18]), 'I', 4)
    data += struct.pack('<I', hs[0])
    for i, h in enumerate(hs):
        nx = hs[i + 1] if i < n else 0
        data += struct.pack('<II', nx, kinds[i])
    if not finish and r.random() < 0.3:
        data = data[:r.randrange(len(data))]
    elif not finish and r.random() < 0.3:
        k = r.choice([13, 14, 15] + [KIND_K[x] for x in kinds if x in KIND_K])
        set_return(log, k, r.choice([1, 0xffffffff, r.getrandbits(32) | 1]))
    regions[STREAM] = memory_stream(STREAM, DATA, len(data))
    regions[DATA] = bytes(data) if data else b'\0'
    size = 0x100 + sum(SIZES.get(k, 0) + 0x10 for k in kinds)
    regions[HEAP] = heap(size, r.randint(1, n + 1) if r.random() < 0.05 else 0)
    regions[LIST_HEAD] = r.bytes(4)
    regions[0x763f84] = r.bytes(1)   # (the sprites' draw flag)
    if finish:
        table = bytearray(r.bytes(TABLE_END - TABLE))
        empty = r.choice([0, 0.02, 0.5, 1])
        for i in range(10240):
            if r.random() < empty:
                put(table, SLOT * i, 'I', 0)
        regions[TABLE] = table
    else:
        regions[TABLE] = r.bytes(SLOT * 41)


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, FIELDS: bytearray(r.bytes(0x50))}
    for k in range(13, 32):   # (a recorder's answer is its own number otherwise: a serializer failing)
        set_return(log, k, 0)
    (save_case if op == 'save' else load_case)(r, regions, log)
    return Call(CALLS[op], [WORLD, STREAM], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
