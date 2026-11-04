# A sign's text drawn by GDI and Uniscribe (src/game/gfx/signrender: 0x5eab70) against the original: the imports recorders; the section's bytes, the text's width at each try and the advances compared.
# usage: signrender.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob, fill, global_heap, heap, log_region, put, recorder, set_return, strings  # noqa: E402

DIB, MEMDC, BKMODE, FONT, SELECT, DELETE, BK, TEXTCOLOUR, DELDC = 1, 2, 3, 4, 5, 6, 7, 8, 9
ANALYSE, WIDTHS, FREE, PSIZE, OUT = 10, 11, 12, 13, 14
SLOTS = {0x6fd04c: (DIB, 6), 0x6fd050: (MEMDC, 1), 0x6fd040: (BKMODE, 2), 0x6fd048: (FONT, 1), 0x6fd044: (SELECT, 2), 0x6fd038: (DELETE, 1), 0x6fd034: (BK, 2), 0x6fd03c: (TEXTCOLOUR, 2),
         0x6fd05c: (DELDC, 1), 0x6fd3a8: (ANALYSE, 13), 0x6fd3a4: (WIDTHS, 2), 0x6fd3a0: (FREE, 1), 0x6fd3b0: (PSIZE, 1), 0x6fd3ac: (OUT, 8)}
B = Bench([(0x5eab70, 0x5eb090), (0x64bf70, 0x64bf8e), (0x5e7840, 0x5e78ac)] + gamestring.B_RANGES)
SELF, IMAGE, TEXT, PIXELS, SIZES, SIZE_TABLE, SELECT_TABLE, FONT_TABLE = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00
BITS = RAM + 0x1000
ADVANCES = 0xf85f00


def case(r):
    log = log_region()
    table = bytearray(0x600)
    for slot, (k, n) in SLOTS.items():
        put(table, slot - IMPORT_TABLE, 'I', recorder(k, n, True))
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x8000)}
    length = r.choice([0, 1, 2, 5, 20])
    width, height = r.choice([0, 1, 3, 8, 16]), r.choice([0, 1, 4, 8])
    regions[TEXT] = bytes(r.randrange(1, 0xffff).to_bytes(2, 'little') * length) + bytes(2)
    image = bytearray(0x10)
    struct.pack_into('<4I', image, 0, width, height, r.choice([1, 1, 2]), PIXELS if not length else 0)
    regions[IMAGE] = image
    regions[PIXELS] = r.bytes(0x100)
    self = bytearray(r.bytes(0x100))
    put(self, 0x10, 'i', r.randrange(-32, 0) if r.random() < 0.9 else r.randrange(1, 32))
    put(self, 0x14, 'i', r.choice([0, 0, r.randrange(1, 40), r.randrange(1, 4)]))
    put(self, 8, 'i', r.choice([100, r.randrange(50, 150), r.randrange(-50, 50)]))
    regions[SELF] = self
    regions[ADVANCES] = bytes(0x100)
    # The section: zero rows now and then, for the rows found; a pixel only in the last column now and then.
    bits = bytearray(r.bytes(0x1000))
    stride = max(2 * width, 1)
    for y in range(2 * height):
        if r.random() < 0.5:
            bits[y * stride:(y + 1) * stride] = bytes(stride)
            if r.random() < 0.3 and width:
                bits[y * stride + stride - 1] = 0xff
    regions[BITS] = bits
    set_return(log, DIB, 0x3000 if r.random() < 0.95 else 0)
    strings(log, DIB, 0, 1 << 3)
    blob(log, DIB, 1, 0xffffffff)
    fill(log, DIB, 3, struct.pack('<I', BITS))
    set_return(log, MEMDC, 0x4000)
    answers(log, SELECT, SELECT_TABLE)
    regions[SELECT_TABLE] = struct.pack('<64I', *[0x5000 + i for i in range(64)])
    answers(log, FONT, FONT_TABLE)
    regions[FONT_TABLE] = struct.pack('<64I', *[0x6000 + i for i in range(64)])
    blob(log, FONT, 0, 3)
    strings(log, ANALYSE, 0, 1 << 7 | 1 << 8 | 1 << 12)
    blob(log, ANALYSE, 9, 0xfffff)
    fill(log, ANALYSE, 12, struct.pack('<4I', 0x7000, 0x7100, 0x7200, 0x7300), 1, records=4)
    fill(log, WIDTHS, 1, struct.pack('<20i', *[r.randrange(-40, 400) for _ in range(20)]), 2)
    blob(log, FREE, 0, 1)
    # The widths: the first of 2 * width or more in most cases, then either side. The log keeps 40 calls and calls past them are not counted,
    # so from the fifth on (past the log) too small, and the search ends.
    sizes = [r.randrange(2 * width, 2 * width + 9) if r.random() < 0.7 else r.randrange(2 * width - 9, 2 * width)] + \
            [r.randrange(2 * width - 9, 2 * width + 9) for _ in range(3)] + [2 * width - 1] * 36
    regions[SIZES] = struct.pack('<80i', *[v for s in sizes for v in (s, r.randrange(1, 40))])
    answers(log, PSIZE, SIZE_TABLE)
    regions[SIZE_TABLE] = struct.pack('<40I', *[SIZES + 8 * i for i in range(40)])
    return Call(0x5eab70, [SELF, 0x2000, TEXT, IMAGE], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
