# The WAD archive's directory tree (src/game/files/waddir) against the originals: folders, files, their node lists, the pools of 100-element blocks and the
# finder are built by the case in an arena (Mem_Free only records what it is given, so a "freed" block need not come from the heap), names from a list of
# WAD-like paths ('\' and '/' separated, upper and lower case, leading / trailing / doubled separators); WadDir_Load reads a WAD (0x58-byte header, 0x28-byte
# records, the name block) from fncmp's memory stream, now and then cut short, with a bad name offset or length, or the name block elsewhere; the pools'
# used counts at and around 100 (a new block). tolower is the CRT's in the C locale (0xfb1440 0; driver.cpp's WadDir env has an ASCII stand-in).
# usage: waddir.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Load,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, memory_stream  # noqa: E402

B = Bench([(0x637460, 0x638840), (0x61ade0, 0x61ae00), (0x67a400, 0x690000), (0x5f6000, 0x5f9000), (0x618000, 0x61b000), (0x62f000, 0x631000),
           (0x405000, 0x406000), (0x5e4000, 0x5e5000), (0x636b00, 0x636d00)])
THIS, RESULT, ARGS = RAM, RAM + 0x100, RAM + 0x110
ARENA, ARENA_SIZE = RAM + 0x1000, 0xf000
STREAM, WAD, WAD_SIZE = RAM + 0x10000, RAM + 0x10100, 0x3f00
LOCALE = 0xfb1440   # the CRT's LC_CTYPE handle: 0, the C locale
NAMES = [b'data\\x.wad', b'Ride/A.TXT', b'a\\b\\c\\d.e', b'noslash', b'', b'\\lead', b'trail\\', b'x\\\\y', b'A/b\\C', b'RIDES\\coaster\\track.bin',
         b'rides\\Coaster\\car.bin', b'sfx/', b'/', b'Q\\R', b'q\\r\\s', b'.\\dot']
# name: (address, this: d a WadDir, f a folder, l a list, p a pool, n a finder, w a file, s a string list, - none; stack arguments; conv; returns)
OPS = {
    'DirCtor': (0x637460, 'd', '', 't', 'a'), 'DirDtor': (0x637550, 'd', '', 't', 'v'), 'DirClear': (0x637630, 'd', '', 't', 'v'),
    'Load': (0x637690, 'd', 'RSb', 't', 'a'), 'Normalize': (0x637a10, '-', 'W', 'c', 'a'),
    'FolderCtor': (0x637f60, 'f', '', 't', 'a'), 'FolderDtor': (0x637ae0, 'f', '', 't', 'v'), 'AddFile': (0x637b50, 'f', 'Nddhpb', 't', 'v'),
    'AddPath': (0x637be0, 'f', 'Npb', 't', 'a'), 'FileCtor': (0x637f30, 'w', '', 't', 'a'), 'FileDtor': (0x637f40, 'w', '', 't', 'v'),
    'FolderListDeletingDtor': (0x637f90, 'l', 'F', 't', 'a'), 'FolderListDtor': (0x637fb0, 'l', '', 't', 'v'),
    'FileListDeletingDtor': (0x637ff0, 'l', 'F', 't', 'a'), 'FileListDtor': (0x638010, 'l', '', 't', 'v'),
    'FolderListBaseDeletingDtor': (0x6387a0, 'l', 'F', 't', 'a'), 'FileListBaseDeletingDtor': (0x6387f0, 'l', 'F', 't', 'a'),
    'FolderPoolFree': (0x638660, 'P', '', 't', 'v'), 'FilePoolFree': (0x6386b0, 'p', '', 't', 'v'), 'FilePoolAlloc': (0x638700, 'p', '', 't', 'a'),
    'StringListCtor': (0x638640, 's', '', 't', 'a'), 'StringListCtor2': (0x6380d0, 's', '', 't', 'a'), 'FinderDtor': (0x61ade0, 'n', '', 't', 'v'),
}


class Arena:
    """Case memory at ARENA: blocks handed out 16-byte aligned, random bytes where nothing is put."""
    def __init__(self, r):
        self.r, self.data, self.top = r, bytearray(r.bytes(ARENA_SIZE)), 0

    def alloc(self, n):
        at = ARENA + self.top
        self.top += (n + 15) & ~15
        if self.top > 0xee00:   # (the lists and pools the case copies out sit above)
            raise OverflowError
        return at

    def put(self, at, fmt, *v):
        struct.pack_into('<' + fmt, self.data, at - ARENA, *v)

    def text(self, s):
        at = self.alloc(len(s) + 1)
        self.data[at - ARENA:at - ARENA + len(s) + 1] = s + b'\0'
        return at


