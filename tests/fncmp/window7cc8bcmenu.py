# The handler of the window [0x7cc8bc]'s name list (src/game/ui/window7cc8bcmenu: Window7cc8bc_ListHandler, 0x4be290) against the original. Recorders (driver.cpp's kWindow7cc8bcMenuEnv): Window7059f0_NthItemData 10 (its out filled), Window7059f0_NthItemCell 11 (its out filled), OnlineChat_Instance 12, 0x4ea9a0 13, Window7cc8cc_PlaceAtMouse 14, Window705a50_SetState 15 (bytes), Window_SetDwordA0 16, Window_CallDefault 17. Window_FindChild runs.
# usage: window7cc8bcmenu.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, LOG_BYTES, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x4be290, 0x4be391), (0x65d7bc, 0x65d840)],
          hooks={0x664c71: _recorder(10, 2), 0x664bf0: _recorder(11, 3), 0x46cf10: _recorder(12, 0, cdecl=True), 0x4ea9a0: _recorder(13, 1), 0x4bf370: _recorder(14, 2, cdecl=True), 0x668bec: _recorder(15, 2), 0x65e6d5: _recorder(16, 2), 0x65f66e: _recorder(17, 3)})
TOP, MID, LIST, A, A1, A2, C, C1, C2 = (RAM + 0x100 * k for k in range(9))


def node(r, ident, kid=0):
    out = bytearray(r.bytes(0x100))
    put(out, 0x54, 'I', 0)
    put(out, 0x5c, 'I', kid)
    put(out, 0x8c, 'I', ident)
    return out


def case(r):
    log = log_region()
    struct.pack_into('<I', log, LOG_BYTES + 4 * 15, 3)   # (the state and notify bytes: the upper halves the registers')
    strings(log, 10, 0, out=0b10)
    strings(log, 11, 0, out=0b100)
    fill(log, 10, 1, struct.pack('<I', r.choice([0, 1, 8, 9, r.getrandbits(32)])))
    fill(log, 11, 2, r.bytes(4), which=1)
    set_return(log, 12, r.getrandbits(32))
    set_return(log, 13, r.choice([0, 0, 0, 0x100, 0xffffff00, 1, r.getrandbits(32)]))
    set_return(log, 17, r.getrandbits(32))
    regions = {LOG: log, TOP: node(r, 1, MID), MID: node(r, 0xc3123, LIST), LIST: node(r, 0xc3124), A: node(r, 2, A1), A1: node(r, 0x1789, A2), A2: node(r, 0x178c), C: node(r, 3, C1), C1: node(r, 0x3003, C2), C2: node(r, 0x3006),
               0x7cc8bc: struct.pack('<I', TOP), 0x7cc8c4: struct.pack('<2I', A, C), 0x7cc8cc: r.bytes(4)}
    message = r.choice([0x402, 0x402, 0x402, 0x401, 0x403, r.getrandbits(32)])
    return Call(0x4be290, [r.getrandbits(32), message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
