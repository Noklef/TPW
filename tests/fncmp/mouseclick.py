# The click on what the help line shows (src/game/ui/mouseclick: MouseClick_OnShown, 0x4879d0) against the original. The calls' recorders as driver.cpp's kMouseClickEnv (every callee but
# the mouse modes and the accessors); the mode a recorder object answering from slot 9 or none, the holder set or not, a heap failing now and then. ObjectControls_Find's id is pushed with
# uninitialised stack in its high word, which the callee does not read.
# usage: mouseclick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, IMPORT_TABLE as IAT, LOG, RAM, Bench, Call, _recorder, fill, heap, import_table, log_region, put, set_return, strings, vtable  # noqa: E402
from focus import MODES  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import mousetile  # noqa: E402
import recorded_actions as ra  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

HAS, OBJECT_AT, PLAY, RECORDED, PANEL, OPEN, OPEN_FOR, FOCUS = 10, 11, 12, 13, 14, 15, 16, 17
FIND, TABLE, MONEY, VISITOR, STAFF, FIRST = 18, 19, 20, 21, 22, 23
KEY = 7
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731


def _find(e):
    sp = e.r['ESP'] + (4 if e.tail else 0)
    e.write(sp, struct.pack('<I', e.u32(sp) & 0xffff))
    return _recorder(FIND, 2)(e)


B = Bench(sorted(set(MODES + [(0x4879d0, 0x488168), (0x46c580, 0x46c5c6), (0x46c5d0, 0x46c5d3), (0x46d550, 0x46d59d), (0x6b71c0, 0x6b71c6), (0x46d5a0, 0x46d65f), (0x48c920, 0x48c950), (0x48c960, 0x48c97f),
                              (0x403be0, 0x403f00), (0x52f880, 0x52f8e0), (0x52fa90, 0x52fac0), (0x52fad0, 0x52fb00), (0x51ab20, 0x51ab70), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f), (0x412e90, 0x412f00)])),
          hooks={0x5234d0: C(30, 3), 0x52f200: C(24, 2), 0x48c8d0: C(HAS, 0), 0x527e80: C(OBJECT_AT, 3), 0x51bfc0: C(PLAY, 6), 0x52f1b0: C(RECORDED, 2), 0x486920: C(PANEL, 1), 0x498d80: C(OPEN, 0), 0x4988c0: C(OPEN_FOR, 1), 0x5276d0: C(FOCUS, 1),
                 0x4d3d10: _find, 0x5195d0: _recorder(TABLE, 0), 0x6ad810: _recorder(MONEY, 0), 0x4b79b0: _recorder(VISITOR, 1), 0x4b6080: _recorder(STAFF, 1), 0x4d99e0: _recorder(FIRST, 1)})
IMPORTS = {'GetAsyncKeyState': (KEY, 0x6fd320, 1, 0, 0)}
TILES = 200
MODE, MODE_VT, HOLDER, CONTROLS, WORD, SOURCE, THINGS, WORLD = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200, RAM + 0xc300, RAM + 0xc400, RAM + 0xc500, RAM + 0x1000, RAM + 0x80000


def case(r):
    log = log_region()
    regions = mousetile.tile(r).regions   # (the world of the tile mode's Assign with a definition: the object's mode)
    count = struct.unpack_from('<i', regions[ra.DEFS], 0x528)[0]
    regions.update({LOG: log, IAT: import_table(log, IMPORTS), HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 0, 1, 2, 3, 4]))})
    regions[0x7cf83c] = struct.pack('<I', r.deal([WORLD] * 19 + [0]))
    set_return(log, HAS, r.deal([0] * 19 + [1]))
    set_return(log, 9, r.deal([0, 0, 0, 1, 1, 1, 2, 9, r.getrandbits(32)]))
    regions[MODE_VT] = vtable({0: 1})
    regions[MODE] = put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT)
    regions[0x7b05d4] = put(put(bytearray(r.bytes(0x18)), 4, 'I', r.deal([MODE, MODE, MODE, 0])), 0x14, 'I', r.deal([HOLDER, HOLDER, 0]))
    regions[0xfb3518] = bytes([r.choice([1, 3, 0xff])])
    regions[0xfb3b7c] = struct.pack('<I', 1)
    regions[0x7c24f4] = struct.pack('<I', r.deal(list(range(1, 15)) * 2 + [4, 4, 5, 5, 5, 3, 6, 0, 15, r.getrandbits(32)]))
    tile = r.deal([r.randrange(1, TILES)] * 12 + [0, 1, 0x7f, 0x80, 0x81])
    regions[0x7b05cc] = struct.pack('<H', tile)
    regions[0x7b05a8] = struct.pack('<III', r.getrandbits(32), r.deal([0, THINGS, THINGS, r.getrandbits(32)]), r.getrandbits(32))
    # the things and their 20-byte slots
    things = bytearray(r.bytes(0x40 * 5))
    for i in range(5):
        put(things, 0x40 * i + 0xe, 'H', r.randrange(1, count + 1))
    table = bytearray(r.bytes(20 * TILES))
    for i in range(TILES):
        put(table, 20 * i, 'I', THINGS + 0x40 * (i % 5))
    regions[THINGS] = things
    regions[0x7cfb90] = table
    # the cells and records
    cells = bytearray(r.bytes(0x44 * TILES))
    records = bytearray(r.bytes(40 * TILES))
    for i in range(TILES):
        put(cells, 0x44 * i + 8, 'I', r.choice([3, 3, 3, 1, 4, r.getrandbits(32)]))
        put(cells, 0x44 * i + 0x10, 'H', r.randrange(TILES))
        put(records, 40 * i + 8, 'I', r.choice([0xb, 0xb, 0xc, 0xc, 0x10, 0x10, 0x19, 0x19, 0x18, 0, r.getrandbits(32)]))
        put(records, 40 * i + 0x10, 'H', r.randrange(TILES))
    regions[WORLD + 0x294] = cells
    regions[WORLD + 0x1102b0] = records
    # the calls' answers
    strings(log, OBJECT_AT, 0, out=1)
    fill(log, OBJECT_AT, 0, struct.pack('<H', r.deal([0, 1, 2, 3, r.randrange(TILES)])))
    strings(log, FIRST, 0, out=1)
    set_return(log, FIRST, WORD)
    regions[WORD] = struct.pack('<H', r.deal([0, 1, 2, r.randrange(TILES), r.randrange(TILES)]))
    set_return(log, KEY, r.deal([0x8000, 0xffff8000, 0x8001, 0, 1, 0x7fff, 0xffff0000]))
    price = r.deal([0, 100, 200, 0x7fffffff, 0x80000000])
    set_return(log, FIND, CONTROLS)
    regions[CONTROLS] = put(bytearray(r.bytes(0x10)), 4, 'I', price)
    set_return(log, TABLE, SOURCE)
    set_return(log, MONEY, r.deal([0, 100, 150, 0x80000000, 0x7fffffff]))
    set_return(log, PLAY, r.getrandbits(32))
    regions[0x803a2c] = struct.pack('<I', r.getrandbits(32))
    return Call(0x4879d0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
