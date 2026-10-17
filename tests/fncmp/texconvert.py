# The square texture conversion (src/game/render/texformat: 0x562f70 TexFormat_ConvertSquare) against the original: side * side ARGB pixels into each
# of the four texture formats (0x8bd554) and some unknown ones (the error then pushed onto the engine's list at 0x879ad0, as in enginecreate.py); the
# output now and then overlapping the input either way, and sides whose square wraps to 0.
# usage: texconvert.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _cdecl_log, log_region, strings  # noqa: E402

B = Bench([(0x562f70, 0x563099), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)], hooks={0x564650: _cdecl_log})
FROM, TO = RAM + 0x400, RAM + 0x2000


def case(r):
    log = log_region()
    strings(log, 0x3f, 1)
    side = r.choice([0, 1, 2, 3, 5, 8, 11, 16, 0x10000])
    to = r.choice([TO, TO, TO, FROM, FROM - 4, FROM + 4, FROM - 0x100])
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    fmt = r.choice([0, 1, 2, 3] * 4 + [4, 7, 0xffffffff])
    regions = {RAM: r.bytes(0x1400), TO: r.bytes(0x800), 0x879ad0: errors, 0x8bd508: struct.pack('<I', flags) + r.bytes(0x48) + struct.pack('<I', fmt),
               LOG: log}
    return Call(0x562f70, [to, FROM, side], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
