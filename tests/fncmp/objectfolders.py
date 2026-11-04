# The object definitions' folders searched (src/game/world/objectdefs: 0x413930) against the original, the directory tests, Strings and stream real, the rest recorders: 0-3 sources, each finding entries by attributes and names (a directory, a dot directory, a wad, a file), the object's allocation or constructor failing, each open and test answering yes or no, the mode easy or not, a load failing.
# usage: objectfolders.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, fill, heap, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402
from objectdefsload import TEXTS, Texts, string  # noqa: E402

DELETE, FIRST, NEXT, CTOR, HAS, ADD, LOAD = 0, 1, 2, 3, 4, 5, 6
OWN, EASY, MODE, ATEXIT = 14, 15, 25, 29
B = Bench([(0x413930, 0x413c02), (0x5c5230, 0x5c524b), (0x5c53c0, 0x5c5480), (0x5c5480, 0x5c5510), (0x424cf0, 0x424d10), (0x424d30, 0x424db0), (0x5f5fc0, 0x5f7f00), (0x5da3c0, 0x5da3c1), (0x409cd0, 0x409cdc), (0x67a400, 0x6902c0)],
          hooks={0x41dc80: _recorder(CTOR, 2), 0x41ede0: _recorder(HAS, 1), 0x41eaa0: _recorder(ADD, 1), 0x413c10: _recorder(LOAD, 3), 0x41f170: _recorder(OWN, 1), 0x41f190: _recorder(EASY, 1), 0x550ca0: _recorder(MODE, 0), 0x67a7d0: _recorder(ATEXIT, 1, cdecl=True)})
DEFS, FOLDERS, LIST, NODES, SOURCE, VT, OBJECT, OBJECT_VT, STATUS, TABLES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600, RAM + 0x700, RAM + 0x800, RAM + 0x900
FOUND, ENDED = STATUS, STATUS + 4
OPS = ['some', 'some', 'some', 'no list', 'empty', 'no object', 'easy', 'plain', 'load fails'] * 2 + ['no memory']   # (no memory crashes in both: the String's, unchecked)
NAMES = [b'Big', b'.', b'..', b'Pack.wad', b'Pack.WAD', b'Note.txt', b'a', b'Rides2']


def case(r):
    op = r.deal(OPS)
    log = log_region()
    log_fold(log)
    texts = Texts()
    tables = bytearray(0xa0 * 5)

    def answer(n, k, choices):
        answers(log, k, TABLES + 0xa0 * n)
        put(tables, 0xa0 * n, '40I', *[r.choice(choices) for _ in range(40)])

    answer(0, OWN, [1, 1, 1, 0, 2])
    answer(1, EASY, [1, 1, 0, 2])
    answer(2, HAS, [0, 0, 1, 2])
    answer(3, LOAD, [0] if op == 'load fails' else [1, 1, 1, 1, 2, 0])
    length = r.randrange(0, 12)
    answer(4, NEXT, [FOUND])
    put(tables, 0xa0 * 4, '40I', *[r.choice([FOUND, FOUND, ENDED]) if i < length else ENDED for i in range(40)])
    set_return(log, FIRST, r.choice([FOUND, FOUND, FOUND, ENDED]))
    set_return(log, CTOR, 0 if op == 'no object' else OBJECT)
    strings(log, FIRST, 0, 1 | 4)
    strings(log, NEXT, 0, 3)
    strings(log, CTOR, 0, 1)
    blob(log, CTOR, 1, 2)   # (the path String by its length)
    strings(log, OWN, 0, 1)
    strings(log, EASY, 0, 1)

    def entries(k, arg, which):
        fill(log, k, arg, bytes(r.choice([0x10, 0x10, 0, 0x20, 0x11]) for _ in range(8)), which=which, offset=4, records=8)
        fill(log, k, arg, b''.join(name.ljust(10, b'\0') for name in r.sample(NAMES, 8)), which=which + 1, records=8, through=0x20)

    entries(FIRST, 2, 0)
    entries(NEXT, 1, 2)
    count = 0 if op == 'empty' else r.randrange(1, 4)
    nodes = bytearray(0x20)
    for i in range(count):
        put(nodes, 8 * i, 'II', SOURCE + 8 * i, NODES + 8 * (i + 1) if i + 1 < count else 0)
    sources = b''.join(struct.pack('<II', VT, r.getrandbits(32)) for _ in range(3))
    folders = bytearray(r.bytes(0x40))
    folders[0x18:0x2c] = string(texts, r.choice([b'Data\\Rides\\', b'R\\', b'']))
    put(folders, 0x2c, 'I', 0 if op == 'no list' else LIST)
    wad = string(texts, b'.wad')
    regions = {LOG: log, DEFS: r.bytes(0x10), FOLDERS: folders, LIST: put(bytearray(r.bytes(0x10)), 4, 'I', NODES if count else 0), NODES: nodes, SOURCE: sources, VT: vtable({1: 3, 2: 2}), OBJECT: struct.pack('<I', OBJECT_VT) + r.bytes(0x34), OBJECT_VT: vtable({DELETE: 1}, 8),
               STATUS: struct.pack('<ii', r.randrange(0, 5), -r.randrange(1, 5)), TABLES: tables, TEXTS: texts.buf, HEAP: heap(0x4000, 1 if op == 'no memory' else 0),
               0x78a2f0: r.bytes(4), 0xf80808: wad, 0xfb3b78: struct.pack('<II', r.choice([0, 0, 1, 0xff, r.getrandbits(32)]), 2 if op == 'easy' else 0 if op == 'plain' else r.choice([2, 0, 1]))}
    return Call(0x413930, [DEFS, FOLDERS], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
