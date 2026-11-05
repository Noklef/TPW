# A texture surface's next surface locked (src/game/gfx/texsurface: 0x58b810 TexSurfaceA_LockNext) against the original: the shared description
# cleared to its size, the next surface (the recorder 0x58b1f0) kept and locked through its slot 0x64, a failure logged, whether it gave 0.
# usage: texsurfacelock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, log_region, put, set_return, vtable  # noqa: E402

MESSAGE, DEBUG, NEXT, LOCK = 3, 4, 6, 25
B = Bench([(0x58b810, 0x58b885)], hooks={0x564650: _recorder(DEBUG, 1, cdecl=True), 0x56a770: _recorder(MESSAGE, 1, cdecl=True), 0x58b1f0: _recorder(NEXT, 2)})
SELF, SURFACE, VT, BASES = RAM, RAM + 0x100, RAM + 0x140, RAM + 0x400


def case(r):
    log = log_region()
    offset = r.choice([0x10, 0x20, 0x40])
    m = bytearray(r.bytes(0x80))                                            # (-0x3c the sub-object's table, its +4 the virtual base's offset; the side -0x34 from there)
    put(m, 0, 'I', BASES)
    put(m, 8 + offset, 'I', r.getrandbits(32))
    set_return(log, NEXT, SURFACE)
    set_return(log, MESSAGE, r.getrandbits(32))
    set_return(log, LOCK, r.choice([0, 0, 1, r.getrandbits(31) | 0x80000000, r.getrandbits(31)]))
    blob(log, LOCK, 2, 0x7fffffff)
    regions = {LOG: log, SURFACE: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 40, com={LOCK: 5}), 0x879cd4: r.bytes(4), SELF: bytes(m),
               BASES: struct.pack('<II', r.getrandbits(32), offset), 0x8c48a8: r.bytes(0x7c)}
    return Call(0x58b810, [SELF + 0x3c], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
