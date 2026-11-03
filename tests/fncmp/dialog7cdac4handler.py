# Dialog7cdac4_Handler (src/game/ui/dialog7cdac4, 0x4c4480) against the original. Window_FindChild real over dialog7cb400.py's children (0x11e3 and 0x11e4 among decoys, missing now and then: Window_SetFocus is a recorder, its ECX logged), the other calls recorders numbered as kDialog7cdac4HandlerEnv from 10; the guard byte set or clear.
# usage: dialog7cdac4handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

B = Bench([(0x4c4480, 0x4c4667), (0x65d7bc, 0x65d840)],
          hooks={0x658d9f: _recorder(10, 4, cdecl=True), 0x658ccc: _recorder(11, 4, cdecl=True), 0x486b60: _recorder(12, 0, cdecl=True), 0x486b70: _recorder(13, 0, cdecl=True), 0x4862a0: _recorder(14, 0, cdecl=True), 0x47ee90: _recorder(15, 4, cdecl=True),
                 0x550ca0: _recorder(16, 0), 0x67a7d0: _recorder(17, 1, cdecl=True), 0x48b390: _recorder(18, 0, cdecl=True), 0x5b7ba0: _recorder(19, 7), 0x5d6060: _recorder(20, 0), 0x65e59b: _recorder(21, 1)})
WINDOW, FIRST, SECOND, SCENE, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x340, RAM + 0x380, RAM + 0x1000


def case(r):
    log = log_region()
    regions = {LOG: log}
    ids = [i for i in (0x11e3, 0x11e4) if r.random() < 0.95]
    first, _ = kids(r, regions, KIDS, ids)
    regions[WINDOW] = put(bytearray(r.bytes(0x200)), 0x5c, 'I', first)
    regions[FIRST], regions[SECOND], regions[SCENE] = r.bytes(0x10), r.bytes(0x10), r.bytes(0x20)
    regions[0xf82884] = struct.pack('<I', FIRST)
    regions[0xf82888] = struct.pack('<I', SECOND)
    regions[0xf85614] = struct.pack('<I', SCENE)
    regions[0xf82840] = r.bytes(4)
    regions[0xfb34bb] = bytes([r.choice([0, 1, 0xfe, 0xff, r.getrandbits(8)])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([2, 2, 0, 1, r.getrandbits(32)]))
    regions[0x7cdac4] = r.bytes(4)
    for k in (10, 11, 14, 17, 18, 21):
        set_return(log, k, r.getrandbits(32))
    set_return(log, 15, r.getrandbits(32))
    message = r.deal([0x14, 0x15, 0x100, 0x100, 0x100, 0x802, 0x803, 0x804, 0x801, 0x13, 0x16, 0xff, 0x805, r.getrandbits(32)])
    a = r.choice([0xffffffff, 0xfffffffe, 0x11e4, 0x11e4, 0x11e3, 0, r.getrandbits(32)])
    return Call(0x4c4480, [WINDOW, message, a, r.getrandbits(32)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
