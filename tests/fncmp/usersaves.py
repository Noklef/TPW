# The saves in the chosen user's theme folder (src/game/files/usersaves: 0x5ac5d0-0x5ac8eb, 0x5ac8f0, 0x5accf0) against the originals. The strings and the heap real; the theme folder a recorder writing a
# wide string; WideCharToMultiByte / MultiByteToWideChar recorders (the wide path hashed; the narrow text and the names written); the local handle's calls recorders (`this` masked) answering found or
# ended by a table, the search's entries written by the recorders with a pattern's folder before the name (driver.cpp's saveSearchFirst / saveSearchNext, _search here, the same). Globals: every string varied.
# usage: usersaves.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob_bytes, echo, fill, global_heap, heap, import_table, log_region, mask_this, put, recorder, set_return, strings  # noqa: E402
import gamestring  # noqa: E402

CTOR, OPEN, FIRST, NEXT, DTOR, THEME, BEFORE, LOAD, SAVE = range(1, 10)
W2M, M2W = 40, 41
SINGLETON = 27   # (driver.cpp's kUserSlotsEnv)


def _search(k, n, entry_at):
    record = _recorder(k, n)

    def hook(e):
        entry = e.u32(e.r['ESP'] + (4 if e.tail else 0) + entry_at)
        e.write(entry + 0x24, b'dir\\')
        e.write(entry + 0x20, struct.pack('<I', entry + 0x28))
        record(e)
    return hook


B = Bench([(0x5ac5d0, 0x5ac8ee), (0x5ac8f0, 0x5acca3), (0x5accf0, 0x5ad0c9), (0x5c5230, 0x5c524b), (0x5f5bc0, 0x5f5c00), (0x424dc0, 0x424e10), (0x424e10, 0x424e6e), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5f8240: _recorder(CTOR, 0), 0x5f8510: _recorder(OPEN, 2), 0x5f8370: _search(FIRST, 3, 8), 0x5f83a0: _search(NEXT, 2, 4), 0x5f82a0: _recorder(DTOR, 0), 0x5c8890: _recorder(THEME, 1),
                 0x5c8a10: _recorder(BEFORE, 0), 0x414d40: _recorder(LOAD, 3), 0x414920: _recorder(SAVE, 3), 0x5c7420: _recorder(SINGLETON, 0)})
