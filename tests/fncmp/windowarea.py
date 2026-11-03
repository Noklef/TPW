# A window's area rects (src/game/ui/windowarea: 0x661810, 0x66b438, 0x65d616) against the original. GuiRect's real code on a rect with its optional rect list (none or a list node).
# usage: windowarea.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region  # noqa: E402

B = Bench([(0x661810, 0x66182c), (0x66b438, 0x66b454), (0x65d616, 0x65d69f), (0x659d00, 0x659dbc), (0x659dbc, 0x659de3), (0x659de5, 0x659e3e), (0x659e3e, 0x659e83), (0x65a9d0, 0x65aa00), (0x65aa20, 0x65ab40), (0x661700, 0x66176a), (0x66bff7, 0x66c01b), (0x66c01a, 0x66c7dc), (0x659e83, 0x65a1c3), (0x65a3e9, 0x65a4a3), (0x65a1c3, 0x65a3e9), (0x5da3c0, 0x5da3c1)])
AREA, OUT, WINDOW = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def rect(r):
    """a GuiRect with no list: the vtable, four words, the list pointer null and the rest"""
    buf = bytearray(r.bytes(0x18))
    struct.pack_into('<I', buf, 0, 0x705920)
    for off in range(0xc, 0x18, 4):
        struct.pack_into('<I', buf, off, 0)
    return buf


def case(r):
    regions = {LOG: log_region(), HEAP: heap(0x400), OUT: rect(r)}
    area = bytearray(r.bytes(0x50))
    area[0x20:0x38] = rect(r)
    area2 = bytearray(area)
    area2[0x2c:0x44] = rect(r)
    op = r.randrange(3)
    if op == 0:
        regions[AREA] = area
        return Call(0x661810, [AREA, OUT], regions, conv='t', ret='a')
    if op == 1:
        regions[AREA] = area2
        return Call(0x66b438, [AREA, OUT], regions, conv='t', ret='a')
    regions[AREA] = area
    window = bytearray(r.bytes(0xa0))
    struct.pack_into('<I', window, 0x9c, r.choice([AREA, 0]))
    regions[WINDOW] = window
    return Call(0x65d616, [WINDOW, OUT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