def name(r):
    s = r.choice(NAMES)
    return bytes(c ^ 0x20 if chr(c).isalpha() and r.random() < 0.3 else c for c in s)


def nodes(a, r, items):
    """A PtrList's nodes (item, next, prev) for items: (head, tail)."""
    at = [a.alloc(0xc) for _ in items]
    for i, (n, item) in enumerate(zip(at, items)):
        a.put(n, 'III', item, at[i + 1] if i + 1 < len(at) else 0, at[i - 1] if i else 0)
    return (at[0], at[-1]) if at else (0, 0)


def plist(a, r, at, items):
    head, tail = nodes(a, r, items)
    a.put(at, 'IIII', r.getrandbits(32), head, tail, len(items) if r.random() < 0.9 else r.getrandbits(8))


def wfile(a, r, at=None):
    at = at or a.alloc(0x14)
    a.put(at, 'IB', a.text(name(r)), r.choice([0, 1, 1, 0x80]))
    return at


def folder(a, r, depth=1, at=None):
    at = at or a.alloc(0x28)
    a.put(at, 'IB', a.text(name(r)), r.choice([0, 1, 1, 0x80]))
    subs = [folder(a, r, depth - 1) for _ in range(r.choice([0, 0, 1, 2]))] if depth > 0 else []
    plist(a, r, at + 8, subs)
    plist(a, r, at + 0x18, [wfile(a, r) for _ in range(r.choice([0, 1, 3]))])
    return at


def pool(a, r, at, kind):
    """A WadPool at `at`: a chain of 0-2 blocks of count elements (folders or files), the newest first; used around the count."""
    nxt, count = 0, 0
    for _ in range(r.choice([0, 1, 1, 1, 2])):
        count = r.choice([0, 1, 3, 100, 100]) if kind == 'f' else r.choice([0, 1, 3, 100])
        items = 0
        if r.random() < 0.9:
            size = 0x28 if kind == 'f' else 0x14
            block = a.alloc(4 + count * size)
            a.put(block, 'I', count)
            for i in range(count):
                if r.random() < 0.1 or count > 3:   # (big blocks: a few real elements, the rest constructed-empty)
                    a.put(block + 4 + i * size, 'IB', 0, 0) if size == 0x14 else a.put(block + 4 + i * size, 'IB35x', 0, 0)
                elif kind == 'f':
                    folder(a, r, 0, block + 4 + i * size)
                else:
                    wfile(a, r, block + 4 + i * size)
            items = block + 4
        b = a.alloc(8)
        a.put(b, 'II', nxt, items)
        nxt = b
    used = r.choice([0, 1, 99, 100, 100, 101, 0x7fffffff, count, max(count - 1, 0)]) if nxt else 100   # (below 0: a wild write)
    a.put(at, 'Ii', nxt, used)


def strings(a, r, at):
    """A StringList (vtable, head, tail, count) of 0-3 nodes: a String (vtable, length, data, capacity, grow), next, prev."""
    ns = [a.alloc(0x1c) for _ in range(r.choice([0, 1, 3]))]
    for i, n in enumerate(ns):
        a.put(n, '5I2I', r.getrandbits(32), 3, a.text(b'abc') if r.random() < 0.8 else 0, 4, 0, ns[i + 1] if i + 1 < len(ns) else 0, ns[i - 1] if i else 0)
    a.put(at, '4I', r.getrandbits(32), ns[0] if ns else 0, ns[-1] if ns else 0, len(ns))


def finder(a, r, at=None):
    at = at or a.alloc(0x60)
    a.put(at, '5I', r.getrandbits(32), 1, a.text(b'*.wad') if r.random() < 0.8 else 0, 8, 0)
    a.put(at + 0x20, '5I', r.getrandbits(32), 1, a.text(b'x\\') if r.random() < 0.8 else 0, 8, 0)
    strings(a, r, at + 0x3c)
    strings(a, r, at + 0x4c)
    return at


