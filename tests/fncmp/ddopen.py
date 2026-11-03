# The display's DirectDraw start-up (src/game/render/ddopen: 0x5633e0, 0x563460) against the originals, with the real view setters, error list, messages and debug log (its file calls recorders, as in enginedebug.py).
# The object's COM members are there from the start (the out arguments are not written, but a new or queried IDirectDraw is), each with its vtable of COM recorders; the calls succeed but for one call now and then (a code with a short
# message: the error list's overrun is engineerrors.py's). Enumeration fills in a count over random GUIDs; the windows' rectangles and the surfaces' masks are random.
# usage: ddopen.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Open,Enum)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import IMPORT_TABLE, LOG, RAM, Bench, Call, answers, blob, fill, import_table, log_fold, log_region, put, recorder  # noqa: E402
import enginedebug  # noqa: E402

B = Bench([(0x5633e0, 0x56344f), (0x563460, 0x564098), (0x5640a0, 0x564132), enginedebug.RANGE, enginedebug.FALLBACK, (0x56a770, 0x56ae60), (0x56af20, 0x56af9c), (0x582eb0, 0x582f64), (0x5830e0, 0x583216), (0x583360, 0x5833ba),
           (0x5911d0, 0x5911e5), (0x5911f0, 0x591209), (0x6793b0, 0x6793bc), (0x67a400, 0x690000)], hooks=enginedebug.HOOKS)
OPS = [k for k in os.environ.get('OPS', 'Open,Open,Open,Open,Enum').split(',') if k]
SELF, DRAW, DRAWVT, SURFACE, BACK, SURFACEVT, CLIPPER, CLIPPERVT, TABLES, GUID = RAM, RAM + 0x800, RAM + 0x810, RAM + 0x900, RAM + 0x910, RAM + 0x920, RAM + 0xa00, RAM + 0xa10, RAM + 0xb00, RAM + 0x4000
VIEW, RESULT, ERRORS = 0x8bcbc8, 0x879cd4, 0x879ad0
QI, RELEASE, BLT, ATTACHED, SURFDESC, SETCLIP, SETWND, MAKECLIP, MAKESURF, CAPS, MODE, RESTORE, COOP, SETMODE, IDENT = 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 16, 17, 18, 19
ENUM, CREATE, SHOW, UPDATE, CLIENT, FRAME, MOVE, PLACE = 20, 21, 22, 23, 24, 25, 26, 27
KEPT = [QI, ATTACHED, SETCLIP, SETWND, MAKECLIP, MAKESURF, CAPS, MODE, COOP, SETMODE, IDENT, CREATE]
IMPORTS = {'DirectDrawEnumerateA': (ENUM, 0x6fd014, 2, 0, 0), 'DirectDrawCreate': (CREATE, 0x6fd018, 3, 0, 0), 'ShowWindow': (SHOW, 0x6fd368, 2, 0, 0), 'UpdateWindow': (UPDATE, 0x6fd36c, 1, 0, 0), 'GetClientRect': (CLIENT, 0x6fd390, 2, 0, 2),
           'GetWindowRect': (FRAME, 0x6fd38c, 2, 0, 2), 'MoveWindow': (MOVE, 0x6fd31c, 6, 0, 0), 'SetWindowPos': (PLACE, 0x6fd380, 7, 0, 0)}


def vt(slots):
    table = bytearray(0x80 * 4)
    for slot, (k, n) in slots.items():
        struct.pack_into('<I', table, 4 * slot, recorder(k, n, True))
    return table


def masks(r):
    return struct.pack('<5I', r.deal([16, 32, r.getrandbits(32)]), *[r.choice([0, 0xf800, 0x7e0, 0x1f, 0xff0000, 0xff00, 0xff, 0xff000000, r.getrandbits(32)]) for _ in range(4)])


