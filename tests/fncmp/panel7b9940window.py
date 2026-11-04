# The front end's panel shown again with its window made (src/game/ui/panel7b9940: Panel7b9940_ShowWindow, 0x48a410) against the original. Every call a recorder, in Panel7b9940_Open's slots (driver.cpp's
# kPanel7b9940OpenEnv: their own benches have them), Window7cc8bc_Create 31. SceneFlags_Touch runs. The window failing now and then.
# usage: panel7b9940window.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_mask, log_region, put, set_return  # noqa: E402

B = Bench([(0x48a410, 0x48a51a), (0x5836b0, 0x5836c2)],
          hooks={0x659064: _recorder(10, 0, cdecl=True), 0x576410: _recorder(15, 2), 0x4bf460: _recorder(31, 0, cdecl=True), 0x65dd7b: _recorder(16, 7), 0x65e526: _recorder(18, 1), 0x65f11d: _recorder(20, 1), 0x65fd0e: _recorder(21, 2),
                 0x659607: _recorder(29, 0, cdecl=True), 0x65961c: _recorder(30, 0, cdecl=True)})
TABLE = RAM


def case(r):
    log = log_region()
    fail_mask(log, 16, r.choice([1, 1, 1, 0]))
    set_return(log, 29, r.getrandbits(32))
    set_return(log, 30, r.getrandbits(32))
    second, virtual = r.choice([(0x10, 8), (0x20, 8), (0x40, 8), (0x40, 0x20)])
    regions = {LOG: log, TABLE: struct.pack('<III', r.getrandbits(32), virtual, second), 0x7b9940: put(bytearray(r.bytes(0x100)), 0, 'I', TABLE), 0x8bd508: r.bytes(8), 0x87a6d0: r.bytes(4), 0xfaa5d8: r.bytes(4), 0x7cb2b0: r.bytes(4),
               0x78d90e: bytes([r.choice([0, 0, 1, 0x80])]), 0x6ff148: r.bytes(4)}
    return Call(0x48a410, [], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
