# The game panel 0x7cc328's window handler (src/game/ui/panel7cc328handler: Panel7cc328_Handler 0x4b4fc0) against the original. Window_FindChild among decoys real; recorders (driver.cpp's kPanel7cc328HandlerEnv, in its order from 10): the second text hashed, the items' data from fills.
# usage: panel7cc328handler.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from dialog7cb400 import kids  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings  # noqa: E402

C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4b4fc0, 0x4b5276), (0x65d7bc, 0x65d840)],
          hooks={0x48cf70: _recorder(10, 0), 0x664c71: _recorder(11, 2), 0x6644ea: _recorder(12, 5), 0x65ef90: _recorder(13, 2), 0x65f024: _recorder(14, 1), 0x486b40: C(15, 1), 0x48cd10: _recorder(16, 0), 0x48caf0: _recorder(17, 0), 0x48cbe0: _recorder(18, 0), 0x495aa0: C(19, 2), 0x496620: C(20, 2),
                 0x48cfa0: _recorder(21, 0), 0x4b4510: _recorder(22, 1), 0x475550: C(23, 1), 0x48d200: C(24, 4), 0x4b6080: _recorder(25, 1)})
WINDOW, PANEL, STAFF, KIDS = RAM + 0x100, RAM + 0x300, RAM + 0x800, RAM + 0x2000
LIST_ID = 0x1e7b
CHILD = 0x4321


def case(r):
    log = log_region()
    state = bytearray(r.bytes(0x34))
    put(state, 0x10, 'I', PANEL)  # (0x7cc338)
    put(state, 0x24, 'I', r.choice([0, 3, r.getrandbits(32)]))  # (0x7cc34c)
    regions = {LOG: log, 0x7cc328: state, 0x756f40: r.bytes(4), 0x7c2658: r.bytes(4), 0x7cc3b0: r.bytes(4)}
    first, ids = kids(r, regions, KIDS, (LIST_ID, CHILD))
    regions[WINDOW] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    regions[PANEL] = put(bytearray(r.bytes(0x100)), 0x5c, 'I', first)
    put(regions[ids[LIST_ID]], 0x150, 'i', r.choice([0, 1, 2, 3, 5, -1]))
    staff = bytearray(r.bytes(0x400 * 2))
    for at in (0x1fc, 0x5fc):
        put(staff, at, 'f', r.choice([0.0, 1.0, 50.5, 99.9, 100.0, 255.0, 256.0, 1000.5, -1.5, r.uniform(0, 300)]))
    regions[STAFF] = staff
    fill(log, 11, 1, b''.join(struct.pack('<I', r.choice([STAFF, STAFF + 0x400])) for _ in range(8)), records=8)
    strings(log, 11, 0, 1 << 1)
    blob(log, 12, 1, 2)
    set_return(log, 13, r.choice([0, 1, 0x10000, 0x10001]))
    set_return(log, 24, r.getrandbits(32))
    message = r.deal([5, 0x10, 0x10, 0x10, 0x14, 0x15, 0x15, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x400, 0x401, 0x401, 0x401, 0x406, 0xf, 0x16, 0x13, 0xff, 0x101, 0x3ff, 0x402, 0x405])
    a = {0x10: r.choice([0x80080] * 8 + [0x80081]), 0x100: r.choice([0xffffffff, 0x1e7a, 0x1e83, 0x1e84, 0x1e85, 0x1e86, 0x1e87, 0x1e88, 0x1e89, 0x1e82, 0x1e79, 0xfffffffe, 0x1e7b])}.get(message, r.choice([CHILD, CHILD, CHILD, 0x7777]))
    b = r.choice([0, 1, 2, 0xffffffff, 0xffffffff, r.getrandbits(32)])
    return Call(0x4b4fc0, [WINDOW, message, a, b], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
