# The handler of the window [0x7cb2a4] (src/game/ui/window7cb2a4: Window7cb2a4_Handler, 0x4a0fb0) against the original. Recorders (their own benches have them; driver.cpp's kWindow7cb2a4Env): Setting_ReadMode 10, atexit 11, GlobalSave_Instance 12,
# GlobalSave_CountSetLevel 13, GlobalSave_CountSetLeft 14, Window_SetShown 15, swprintf 16 (its buffer on the stack); the children's slot 9 (the same). Window_FindChild and Window_CallDefault run.
# usage: window7cb2a4.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, _recorder, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x4a0fb0, 0x4a1185), (0x65d7bc, 0x65d840), (0x65f66e, 0x65f697)],
          hooks={0x550ca0: _recorder(10, 0), 0x67a7d0: _recorder(11, 1, cdecl=True), 0x409410: _recorder(12, 0, cdecl=True), 0x5af680: _recorder(13, 0), 0x5af5d0: _recorder(14, 0), 0x65d9dd: _recorder(15, 1), 0x67af00: _recorder(16, 3, cdecl=True)})
WINDOW, KIDS, VT = RAM, RAM + 0x200, RAM + 0x800


def case(r):
    log = log_region()
    set_return(log, 12, r.getrandbits(32))
    level, left = r.choice([0, 0, 1, 5, 0x7fffffff, r.getrandbits(32)]), r.choice([0, 0, 1, 5, -1 & 0xffffffff, r.getrandbits(32)])
    set_return(log, 13, level)
    set_return(log, 14, left)
    strings(log, 16, 0, out=1)
    strings(log, 9, 0, out=1)
    ids = [0x34, 0x35, 0x36, 0x37]
    r.shuffle(ids)
    kids = bytearray()
    for i, ident in enumerate(ids):
        kid = bytearray(r.bytes(0x100))
        put(kid, 0, 'I', VT)
        put(kid, 0x8c, 'I', ident)
        put(kid, 0x54, 'I', KIDS + 0x100 * (i + 1) if i < 3 else 0)
        put(kid, 0x5c, 'I', 0)
        kids += kid
    window = put(put(bytearray(r.bytes(0x120)), 0x5c, 'I', KIDS), 0x118, 'I', CDECL_LOG4)
    message = r.choice([0x1e] * 6 + [0x15, 0x15, 0x14, 0x16, 0x1d, 0x1f, r.getrandbits(32)])
    regions = {LOG: log, WINDOW: window, KIDS: bytes(kids), VT: vtable({9: 1}), 0xfb34c1: bytes([r.choice([1, 1, 3, 0, 0xfe])]), 0xfb3b7c: struct.pack('<I', r.choice([1, 1, 2, 2, 0, 0x102])),
               0x752ec4: struct.pack('<2I', r.choice([left, left, 0xffffffff, r.getrandbits(32)]), r.choice([level, level, 0xffffffff, r.getrandbits(32)]))}
    return Call(0x4a0fb0, [WINDOW, message, r.getrandbits(32), r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
