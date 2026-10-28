# The game mode's map click (src/game/world/mapclick) against the original: GameMode_MapClick (0x524960) in each game mode ([0x81ae2c] dealt),
# its calls outside the accessors recorders (the emulator's hooks= on their addresses, the driver's MapClick env: driver.cpp), so the log holds
# each line, sound, cell test and advisor post in order. The inlined advisor post (0x409cd0's message, 0x59b590 (id), the world's advisor
# 0x5194d0 and its post 0x59a940) is one cdecl recorder (slot 9) logging the id, as the port's `advise`. The world [0x7cf83c] is mapped where
# the cells and 0x44-byte records of ids 0..0x90 lie (clicks at columns 0-7, rows 0-1): their parents (+0x10) and ids (+0) are such ids too.
# The tests answer as dealt: the cell type tests, Object_CanOpen, 0x52a970's run, 0x52c6f0, HoardingGrid_CheckArea and the lines (0x81's end)
# per call zero or not (fail_mask), 0x539760 1 / 9 / other, 0x535670 1 / other, 0x52b400 -1 / other (answers); the path nudge and the
# lines' ends written through their out arguments (fill) or not; the entrance ends (0x52a050, 0x52a1a0) and Cell_FirstObject's index answer
# fixture words.
# usage: mapclick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import (LOG, LOG_BYTES, RAM, Bench, Call, _recorder, answers, fail_mask, fill, log_region,  # noqa: E402
                   set_return, strings)

WORLD = 0x31000000
RECORDS = WORLD + 0x294             # the 0x44-byte records (id 0 first)
CELLS = WORLD + 0x6cde * 0x28       # the 0x28-byte cells (id 0 first)
IDS = 0x92                          # ids 0..IDS-1 mapped (row 1's column 7 is 0x88)
THING, THING2 = RAM + 0x1000, RAM + 0x1100
TABLES = RAM + 0x2000               # answers(): 0x539760's, 0x535670's, 0x52b400's, MapCell_Word24's (0xa0 bytes each)
WORDS = RAM + 0x3000                # 0x52a050's, 0x52a1a0's and Cell_FirstObject's words

CDECL = {0x5234d0: (30, 3), 0x446050: (12, 0), 0x446300: (15, 1), 0x435570: (16, 1), 0x4419b0: (13, 4), 0x445a70: (23, 2), 0x527ee0: (31, 3),
         0x528a70: (17, 6), 0x52a970: (5, 1), 0x52b400: (8, 3), 0x52bdd0: (14, 4), 0x52c6f0: (6, 4), 0x52ccb0: (15, 4), 0x52f200: (21, 2),
         0x52f580: (17, 2), 0x52fc80: (17, 1), 0x530120: (18, 1), 0x5327e0: (16, 4), 0x536100: (0, 5), 0x545450: (19, 1),
         0x51bfc0: (10, 6), 0x53aac0: (1, 5), 0x523230: (26, 2), 0x523070: (2, 2), 0x5276d0: (10, 1), 0x52a050: (3, 3), 0x52a1a0: (4, 4),
         0x52a3a0: (11, 3), 0x52a420: (12, 3), 0x52a490: (27, 2), 0x52a700: (28, 2), 0x530830: (21, 1), 0x530e80: (13, 0), 0x531a50: (7, 5),
         0x532960: (14, 0), 0x4a2aa0: (11, 1), 0x52f1b0: (29, 2)}
THISCALL = {0x4de1f0: (26, 0), 0x4df390: (27, 0), 0x4df290: (16, 0), 0x4d0b10: (28, 0), 0x4d99e0: (20, 1), 0x506e50: (30, 2), 0x50c3e0: (22, 2),
            0x522700: (24, 1), 0x5227e0: (25, 1), 0x53ad30: (12, 0), 0x53ad90: (13, 0), 0x53adb0: (14, 0), 0x53aec0: (15, 0), 0x539760: (18, 3),
            0x535670: (19, 2), 0x403c20: (31, 3), 0x403c80: (31, 4), 0x5195d0: (29, 0), 0x4d01f0: (30, 1)}


def pops(count, before=None):
    """A stand-in that pops `count` bytes of stack arguments (after `before`, if any)."""
    def hook(e):
        if before:
            before(e)
        if e.tail:
            ret = e.u32(e.r['ESP'])
            e.r['ESP'] += count
            e.write(e.r['ESP'], struct.pack('<I', ret))
        else:
            e.r['ESP'] += count
    return hook


_advise_log = _recorder(9, 1, cdecl=True)


def _advise(e):   # 0x59b590 (thiscall: the id and 0x409cd0's 20-byte message by value): logs the id, answers this
    ecx = e.r['ECX']
    _advise_log(e)
    e.r['EAX'] = ecx