SLOTS, SELF, STATUS, TABLE, NAME_S, NAME_T, EXT_T, FOLDER_T, ITEMS, ITEM_T = (RAM + a for a in (0, 0x100, 0x200, 0x300, 0x400, 0x480, 0x500, 0x580, 0x600, 0x800))
FOUND, ENDED, COMPAT = STATUS, STATUS + 4, STATUS + 8
GLOBALS_T = RAM + 0xc00
OPS_TABLE = {'List': 0x5ac8f0, 'Newest': 0x5accf0, 'Load': 0x5ac650, 'Save': 0x5ac780, 'LoadA': 0x5ac5d0, 'LoadB': 0x5ac5f0, 'SaveA': 0x5ac610, 'SaveB': 0x5ac630}
OPS = [n for n in OPS_TABLE if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
# The search outcomes dealt, so the short paths come up too.
FLOWS = ['full'] * 5 + ['none', 'open fails', 'first fails', 'empty']


def wide(text):
    return text.encode('utf-16-le')


def case(r):
    op = r.deal(OPS)
    address = OPS_TABLE[op]
    flow = r.deal(FLOWS)
    log = log_region()
    table = import_table(log, {'WideCharToMultiByte': (W2M, 0x6fd1dc, 8, 0, 0), 'MultiByteToWideChar': (M2W, 0x6fd174, 6, 1 << 2, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    regions = {LOG: log, IMPORT_TABLE: table, HEAP: heap(0x4000, r.choice([0] * 39 + [r.randrange(1, 12)])), gamestring.LOCALE: bytes(4), STATUS: struct.pack('<iii', r.randrange(0, 5), -r.randrange(1, 5), 0),
               SLOTS: r.bytes(0x10), 0xf81ebc: struct.pack('<I', SLOTS if r.random() < 0.8 else 0)}
    set_return(log, SINGLETON, SLOTS)
    for k in (CTOR, OPEN, FIRST, NEXT, DTOR):
        mask_this(log, k)
    # The theme folder's wide string, and the narrow text its conversion answers (the wide path hashed, so what went into it shows).
    n = r.choice([1, 2, 3])
    regions[FOLDER_T] = wide('fold'[:n]) + bytes(2) + r.bytes(0x20)
    fill(log, THEME, 0, struct.pack('<5I', gamestring.VT[1][0], n, FOLDER_T, 8, 8), which=0)
    strings(log, THEME, 0, out=1)
    echo(log, THEME, 0)
    narrow = b'p' * r.randrange(1, 6) + b'\\x'
    set_return(log, W2M, len(narrow) + 1)
    fill(log, W2M, 4, narrow + b'\0', which=1)
    blob_bytes(log, W2M, 2, 0xffffffff)
    # The strings the code reads.
    addresses = [0xf7b888, 0xf7ad98, 0xf7adf8]
    for i, base in enumerate(addresses):
        gamestring.string(r, 1, base, GLOBALS_T + 0x60 * i, regions, wide(r.choice(['\\', '.a', '.b', '', 'zz'])))
    for i, (base, text) in enumerate(((0xf7b948, b'*'), (0xf7b548, b'.sav'), (0xf7b398, b'.qs'), (0xf7aed0, b'\\'))):
        gamestring.string(r, 0, base, GLOBALS_T + 0x200 + 0x40 * i, regions, text if r.random() < 0.8 else r.choice([b'', b'ab', b'.long']))
    strings(log, LOAD, 1)
    strings(log, SAVE, 1)
    set_return(log, LOAD, r.getrandbits(32))
    set_return(log, SAVE, r.getrandbits(32))
    if op in ('Load', 'Save', 'LoadA', 'LoadB', 'SaveA', 'SaveB'):
        gamestring.string(r, 1, NAME_S, NAME_T, regions, wide(r.choice(['a', 'bc', 'def', ''])))
        if op in ('Load', 'Save'):
            gamestring.string(r, 1, EXT_T + 0x40, EXT_T, regions, wide(r.choice(['.x', '', 'yy'])))
            return Call(address, [NAME_S, EXT_T + 0x40], regions, conv='s', ret='a')
        return Call(address, [NAME_S], regions, conv='s', ret='a')
    # The search.
    set_return(log, OPEN, ENDED if flow == 'open fails' else FOUND)
    strings(log, OPEN, 1 << 1, out=1)
    set_return(log, FIRST, ENDED if flow == 'first fails' else FOUND)
    strings(log, FIRST, 1 << 1, out=0b101)
    strings(log, NEXT, 0, out=0b11)
    length = 0 if flow == 'empty' else r.randrange(0, 9)
    answers(log, NEXT, TABLE)
    regions[TABLE] = struct.pack('<40I', *[FOUND if i < length else ENDED for i in range(40)])
    attrs = [0x10, 0, 0x20, 0x20, 0, 0x11, 0x20, 0]
    names = [b'a.sav', b'B.sav', b'.', b'..', b'cd.qs', b'long.sav', b'.hid', b'x']
    if flow == 'none':
        attrs = [0x10] * 8
        names = [b'a', b'b', b'c', b'd', b'e', b'f', b'g', b'h']

    def entries(k, arg, which, count):
        rows = []
        for _ in range(count):
            hi = r.choice([0, 1, 1, 2, 0xffffffff])
            rows.append(struct.pack('<III', r.choice(attrs), r.choice([0, 1, 5, 0x80000000, 0xffffffff]), hi))
        fill(log, k, arg, b''.join(rows), which=which, offset=4, records=count)
        fill(log, k, arg, b''.join(name.ljust(10, b'\0') for name in r.sample(names, count)), which=which + 1, records=count, through=0x20)

    entries(FIRST, 2, 2, 1)
    entries(NEXT, 1, 4, 8)
    if op == 'Newest':
        return Call(address, [], regions, ret='v')
    # The list as it was: nodes and items, its count now and then short of them.
    count = r.choice([0, 0, 1, 2, 3])
    nodes = [ITEMS + 0x100 + 0x10 * i for i in range(count)]
    part = bytearray(r.bytes(0x40))
    for i, node in enumerate(nodes):
        item = ITEMS + 0x40 * i if r.random() < 0.9 else 0
        if item:
            gamestring.string(r, 1, item, ITEM_T + 0x40 * i, regions, wide(r.choice(['q', 'rs', ''])))
        put(part, 0x10 * i, 'III', item, nodes[i + 1] if i + 1 < count else 0, nodes[i - 1] if i else 0)
    regions[ITEMS + 0x100] = bytes(part)
    told = count if r.random() < 0.7 else r.randrange(0, count + 1)
    regions[SELF] = struct.pack('<4I', r.getrandbits(32), nodes[0] if nodes else 0, nodes[-1] if nodes else 0, told)
    set_return(log, M2W, 3)
    fill(log, M2W, 4, wide('nm') + bytes(2), which=6)
    return Call(address, [SELF], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
