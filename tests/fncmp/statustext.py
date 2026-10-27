# An object's status shown on a window (src/game/world/objectstatus: 0x4861e0) against the original. language.py's texts() surroundings
# (the converter [0x78ba7c] on or off, the record table [0x78ba94] of 0-4 records, some not text records or unterminated) with the
# status's text id from the table 0x74fbc0 at one of them mostly (the table's base moved back by it, the count about it); the status 0-27
# mostly (the colour table 0x74fb50's), 28-31 now and then (the text ids' table read as colours, as the original does). The window
# a block whose slot 9 is a recorder (its answer random now and then: EAX compared); its objects +0x98 / +0x9c recorders or none (their
# slot 3 told the event 0x40000, the colour's bytes in the second event's +0x10-+0x13 hashed).
# ObjectStatus_ShowOnPanel (0x48d160): a panel whose status +8 is one of those (0 hides the window) with the window +0x1c that window or
# none, the shown thing [0x7c2658] set or 0; Window_SetShown's (0x65d9dd) surroundings: the window shown or not (flag 0 of +0x48), no
# parent or children, its handler +0x114 a logger or none, the objects' slot 5 recorders (told it), the hover's root [0xfaa5d8] none (the
# hover not updated), the manager [0x77c48c]'s slot 13 a recorder.
# usage: statustext.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Show,Panel)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import CDECL_LOG4, LOG, RAM, Bench, Call, blob_bytes, log_region, put, set_return, vtable  # noqa: E402
from language import text  # noqa: E402

B = Bench([(0x4861e0, 0x48627d), (0x65c5d5, 0x65c672), (0x6adc80, 0x6add25), (0x6ace60, 0x6acea0), (0x48d160, 0x48d197), (0x65d9dd, 0x65da8d),
           (0x65d90b, 0x65d9dd), (0x65d773, 0x65d7bc), (0x6589f9, 0x658aab)])
WIN, WVT, OBJ, OVT, PANEL, MANAGER = RAM + 0x2000, RAM + 0x2200, RAM + 0x2300, RAM + 0x2400, RAM + 0x2500, RAM + 0x2600
# the text ids at 0x74fbc0 (statuses 0-31; past 27 the exe's next data), as the exe holds them
IDS = [0, 0x16d, 0x16e, 0x16f, 0x170, 0x174, 0x172, 0x173, 0x171, 0x175, 0x176, 0x17a, 0x178, 0x179, 0x177, 0x17b, 0x17c, 0x180, 0x17e, 0x17f,
       0x17d, 0x181, 0x184, 0x185, 0x186, 0x182, 0x183, 1, 0x74fde0, 0x74fde0, 0xffffffff, 0x10001]


def case(r):
    op = r.deal(OPS)
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
    if op == 'Show':
        return Call(0x4861e0, [WIN, status], regions, ret='a')
    status = r.choice([status, status, 0])   # (0 hides the window; its colour and text then status 0's)
    if status == 0 and r.random() < 0.8:
        put(table, 8, 'II', 1, (RAM + 0xb00 + 4 * r.randrange(0, 5)) & 0xffffffff)
    put(win, 0x48, 'I', r.getrandbits(32))
    put(win, 0x5c, 'II', 0, 0)
    put(win, 0x114, 'I', r.choice([CDECL_LOG4, 0]))
    regions[OVT] = vtable({3: 2, 5: 1})
    panel = bytearray(r.bytes(0x20))
    put(panel, 8, 'I', status)
    put(panel, 0x1c, 'I', r.choice([WIN, WIN, WIN, 0]))
    regions[PANEL] = panel
    regions[0x7c2658] = struct.pack('<I', r.choice([0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]))
    regions[0xfaa5d8] = bytes(4)
    regions[0x77c48c] = struct.pack('<I', MANAGER)
    regions[MANAGER] = struct.pack('<I', MANAGER + 0x40) + bytes(0x3c) + vtable({13: 1})
    if r.random() < 0.5:
        set_return(log, 13, r.getrandbits(32))
    return Call(0x48d160, [PANEL], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Show,Panel').split(',') if k]


if __name__ == '__main__':
    B.main(case)