def wad(r, block):
    """The WAD's bytes and size: a header, count records and the names, then (sometimes) cut short or with a bad record."""
    count = r.choice([0, 1, 2, 3, 5, 8, 12]) if not r.wild else r.choice([0, 1, 4, 12, 0x40])
    names = [name(r) for _ in range(count)]
    offset = 0x58 + 0x28 * count + r.choice([0, 0, 4, 0x10])
    blob, at = b'', []
    for n in names:
        at.append(offset + len(blob))
        blob += n + b'\0'
    data = bytearray(r.bytes(0x58))
    struct.pack_into('<3I', data, 0x48, count, offset if r.random() < 0.9 else r.choice([0, offset + 1, r.getrandbits(32)]), len(blob))
    for i, n in enumerate(names):
        rec = bytearray(r.bytes(0x28))
        struct.pack_into('<3I', rec, 0, r.getrandbits(32), at[i] if r.random() < 0.95 else r.choice([at[i] + 1, at[i] ^ 0x80000000]),
                         len(n) if r.random() < 0.95 else r.choice([0, len(n) + 1, len(n) - 1 if n else 0]))
        struct.pack_into('<3I', rec, 0x10, r.getrandbits(32), r.choice([0, 0, 1, r.getrandbits(32)]), r.getrandbits(32))
        data += rec
    data += r.bytes(offset - len(data)) + blob
    size = len(data)
    if r.random() < 0.15:
        size = r.randrange(size + 1)
    return bytes(data) + r.bytes(WAD_SIZE - len(data)), size


def case(r):
    op = r.choice(PICK)
    address, kind, args, conv, ret = OPS[op]
    a = Arena(r)
    obj = bytearray(r.bytes(0x100))
    regions = {HEAP: heap(0x10000), LOCALE: bytes(4), RESULT: r.bytes(0x10)}
    if kind == 'd':   # root, names, finder, folder pool, file pool
        struct.pack_into('<3I', obj, 0, folder(a, r, 2) if r.random() < 0.7 else 0, a.text(b'names') if r.random() < 0.6 else 0,
                         finder(a, r) if r.random() < 0.7 else 0)
        pool(a, r, ARENA + 0xef00, 'f')
        pool(a, r, ARENA + 0xef08, 'w')   # (the two pools are back to back)
        obj[0xc:0x1c] = a.data[0xef00:0xef10]
    elif kind == 'f':
        obj[:0x28] = a.data[folder(a, r, 2) - ARENA:][:0x28]
    elif kind == 'w':
        obj[:0x14] = a.data[wfile(a, r) - ARENA:][:0x14]
    elif kind == 'l':
        plist(a, r, ARENA + 0xef00, [r.getrandbits(32) for _ in range(r.choice([0, 1, 2, 5]))])
        obj[:0x10] = a.data[0xef00:0xef10]
    elif kind in 'pP':
        pool(a, r, ARENA + 0xef00, 'f' if kind == 'P' else 'w')
        obj[:8] = a.data[0xef00:0xef08]
    elif kind == 'n':
        obj[:0x60] = a.data[finder(a, r) - ARENA:][:0x60]
    regions[THIS] = obj
    stack = []
    for x in args:
        if x == 'R':
            stack.append(RESULT)
        elif x == 'S':   # the stream over the WAD
            data, size = wad(r, True)
            regions[WAD] = data
            regions[STREAM] = memory_stream(STREAM, WAD, size)
            stack.append(STREAM)
        elif x == 'b':
            stack.append(r.choice([0, 1, 0x100, 0x101, r.getrandbits(32)]))
        elif x == 'W':   # a TextWriter (vtable, length, data, capacity, overflow) over a name
            s = name(r) + r.bytes(4)
            text = a.text(s)
            struct.pack_into('<5I', obj, 0x80, r.getrandbits(32), r.choice([len(s) - 4, len(s), 0, 1]), text, len(s) + 1, 0)
            regions[THIS] = obj
            stack.append(THIS + 0x80)
        elif x == 'N':   # a name to file (written to: the separators cut)
            stack.append(a.text(name(r) + (b'\\' + name(r) if r.random() < 0.4 else b'')))
        elif x == 'p':   # the pool to take from
            pool(a, r, ARENA + 0xef20, 'f' if op == 'AddPath' else 'w')
            stack.append(ARENA + 0xef20)
        elif x == 'h':
            stack.append(r.getrandbits(32))
        elif x == 'd':
            stack.append(r.choice([0, 0x58, 0x80, r.getrandbits(32)]))
        elif x == 'F':
            stack.append(r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    regions[ARENA] = a.data
    return Call(address, ([THIS] if kind != '-' else []) + stack, regions, conv=conv, ret=ret)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
