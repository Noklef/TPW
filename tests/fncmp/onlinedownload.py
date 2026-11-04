# The online session's download start (src/game/online/session: 0x5b8910) against the original: the old client dropped, the queue of missing files rebuilt from the item's bits, the last file's name, then the FTP client made or the end reached. The strings real;
# the lookups, the file's existence, _wsplitpath, the online folder (its recorder writes a string object through the out pointer) and the client's two calls recorders; the conversions' imports recorders answering a text of their own, as in onlinenext.py.
# usage: onlinedownload.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, global_heap, heap, log_fold, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

FTPDELETE, SAY, W2M, M2W, ASSIGN, FINDPAIR, SPLIT, SLOTS, FOLDER, EXISTS, FTPCTOR, CONNECT, END, CITY, KIND, PICK, START, CALLBACK = range(18)
B = Bench([(0x5b8910, 0x5b901b), (0x5b9020, 0x5b9040), (0x424db0, 0x424e6e), (0x5c5480, 0x5c5510), (0x5c5520, 0x5c5530), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5bdfd0: _recorder(ASSIGN, 1), 0x5bd9d0: _recorder(FINDPAIR, 2), 0x67dda0: _recorder(SPLIT, 5, cdecl=True), 0x5c7420: _recorder(SLOTS, 0), 0x5c8980: _recorder(FOLDER, 1, com=True),
                 0x5f5cd0: _recorder(EXISTS, 1, cdecl=True), 0x5d1760: _recorder(FTPCTOR, 2), 0x5d1ac0: _recorder(CONNECT, 4), 0x5b90e0: _recorder(END, 1), 0x5b72f0: _recorder(CITY, 0),
                 0x5bd670: _recorder(KIND, 0), 0x4bae30: _recorder(PICK, 0, cdecl=True)})
SESSION, OWNER, ITEM, KIND_ITEM, TABLE, FTP, VT, NODES, TEXTS = RAM, RAM + 0x800, RAM + 0x900, RAM + 0xa00, RAM + 0xb00, RAM + 0xc00, RAM + 0xd00, RAM + 0xe00, RAM + 0x1000
QUEUED = RAM + 0x1800
WORD_TEXT, ITEM_TEXT, KIND_TEXT, PASS_TEXT, NARROW = TEXTS, TEXTS + 0x40, TEXTS + 0x80, TEXTS + 0xc0, TEXTS + 0x100
GLOBALS = {0xf816c0: (0, b'C:\\tp\\'), 0xf7ccd8: (0, b'in'), 0xf80be0: (0, b'\\'), 0xf7d1e8: (0, b'.dat')}


def wide(text):
    return text.encode('utf-16-le')


