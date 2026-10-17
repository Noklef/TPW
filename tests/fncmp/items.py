# The online items of src/game/online/items against the originals. The constructors from a record (0x5bdac0, 0x5bde30, 0x5be1c0, 0x5be3e0, 0x5be610) get a
# record of random fields whose text pointers point at wide texts (now and then empty; the 0x70293c record's +0x44 at a narrow one), an owner whose list
# (+0x2c) holds no, one or two nodes, and the game's String / WString code with the heap; 0x702938's floats (times 1.01, x87) come from r.f(). 0x702940's
# narrow strings go through String_AssignFromWide: WideCharToMultiByte is a recorder (as in wideconv.py) answering a size and writing a text,
# GlobalAlloc / GlobalFree the heap stubs. The empty constructor (0x5bdcd0) runs over random bytes; the assignment (0x5bdfd0) between two built items
# (strings of up to 24 units), now and then the item itself. The owner's loads (0x5bc940, 0x5bd1b0, 0x5bd5f0, 0x5bda40) get 0-3 records (now and then a
# negative count) and a heap whose allocations now and then fail (an item's: none made; a node's: both write through the null).
# usage: items.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, fill, global_heap, heap, import_table, log_region, put, set_return  # noqa: E402

# kind: (constructor, record size, the record's wide text pointers, its narrow ones, the owner's load, the item's size)
KINDS = {'938': (0x5bdac0, 0x30, [4, 8, 0x1c], [], 0x5bc940, 0x68), '93c': (0x5bde30, 0x66, [4, 8, 0xc, 0x4c], [0x44], None, 0xc0),
         '940': (0x5be1c0, 0x10, [4, 8, 0xc], [], 0x5bd1b0, 0x48), '944': (0x5be3e0, 0x26, [8, 0xc, 0x16, 0x1a, 0x22], [], 0x5bd5f0, 0x80),
         '948': (0x5be610, 0x34, [4, 8, 0x24], [], 0x5bda40, 0x6c)}
OPS_ALL = ['C' + k for k in KINDS] + ['Add' + k for k in KINDS if KINDS[k][4]] + ['Empty', 'Assign']
B = Bench([(0x5bc940, 0x5bc9c0), (0x5bd1b0, 0x5bd230), (0x5bd5f0, 0x5bd670), (0x5bda40, 0x5bdbf0), (0x5bdcd0, 0x5bde10), (0x5bde30, 0x5be0e0),
           (0x5be1c0, 0x5be300), (0x5be3e0, 0x5be510), (0x5be610, 0x5be730), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x424db0, 0x424e6e),
           (0x5f5660, 0x5f5690)])
ITEM, OTHER, OWNER, LIST, NODES, RECORDS, TEXTS, ARRAY, BLOCKS = (RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x1000, RAM + 0x2000,
                                                                  RAM + 0x3000, RAM + 0x4000)
WC2MB = 41   # WideCharToMultiByte's recorder slot
WIDE_CHARS = [0x61, 0x62, 0x41, 0x7a, 0x30, 0xe9, 0x4e2d, 0xffff]


def texts(r, regions):
    """Six wide texts and two narrow ones; returns their addresses."""
    wide, narrow = [], []
    for i in range(8):
        n = r.choice([0, 1, 3, 8, 17])
        at = TEXTS + 0x80 * i
        if i < 6:
            regions[at] = struct.pack('<%dH' % n, *(r.choice(WIDE_CHARS) for _ in range(n))) + bytes(2) + r.bytes(4)
            wide.append(at)
        else:
            regions[at] = bytes(r.choice(b'abcXYZ09 \x80\xe9') for _ in range(n)) + bytes(1) + r.bytes(4)
            narrow.append(at)
    return wide, narrow


def record(r, kind, wide, narrow):
    _c, size, wides, narrows, _a, _s = KINDS[kind]
    rec = r.bytes(size)
    for off in wides:
        put(rec, off, 'I', r.choice(wide))
    for off in narrows:
        put(rec, off, 'I', r.choice(narrow))
    if kind == '938':
        for off in (0xc, 0x10, 0x14):
            put(rec, off, 'I', r.f())
    return rec


def owner(r, regions):
    """The owner with its list of 0-2 nodes (items elsewhere)."""
    own = r.bytes(0x40)
    put(own, 0x2c, 'I', LIST)
    n = r.choice([0, 1, 2])
    lst = r.bytes(0x10)
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    nodes = r.bytes(0x20)
    for k in range(n):
        put(nodes, 0x10 * k, 'III', RAM + 0x800 + 0x40 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
    regions.update({OWNER: own, LIST: lst, NODES: nodes})


def converter(r, regions):
    """WideCharToMultiByte (for String_AssignFromWide) and the GlobalAlloc / GlobalFree stubs."""
    log = log_region()
    table = import_table(log, {'WideCharToMultiByte': (WC2MB, 0x6fd1dc, 8, 0, 0)})
    for at, stub in global_heap().items():
        table[at - IMPORT_TABLE:at - IMPORT_TABLE + 4] = stub
    size = r.choice([0, 1, 4, 9, 0x20]) if r.random() < 0.9 else r.choice([-1, 0x7fffffff])
    set_return(log, WC2MB, size)
    fill(log, WC2MB, 4, bytes(r.choice(b'abc\x80') for _ in range(min(max(size, 0), 0x20))) + (bytes(1) if r.random() < 0.8 else b''))
    regions.update({LOG: log, IMPORT_TABLE: table})


def built(r, base, regions, slot):
    """An 0x70293c item at base: its five strings built (blocks at BLOCKS + 0x800 * slot + 0x100 * i), its other fields random."""
    item = r.bytes(0xc0)
    for i, (off, wide) in enumerate(((0xc, 1), (0x20, 1), (0x34, 1), (0x7c, 0), (0x94, 1))):
        gamestring.string(r, wide, 'tmp', BLOCKS + 0x800 * slot + 0x100 * i, regions)
        item[off:off + 0x14] = regions.pop('tmp')[:0x14]
    regions[base] = item
    return base


def case(r):
    op = r.deal(OPS)
    regions = {HEAP: heap(0x4000, r.choice(range(1, 8)) if op.startswith('Add') and r.random() < 0.3 else 0)}
    if op == 'Empty':
        regions[ITEM] = r.bytes(0xc0)
        return Call(0x5bdcd0, [ITEM], regions, conv='t', ret='a')
    if op == 'Assign':
        built(r, ITEM, regions, 0)
        other = ITEM if r.random() < 0.1 else built(r, OTHER, regions, 1)
        return Call(0x5bdfd0, [ITEM, other], regions, conv='t', ret='a')
    kind = op[-3:]
    ctor, size, _w, _n, add, item_size = KINDS[kind]
    wide, narrow = texts(r, regions)
    owner(r, regions)
    if kind == '940':
        converter(r, regions)
    if op.startswith('C'):
        regions[RECORDS] = record(r, kind, wide, narrow) + r.bytes(8)
        regions[ITEM] = r.bytes(item_size)
        return Call(ctor, [ITEM, RECORDS, OWNER], regions, conv='t', ret='a')
    n = r.choice([0, 1, 2, 3]) if r.random() < 0.9 else r.choice([-1, -0x80000000])
    recs = bytearray()
    for _ in range(max(n, 0)):
        recs += record(r, kind, wide, narrow)
    regions[RECORDS] = bytes(recs) + r.bytes(8)
    regions[ARRAY] = struct.pack('<iI', n, RECORDS) + r.bytes(8)
    return Call(add, [OWNER, ARRAY], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
