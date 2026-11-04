# A side's two surfaces made (src/game/gfx/surfacepool: 0x58b0b0) against the original, the side's index real, the device and surfaces COM recorders, the rest recorders: no format, a make or the surface's call failing or answering a positive result, each surface.
# usage: surfacemake.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

FORMAT, PUSH, DEBUG, MESSAGE, MAKE, SET = 1, 2, 3, 4, 6, 37
B = Bench([(0x58b0b0, 0x58b1e5), (0x5848b0, 0x584a21)], hooks={0x573820: _recorder(FORMAT, 2), 0x56af20: _recorder(PUSH, 2), 0x564650: _recorder(DEBUG, 1, cdecl=True), 0x56a770: _recorder(MESSAGE, 1, cdecl=True)})
POOL, HOLDER, DEVICE, DEVICE_VT, SURFACE, SURFACE_VT, RECORD, MADE, SETS = RAM, RAM + 0x100, RAM + 0x120, RAM + 0x140, RAM + 0x1c0, RAM + 0x1e0, RAM + 0x300, RAM + 0x340, RAM + 0x3e0
OPS = ['made', 'made', 'made', 'no format', 'make fails', 'make positive', 'set fails', 'set positive']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    made = [0] * 40
    sets = [0] * 40
    at = r.randrange(0, 2)
    if op == 'make fails':
        made[at] = r.getrandbits(31) | 0x80000000
    elif op == 'make positive':
        made[at] = r.randrange(1, 1 << 31)
    elif op == 'set fails':
        sets[at] = r.getrandbits(31) | 0x80000000
    elif op == 'set positive':
        sets[at] = r.randrange(1, 1 << 31)
    answers(log, MAKE, MADE)
    answers(log, SET, SETS)
    blob(log, MAKE, 1, 0x7fffffff)
    fill(log, MAKE, 2, struct.pack('<I', SURFACE))
    strings(log, FORMAT, 0, 1)
    fill(log, FORMAT, 0, struct.pack('<I', RECORD), which=1)
    set_return(log, FORMAT, 0 if op == 'no format' else r.choice([1, 1, 2, 0x100]))
    side = 1 << r.randrange(0, 9)
    regions = {LOG: log, POOL: r.bytes(0x90), HOLDER: put(bytearray(r.bytes(0x10)), 8, 'I', DEVICE), DEVICE: struct.pack('<I', DEVICE_VT), DEVICE_VT: vtable({}, com={MAKE: 4}, size=8), SURFACE: struct.pack('<I', SURFACE_VT), SURFACE_VT: vtable({}, com={SET: 2}, size=38),
               RECORD: r.bytes(0x20), MADE: struct.pack('<40I', *made), SETS: struct.pack('<40I', *sets), 0x879a24: struct.pack('<I', HOLDER), 0x879da8: r.bytes(4), 0x879cd4: r.bytes(4)}
    return Call(0x58b0b0, [POOL, side, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
