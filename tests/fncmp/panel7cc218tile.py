# The game panel's tile mode (src/game/ui/panel7cc218: Panel7cc218_EnterTileMode, 0x4af200) against the original (panel7cc218.py and others bench the module's others). Every call a recorder (their own benches have them; driver.cpp's
# kPanel7cc218TileEnv): Global7c2654_Set 10, FocusCell_Update 11, Dialog7ca1c8_OpenFor 12, GameHeap_Alloc 13 (the one return: all allocations fail or none), MouseModeTile_Ctor thiscall 14, Global81ecac_SetRecorded 15,
# MouseModeOnlineBase_Ctor thiscall 16, MouseMode_Set stdcall 17, GameMode_MapClick 18, this's slot 0x2c thiscall 19, ObjectDefs_Find thiscall 20 (the id logged as a word: the original's upper half is the flag's). The flag 0 or not, the
# definition's track type 0-4, the holder [0x7b05e8] there or not.
# usage: panel7cc218tile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4af200, 0x4af433)],
          hooks={0x48c940: _recorder(10, 1, cdecl=True), 0x5276d0: _recorder(11, 1, cdecl=True), 0x4988c0: _recorder(12, 1, cdecl=True), 0x67acc0: _recorder(13, 1, cdecl=True), 0x46c580: _recorder(14, 1),
                 0x52fad0: _recorder(15, 1, cdecl=True), 0x46c6a0: _recorder(16, 0), 0x46c350: _recorder(17, 1, com=True), 0x524960: _recorder(18, 4, cdecl=True), 0x412e90: _recorder(20, 1)})
PANEL, VT, THING, DEF, TILE, BASE, BLOCK = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800, RAM + 0x900, RAM + 0xa00


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 20, 1 << 16)
    set_return(log, 13, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    set_return(log, 14, TILE)
    set_return(log, 16, BASE)
    set_return(log, 20, DEF)
    regions = {LOG: log, PANEL: struct.pack('<I', VT) + r.bytes(0x3c), VT: put(vtable({}, size=0x10), 0x2c, 'I', recorder(19, 0)), THING: r.bytes(0x20),
               DEF: put(bytearray(r.bytes(0x100)), 0x9c, 'I', r.choice([0, 1, 1, 2, 2, 3, 4, r.getrandbits(32)])), 0x7c2654: r.bytes(4) + struct.pack('<I', THING),
               0x7b05d4: bytes(put(bytearray(r.bytes(0x18)), 0x14, 'I', r.choice([0, 0, RAM + 0xb00])))}
    return Call(0x4af200, [PANEL, r.choice([0, 0, 1, 1, 1, 0x10000, 0xffff0000, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