HOOKS = {a: _recorder(k, n, cdecl=True) for a, (k, n) in CDECL.items()}
HOOKS.update({a: _recorder(k, n) for a, (k, n) in THISCALL.items()})
HOOKS.update({0x409cd0: pops(4), 0x59b590: pops(0x18, _advise), 0x5194d0: lambda e: e.r.__setitem__('EAX', 0), 0x59a940: pops(4)})
B = Bench([(0x524960, 0x52766a), (0x52f860, 0x52f890), (0x52f840, 0x52f860), (0x52fab0, 0x52fae0), (0x52faf0, 0x52fb20), (0x52f1f0, 0x52f210),
           (0x52fbc0, 0x52fbd0), (0x52fbe0, 0x52fbf0), (0x52fbd0, 0x52fbe0), (0x52fbf0, 0x52fc40), (0x424ec0, 0x424ee0), (0x481590, 0x4815b0),
           (0x48c920, 0x48c926), (0x4d8c00, 0x4d8c30), (0x5da3c0, 0x5da3c1)],
          hooks=HOOKS)

MODES = [0, 1, 3, 4, 0xb, 0xe, 0x10, 0x13, 0x14, 0x15, 0x16, 0x19, 0x1a, 0x32, 0x33, 0x34, 0x35, 0x36, 0x38, 0x39, 0x3a, 0x3b,
         1, 3, 4, 4, 0xb, 0xe, 0x10, 0x13, 0x1a, 0x1a, 0x3b, 0x3b]   # (the longer ones twice or more)


def mask(r):
    return r.choice([0, 0xffffffff, r.getrandbits(32), r.getrandbits(32)])


def table(r, values):
    return b''.join(struct.pack('<I', r.choice(values) & 0xffffffff) for _ in range(40))


