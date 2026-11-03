# The key action on the cell under the cursor (src/game/core/keyactions: KeyAction_ClickCell 0x40bda0) against the original: GameMode_Set (k 21), Sound_PlayAt (k 22), GameMode_MapClick (k 23) and GameMode_StepBack (k 24)
# recorders; the mouse mode [0x7b05d8] a recorder object (slot 9 answering 0, 1, 3 or another) or none, the holder [0x7b05e8] set or not, game mode 0 or not, the cell of type 1 or not, any cell id (0 too).
# usage: keyclickcell.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, set_return, vtable  # noqa: E402
from focus import MODES  # noqa: E402

B = Bench(sorted(set(MODES + [(0x40bda0, 0x40bec6), (0x40bed0, 0x40bf65), (0x52f840, 0x52f872), (0x536310, 0x53631c), (0x67a400, 0x690000)])),
          hooks={0x52f200: _recorder(21, 2, cdecl=True), 0x51bfc0: _recorder(22, 6, cdecl=True), 0x524960: _recorder(23, 4, cdecl=True), 0x52fe50: _recorder(24, 2, cdecl=True)})
MODE, MODE_VT, HOLDER, CELL = RAM + 0xc000, RAM + 0xc100, RAM + 0xc200, RAM + 0xd000


def case(r):
    log = log_region()
    set_return(log, 9, r.deal([0, 0, 1, 1, 3, 3, 2, r.getrandbits(32)]))
    cell_id = r.choice([0, 1, 0x80, 0x81, r.randrange(0x10000), r.randrange(0x4000), r.randrange(0x4000)])
    regions = {LOG: log, HEAP: heap(0x1000, r.deal([0, 0, 0, 0, 1, 2, 3, 4])), MODE_VT: vtable({0: 1}), MODE: put(bytearray(r.bytes(0x10)), 0, 'I', MODE_VT),
               0x7b05cc: put(bytearray(r.bytes(4)), 0, 'H', cell_id),
               0x7b05d4: put(put(bytearray(r.bytes(0x18)), 4, 'I', r.deal([MODE, MODE, MODE, 0])), 0x14, 'I', r.deal([0, HOLDER])),
               0x7cf83c: struct.pack('<I', (CELL - cell_id * 0x44 - 0x294) & 0xffffffff), CELL: put(bytearray(r.bytes(0x44)), 8, 'I', r.choice([1, 1, 1, 0, 2, r.getrandbits(32)])),
               0x81ae2c: struct.pack('<I', r.choice([0, 0, 0, 0x32, r.getrandbits(32)])), 0x803a2c: r.bytes(4),
               0xfb3518: bytes([r.choice([1, 3, 0xff])]), 0xfb3b7c: struct.pack('<I', 1)}
    return Call(0x40bda0, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