def open_case(r):
    log = log_region()
    log_fold(log)
    enginedebug.texts(log)
    if r.random() < 0.1:
        enginedebug.set_return(log, enginedebug.FOPEN, 0)
    table = import_table(log, IMPORTS)
    regions = {LOG: log, IMPORT_TABLE: table}
    this = bytearray(r.bytes(0x800))
    n = r.deal([0, 1, 2, 3, 4])
    put(this, 0, 'I', DRAW if r.random() < 0.3 else 0)
    put(this, 4, '6I', DRAW, DRAW, CLIPPER, SURFACE, BACK, r.choice([0, BACK]))
    put(this, 0x20, '16s', r.choice([b'ddraw.dll', b'nvdd32\0', b'\0']))
    put(this, 0x220, '16s', r.choice([b'some card', b'\0']))
    driver = r.choice([0, 1, 2, 3, r.randrange(0, 4)])
    put(this, 0x78c, 'I', driver if r.random() < 0.5 else r.randrange(0, 4))
    regions[SELF] = this
    fill(log, CREATE, 1, struct.pack('<I', DRAW))
    fill(log, QI, 2, struct.pack('<I', DRAW), which=6)
    fill(log, ENUM, 1, struct.pack('<I', n), which=1, offset=0x788)
    fill(log, CLIENT, 1, struct.pack('<4i', *[r.randrange(-50, 900) for _ in range(4)]), which=2)
    fill(log, FRAME, 1, struct.pack('<4i', *[r.randrange(-50, 900) for _ in range(4)]), which=3)
    fill(log, SURFDESC, 1, masks(r)[4:], which=4, offset=0x58)
    fill(log, MODE, 1, masks(r) + masks(r), which=5, offset=0x54, records=2)
    blob(log, MAKESURF, 1, 0x7fffffff)
    blob(log, MODE, 1, 0x7fffffff)
    blob(log, SURFDESC, 1, 0x7fffffff)
    blob(log, BLT, 5, 0x1ffffff)
    blob(log, ATTACHED, 1, 1)
    tables = bytearray(0xa0 * 32)
    if r.random() < 0.3:   # (one call fails)
        put(tables, 0xa0 * r.choice(KEPT) + 4 * r.choice([0, 0, 1]), 'I', r.choice([1, 0x80004005, 0x80070057]))
    for k in KEPT:
        answers(log, k, TABLES + 0xa0 * k)
    regions[TABLES] = tables
    flags = r.getrandbits(32) & ~0x3000010 | r.deal([0, 0x1000000]) | r.deal([0, 0x2000000]) | r.deal([0, 0x10])
    view = bytearray(r.bytes(0x970))
    put(view, 0x8bd508 - VIEW, 'I', flags)
    put(view, 0x8bd534 - VIEW, 'I', r.getrandbits(32) | 0x800000 if r.random() < 0.95 else r.getrandbits(32) & ~0x800000)
    draw = vt({0: (QI, 3), 2: (RELEASE, 1), 4: (MAKECLIP, 4), 6: (MAKESURF, 4), 0xb: (CAPS, 3), 0xc: (MODE, 2), 0x13: (RESTORE, 1), 0x14: (COOP, 3), 0x15: (SETMODE, 6), 0x1b: (IDENT, 3)})
    surface = vt({2: (RELEASE, 1), 5: (BLT, 6), 0xc: (ATTACHED, 3), 0x16: (SURFDESC, 2), 0x1c: (SETCLIP, 2)})
    clipper = vt({2: (RELEASE, 1), 8: (SETWND, 3)})
    # (the error list logs through its own env slot, the debug log through the file calls: with the text flag on, a full list, which takes nothing and logs nothing: engineerrors.py has that part)
    regions.update({VIEW: view, RESULT: r.bytes(4), ERRORS: bytearray(r.bytes(0x200)) + struct.pack('<I', 0x40 if flags & 0x1000000 else r.randrange(0, 0x41)), 0x879a2c: bytearray(struct.pack('<I', r.choice([0, 1]))), 0x879a28: bytearray(struct.pack('<I', r.choice([0, 1]))),
                    DRAW: struct.pack('<I', DRAWVT), DRAWVT: draw, SURFACE: struct.pack('<I', SURFACEVT), BACK: struct.pack('<I', SURFACEVT), SURFACEVT: surface, CLIPPER: struct.pack('<I', CLIPPERVT), CLIPPERVT: clipper})
    return Call(0x563460, [SELF, r.randrange(1, 2000), r.randrange(1, 2000), r.deal([8, 16, 24, 32]), driver, r.deal([1, 0, 1, 2]), r.deal([0, 1])], regions, conv='t', ret='a')


def enum_case(r):
    this = bytearray(r.bytes(0x800))
    put(this, 0x788, 'i', r.choice([0, 1, 2, 3, 4, 5, r.randrange(-3, 4)]) if r.random() < 0.9 else r.randrange(0, 4))
    if this[0x788] >= 0x80:
        put(this, 0x788, 'i', 4)
    return Call(0x5633e0, [GUID if r.random() < 0.7 else 0, r.getrandbits(32), r.getrandbits(32), SELF], {SELF: this, GUID: r.bytes(16)}, conv='s', ret='a')


def case(r):
    return open_case(r) if r.deal(OPS) == 'Open' else enum_case(r)


if __name__ == '__main__':
    B.main(case)
