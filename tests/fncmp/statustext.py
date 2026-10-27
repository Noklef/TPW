# An object's status shown on a window (src/game/world/objectstatus: 0x4861e0) against the original. language.py's texts() surroundings
# (the converter [0x78ba7c] on or off, the record table [0x78ba94] of 0-4 records, some not text records or unterminated) with the
# status's text id from the table 0x74fbc0 at one of them mostly (the table's base moved back by it, the count about it); the status 0-27
# mostly (the colour table 0x74fb50's), 28-31 now and then (the text ids' table read as colours, as the original does). The window
# a block whose slot 9 is a recorder (its answer random now and then: EAX compared); its objects +0x98 / +0x9c recorders or none (their
# slot 3 told the event 0x40000, the colour's bytes in the second event's +0x10-+0x13 hashed).
# usage: statustext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, put, set_return, vtable  # noqa: E402
from language import text  # noqa: E402

B = Bench([(0x4861e0, 0x48627d), (0x65c5d5, 0x65c672), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0)])
WIN, WVT, OBJ, OVT = RAM + 0x2000, RAM + 0x2200, RAM + 0x2300, RAM + 0x2400
# the text ids at 0x74fbc0 (statuses 0-31; past 27 the exe's next data), as the exe holds them
IDS = [0, 0x16d, 0x16e, 0x16f, 0x170, 0x174, 0x172, 0x173, 0x171, 0x175, 0x176, 0x17a, 0x178, 0x179, 0x177, 0x17b, 0x17c, 0x180, 0x17e, 0x17f,
       0x17d, 0x181, 0x184, 0x185, 0x186, 0x182, 0x183, 1, 0x74fde0, 0x74fde0, 0xffffffff, 0x10001]


def case(r):
    log = log_region()
    lang = bytearray(r.bytes(0x600))
    conv = bytearray(r.bytes(0x10))
    put(conv, 8, 'BBHI', r.choice([0x80, 0x81, 0xa0, 0, 0xff, r.getrandbits(8)]), r.getrandbits(8), r.choice([0, 3, 0x80, 0x400, 0x7ff, r.randrange(0x800)]),
        RAM + 0x4000)
    put(lang, 0x54c, 'II', RAM + 0x800, r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    regions = {0x78b530: lang, RAM + 0x800: conv, RAM + 0x4000: r.bytes(0x1000), 0x7ba058: bytes([0x55]) * 0x404, LOG: log}
    status = r.choice(list(range(28)) * 3 + [28, 29, 30, 31])
    ident = IDS[status]
    # the records: the status's id at one of them mostly (the table's base moved back by it, the count about it), else 0-4 records
    table = bytearray(r.bytes(0x20))
    if r.random() < 0.8:
        put(table, 8, 'II', (ident + r.choice([1, 1, 1, 2, 0, -1])) & 0xffffffff, (RAM + 0xb00 + 4 * r.randrange(0, 5) - 4 * ident) & 0xffffffff)
    else:
        put(table, 8, 'II', r.randrange(0, 5), RAM + 0xb00)
    regions[RAM + 0xa00] = table
    put(lang, 0x564, 'I', RAM + 0xa00)
    pool = bytearray()
    for _ in range(5):
        body = text(r) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    regions[RAM + 0xc00] = bytes(pool)
    regions[RAM + 0xb00] = struct.pack('<5I', *[RAM + 0xc00 + 0x40 * i for i in range(5)])
    # the window: slot 9 a recorder; +0x98 / +0x9c objects whose slot 3 is told the colour
    win = bytearray(r.bytes(0x120))
    put(win, 0, 'I', WVT)
    put(win, 0x98, 'II', r.choice([OBJ, 0]), r.choice([OBJ, 0, 0]))
    regions[WIN] = win
    regions[WVT] = vtable({9: 1})
    regions[OBJ] = struct.pack('<I', OVT) + r.bytes(0x1c)
    regions[OVT] = vtable({3: 2})
    blob_bytes(log, 3, 1, 0, (0x10, 0xf, 0x10))
    if r.random() < 0.5:
        set_return(log, 9, r.getrandbits(32))
    return Call(0x4861e0, [WIN, status], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
