# The display and GUI brought up and taken down (src/game/ui/guistartup: 0x489ca0, 0x489ec0) against the original. Every callee a recorder as driver.cpp's kGuiStartupEnv and
# kGuiTeardownEnv (the slots shared, only one of the two runs in a case); the imports recorders in the table; the layout's set bytes through its pointer.
# usage: guistartup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

SCREEN, SETDD, SETWIN, TIME, STATUS, LISTEN, FONT, HEBREW, INIT, PALETTE, HANDLER, POOL, OUTONE, TABLE, ANI, TGA, SETTABLE, SHOWN, SETCUR, REFRESH, MEM, DISK, LAYOUT, READY, DIALUP, SHOW = range(6, 32)
CLEAR, CLOSET, ENDT, CLOSES, LISTENEND, SHUT, POOLFREE, FONTFREE, STRAY, END = range(6, 16)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x489ca0, 0x489ebe), (0x489ec0, 0x489f46), (0x5da3c0, 0x5da3c1)],
          hooks={0x5fa1e0: C(SCREEN, 5), 0x5f8f80: C(SETDD, 1), 0x5f86d0: C(SETWIN, 1), 0x5f55c0: C(TIME, 1), 0x5f5600: C(STATUS, 1), 0x47e7d0: C(LISTEN, 0), 0x658243: _recorder(FONT, 5),
                 0x41a850: _recorder(HEBREW, 0), 0x6584df: C(INIT, 3), 0x659674: C(PALETTE, 1), 0x6590ea: C(HANDLER, 1), 0x669440: C(POOL, 4), 0x44e600: C(OUTONE, 4), 0x661cca: _recorder(TABLE, 1),
                 0x489720: C(ANI, 0), 0x4899c0: C(TGA, 0), 0x659248: C(SETTABLE, 1), 0x6591d4: C(SHOWN, 1), 0x6590f7: C(SETCUR, 1), 0x6591e3: C(REFRESH, 0), 0x4891d0: C(MEM, 0), 0x4893c0: C(DISK, 0),
                 0x48fc40: _recorder(LAYOUT, 2), 0x492470: C(READY, 0),
                 0x661c36: _recorder(CLEAR, 0), 0x5f5620: C(CLOSET, 0), 0x5f55e0: C(ENDT, 0), 0x5fa060: C(CLOSES, 0), 0x47e990: C(LISTENEND, 0), 0x658708: C(SHUT, 0), 0x669586: C(POOLFREE, 0),
                 0x657dff: _recorder(FONTFREE, 0), 0x424ec0: C(STRAY, 1)})
DRAW, STATE, KVTABLE, OBJECTS = RAM + 0x1000, RAM + 0x1100, RAM + 0x1200, RAM + 0x1400
INNER = (1 << 4) | (1 << 5) | (1 << 9) | (7 << 16)


def startup(r, log, regions):
    table = bytearray(0x800)
    put(table, 0x6fd30c - IMPORT_TABLE, 'I', recorder(SHOW, 1, com=True))
    put(table, 0x6fd5ac - IMPORT_TABLE, 'I', recorder(DIALUP, 0))
    regions[IMPORT_TABLE] = table
    regions[DRAW] = r.bytes(4)
    regions[0x879a24] = struct.pack('<I', DRAW)
    regions[0x7a1a8c] = struct.pack('<I', r.getrandbits(32))
    regions[0x788270] = struct.pack('<I', r.choice([0, 0, r.getrandbits(32)]))
    regions[0x7a130c] = r.bytes(4)
    regions[0x7b9fe0] = r.bytes(0x14)
    regions[STATE] = r.bytes(4)
    block = bytearray(r.bytes(8))
    put(block, 0, 'h', r.choice([480, 600, 768, -3, r.randrange(1, 0x7fff)]))
    put(block, 4, 'h', r.choice([640, 800, 1024, -3, r.randrange(1, 0x7fff)]))
    regions[0xfaa5c0] = block
    regions[0x7c2128] = r.bytes(2)
    regions[0x7c24d4] = r.bytes(4)
    set_return(log, STATUS, STATE)
    set_return(log, HEBREW, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, DIALUP, r.getrandbits(32))
    strings(log, SCREEN, 0, out=1)
    strings(log, TIME, 0, out=1)
    strings(log, STATUS, 0, out=1)
    blob_bytes(log, LAYOUT, 1, 0, (0x10, INNER))
    return Call(0x489ca0, [], regions, ret='v')


def teardown(r, log, regions):
    table = bytearray(0x800)
    put(table, 0x6fd5b0 - IMPORT_TABLE, 'I', recorder(END, 0))
    regions[IMPORT_TABLE] = table
    regions[KVTABLE] = vtable({0: 1})
    regions[OBJECTS] = struct.pack('<III', KVTABLE, KVTABLE, KVTABLE)
    for i, holder in enumerate((0x7b9a04, 0x7c231c, 0x7b9804)):
        regions[holder] = struct.pack('<I', r.choice([OBJECTS + 4 * i, OBJECTS + 4 * i, 0]))
    regions[0x7c249c] = r.bytes(4)
    set_return(log, END, r.getrandbits(32))
    return Call(0x489ec0, [], regions, ret='a')


def case(r):
    log = log_region()
    regions = {LOG: log}
    return (startup if r.randrange(4) else teardown)(r, log, regions)


if __name__ == '__main__':
    B.main(case)
