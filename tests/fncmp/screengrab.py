# The screen grab's conversion to 24-bit pixels (src/game/gfx/screengrab: 0x561460) against the original: display depths (0x8bcbe4) of 16,
# 24, 32 and others; 0-40 pixels; for 16 bits the display's masks and shifts at 0x8bcc54-0x8bcc74 as 5:6:5, 5:5:5 or any (shift counts past
# 31 or negative), the tracked allocation failing now and then (fncmp's heap; the error then pushed onto the engine's list at 0x879ad0, as in
# ptrarray_add.py) and the tracked table as in trackedglobal.py. And the grab's file name set (0x5617c0): none, or names of 0 to 0x90 characters
# (the longest running past the 0x80 bytes at +0xc, over +0x8c). The next free name (0x550460): Scr%05ld.tga of the counter at 0x879084
# opened (fileopen.py's file library and imports) until CreateFileA fails, after 0-3 opened.
# usage: screengrab.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, fail_after, global_heap, heap, import_table, log_region, put, set_return, strings  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import fileopen  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x561460, 0x561606), (0x5617c0, 0x5617fe), (0x579b40, 0x579c13), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000),
                  (0x550460, 0x55050d)] + fileopen.RANGES), hooks={0x564650: _cdecl_log})
COUNT, TABLE = 0x8bb214, 0x87b1f8   # the tracked blocks: their count and (block, block) pairs
GRAB, PIXELS = RAM, RAM + 0x100
FORMATS = [(0xf800, 11, 5, 0x7e0, 5, 6, 0x1f, 0, 5), (0x7c00, 10, 5, 0x3e0, 5, 5, 0x1f, 0, 5)]


def convert(r):
    log = log_region()
    strings(log, 0x3f, 1)
    regions = {HEAP: heap(0x400, r.choice([0, 0, 0, 1])), **global_heap(), LOG: log}
    depth = r.choice([0x10, 0x10, 0x10, 0x18, 0x20, 0x20, 0, 8, 0x11, r.getrandbits(32)])
    regions[0x8bcbe4] = struct.pack('<I', depth)
    if r.random() < 0.8:
        a, ap, aw, c, cp, cw, b, bp, bw = r.choice(FORMATS)
    else:
        a, c, b = (r.choice([r.getrandbits(16), r.getrandbits(32)]) for _ in range(3))
        ap, aw, cp, cw, bp, bw = (r.choice([r.randrange(17), r.getrandbits(32)]) for _ in range(6))
    regions[0x8bcc54] = struct.pack('<9I', a, ap, aw, c, cp, cw, b, bp, bw)
    table = bytearray(8 * 8)
    for k in range(8):
        v = r.choice([PIXELS, HEAP + 0x10, r.getrandbits(32) | 0x80000000])
        put(table, 8 * k, '2I', v, v)
    regions[TABLE] = table
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 2, 3, 5]))
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    regions[0x879ad0] = errors
    regions[0x8bd508] = struct.pack('<I', r.getrandbits(32))
    n = r.choice([0, 1, 2, 3, 4, 7, 16, 40, r.randrange(41)])
    regions[PIXELS] = r.bytes(0xa0)
    regions[GRAB] = put(bytearray(r.bytes(0x10)), 0, '3I', r.choice([PIXELS, PIXELS, PIXELS, 0]), r.getrandbits(32), r.choice([n, n, n, 0, -1 & 0xffffffff]))
    return Call(0x561460, [GRAB], regions, conv='t', ret='a')


def name(r):   # 0x5617c0 (thiscall, name): copied to +0xc and +0x8c pointed at it; none leaves them; returns 1
    grab = bytearray(r.bytes(0x100))
    n = r.choice([0, 1, 3, 4, 5, 12, 0x7f, 0x80, 0x90, r.randrange(0x90)])
    text = bytes(r.choice(b'abcdefghijklmnopqrstuvwxyz0123456789./') for _ in range(n)) + b'\0'
    return Call(0x5617c0, [GRAB, 0 if r.random() < 0.1 else PIXELS], {GRAB: grab, PIXELS: text + r.bytes(8)}, conv='t', ret='a')


def next_name(r):
    """0x550460 (cdecl): the counter at 0x879084 any (about the int's top now and then); fileopen.py's imports, CreateFileA opening 0-3
    files before it fails (fail_after), GetFileAttributesA mostly a folder (the folder "" opened), the others mostly answering as there."""
    log = log_region()
    for imp, k in fileopen.K.items():
        if imp != 'CreateFileA' and r.random() < 0.9:
            set_return(log, k, r.choice({'GetFileAttributesA': [0x10, 0x10, 0x10, 0x11, 0xffffffff, 0x20], 'ReadFile': [1, 1, 0],
                                         'IsDBCSLeadByte': [0, 0, 1], 'InterlockedDecrement': [0, 1, 1, 2], 'InterlockedIncrement': [1, 2],
                                         'CreateSemaphoreA': [0, 0x44, 0x44], 'WaitForSingleObject': [0, 0, 0x102], 'GetCommandLineA': [0],
                                         'GetFileSize': [0, 0x58, 0x1000]}.get(imp, [0, 1, r.getrandbits(32)])))
    fail_after(log, fileopen.K['CreateFileA'], r.choice([0, 1, 1, 2, 3]))
    base = r.choice([b'C:\\tp\\', b'D:\\'])
    globals_ = bytearray(r.bytes(0x318))
    globals_[0:8] = b'tp.exe\0\0'
    globals_[0x108:0x109 + len(base)] = base + b'\0'
    regions = {LOG: log, HEAP: heap(0x10000), IMPORT_TABLE: import_table(log, fileopen.IMPORTS), fileopen.GLOBALS: globals_,
               fileopen.PATHS: r.bytes(0x210), fileopen.LOCALE: bytes(4), 0x8798d0: r.bytes(0x100),
               0x879084: struct.pack('<i', r.choice([0, 1, 41, 99998, 0x7ffffffe, -3, r.getrandbits(31)]))}
    return Call(0x550460, [], regions, ret='v')


def case(r):
    k = r.deal([0, 0, 0, 1, 2])
    return name(r) if k == 0 else convert(r) if k == 1 else next_name(r)

if __name__ == '__main__':
    B.main(case)
