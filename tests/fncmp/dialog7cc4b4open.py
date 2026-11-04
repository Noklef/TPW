# The dialog [0x7cc4b4] built (src/game/ui/dialog7cc4b4open: 0x4b8ca0) against the original. Window_FindChild among decoys real, the heap served and refusing now and then; the others recorders as driver.cpp's kDialog7cc4b4OpenEnv.
# usage: dialog7cc4b4open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import VT, kids  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, recorder, set_return  # noqa: E402

BUILD, PART, PART530, IMPL, FONT, SET4C, N100, NC00, N1, SHOWN, UPDATE, N40000, FINISH, SLOT = range(10, 24)
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4b8ca0, 0x4b8e7d), (0x65d7bc, 0x65d840)],
          hooks={0x65fd0e: _recorder(BUILD, 2), 0x48f6a0: _recorder(PART, 1), 0x491920: _recorder(PART530, 1), 0x65d1c8: _recorder(IMPL, 1), 0x485a70: C(FONT, 1), 0x65c388: _recorder(SET4C, 1), 0x65f5b7: _recorder(N100, 1),
                 0x65c428: _recorder(NC00, 2), 0x65d3a3: _recorder(N1, 1), 0x65d9dd: _recorder(SHOWN, 1), 0x4b9340: C(UPDATE, 0), 0x65c5d5: _recorder(N40000, 4), 0x4bbbd0: C(FINISH, 0)})
DIALOG, HOLDER, MADE = (RAM + 0x1000 * k for k in range(4, 7))
KIDS1, KIDS2, KIDS3 = RAM + 0x10000, RAM + 0x12000, RAM + 0x14000


def case(r):
    log = log_region()
    regions = {LOG: log, HEAP: heap(0x2000, r.deal([0] * 8 + [1, 2, 3])), HOLDER: put(bytearray(r.bytes(8)), 4, 'I', r.deal([DIALOG + 0x800] * 9 + [0]))}
    regions[0xf82884] = struct.pack('<I', HOLDER)
    regions[DIALOG + 0x800] = r.bytes(0x100)
    regions[VT] = put(bytearray(0x40), 0x24, 'I', recorder(SLOT, 1))
    first, top = kids(r, regions, KIDS1, (0x1e0ec, 0x1e0ea, 0x1e0ef))
    regions[DIALOG] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    first, _ = kids(r, regions, KIDS2, (0x1e0ee, 0x1e0ed))
    put(regions[top[0x1e0ec]], 0x5c, 'I', first)
    first, _ = kids(r, regions, KIDS3, (0x1e0eb,))
    put(regions[top[0x1e0ea]], 0x5c, 'I', first)
    regions[0x7cc4b0] = put(bytearray(r.bytes(0x10)), 4, 'I', r.deal([0] * 9 + [DIALOG]))
    set_return(log, BUILD, DIALOG)
    set_return(log, PART, r.deal([MADE + 0x10, MADE + 0x10, 0]))
    set_return(log, PART530, r.deal([MADE + 0x20, MADE + 0x20, 0]))
    set_return(log, FONT, r.getrandbits(32))
    return Call(0x4b8ca0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
