# The money display's update (src/game/ui/moneydisplay: 0x4a0ab0) against the original. Every callee a recorder as driver.cpp's kMoneyDisplayEnv; GuiRect's real code on the
# stack rects, Window_FindChild real over a child list of 0x30 (or not), 0x31 and 0x32. The rect getters fill their rects; the mouse mode there or none, the holder there or made
# (the heap failing now and then).
# usage: moneydisplay.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob_bytes, fill, heap, log_region, put, set_return, strings, vtable  # noqa: E402

MONEY, AMOUNT, NOTIFY, AREA, MOVE, GETRECT, NOTIFY1, BASE, SETMODE, FLAG, COST, SHOWN = range(12, 24)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4a0ab0, 0x4a0dad), (0x65d7bc, 0x65d832), (0x659d00, 0x659dbc), (0x659dbc, 0x659de3), (0x659de5, 0x659e3e), (0x659e3e, 0x659e83), (0x65a9d0, 0x65aa00), (0x661700, 0x66176a), (0x66bff7, 0x66c01b),
           (0x66c01a, 0x66c7dc), (0x659e83, 0x65a1c3), (0x65a3e9, 0x65a4a3), (0x65a1c3, 0x65a3e9), (0x5da3c0, 0x5da3c1)],
          hooks={0x5195d0: _recorder(MONEY, 0), 0x6ad810: _recorder(AMOUNT, 0), 0x65c5d5: _recorder(NOTIFY, 4), 0x65d616: _recorder(AREA, 1), 0x65ce10: _recorder(MOVE, 2), 0x65d42e: _recorder(GETRECT, 2),
                 0x65d3a3: _recorder(NOTIFY1, 1), 0x46c6a0: _recorder(BASE, 0), 0x46c350: _recorder(SETMODE, 1), 0x447990: C(FLAG, 0), 0x52fbc0: C(COST, 0), 0x65d9dd: _recorder(SHOWN, 1)})
KVTABLE, WINDOW, C30, C31, C32, MODE, WORLD, NODE = RAM + 0x1000, RAM + 0x2000, RAM + 0x2200, RAM + 0x2400, RAM + 0x2600, RAM + 0x2800, RAM + 0x2900, 0x180


def node(r, ident, nxt, child=0):
    buf = bytearray(r.bytes(NODE))
    put(buf, 0, 'I', KVTABLE)
    put(buf, 0x54, 'I', nxt)
    put(buf, 0x5c, 'I', child)
    put(buf, 0x8c, 'I', ident)
    return buf


def case(r):
    log = log_region()
    regions = {LOG: log, KVTABLE: vtable({7: 1, 9: 0}), HEAP: heap(0x400, r.choice([0] * 6 + [1, 2]))}
    has30 = r.random() < 0.8
    regions[WINDOW] = node(r, 1, 0, C30 if has30 else C31)
    regions[C30] = node(r, 0x30, C31)
    regions[C31] = node(r, 0x31, C32)
    regions[C32] = node(r, 0x32, 0)
    regions[MODE] = put(bytearray(r.bytes(8)), 0, 'I', KVTABLE)
    regions[0x7cf6ec] = struct.pack('<I', RAM + 0x2a00)
    regions[RAM + 0x2a00] = struct.pack('<I', WORLD)
    money = r.choice([0, -1, 1, 999, 1000, 1001, -1000, 4000, 5000, 6000, r.randrange(-3000, 7000), r.getrandbits(32) - (1 << 31)])
    set_return(log, MONEY, RAM + 0x2b00)
    set_return(log, AMOUNT, money)
    regions[0x7cb2dc] = struct.pack('<i', r.choice([money, money, money + 1, r.getrandbits(31)]))
    regions[0x7cb27c] = r.bytes(4)
    regions[0x752e2c] = struct.pack('<i', r.choice([0, 1, 2, 3, 4, 5, -1]))
    regions[0x752e30] = r.bytes(0x18)
    cost = r.choice([-999, -9999, 0, 5, r.getrandbits(32) - (1 << 31)])
    regions[0x752ec0] = struct.pack('<i', r.choice([cost, -999, -9999, 7]))
    set_return(log, COST, cost)
    regions[0x7b05e8] = struct.pack('<I', r.choice([RAM + 0x2c00, RAM + 0x2c00, 0]))
    regions[0x7b05e0] = r.bytes(4)
    regions[0x7b05d4] = r.bytes(4)
    regions[0x7b05d8] = struct.pack('<I', r.choice([MODE, MODE, MODE, MODE, 0]))
    set_return(log, 9, r.choice([3, 4, 8, 8, 0, r.getrandbits(32)]))
    set_return(log, FLAG, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, BASE, RAM + 0x2d00)
    strings(log, AREA, 0, out=1)
    strings(log, GETRECT, 0, out=1)
    fill(log, AREA, 0, r.bytes(8), which=0, offset=4)
    fill(log, GETRECT, 0, r.bytes(16), which=1, offset=4, records=2)
    blob_bytes(log, MOVE, 0, 0x3)
    return Call(0x4a0ab0, [WINDOW], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
