# The track pieces' texture tables and the map's reset (src/game/world/tracktextures: 0x522900, 0x522aa0, 0x522c60, 0x522e40, 0x52f050) against the original: the base path, the formatting, the folder's opening, the loader and the cell resets recorders.
# usage: tracktextures.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, set_return, strings  # noqa: E402

BASE, FORMAT, OPEN, LOAD, CELL, TRACK, LIST, ENTRANCE = range(1, 9)
CDECL = {0x40ef40: (BASE, 0), 0x67a650: (FORMAT, 3), 0x461a60: (OPEN, 3), 0x4629d0: (LOAD, 11), 0x527940: (LIST, 1), 0x5323f0: (ENTRANCE, 0)}
hooks = {at: _recorder(k, n, cdecl=True) for at, (k, n) in CDECL.items()}
hooks.update({0x536490: _recorder(CELL, 1), 0x53af00: _recorder(TRACK, 0)})
B = Bench([(0x522900, 0x5229b0), (0x522aa0, 0x522b5a), (0x522c60, 0x522d1a), (0x522e40, 0x522eed), (0x52f050, 0x52f18a), (0x46b3d0, 0x46b3e0)], hooks=hooks)
TABLES = 0x763380
NAMES = RAM + 0x1000
ANSWERS = RAM + 0x2000
WORLD = RAM + 0x100000
GLOBALS_U32 = [0x81ede4, 0x81ede8, 0x81d740, 0x81b1b0, 0x81af4c, 0x81d7a4, 0x81b134, 0x81ae2c, 0x81af64, 0x816d50, 0x816d58, 0x816d60, 0x816d5c, 0x816d4c, 0x81b0cc, 0x81ecac, 0x81d7a8, 0x81b46c, 0x820a8c, 0x8186ac, 0x81b604]
GLOBALS_U8 = [0x81b4cc, 0x81af7c]
GLOBALS_U16 = [0x8186ec, 0x818698]


def case(r):
    log = log_region()
    name = bytearray(b'-dash\0\0\0' + b'first\0\0\0' + b'second\0\0')
    table = bytearray(r.randbytes(0x763760 - TABLES))
    for slot in range(0x763388, 0x7633e8, 12):   # (the queue's: always a name)
        struct.pack_into('<I', table, slot + 4 - TABLES, NAMES + r.choice([8, 16, 0]))
    for slot in [*range(0x7633e8, 0x763604, 12), *range(0x763608, 0x763758, 12)]:   # (the hoarding's table 4 bytes after the water ride's)
        struct.pack_into('<I', table, slot + 4 - TABLES, r.choice([0, 0, NAMES, NAMES + 8, NAMES + 16, NAMES + 16]))
    set_return(log, BASE, RAM + 0x3000)
    fill(log, FORMAT, 0, b'folder\0')
    strings(log, FORMAT, 0, out=1)
    answers(log, OPEN, ANSWERS)
    answers(log, LOAD, ANSWERS + 0x100)
    strings(log, OPEN, 3)
    strings(log, LOAD, 3)
    regions = {LOG: log, TABLES: table, NAMES: name, 0x816d68: r.randbytes(8), 0x8179f8: r.randbytes(8), ANSWERS: struct.pack('<40I', *([r.choice([0, 0, 1]) for _ in range(40)])), ANSWERS + 0x100: struct.pack('<40I', *[r.getrandbits(32) for _ in range(40)])}
    which = r.choice([0x522900, 0x522aa0, 0x522c60, 0x522e40, 0x52f050, 0x52f050])
    if which != 0x52f050:
        return Call(which, [], regions, conv='c', ret='v')
    columns = r.choice([0, 1, 2, 3, -1, r.randrange(1, 6)])
    rows = r.choice([0, 1, 2, -1, r.randrange(1, 6)])
    regions.update({at: r.randbytes(4) for at in GLOBALS_U32})
    regions.update({at: r.randbytes(1) for at in GLOBALS_U8})
    regions.update({at: r.randbytes(2) for at in GLOBALS_U16})
    regions[0x7cf83c] = struct.pack('<I', WORLD)
    return Call(0x52f050, [columns & 0xffffffff, rows & 0xffffffff], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
