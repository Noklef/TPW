# The dialog [0x7cc984] opened (src/game/ui/dialog7cc984open: 0x4c0ae0) against the original. The strings, Format (stringformat.py's stand-ins), the wide CRT and Window_FindChild real; every other callee a recorder as driver.cpp's
# kDialog7cc984OpenEnv; 0x6acd60 the log stub with its three number arguments hashed. The dialog a tree of its children and rows among dialog7cb398.py's decoys (their slot 9 a recorder). A row's text compared by its first 32 dwords in the heap
# (blob: a C string's hash would stop at the first unit's zero byte); the names' units without a zero byte, so strings() takes them whole.
# usage: dialog7cc984open.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, LOG_BYTES, RAM, Bench, Call, _cdecl_log4, _recorder, blob, fill, heap, log_fold, log_region, put, set_return, strings, text_args, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
from dialog7cb398 import KVTABLE, NODE, children, window  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from stringformat import _vsprintf, _wtoi  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops

ASSIGN, TOP, BUILD, DEPTH, OFFSET, HOLDER_CTOR, FIND, VALUE, THEME, TEXT, PART, IMPL, FONT, SET4C, NOTIFY, SHOWN, SAVE, BYTE25, CENSOR, N3000, NC00, PART2 = range(10, 32)
HOVER, FOCUS, PUSH = 8, 7, 6
C = lambda k, n: _recorder(k, n, cdecl=True)  # noqa: E731
B = Bench([(0x4c0ae0, 0x4c1940), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x6918f0, 0x691a30), (0x65d7bc, 0x65d832), (0x405280, 0x405390), (0x479230, 0x479270), (0x47ba30, 0x47ba60), (0x47bad0, 0x47bc10), (0x4f23f0, 0x4f2450),
           (0x5b6480, 0x5b6484)],
          hooks={0x67c1e0: _vsprintf(False), 0x67f4d0: _vsprintf(True), 0x67f4a0: _wtoi, 0x6acd60: _cdecl_log4,
                 0x5bdfd0: _recorder(ASSIGN, 1), 0x659409: C(TOP, 2), 0x65fd0e: _recorder(BUILD, 2), 0x65f16b: _recorder(DEPTH, 1), 0x65c7fe: _recorder(OFFSET, 2), 0x5b50b0: _recorder(HOLDER_CTOR, 0), 0x5bc9c0: _recorder(FIND, 1),
                 0x5bd270: _recorder(VALUE, 1), 0x4099b0: _recorder(THEME, 2), 0x485b00: C(TEXT, 1), 0x48f6a0: _recorder(PART, 1), 0x65d1c8: _recorder(IMPL, 1), 0x485a70: C(FONT, 1), 0x65c388: _recorder(SET4C, 1),
                 0x65f5b7: _recorder(NOTIFY, 1), 0x65d9dd: _recorder(SHOWN, 1), 0x5aef50: _recorder(SAVE, 0), 0x5b09b0: _recorder(BYTE25, 0), 0x5d3230: _recorder(CENSOR, 2), 0x65c4b9: _recorder(N3000, 2), 0x65c428: _recorder(NC00, 2),
                 0x491920: _recorder(PART2, 1), 0x492d20: C(HOVER, 1), 0x65e59b: _recorder(FOCUS, 1), 0x5d6110: _recorder(PUSH, 2)})
DIALOG, BOX, KIDS, ROWS, DECOYS = 0x33200000, 0x33210000, 0x33220000, 0x33230000, 0x33500000
WORDS, NAMES, THEME_TEXT, RECORD, KNOWN, FRAME, HOLDER, SAVE_INFO, SCENE, LABEL, OWN = (RAM + 0x20000 + 0x400 * k for k in range(11))


def text(r, n):
    return b''.join(struct.pack('<H', r.randrange(0x21, 0x7f) | r.randrange(1, 0x7f) << 8) for _ in range(n)) + b'\0\0'


