# Class A's surface made (src/game/gfx/texsurface: 0x58b4f0) against the original: the device's CreateSurface failing once, twice or not, the format's record found or not, 0-4 levels with the attached surfaces and interfaces each failing now and then (a failure below 0 and one above), the virtual base's fields at usual offsets and at one where the width replaces the vbtable pointer; the calls recorders.
# usage: texsurfcreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

FORMAT, PUSH, MESSAGE, DEBUG, CLOCK = range(1, 6)
QI, CREATE, ATTACHED = 0, 6, 12   # (the slots)
B = Bench([(0x58b4f0, 0x58b78f)], hooks={0x573820: _recorder(FORMAT, 2), 0x56af20: _recorder(PUSH, 2), 0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(DEBUG, 1, cdecl=True), 0x5645a0: _recorder(CLOCK, 0)})
OBJECT, ENGINE, DEVICE, DEVICE_VT, SURFACES, SURFACE_VT, PIXEL_FORMAT, VBTABLE, VBTABLE2, TABLES = (RAM + 0x400 * k for k in range(10))
SUB = OBJECT + 0x200
FAILURES = [0x88760000 + 0x10, 1, 0x80004005]


def results(r, count, rate):
    return [r.choice(FAILURES) if r.randrange(rate) == 0 else 0 for _ in range(count)] + [0] * (40 - count)


def case(r):
    log = log_region()
    obj = bytearray(r.bytes(0x400))
    surfaces = [SURFACES + 0x10 * k for k in range(8)]
    offset = r.choice([0x50, 0x80, 0, 0xfffffff8] if r.randrange(8) == 0 else [0x50])
    put(obj, 0x200 - 0x3c, 'I', VBTABLE)
    for k in range(4):
        put(obj, 0x200 - 0x28 + 4 * k, 'I', surfaces[k])
    levels = r.choice([0, 1, 1, 2, 3, 4])
    width = VBTABLE2 if offset == 0xfffffff8 else r.choice([64, 256, r.getrandbits(32)])
    height = r.choice([64, 256, r.getrandbits(32)])
    set_return(log, FORMAT, r.choice([1, 1, 1, 1, 1, 1, 1, 0, 2]))
    strings(log, FORMAT, 0, 1)
    fill(log, FORMAT, 0, struct.pack('<I', PIXEL_FORMAT))
    set_return(log, MESSAGE, r.getrandbits(32))
    blob(log, CREATE, 1, 0x7fffffff)
    fill(log, CREATE, 2, struct.pack('<I', surfaces[0]), which=1)
    blob(log, ATTACHED, 1, 1)
    fill(log, ATTACHED, 2, struct.pack('<3I', *surfaces[1:4]), which=2, records=3)
    fill(log, QI, 2, struct.pack('<4I', *surfaces[4:8]), which=3, records=4)
    answers(log, CREATE, TABLES)
    answers(log, ATTACHED, TABLES + 0x100)
    answers(log, QI, TABLES + 0x200)
    answers(log, CLOCK, TABLES + 0x300)
    tables = results(r, 2, 3) + results(r, 4, 6) + results(r, 4, 6) + [r.getrandbits(32) for _ in range(40)]
    regions = {LOG: log, OBJECT: obj, ENGINE: struct.pack('<3I', r.getrandbits(32), r.getrandbits(32), DEVICE) + r.bytes(4), DEVICE: struct.pack('<I', DEVICE_VT) + r.bytes(0xc),
               DEVICE_VT: vtable({}, com={CREATE: 4}), SURFACES: b''.join(struct.pack('<I', SURFACE_VT) + r.bytes(0xc) for _ in surfaces), SURFACE_VT: vtable({}, com={QI: 3, ATTACHED: 3}),
               PIXEL_FORMAT: r.bytes(0x20), VBTABLE: struct.pack('<2I', r.getrandbits(32), offset), VBTABLE2: struct.pack('<2I', r.getrandbits(32), 0x50), TABLES: struct.pack('<160I', *tables),
               0x879a24: struct.pack('<I', ENGINE), 0x879da8: r.bytes(4), 0x879cd4: r.bytes(4), 0x8c4898: r.bytes(4)}
    return Call(0x58b4f0, [SUB, width, height, r.randrange(8), levels], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
