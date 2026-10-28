# The game mode's map preview (src/game/world/mappreview) against the original: GameMode_MapPreview (0x5234d0) in each game mode ([0x81ae2c]
# dealt), its calls outside the accessors recorders (the emulator's hooks= on their addresses, the driver's MapPreview env: driver.cpp), so the
# log holds each line, footprint, test and call in order. The world [0x7cf83c] (and [0x8023a0], MapHandle_Entry28's) is mapped where the cells
# and 0x44-byte records of ids 0..IDS lie (x -1..9, y -1..5): their parents (+0x10) and handles (+0) are such ids too. The tests answer as
# dealt: the footprint, GlobalSave_HasKey, CoasterPiece_CanPlaceAt and the area tests per call zero or not (fail_mask), MapCell_CanBuildTrack
# 9 / 1 / other (answers); ObjectDefs_Find the fixture definition, the money table its record, Cell_FirstObject a fixture handle word; the
# lines' ends written through their out arguments (fill) or not (in modes 0x15 / 0x16 only at or past the coaster rectangle's far side, so
# that its loops end).
# usage: mappreview.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, blob, fail_mask, fill, log_region,  # noqa: E402
                   set_return, strings)

WORLD = 0x31000000
RECORDS = WORLD + 0x294             # the 0x44-byte records (id 0 first)
CELLS = WORLD + 0x6cde * 0x28       # the 0x28-byte cells (id 0 first)
IDS = 0x290                         # ids 0..IDS-1 mapped (row 5's column 9 is 0x28a)
THING, THING2 = RAM + 0x1000, RAM + 0x1100
DEF = RAM + 0x1200                  # ObjectDefs_Find's answer
MONEY = RAM + 0x1400                # GameTable7cfb90_Get1da726_5195d0's (Source_Field0c reads +0xc)
TABLES = RAM + 0x2000               # answers(): MapCell_CanBuildTrack's
WORDS = RAM + 0x3000                # Cell_FirstObject's handle word

CDECL = {0x536100: (0, 5), 0x53aac0: (1, 5), 0x528a70: (2, 6), 0x52f1b0: (3, 2), 0x527960: (4, 3), 0x524960: (5, 4), 0x52f580: (6, 2),
         0x530e80: (7, 0), 0x52b400: (8, 3), 0x531a50: (9, 5), 0x532a50: (10, 4), 0x434740: (11, 7), 0x52f950: (12, 0)}
THISCALL = {0x539760: (13, 3), 0x4d4b70: (14, 1), 0x4d99e0: (15, 1), 0x412e90: (16, 1), 0x5195d0: (17, 0)}

HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CDECL.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in THISCALL.items()})
B = Bench([(0x5234d0, 0x52473a), (0x52f860, 0x52f890), (0x52f840, 0x52f860), (0x52f1f0, 0x52f210), (0x52fb90, 0x52fba0), (0x52fbc0, 0x52fbd0),
           (0x53c8c0, 0x53c8d0), (0x4d8300, 0x4d8340), (0x53ad30, 0x53adc0), (0x522850, 0x522854), (0x53aec0, 0x53aeeb), (0x536c50, 0x536c60),
           (0x536310, 0x536390), (0x5363f0, 0x536483), (0x535db0, 0x535dc3), (0x4d0af0, 0x4d0b40), (0x434710, 0x434740), (0x424ec0, 0x424ee0),
           (0x6ad810, 0x6ad814), (0x5da3c0, 0x5da3c1)],
          hooks=HOOKS)

MODES = [0, 1, 3, 0x38, 4, 0x3b, 0xb, 0x10, 0x19, 0x39, 0x3a, 0x36, 0xe, 0x13, 0x1a, 0x15, 0x16,
         1, 4, 4, 4, 0x3b, 0x39, 0x39, 0x3a, 0x36, 0x1a, 0x1a, 0x1a, 0x15, 0x15, 0x16, 0x16]   # (the longer ones more)


def mask(r):
    return r.choice([0, 0xffffffff, r.getrandbits(32), r.getrandbits(32)])


def table(r, values):
    return b''.join(struct.pack('<I', r.choice(values) & 0xffffffff) for _ in range(40))