def case(r):
    log = log_region()
    log_fold(log)   # (it makes more calls than the log's 40 entries)
    regions = {LOG: log, HEAP: heap(0x8000, r.deal([0] * 9 + [r.randrange(1, 16)])), LOCALE: bytes(4)}
    # the tree
    for at in (DIALOG, BOX):
        window(r, regions, at, DIALOG if at == BOX else 0, 0)
    kids = [(KIDS + i * NODE, ident) for i, ident in enumerate((0xe592f, 0xe5930, 0xe5931))]
    for at, _ in kids:
        window(r, regions, at, DIALOG, 0)
    put(regions[DIALOG], 0x5c, 'I', children(r, regions, kids + [(BOX, 0xe5932)], DIALOG, DECOYS, [0, 0x1234]))
    rows = [(ROWS + i * NODE, 0xe5933 + i) for i in range(8)]
    for at, _ in rows:
        window(r, regions, at, BOX, 0)
    put(regions[BOX], 0x5c, 'I', children(r, regions, rows, BOX, DECOYS + 0x4000, [0, 0x1234]))
    regions[KVTABLE] = vtable({9: 1})
    # the record's fields: the name of the session, a name, two more (empty now and then)
    own = text(r, r.deal([0, 3, 9]))
    name = own if r.random() < 0.3 else text(r, r.deal([0, 3, 40]))
    names = bytearray(0x400)
    fields = bytearray(r.bytes(0xd0))
    for i, (off, wide) in enumerate(((0x18, text(r, r.deal([0, 3, 40]))), (0x2c, text(r, r.deal([0, 3, 40]))), (0x40, name), (0xa0, text(r, r.deal([0, 0, 3, 20]))))):
        names[0x100 * i:0x100 * i + len(wide)] = wide
        put(fields, off, 'I', NAMES + 0x100 * i)
    put(fields, 0, 'I', 0 if r.random() < 0.97 else DIALOG)
    put(fields, 0xbc, 'I', r.deal([0x7fffffff, 0x7fffffff, 0, 1, r.getrandbits(32)]))
    put(fields, 0xc0, 'I', r.deal([0, 0, 1, r.getrandbits(32)]))
    regions[0x7cc984] = fields
    regions[NAMES] = names
    # the session and the save
    holder = bytearray(r.bytes(0x100))
    put(holder, 0x14, 'I', OWN)
    regions[OWN] = own
    regions[HOLDER] = holder
    regions[0xf7d88c] = struct.pack('<I', r.deal([HOLDER, HOLDER, HOLDER, 0]))
    regions[0xf7c58c] = struct.pack('<I', r.deal([SAVE_INFO, SAVE_INFO, SAVE_INFO, 0]))
    regions[SAVE_INFO] = r.bytes(0x10)
    regions[RECORD] = r.bytes(0x20)
    regions[KNOWN] = put(bytearray(r.bytes(0x80)), 0x60, 'I', r.deal([0, 0, 0, 1, r.getrandbits(32)]))
    regions[THEME_TEXT] = bytes(r.randrange(1, 0xff) for _ in range(r.deal([0, 4, 40]))) + b'\0'
    regions[WORDS] = text(r, r.deal([0, 1, 4, 30]))
    regions[LABEL] = text(r, r.deal([0, 3, 12]))
    regions[0xf82884] = struct.pack('<I', FRAME)
    regions[FRAME] = r.bytes(8)
    regions[0xf85614] = struct.pack('<I', SCENE)
    regions[SCENE] = r.bytes(0x20)
    regions[0x78ba94] = r.bytes(4)
    regions[0x7ba058] = bytes(0x40)
    regions[0x786b68] = r.bytes(4)
    # the recorders
    set_return(log, TOP, r.deal([0, 1, 100, r.getrandbits(16)]))
    set_return(log, BUILD, DIALOG)
    set_return(log, HOLDER_CTOR, HOLDER)
    set_return(log, SAVE, SAVE_INFO)
    set_return(log, FIND, KNOWN)
    set_return(log, VALUE, THEME_TEXT)
    set_return(log, TEXT, WORDS)
    set_return(log, BYTE25, r.deal([0, 0, 1, r.getrandbits(32) | 1]))
    strings(log, THEME, 0, 0b11)
    if r.deal([0, 1]):
        fill(log, THEME, 1, struct.pack('<I', LABEL), offset=8)
    strings(log, CENSOR, 0, 0b01)
    blob(log, CENSOR, 0, 0b1110)
    blob(log, NOTIFY, 0, 0xffffffff)
    strings(log, 9, 1)
    put(log, LOG_BYTES + 4 * SHOWN, 'I', 1)
    text_args(log, 3)
    args = [RECORD, r.deal([0, 0, 0x33600000]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)]
    return Call(0x4c0ae0, args, regions, ret='v')


if __name__ == '__main__':
    B.main(case)
