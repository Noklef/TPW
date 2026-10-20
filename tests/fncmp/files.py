# The file units (the quickload indexes 0x461790-0x461a5c) against the originals.
# usage: files.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x461790, 0x461a60), (0x45b9d0, 0x45ba15), (0x67a400, 0x690000)])
TABLES, LISTS, TEXT, ARG = RAM, RAM + 0x1000, RAM + 0x10000, RAM + 0x20000
MARKS = 0x7a3d3c  # 0x7a3d3c in use, 0x7a3e5c second table, 0x7a445c loaded, 0x7a45dc base table
STEMS = [b'coaster', b'Wild.Mouse', b'a.b\\c', b'dir.x\\file', b'', b'x', b'Ride_01', b'.hidden', b'TRACK']
EXTS = [b'', b'.wad', b'.WAD', b'.', b'.sam', b'.Wad.bak', b'\\', b'.\\']


def name(r):
    s = r.choice(STEMS) + r.choice(EXTS)
    if r.random() < 0.3:
        s = bytes(c ^ 0x20 if chr(c).isalpha() and r.random() < 0.5 else c for c in s)
    if r.random() < 0.1:
        s = r.bytes(r.randrange(12)).replace(b'\0', b'')
    return s


def quickload(r, base=RAM):
    """Both tables of up to count indexes (each a list of names), the marks and a name to look for (the tables, lists, text and name at
    base on, as TABLES, LISTS, TEXT and ARG are at RAM)."""
    TABLES, LISTS, TEXT, ARG = base, base + 0x1000, base + 0x10000, base + 0x20000  # noqa: N806
    count = r.choice([1, 1, 1, 2, 3, 0]) if not r.wild else r.choice([0, 1, 3, 0xffffffff])
    tables, lists, text = bytearray(0x200), bytearray(0x8000), bytearray()
    for t in range(2):
        for e in range(4):
            n = r.choice([0, 1, r.randrange(6), r.randrange(30)])
            at = (t * 4 + e) * 0x100
            put(tables, t * 0x100 + e * 12, 'III', n, LISTS + at, TEXT)
            for i in range(n):
                put(lists, at + 4 * i, 'I', TEXT + len(text))
                text += name(r) + b'\0'
    marks = bytearray(0x8a4)
    put(marks, 0, 'I', r.choice([0, 1]))
    put(marks, 0x120, 'I', TABLES + 0x100)
    put(marks, 0x720, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    put(marks, 0x8a0, 'I', TABLES)
    arg = name(r) + b'\0'
    return {TABLES: tables, LISTS: lists, TEXT: bytes(text) + b'\0', ARG: arg, MARKS: marks,
            0x6fe228: put(bytearray(4), 0, 'I', count)}, count


def case(r):
    op = r.choice(OPS)
    if op in ('Init', 'Free'):
        marks = bytearray(r.bytes(0x8a4))
        if op == 'Free':
            put(marks, 0x120, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
            put(marks, 0x8a0, 'I', r.choice([0, HEAP + 0x80, r.getrandbits(32)]))
        regions = {MARKS: marks, HEAP: heap(), 0x6fe228: put(bytearray(4), 0, 'I', r.choice([0, 1, 2, 3, 10])),
                   **global_heap()}
        return Call(0x461790 if op == 'Init' else 0x4617e0, [], regions)
    regions, count = quickload(r)
    second = r.choice([0, 1, 1, 2, 0x100])
    index = r.randrange(min(count, 4)) if 0 < count < 0x100 else 0
    if op == 'Has':
        return Call(0x461820, [ARG, second, index], regions, ret='a')
    if op == 'Find':
        return Call(0x4618a0, [ARG, second], regions, ret='a')
    return Call(0x461920, [ARG, index, second], regions, ret='a')


KINDS = ['Init', 'Free', 'Has', 'Find', 'HasStem', 'HasStem']
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
