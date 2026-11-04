# The texture cache's surfaces of all sizes made (src/game/gfx/texcache: 0x58d670) against the original, TexCache_AddSurfaces and Object_FreeSixPointerArrays the game's: the six sizes' budgets none, under or over some areas of their surfaces, the render device's slot 4 and the surfaces' slot 0 answering in turn from tables (a size failing once gives the last entry of the side's indexes -1 and 8 up, sic: side 0 and side 1); the counts of the six sizes all 0 now and then (+0x18ec read past the table, sic); the debug flag's texts hashed by the log's hook, the C library's sprintf the game's. Budget (0x58d400): the device's memory (slot 2) none (then measured by a surface run) or of any size, the cache's flags and choices, its shares any; the error pushed on the game's list when too small.
# usage: texcachecreate.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Create,Budget)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _cdecl_log, answers, heap, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x58d670, 0x58d8e0), (0x58d400, 0x58d670), (0x58d1d0, 0x58d377), (0x58d380, 0x58d400), (0x5848b0, 0x584a21), (0x590fd0, 0x590fe7), (0x5906e0, 0x590750),
           (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)],
          hooks={0x564650: _cdecl_log})
OPS = [k for k in os.environ.get('OPS', 'Create,Budget').split(',') if k]
CACHE, DEV, DVT, SURF, SVT, RECORDS, HELD, HELD_VT, ANSWERS = RAM + 0x400, RAM + 0x2800, RAM + 0x2840, RAM + 0x2900, RAM + 0x2940, RAM + 0x3000, RAM + 0x3400, RAM + 0x3440, RAM + 0x3800
SIDES = [0x100, 0x80, 0x40, 0x20, 0x10, 8]
SIZES = [2, 2, 2, 4, 0x100, 0x80, 0x40, 0x20]   # (0x7022d8)
SLOT_4, SLOT_0 = ANSWERS, ANSWERS + 0x100
DEVICE = 2   # (the render device's memory)


def entries(r, cache, index, count):
    """`count` entries of a side index (-1 too): each a record whose first dword holds a recorder object or none."""
    for i in range(count):
        put(cache, 4 * ((index << 8) + i) + 0x400, 'I', RECORDS + 0x20 * r.randrange(8))
    return count


def world(r, fmt):
    log = log_region()
    cache = bytearray(r.bytes(0x2400 + 0x400))   # (from side index -1's entries)
    empty = r.randrange(4) == 0
    for i in range(6):
        put(cache, 0x400 + 0x1800 + 4 * i, 'i', 0 if empty else entries(r, cache, i, r.choice([0, 0, 0, 1, 2, 3])))
    regions = {LOG: log, RAM: cache, HEAP: heap(0x2100), 0x8c4b70: r.bytes(0x1840), 0x8bd554: struct.pack('<I', fmt), 0x8c4898: r.bytes(4), 0x8bd52c: struct.pack('<I', DEV),
               0x8bd508: struct.pack('<I', r.choice([0, 0, 0x1000000, r.getrandbits(32)])), DEV: struct.pack('<I', DVT) + r.bytes(0xc), DVT: vtable({}, 8),
               SURF: struct.pack('<I', SVT) + r.bytes(0xc), SVT: vtable({0: 4, 9: 1}, 12), HELD: struct.pack('<I', HELD_VT) + r.bytes(0xc), HELD_VT: vtable({9: 1}, 12),
               RECORDS: b''.join(struct.pack('<I', r.choice([HELD, HELD, 0])) + r.bytes(0x1c) for _ in range(8)),
               SLOT_4: b''.join(struct.pack('<I', r.choice([SURF, SURF, SURF, 0])) for _ in range(0x40)), SLOT_0: b''.join(struct.pack('<I', r.choice([1, 1, 1, 0])) for _ in range(0x40))}
    strings(log, 0x3f, 1)
    answers(log, 4, SLOT_4)
    answers(log, 0, SLOT_0)
    return regions, log, cache


def create(r):
    fmt, levels = r.randrange(8), r.choice([0, 1, 2, 3])
    regions, _log, cache = world(r, fmt)
    put(cache, 0x400 + 0x17fc, 'i', entries(r, cache, -1, r.choice([0, 0, 1, 2])))
    put(cache, 0x400 + 0x1820, 'i', entries(r, cache, 8, r.choice([0, 0, 1, 2])))
    for i in range(6):
        area = SIZES[fmt] * SIDES[i] * SIDES[i]
        if levels > 1:
            area = area * 4 // 3
        put(cache, 0x400 + 0x1860 + 4 * i, 'i', r.choice([0, 0, 0, area // 2, area, area, area * 3 // 2, area * 4]))
    put(cache, 0x400 + 0x18e8, 'I', levels)
    return Call(0x58d670, [CACHE], regions, conv='t', ret='a')


def budget(r):
    fmt = r.randrange(8)
    regions, log, cache = world(r, fmt)
    memory = r.choice([0, 0, 0x40 * SIZES[fmt] << 16, 0x41 * SIZES[fmt] << 16, 0x10000, 0x20000, 0x100000, 0x400000, 0x1000000, 0x4000000, 0x7fff0000, r.randrange(0x10000), r.getrandbits(32)])
    set_return(log, DEVICE, memory)
    regions[0x8c1620] = struct.pack('<I', r.choice([0, 0, SIZES[fmt] << 16, 4 * SIZES[fmt] << 16, SIZES[fmt] << 15]))
    regions[0x8c1624] = r.bytes(4)
    regions[0x8bd524] = struct.pack('<I', r.choice([1, 1, 0, 2]))
    regions[0x879ad0] = put(bytearray(r.bytes(0x204)), 0x200, 'I', r.choice([0, 1, 0x3f, 0x40]))
    put(cache, 0x400 + 0x1920, 'I', r.choice([0, 0, 0, 1, 2, r.getrandbits(32)]))
    put(cache, 0x400 + 0x18cc, 'I', r.choice([0, 2, r.getrandbits(32)]))
    r.floats(cache, [0x400 + 0x187c + 4 * i for i in range(6)], r.choice([1.0, 0.001, 1e6]))
    return Call(0x58d400, [CACHE], regions, conv='t', ret='a')


def case(r):
    return create(r) if r.deal(OPS) == 'Create' else budget(r)


if __name__ == '__main__':
    B.main(case)