def case(r):
    mode = r.deal(MODES) if r.random() < 0.95 else r.choice([2, 0x17, 0x37, r.getrandbits(8)])
    cx, cy = lambda: r.randrange(0, 10), lambda: r.randrange(0, 4)
    cid = lambda: (cy() << 7) + cx() + 1   # noqa: E731
    a, b = r.randrange(0, 5), r.randrange(0, 3)
    c, d = a + r.randrange(0, 4), b + r.randrange(0, 3)   # the coaster rectangle (Coasters_GetState)

    log = log_region()
    for k in (2, 9, 10, 11, 14):   # the zero-or-not answers
        if r.random() < 0.7:
            fail_mask(log, k, mask(r))
    answers(log, 13, TABLES)
    set_return(log, 15, WORDS)
    set_return(log, 16, DEF)
    set_return(log, 17, MONEY)
    strings(log, 0, 0, out=0x10)    # the lines' end pointers (stack): the row's
    strings(log, 1, 0, out=0x10)
    blob(log, 0, 3, 1)              # and the column they are given (Rect's insets; a fill's when one writes it)
    blob(log, 1, 3, 1)
    strings(log, 15, 0, out=1)      # Cell_FirstObject's out word
    struct.pack_into('<I', log, LOG_BYTES + 4 * 9, 0x10)        # HoardingGrid_CheckArea's charge: a BYTE
    struct.pack_into('<I', log, LOG_BYTES + 4 * 14, 1 << 16)    # GlobalSave_HasKey's and ObjectDefs_Find's ids: WORDs
    struct.pack_into('<I', log, LOG_BYTES + 4 * 16, 1 << 16)
    area = mode in (0x15, 0x16)
    far_x = lambda: r.randrange(c, c + 2) if area else r.choice([cx(), cx(), r.randrange(-1, 11)])   # noqa: E731
    far_y = lambda: r.randrange(d, d + 2) if area else r.choice([cy(), cy(), r.randrange(0 if mode == 0x1a else -1, 6)])   # noqa: E731
    if r.random() < 0.5:
        fill(log, 0, 3, struct.pack('<4i', *[far_x() for _ in range(4)]), which=0, records=4)
        fill(log, 0, 4, struct.pack('<4i', *[far_y() for _ in range(4)]), which=1, records=4)
    if r.random() < 0.4:
        fill(log, 1, 3, struct.pack('<4i', *[far_x() for _ in range(4)]), which=2, records=4)
        fill(log, 1, 4, struct.pack('<4i', *[far_y() for _ in range(4)]), which=3, records=4)

    records = bytearray(r.bytes(IDS * 0x44))
    cells = bytearray(r.bytes(IDS * 0x28))
    for i in range(IDS):
        struct.pack_into('<H', records, i * 0x44, cid())
        struct.pack_into('<I', records, i * 0x44 + 8, r.choice([0, 0, 1, 1, 1, 1, 3, 4, 4, 4, 7, 9, 0xa, 0xb, 0x1e, r.getrandbits(32)]))
        struct.pack_into('<H', records, i * 0x44 + 0x10, cid())
        struct.pack_into('<I', cells, i * 0x28 + 8, r.choice([0, 0, 1, 1, 0xb, 0xc, 0xd, 0x10, 0x11, 0x12, 0x19, 0x19, r.getrandbits(32)]))
        cells[i * 0x28 + 0xd] = r.choice([1, 4, 0x10, 0x40, 1, 4, 0x10, 0x40, 0, r.getrandbits(8)])
        struct.pack_into('<H', cells, i * 0x28 + 0x10, cid())

    thing = bytearray(r.bytes(0x40))
    struct.pack_into('<I', thing, 0x10, r.choice([0, 0x5a, 0xb4, 0x10e, r.getrandbits(32)]))
    thing[2] = r.choice([3, 3, r.getrandbits(8)])
    thing2 = bytearray(r.bytes(0x40))
    thing2[2] = r.choice([3, r.getrandbits(8)])
    things = bytearray(r.bytes(0x50))
    for i in range(4):
        struct.pack_into('<I', things, i * 0x14, r.choice([THING, THING2]))

    small = lambda: r.choice([-2, -1, 0, 1, 2, r.getrandbits(32)])   # noqa: E731
    definition = bytearray(r.bytes(0x200))
    struct.pack_into('<i', definition, 0xc4, r.choice([0, 1, -1, 5]))
    for off in range(0xa4, 0xc4, 4):
        struct.pack_into('<I', definition, off, small() & 0xffffffff)
    money = bytearray(r.bytes(0x10))
    struct.pack_into('<i', money, 0xc, r.choice([0, 100, -5, 0x7fffffff, r.getrandbits(31)]))

    near = bytearray(r.bytes(0x80))   # 0x818680..0x818700
    struct.pack_into('<H', near, 4, cid())                                     # 0x818684
    struct.pack_into('<H', near, 0x14, cid())                                  # 0x818694
    struct.pack_into('<H', near, 0x18, cid() if r.random() < 0.9 else 0)       # 0x818698
    struct.pack_into('<I', near, 0x2c, THING)                                  # 0x8186ac
    struct.pack_into('<H', near, 0x60, r.getrandbits(16) if r.random() < 0.85 else 0)   # 0x8186e0

    focus = bytearray(r.bytes(0x58))   # 0x7639a0..0x7639f8
    struct.pack_into('<i', focus, 0, r.choice([-1, -1, r.getrandbits(31)]))   # 0x7639a0
    struct.pack_into('<ii', focus, 4, cx(), r.choice([0, 2]))                  # 0x7639a4 / a8
    struct.pack_into('<H', focus, 0x10, r.choice([0xffff, r.getrandbits(16), r.getrandbits(16)]))   # 0x7639b0
    struct.pack_into('<I', focus, 0x20, r.choice([0, r.getrandbits(32), r.getrandbits(32)]))  # 0x7639c0
    struct.pack_into('<ii', focus, 0x50, cx(), cy())                           # 0x7639f0 / f4

    start = (-1, -1) if r.random() < 0.35 else r.choice([(cx(), cy()), (cx(), cy()), (-1, cy()), (cx(), -1)])
    if mode in (0x1a, 0x16):
        x, y = r.randrange(-1, 11), cy()   # (their cells read without a bounds test)
    else:
        x, y = r.choice([(cx(), cy()), (cx(), cy()), (r.randrange(-1, 11), r.randrange(-1, 6)), (0, 0), (128, 2)])
    if r.random() < 0.1 and start[0] >= 0 and start[1] >= 0:
        x, y = start
    flags = r.choice([0, 0xc, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16) << 16])

    def word(v):
        return struct.pack('<I', v & 0xffffffff)
    c10 = bytearray(r.bytes(0x48))
    c10[0x44] = r.choice([0, 1])                                               # 0x818c54
    regions = {
        LOG: log, RECORDS: records, CELLS: cells, THING: thing, THING2: thing2, DEF: definition, MONEY: money,
        TABLES: table(r, [9, 9, 1, 0, r.getrandbits(32)]), WORDS: struct.pack('<HH', r.randrange(4), 0),
        0x7cf83c: word(WORLD), 0x8023a0: word(WORLD + 0x2d8), 0x7cfb90: things, 0x81ae2c: word(mode),
        0x785a2c: word(r.choice([0, 0, 1])), 0x8187f8: word(r.choice([0, 1, 1])), 0x816d48: bytearray(r.bytes(0x1c)),
        0x818680: near, 0x818c10: c10, 0x7639a0: focus, 0x81ede4: struct.pack('<ii', *start),
        0x81d740: word(r.randrange(2, 12)), 0x81b1b0: word(r.randrange(3, 8)), 0x81d7a4: word(r.choice([0, 0x5a, 0xb4, 0x10e, r.getrandbits(32)])),
        0x81af4c: r.bytes(4), 0x81af64: r.bytes(4), 0x871ee0: r.bytes(4), 0x81b604: r.bytes(4), 0x874fc4: r.bytes(4),
        0x79221c: word(a), 0x790fd8: word(b), 0x79104c: word(c), 0x791cf8: word(d),
    }
    return Call(0x5234d0, [x & 0xffffffff, y & 0xffffffff, flags], regions)


if __name__ == '__main__':
    B.main(case)
