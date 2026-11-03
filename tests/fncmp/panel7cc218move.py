# The game panel's move mode (src/game/ui/panel7cc218: Panel7cc218_EnterMoveMode, 0x48cfa0) against the original (panel7cc218tile.py benches its twin). Every call a recorder (their own benches have them; driver.cpp's kPanel7cc218TileEnv):
# Global7c2654_Set 10, FocusCell_Update 11, GameHeap_Alloc 13 (the one return), MouseModeOnlineBase_Ctor 16, MouseMode_Set stdcall 17, GameMode_MapClick 18, this's slot 0x2c 19, Object_GetDef 21, RideQueue_Trace 22, PairTable_Commit 23,
# GameMode_Set 24, MouseModeTile_CtorKind 25 (the id a word: the upper half is the stack's), Global8186e0_SetRecorded 26 (the word it points to hashed), Global81d7a4_SetRecorded 27.
# usage: panel7cc218move.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, blob_bytes, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x48cfa0, 0x48d158)],
          hooks={0x48c940: _recorder(10, 1, cdecl=True), 0x5276d0: _recorder(11, 1, cdecl=True), 0x67acc0: _recorder(13, 1, cdecl=True), 0x46c6a0: _recorder(16, 0), 0x46c350: _recorder(17, 1, com=True),
                 0x524960: _recorder(18, 4, cdecl=True), 0x4dd4e0: _recorder(21, 0), 0x530120: _recorder(22, 1, cdecl=True), 0x52fc30: _recorder(23, 0, cdecl=True), 0x52f200: _recorder(24, 2, cdecl=True),
                 0x46c5a0: _recorder(25, 2), 0x52f880: _recorder(26, 2, cdecl=True), 0x52f1b0: _recorder(27, 2, cdecl=True)})
PANEL, VT, THING, DEF, TILE, BASE, BLOCK = RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800, RAM + 0x900, RAM + 0xa00


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 25, 1 << 17)
    blob_bytes(log, 26, 0, 0b11)
    set_return(log, 13, r.choice([BLOCK, BLOCK, BLOCK, 0]))
    set_return(log, 16, BASE)
    set_return(log, 21, DEF)
    set_return(log, 22, r.getrandbits(32))
    set_return(log, 25, TILE)
    thing = bytearray(r.bytes(0x20))
    thing[5], thing[7] = r.choice([0, 0x7f, 0x80, 0xff, r.getrandbits(8)]), r.choice([0, 1, 0x7f, 0xff, r.getrandbits(8)])
    regions = {LOG: log, PANEL: struct.pack('<I', VT) + r.bytes(0x3c), VT: put(vtable({}, size=0x10), 0x2c, 'I', recorder(19, 0)), THING: thing,
               DEF: put(bytearray(r.bytes(0x100)), 0x40, 'I', r.choice([0, 0, 1, 0x100, 0x10000, r.getrandbits(32)])), 0x7c2654: r.bytes(4) + struct.pack('<I', THING), 0x81b14c: r.bytes(4), 0x81b3ac: r.bytes(4),
               0x7b05d4: bytes(put(bytearray(r.bytes(0x18)), 0x14, 'I', r.choice([0, 0, RAM + 0xb00])))}
    return Call(0x48cfa0, [PANEL], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
