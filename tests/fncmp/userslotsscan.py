# The user slots' filler (src/game/files/userslots: 0x5c7590) against the original: the four records cleared, the save, users and online folders opened or made, each subfolder of users named "<slot><name>" taken into its record with
# the byte +0x24 of the global save info read from it and its folders made. The strings real; the folders, the handle's open and search, the file creation, the checks of an entry, atoi, the global save's calls and the slot's folder recorders.
# The folders' recorders write a string object through the out pointer, the searches write the name; the conversions' imports recorders answering a text of their own, as in onlinenext.py.
# The answers of the search count the calls in the log, which keeps 40: at most two names taken, or the search would not see its end.
# usage: userslotsscan.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, global_heap, heap, log_fold, log_region, put, recorder, set_return, strings, this_first  # noqa: E402

SAVE, USERS, ONLINE, OPEN, FIRST, NEXT, CREATE, REAL, DIGIT, ATOI, SLOT, CTOR, DTOR, LOAD, BYTE, MAKE, W2M, M2W = range(18)
B = Bench([(0x5c7590, 0x5c7abd), (0x5f8240, 0x5f8250), (0x5f82a0, 0x5f82a8), (0x5f82e0, 0x5f82f8), (0x424db0, 0x424e6e), (0x5f5660, 0x5f5690), (0x5f56f0, 0x5f5710)] + gamestring.B_RANGES,
          hooks={0x5c7bd0: _recorder(SAVE, 1, com=True), 0x5c7c70: _recorder(USERS, 1, com=True), 0x5c8980: _recorder(ONLINE, 1, com=True), 0x5f8510: _recorder(OPEN, 2), 0x5f8370: _recorder(FIRST, 3), 0x5f83a0: _recorder(NEXT, 2),
                 0x5f5a50: _recorder(CREATE, 2, cdecl=True), 0x5c5230: _recorder(REAL, 1, cdecl=True), 0x67a560: _recorder(DIGIT, 1, cdecl=True), 0x67a520: _recorder(ATOI, 1, cdecl=True), 0x5c7de0: _recorder(SLOT, 2),
                 0x5aef50: _recorder(CTOR, 0), 0x5af250: _recorder(DTOR, 0), 0x5afd70: _recorder(LOAD, 1), 0x5afc40: _recorder(BYTE, 0), 0x5c7ac0: _recorder(MAKE, 1)})
SLOTS, GOOD, BAD, TABLES, TEXTS, OLD = RAM, RAM + 0x200, RAM + 0x210, RAM + 0x300, RAM + 0x1000, RAM + 0x2000
NEW = 0x60000


def wide(text):
    return text.encode('utf-16-le')


def table(regions, log, k, at, values):
    data = bytearray(0x100)
    struct.pack_into('<%dI' % min(len(values), 40), data, 0, *values[:40])
    regions[at] = bytes(data)
    answers(log, k, at)


def case(r):
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
    regions = {LOG: log, IMPORT_TABLE: imports, HEAP: heap(0x8000, r.choice([0] * 39 + [9]))}
    # The four records: strings with a text of their own, their dword set.
    slots = bytearray(r.bytes(0x100))
    for i in range(4):
        gamestring.string(r, 1, SLOTS + 0x400 + 0x40 * i, TEXTS + 0x400 + 0x40 * i, regions, wide('o%d' % i))
        slots[24 * i:24 * i + 0x14] = regions.pop(SLOTS + 0x400 + 0x40 * i)[:0x14]
    regions[SLOTS] = slots
    regions[0x78a374] = wide(r.choice(['', 'e', 'em'])) + bytes(4)
    gamestring.string(r, 1, 0xf82508, TEXTS + 0x700, regions, wide(r.choice(['\\', '/'])))
    gamestring.string(r, 1, 0xf818a8, TEXTS + 0x740, regions, wide(r.choice(['save.dat', 'g'])))
    # The folders: a wide string object written through the out pointer.
    for which, (k, address, text) in enumerate(((SAVE, TEXTS + 0x100, 'save'), (USERS, TEXTS + 0x140, 'users'), (ONLINE, TEXTS + 0x180, 'online'), (SLOT, TEXTS + 0x1c0, 'slot'))):
        regions[address] = wide(text).ljust(0x40, b'\0')
        fill(log, k, 0, struct.pack('<5I', 0x7030c8, len(text), address, 0x20, 8), which=which + 2)
        strings(log, k, 0, out=1)
    # The opens: each answers a pointer to its result.
    regions[GOOD] = struct.pack('<I', r.choice([0, 1, 0x7fffffff]))
    regions[BAD] = struct.pack('<I', r.choice([0xffffffff, 0x80000000]))
    opens = [r.choice([GOOD, GOOD, GOOD, BAD]) for _ in range(3)]
    table(regions, log, OPEN, TABLES, opens)
    this_first(log, OPEN)
    this_first(log, FIRST)
    this_first(log, NEXT)
    strings(log, OPEN, 0b10, out=1)
    strings(log, CREATE, 0b10, out=1)
    # The search of the users folder.
    regions[0xf825d0] = struct.pack('<I', r.getrandbits(32))
    more = r.choice([0, 1, 1, 2])
    names = [r.choice([b'1bob', b'2al', b'3c', b'4dee', b'5x', b'9', b'', b'qrs']) for _ in range(7)]
    if more == 2:
        names[0] = r.choice([b'9', b''])   # (two takes at most: the log's room)
    records = b''.join(name.ljust(0x10, b'\0') for name in names)
    table(regions, log, FIRST, TABLES + 0x100, [r.choice([GOOD, GOOD, GOOD, BAD])])
    fill(log, FIRST, 2, records[:0x10], which=6, offset=0x24)
    strings(log, FIRST, 0, out=0b101)
    table(regions, log, NEXT, TABLES + 0x200, [GOOD] * more + [BAD] * 8)
    fill(log, NEXT, 1, records[0x10:0x70], which=7, offset=0x24, records=6)
    strings(log, NEXT, 0, out=0b11)
    # The checks, per call.
    table(regions, log, REAL, TABLES + 0x300, [r.choice([0, 1, 1, 1]) for _ in range(40)])
    strings(log, REAL, 0, out=1)
    table(regions, log, DIGIT, TABLES + 0x400, [r.choice([0, 4, 4, 4]) for _ in range(40)])
    table(regions, log, ATOI, TABLES + 0x500, [r.choice([0, 1, 2, 3, 4, 4, 5, 0xffffffff]) for _ in range(40)])
    strings(log, ATOI, 1)
    strings(log, SLOT, 0, out=1)
    # The global save info: one there or none, none made now and then, its byte.
    regions[0xf7c58c] = struct.pack('<I', r.choice([0, OLD]))
    table(regions, log, CTOR, TABLES + 0x600, [r.choice([NEW, NEW + 0x100, NEW + 0x100, 0]) for _ in range(40)])
    set_return(log, BYTE, r.getrandbits(32))
    strings(log, LOAD, 1)
    blob(log, MAKE, 0, 0b10)
    return Call(0x5c7590, [SLOTS], regions, conv='t', ret='v', steps=100000)


if __name__ == '__main__':
    B.main(case)