def case(r):
    log = log_region()
    log_fold(log)
    table = bytearray(0x600)
    put(table, 0x6fd1e0 - IMPORT_TABLE, 'I', recorder(SAY, 1, True))
    strings(log, SAY, 1)
    put(table, 0x6fd1dc - IMPORT_TABLE, 'I', recorder(W2M, 8, True))
    put(table, 0x6fd174 - IMPORT_TABLE, 'I', recorder(M2W, 6, True))
    put(table, 0x6fd52c - IMPORT_TABLE, 'I', recorder(START, 3))
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    strings(log, W2M, 0, 0b10000)
    blob(log, W2M, 2, 0xf)
    strings(log, M2W, 4, 0b10000)
    set_return(log, W2M, 5)
    fill(log, W2M, 4, b'cona\0conb\0conc\0cond\0', records=4)
    back = wide('back\0')
    set_return(log, M2W, len(back) // 2)
    fill(log, M2W, 4, back, 1)
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x8000, r.choice([0] * 19 + [3]))}
    for address, (kind, text) in GLOBALS.items():
        gamestring.string(r, kind, address, QUEUED + 0x400 + (address & 0x3f0), regions, text)
    gamestring.string(r, 1, 0xf7d740, QUEUED + 0x500, regions, wide('on'))
    session = bytearray(r.bytes(0x400))
    gamestring.string(r, 0, SESSION + 0x3dc, QUEUED + 0x540, regions, b'old')
    session[0x3dc:0x3f0] = regions.pop(SESSION + 0x3dc)[:0x14]
    argument = 0 if r.random() < 0.3 else RAM + 0x7c0
    # The early end: bit 2 or bit 3 of the first flag byte, by the owner's +0x78.
    owner = bytearray(0x100)
    put(owner, 0x78, 'I', r.choice([0, 1]))
    session[0x58] = r.choice([0, 0, 0, 4, 8, 0xc])
    put(owner, 8, 'I', r.getrandbits(32))
    put(owner, 0x4c, 'I', r.getrandbits(32))
    put(owner, 0x84, 'I', NARROW)
    bits = r.choice([0, 1, 2, 3])
    for _ in range(bits):
        spot = r.randrange(0x100)
        owner[0x58 + spot // 8] |= 1 << (spot % 8)
    regions[OWNER] = owner
    put(session, 0x3a4, 'I', OWNER)
    # The client before and the listener before.
    put(session, 0x3a0, 'I', r.choice([0, FTP]))
    ftp = bytearray(8)
    put(ftp, 0, 'I', VT)
    regions[FTP] = ftp
    regions[VT] = vtable({0: 1})
    put(session, 0x39c, 'I', r.choice([0, 0x40000]))
    # The queue before: real strings in real nodes.
    count = r.choice([0, 0, 1, 2])
    nodes = bytearray(0x40)
    for i in range(count):
        at = 0x10 * i
        gamestring.string(r, 1, QUEUED + 0x40 * i, QUEUED + 0x200 + 0x40 * i, regions, wide('q%d' % i))
        put(nodes, at, 'I', QUEUED + 0x40 * i if r.random() < 0.9 else 0)
        put(nodes, at + 4, 'I', NODES + 0x10 * (i + 1) if i + 1 < count else 0)
        put(nodes, at + 8, 'I', NODES + 0x10 * (i - 1) if i else 0)
    regions[NODES] = nodes
    put(session, 0x3cc, 'I', NODES if count else 0)
    put(session, 0x3d0, 'I', NODES + 0x10 * (count - 1) if count else 0)
    put(session, 0x3d4, 'I', count)
    # The lookups.
    set_return(log, FINDPAIR, ITEM)
    item = bytearray(0x100)
    put(item, 0x54, 'I', ITEM_TEXT)
    regions[ITEM] = item
    set_return(log, KIND, KIND_ITEM)
    kind_item = bytearray(0x40)
    put(kind_item, 0x2c, 'I', KIND_TEXT)
    regions[KIND_ITEM] = kind_item
    put(session, 0x100, 'I', RAM + 0x7a0)
    put(session, 0xfc, 'I', RAM + 0x7b0)
    put(session, 0x44, 'I', PASS_TEXT)
    text = bytearray(0x180)
    text[0:0x40] = wide('word').ljust(0x40, b'\0')
    text[0x40:0x80] = wide(r.choice(['c:\\x\\file.dat', 'f'])).ljust(0x40, b'\0')
    text[0x80:0xc0] = wide('host').ljust(0x40, b'\0')
    text[0xc0:0x100] = wide('pass').ljust(0x40, b'\0')
    text[0x100:0x140] = b'name'.ljust(0x40, b'\0')
    regions[TEXTS] = text
    # _wsplitpath's outs on the stack, written per call; the folder's string object written through its out pointer.
    strings(log, SPLIT, 0, out=0b11110)
    fill(log, SPLIT, 2, wide('dir\\\0'.ljust(8, '\0')), which=2)
    fill(log, SPLIT, 3, wide('nam\0'.ljust(8, '\0')), which=3)
    fill(log, SPLIT, 4, wide('.x\0'.ljust(8, '\0')), which=4)
    strings(log, FOLDER, 0, out=1)
    folder = bytearray(0x40)
    folder[:6] = wide('fol')
    regions[QUEUED + 0x600] = folder
    fill(log, FOLDER, 0, struct.pack('<5I', 0x7030c8, 3, QUEUED + 0x600, 0x20, 8), which=5)
    # What exists: per call.
    exists = bytearray(0x100)
    struct.pack_into('<40I', exists, 0, *[r.choice([0, 0, 1]) for _ in range(40)])
    regions[TABLE] = exists
    answers(log, EXISTS, TABLE)
    strings(log, EXISTS, 1)
    strings(log, CONNECT, 0b111)
    set_return(log, FTPCTOR, r.choice([0x50000, 0x50000, 0]))
    set_return(log, CITY, r.choice([0, 1]))
    set_return(log, PICK, r.choice([0, 0x7cc4f0]))
    put(session, 0x3c0, 'I', r.choice([0, recorder(CALLBACK, 2, cdecl=True)]))
    put(session, 0x3c4, 'I', r.getrandbits(32))
    put(session, 0x34, 'I', r.getrandbits(32))
    put(session, 0xd4, 'I', r.getrandbits(32))
    regions[SESSION] = session
    regions[0xf81ebc] = struct.pack('<I', r.choice([0, 0, 0x60000]))
    regions[RAM + 0x7c0] = r.bytes(0x10)
    return Call(0x5b8910, [SESSION, argument], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
