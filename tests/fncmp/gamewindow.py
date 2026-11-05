# The game's window and the engine's start (src/game/core/gamewindow: 0x44e080) against the original: every callee and import a recorder (the class and the rect hashed, the rect adjusted through its fill); the flags and the options' globals varied, each step failing now and then.
# usage: gamewindow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, fill, import_table, log_region, set_return, strings  # noqa: E402

APPLY, LOAD, LOADFOR, START, ERROR, PERSPECTIVE, FIRST, SETC = range(1, 9)
CURSOR, REGISTER, ADJUST, CREATE, SHOW, UPDATE = range(40, 46)
B = Bench([(0x44e080, 0x44e375)], hooks={0x44dd50: _recorder(APPLY, 0, cdecl=True), 0x44dc60: _recorder(LOAD, 1, cdecl=True), 0x44de20: _recorder(LOADFOR, 0, cdecl=True), 0x44df20: _recorder(START, 4, cdecl=True),
                                         0x405710: _recorder(ERROR, 3), 0x578be0: _recorder(PERSPECTIVE, 3), 0x423b50: _recorder(FIRST, 1), 0x423b80: _recorder(SETC, 1)})
MATRIX, ENGINE, OTHER = RAM, RAM + 0x100, RAM + 0x2000
IMPORTS = {'LoadCursorA': (CURSOR, 0x6fd364, 2, 0, 0), 'RegisterClassA': (REGISTER, 0x6fd394, 1, 0, 0), 'AdjustWindowRect': (ADJUST, 0x6fd384, 3, 0, 0), 'CreateWindowExA': (CREATE, 0x6fd398, 12, 0, 0),
           'ShowWindow': (SHOW, 0x6fd368, 2, 0, 0), 'UpdateWindow': (UPDATE, 0x6fd36c, 1, 0, 0)}


def word(r, *values):
    return struct.pack('<I', r.choice(values + (r.getrandbits(32),)))


def case(r):
    log = log_region()
    table = import_table(log, IMPORTS)
    set_return(log, CURSOR, r.getrandbits(32))
    set_return(log, REGISTER, r.choice([0, 0x10000, 0xffff0000, 1, 0xc001, 0xc001, 0xc001, 0xc001, 0xc001, 0xc001]))
    set_return(log, CREATE, r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    set_return(log, LOAD, r.choice([0, 1, 1, 1, 0x100]))
    set_return(log, START, r.choice([0, 1, 1, 1, 1, 0x100]))
    blob(log, REGISTER, 0, 0x3ff)
    blob(log, ADJUST, 0, 0xf)
    fill(log, ADJUST, 0, struct.pack('<4i', -r.randrange(0, 20), -r.randrange(0, 40), r.randrange(0, 2000), r.randrange(0, 2000)))
    other = bytearray(r.bytes(0x800))
    struct.pack_into('<i', other, 0x788, r.choice([-1, 0, 1, 1, 2, 3]))
    regions = {LOG: log, IMPORT_TABLE: table, MATRIX: r.bytes(0x40), ENGINE: r.bytes(0x1900), OTHER: other,
               0x87afe8: struct.pack('<I', MATRIX), 0x8c4b44: struct.pack('<I', ENGINE), 0x879a24: struct.pack('<I', OTHER),
               0x7a1a44: r.bytes(4), 0x7a1968: word(r, 640, 800, 1024), 0x7a1930: word(r, 480, 600, 768), 0x7a130c: r.bytes(4),
               0x7a1a8c: struct.pack('<I', r.getrandbits(32)), 0x79d86c: struct.pack('<I', r.getrandbits(32)),
               0x78d968: word(r, 0, 1, 1, 1, 2), 0x785874: word(r, 0, 1, 2, 3, 4), 0x8bd524: word(r, 0, 1, 1, 2), 0x78d8e4: word(r, 0, 1, 1, 2)}
    return Call(0x44e080, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
