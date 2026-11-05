# A user slot chosen and closed (src/game/files/userslots: 0x5c83b0, 0x5c8650) against the originals. The strings real; the folders, the mailbox's and address book's load and save, the global save's calls, the mode setting, the atexit and the options' save recorders.
# The folders' recorders write a string object through the out pointer; the conversions' imports recorders answering a text of their own, as in userslotsscan.py.
# usage: userslotschoice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Choose,Close)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, blob, echo, fill, global_heap, heap, log_fold, log_region, put, recorder, set_return, strings  # noqa: E402

SAVE_FOLDER, OPEN, BOOK_LOAD, BOOK_SAVE, OUTBOX, WRITE, INSTANCE, THEME, READ, SET, SLOT, CTOR, DTOR, LOAD, BYTE, ATEXIT, W2M, M2W, OPTIONS = range(19)   # (those of UserSlotsScanEnv as in userslotsscan.py)
B = Bench([(0x5c83b0, 0x5c8650), (0x5c8650, 0x5c8870), (0x424db0, 0x424e6e), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5c7de0: _recorder(SLOT, 2), 0x5c7bd0: _recorder(SAVE_FOLDER, 1, com=True), 0x5cb230: _recorder(OPEN, 1), 0x5cd9c0: _recorder(BOOK_LOAD, 1), 0x5cd710: _recorder(BOOK_SAVE, 1), 0x5cb720: _recorder(OUTBOX, 0),
                 0x5aef50: _recorder(CTOR, 0), 0x5af250: _recorder(DTOR, 0), 0x5afd70: _recorder(LOAD, 1), 0x5afc40: _recorder(BYTE, 0), 0x5afc60: _recorder(WRITE, 1), 0x409410: _recorder(INSTANCE, 0, cdecl=True), 0x5b09c0: _recorder(THEME, 1),
                 0x550ca0: _recorder(READ, 0), 0x550d80: _recorder(SET, 1), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True), 0x424820: _recorder(OPTIONS, 1)})
SLOTS, NODES, ITEMS, TEXTS, OLD = RAM, RAM + 0x200, RAM + 0x240, RAM + 0x1000, RAM + 0x2000
NEW = 0x60000


def wide(text):
    return text.encode('utf-16-le')


def base(r):
    log = log_region()
    log_fold(log)
    imports = bytearray(0x600)
    put(imports, 0x6fd1dc - IMPORT_TABLE, 'I', recorder(W2M, 8, True))
    put(imports, 0x6fd174 - IMPORT_TABLE, 'I', recorder(M2W, 6, True))
    for at, stub in global_heap().items():
        imports[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    strings(log, W2M, 0, 0b10000)
    blob(log, W2M, 2, 0xf)
    strings(log, M2W, 4, 0b10000)
    set_return(log, W2M, 5)
    fill(log, W2M, 4, b'cona\0conb\0conc\0cond\0', records=4)
    back = wide('back\0')
    set_return(log, M2W, len(back) // 2)
    fill(log, M2W, 4, back, 1)
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x8000, r.choice([0] * 37 + [4, 5, 6]))}
    regions[SLOTS] = r.bytes(0x180)
    gamestring.string(r, 1, 0xf82508, TEXTS + 0x700, regions, wide(r.choice(['\\', '/'])))
    gamestring.string(r, 1, 0xf818a8, TEXTS + 0x740, regions, wide(r.choice(['save.dat', 'g'])))
    gamestring.string(r, 1, 0xf82028, TEXTS + 0x780, regions, wide(r.choice(['options', 'o'])))
    # The folders: a wide string object written through the out pointer.
    for which, (k, address, text) in enumerate(((SLOT, TEXTS + 0x100, 'slot'), (SAVE_FOLDER, TEXTS + 0x140, 'save'))):
        regions[address] = wide(text).ljust(0x40, b'\0')
        fill(log, k, 0, struct.pack('<5I', 0x7030c8, len(text), address, 0x20, 8), which=which + 2)
        strings(log, k, 0, 1)
    echo(log, SAVE_FOLDER, 0)
    # The global save: none, or one to destroy; made as it is asked for.
    regions[OLD] = bytes(0x50)
    regions[0xf7c58c] = struct.pack('<I', r.choice([0, OLD]))
    set_return(log, CTOR, NEW)
    set_return(log, BYTE, r.choice([0, 1, 0x100]))
    set_return(log, INSTANCE, r.getrandbits(32))
    for k in (OPEN, BOOK_LOAD, BOOK_SAVE, LOAD, WRITE):
        strings(log, k, 1)
    blob(log, OPTIONS, 0, 0xf)
    return log, regions


def choose(r):
    log, regions = base(r)
    # The mode setting and its guard; the theme list.
    regions[0xfb2764] = bytes([r.choice([0, 1, 2, 3, 0x41, 0x40])])
    regions[0xfb3b7c] = struct.pack('<I', r.choice([0, 1, 1, 2, r.getrandbits(32)]))
    count = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x20))
    for i in range(count):
        put(nodes, 8 * i, '2I', ITEMS + 0x10 * i, NODES + 8 * (i + 1) if i + 1 < count else 0)
    regions[NODES] = nodes
    regions[0x786b90] = struct.pack('<I', NODES if count else 0)
    return Call(0x5c83b0, [SLOTS, r.choice([0, 1, 2, 3, 0xffffffff])], regions, conv='t', ret='v')


def close(r):
    log, regions = base(r)
    return Call(0x5c8650, [SLOTS], regions, conv='t', ret='v')


OPS = {'Choose': choose, 'Close': close}


def case(r):
    return OPS[r.choice(os.environ.get('OPS', 'Choose,Choose,Close').split(','))](r)


if __name__ == '__main__':
    B.main(case)