def case(r):
    mode = r.deal(MODES) if r.random() < 0.95 else r.choice([2, 0x17, 0x37, r.getrandbits(8)])
    cx, cy = lambda: r.randrange(0, 8), lambda: r.randrange(0, 2)
    small = lambda: r.choice([cx(), cx(), r.randrange(-1, 9)])   # noqa: E731
    cid = lambda: (cy() << 7) + cx() + 1   # noqa: E731

    log = log_region()
    for k in (0, 1, 5, 6, 7, 12, 13, 14, 16, 22):   # the zero-or-not answers (thiscall and cdecl share a slot's: see driver.cpp)
        if r.random() < 0.7:
            fail_mask(log, k, mask(r))
    answers(log, 18, TABLES)
    answers(log, 19, TABLES + 0xa0)
    answers(log, 8, TABLES + 0x140)
    answers(log, 15, TABLES + 0x1e0)
    set_return(log, 3, WORDS)
    set_return(log, 4, WORDS + 4)
    set_return(log, 20, WORDS + 8)
    strings(log, 0, 0, out=0x18)    # the lines' end pointers (stack or globals)
    strings(log, 1, 0, out=0x18)
    strings(log, 2, 0, out=3)       # the path nudge's
    strings(log, 20, 0, out=1)      # Cell_FirstObject's out word
    for k, bits in ((22, 1), (24, 1), (25, 1), (7, 0x10)):   # the direction bits and CheckArea's charge: BYTE arguments
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, bits)
    if r.random() < 0.5:
        fill(log, 2, 0, struct.pack('<4i', *[small() for _ in range(4)]), which=0, records=4)
        fill(log, 2, 1, struct.pack('<4i', *[cy() for _ in range(4)]), which=1, records=4)
    if r.random() < 0.3:
        fill(log, 1, 3, struct.pack('<4i', *[small() for _ in range(4)]), which=2, records=4)
    if r.random() < 0.3:
        fill(log, 0, 4, struct.pack('<4i', *[cy() for _ in range(4)]), which=3, records=4)

    records = bytearray(r.bytes(IDS * 0x44))
    cells = bytearray(r.bytes(IDS * 0x28))
    for i in range(IDS):
        struct.pack_into('<I', records, i * 0x44 + 8, r.choice([0, 1, 1, 4, r.getrandbits(32)]))   # (+0x29c)
        struct.pack_into('<H', cells, i * 0x28, cid())
        struct.pack_into('<H', cells, i * 0x28 + 0x10, cid())

    thing = bytearray(r.bytes(0x40))
    thing[5], thing[7] = cx(), cy()
    struct.pack_into('<I', thing, 0x10, r.choice([0, 0x5a, 0x5a, 0xb4, 0x10e, r.getrandbits(32)]))
    thing2 = bytearray(thing)
    thing2[5], thing2[7] = cx(), cy()
    things = bytearray(r.bytes(0x50))
    for i in range(4):
        struct.pack_into('<I', things, i * 0x14, r.choice([THING, THING2]))

    near = bytearray(r.bytes(0x60))   # 0x818680..0x8186e0
    struct.pack_into('<H', near, 4, cid() if r.random() < 0.9 else 0)          # 0x818684
    struct.pack_into('<H', near, 0x14, cid() if r.random() < 0.9 else 0)       # 0x818694
    struct.pack_into('<H', near, 0x18, cid())                                  # 0x818698
    struct.pack_into('<I', near, 0x2c, THING if r.random() < 0.97 else 0)      # 0x8186ac
    struct.pack_into('<I', near, 0x30, r.choice([0, r.getrandbits(32)]))       # 0x8186b0
    struct.pack_into('<ii', near, 0x34, cy(), cx())                            # 0x8186b4 / b8
    struct.pack_into('<I', near, 0x40, r.choice([0, 0, r.getrandbits(32)]))    # 0x8186c0
    near[0x58] = r.choice([0, 1])                                              # 0x8186d8
    struct.pack_into('<I', near, 0x5c, r.choice([0, 1, 2, 2, r.getrandbits(32)]))   # 0x8186dc

    focus = bytearray(r.bytes(0x58))   # 0x7639a0..0x7639f8
    struct.pack_into('<ii', focus, 4, cx(), cy())                              # 0x7639a4 / a8
    struct.pack_into('<I', focus, 0xc, r.choice([0, 0x5a, 0xb4, 0x10e, r.getrandbits(32)]))
    struct.pack_into('<H', focus, 0x10, r.choice([0xffff, r.getrandbits(16)]))   # 0x7639b0
    struct.pack_into('<ii', focus, 0x18, cx(), cy())                           # 0x7639b8 / bc
    struct.pack_into('<iiii', focus, 0x40, cx(), cy(), cx(), cy())             # 0x7639e0..ec
    struct.pack_into('<ii', focus, 0x50, cx(), cy())                           # 0x7639f0 / f4

    start = (-1, -1) if r.random() < 0.4 else (cx(), cy())
    x, y = small(), cy()
    if r.random() < 0.1:
        x, y = start if start[0] >= 0 else (x, y)
    flags = r.choice([0, 0xc, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16) << 16])
    use = r.choice([0, 0, 1, r.getrandbits(32)])

    def word(v):
        return struct.pack('<I', v & 0xffffffff)
    regions = {
        LOG: log, RECORDS: records, CELLS: cells, THING: thing, THING2: thing2,
        TABLES: table(r, [1, 9, 9, 0, r.getrandbits(32)]) + table(r, [1, 1, 0, 3]) + table(r, [-1, -1, 0, 1]) + table(r, [r.getrandbits(16) for _ in range(4)] + [0, 1, 0xffff, 0x8000]),
        WORDS: struct.pack('<HHHH', cid(), cid(), r.randrange(4), 0),
        0x7cf83c: word(WORLD), 0x7cfb90: things, 0x81ae2c: word(mode),
        0x785a2c: word(r.choice([0, 0, 1])), 0x81b4cc: bytes([r.choice([0, 0, 0, 1]), 0, 0, 0]),
        0x8187f8: word(r.choice([0, 0, 0, 1])), 0x816d48: bytearray(r.bytes(0x1c)),
        0x818680: near, 0x818c10: bytearray(r.bytes(0x48)), 0x7639a0: focus, 0x763ac0: bytearray(r.bytes(0x48)),
        0x81ede4: struct.pack('<ii', *start), 0x81ecac: word(r.choice([0, 1, 2, 1, 2, 3])), 0x81b0cc: word(r.choice([0, 1, 1, 2])),
        0x81d740: word(r.randrange(2, 10)), 0x81b1b0: word(r.randrange(3, 6)), 0x81d7a4: r.bytes(4), 0x81af4c: r.bytes(4),
        0x81b604: r.bytes(4), 0x820a8c: word(r.choice([0, 1, 1, 2])), 0x81b740: bytearray(r.bytes(0x100)), 0x7c2650: r.bytes(4),
        0x803a2c: r.bytes(4),
    }
    d48 = regions[0x816d48]
    struct.pack_into('<I', d48, 0, r.choice([0, 0, 0, 1]))            # 0x816d48
    struct.pack_into('<I', d48, 0x18, r.choice([0, 1]))               # 0x816d60
    regions[0x818c10][0x44] = r.choice([0, 1])                         # 0x818c54
    return Call(0x524960, [x & 0xffffffff, y & 0xffffffff, flags, use], regions)


if __name__ == '__main__':
    B.main(case)
