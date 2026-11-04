# Class A's mip levels filled (src/game/gfx/texsurface: 0x58baf0) against the original: 1-4 levels, the pool's surfaces and the halving recorders, the surfaces' lock, unlock and load each failing now and then (a failure below 0 and one above); the description a lock fills hashed at the halving and kept as the previous level's; the virtual base's width at usual offsets.
# usage: texsurfmips.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, set_return, vtable  # noqa: E402

PUSH, MESSAGE, DEBUG, POOL, HALVE = 2, 3, 4, 6, 7
LOAD, LOCK, UNLOCK = 5, 25, 32   # (the slots)
B = Bench([(0x58baf0, 0x58bd0d)], hooks={0x56af20: _recorder(PUSH, 2), 0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x564650: _recorder(DEBUG, 1, cdecl=True), 0x58b1f0: _recorder(POOL, 2), 0x58b890: _recorder(HALVE, 2)})
OBJECT, SURFACES, SURFACE_VT, VBTABLE, TABLES = RAM, RAM + 0x400, RAM + 0x800, RAM + 0xc00, RAM + 0xd00
SUB = OBJECT + 0x200
FAILURES = [0x88760000 + 0x10, 1, 0x80004005]


def results(r, rate):
    return [r.choice(FAILURES) if r.randrange(rate) == 0 else 0 for _ in range(40)]


def case(r):
    log = log_region()
    obj = bytearray(r.bytes(0x400))
    surfaces = [SURFACES + 0x10 * k for k in range(16)]
    offset = r.choice([0x50, 0x50, 0x80, 0])
    put(obj, 0x200 - 0x3c, 'I', VBTABLE)
    for k in range(4):
        put(obj, 0x200 - 0x28 + 4 * k, 'I', surfaces[k])
    put(obj, 0x200 - 0x18, 'I', surfaces[4])
    put(obj, 0x200 - 0x10, 'I', r.choice([1, 2, 3, 4, 4]))
    put(obj, 0x200 - 0x34 + offset, 'I', r.choice([0x100, 0x40, 0x80, r.getrandbits(32)]))
    set_return(log, MESSAGE, r.getrandbits(32))
    blob(log, LOCK, 2, 0x7fffffff)
    fill(log, LOCK, 2, b''.join(r.bytes(0x18) for _ in range(4)), records=4, offset=4)
    blob(log, HALVE, 0, 0x7fffffff)
    rate = r.choice([4, 12, 1000])
    answers(log, POOL, TABLES)
    answers(log, LOCK, TABLES + 0x100)
    answers(log, UNLOCK, TABLES + 0x200)
    answers(log, LOAD, TABLES + 0x300)
    pool = [r.choice(surfaces[5:]) for _ in range(40)]
    tables = pool + results(r, rate) + results(r, rate) + results(r, rate)
    regions = {LOG: log, OBJECT: obj, SURFACES: b''.join(struct.pack('<I', SURFACE_VT) + r.bytes(0xc) for _ in surfaces), SURFACE_VT: vtable({}, 33, com={LOAD: 6, LOCK: 5, UNLOCK: 2}),
               VBTABLE: struct.pack('<2I', r.getrandbits(32), offset), TABLES: struct.pack('<160I', *tables), 0x879cd4: r.bytes(4), 0x8c48a8: r.bytes(0x7c)}
    return Call(0x58baf0, [SUB], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
