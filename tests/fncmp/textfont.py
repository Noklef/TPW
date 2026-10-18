# The text font selection (src/game/gfx/text: 0x46c010) against the original: the font table (0x7b059c, 0x18-byte entries) null or not, entries
# with a font (+0xc) or none, indices in it and just past it, modes 0 (the shape driver 0x7031c0), 1 (the plain one 0x7031b0) and others (nothing
# done); the text driver before it (0xfa76a0) none, one with no shutdown or one whose shutdown (cdecl) logs its context and the new driver.
# usage: textfont.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG2, LOG, RAM, Bench, Call, log_region, put  # noqa: E402

B = Bench([(0x46c010, 0x46c080), (0x5f8ec0, 0x5f8f50)])
TABLE, OLD = RAM, RAM + 0x200
FONTS, DRIVER = 0x7b059c, 0xfa76a0  # the font table pointer; the text driver and its context
COUNT = 6


def case(r):
    table = bytearray(r.bytes(COUNT * 0x18))
    for i in range(COUNT):
        put(table, i * 0x18 + 0xc, 'I', r.choice([0, RAM + 0x800 + 0x10 * i, r.getrandbits(32)]))
    old = r.choice([0, OLD, OLD])
    regions = {TABLE: table, LOG: log_region(), FONTS: struct.pack('<I', r.choice([TABLE, TABLE, TABLE, 0])),
               DRIVER: struct.pack('<2I', old, r.getrandbits(32)),
               OLD: struct.pack('<4I', r.getrandbits(32), r.choice([CDECL_LOG2, CDECL_LOG2, 0]), r.getrandbits(32), r.getrandbits(32))}
    index = r.choice([0, 1, 2, COUNT - 1, r.randrange(COUNT), r.randrange(COUNT)])
    mode = r.choice([0, 0, 1, 1, 2, 0xffffffff, r.getrandbits(32)])
    return Call(0x46c010, [index, mode], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
