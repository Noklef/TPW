# The named objects registry of src/game/core/namedobjects against the original: the removal of the 14 named globals (0x408f00) from a registry ({list,
# cached} at [0x786b9c]) of 0 to 6 objects named after them (case varied) or not, duplicates included, or without a list; the objects are deleted through
# a recorder (slot 0), NamedList_Find 0x421770, ListRef_Remove 0x421690 and the name comparison run (the STL lock left out as in vobj3.py).
# Also the registry owner's constructor 0x407b90 (the name assigned from a WString at 0x7861c0, allocations failing or not), destructor
# 0x407c90 and its at-exit one 0x407b80 (an owned list of 0 to 5 nodes, the count short of it or not, objects deleted through a recorder's
# slot 3, a registry of 0 to 3 drained through ListRef_DrainAndDelete or none, frees hashed by heap()).
# And its sorted add 0x409620: a name (a String) into an owned list of 0 to 5 named Strings (sorted or not, near misses, the count short of it
# or not, now and then holding the block the new String will get, now and then a count with no head), allocations failing now and then.
# usage: namedobjects.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Remove,Ctor,Dtor,Static,Add)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, global_heap, heap, log_region, put, vtable  # noqa: E402


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x408f00, 0x40914f), (0x421770, 0x4217e0), (0x421690, 0x421720), (0x590730, 0x590750), (0x424cf0, 0x424d10), (0x67df40, 0x67e040),
           (0x67faf0, 0x67fb20), (0x682720, 0x682780), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000),
           (0x407b80, 0x407b8a), (0x407b90, 0x407c2e), (0x407c90, 0x407d79), (0x421510, 0x421588), (0x5da3c0, 0x5da3c1),
           (0x409620, 0x40978d), (0x424cc0, 0x424ce2), (0x5e2a30, 0x5e2a7e)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
NAMES = [0x786b68, 0x785e78, 0x785f98, 0x7862c8, 0x785f68, 0x7862f8, 0x786d10, 0x7866e8, 0x7862e0, 0x785ed8, 0x785f80, 0x786aa8, 0x786040, 0x7867d8]
WORDS = [b'Font', b'Sky', b'Rides', b'Shops', b'Staff', b'Water', b'Lake', b'Paths', b'Fence', b'Trees', b'Rock', b'Hint', b'Map', b'Sound']
REF, LST, NODES, OBJS, TEXTS, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def varied(r, base):
    return bytes(c ^ 0x20 if r.random() < 0.3 else c for c in base) + bytes(0x10)


def remove(r):
    regions = {HEAP: heap(), LOG: log_region(), VT: vtable({0: 1}), 0x786b9c: struct.pack('<I', REF)}
    for k, (at, word) in enumerate(zip(NAMES, WORDS)):
        key = bytearray(r.bytes(0x14))
        put(key, 8, 'I', TEXTS + 0x20 * k)
        regions[at] = bytes(key)
        regions[TEXTS + 0x20 * k] = word + bytes(0x10)
    n = r.choice([0, 1, 2, 3, 4, 6])
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'III', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, n)
    nodes = bytearray(r.bytes(0x10 * 6))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        o = bytearray(struct.pack('<I', VT) + r.bytes(0x3c))
        put(o, 0xc, 'I', TEXTS + 0x400 + 0x20 * k)
        regions[held] = bytes(o)
        regions[TEXTS + 0x400 + 0x20 * k] = varied(r, r.choice(WORDS) if r.random() < 0.75 else r.choice([b'Fonts', b'Sk', b'', b'Other']))
    ref = bytearray(r.bytes(8))
    put(ref, 0, 'I', LST if r.random() < 0.92 else 0)
    regions.update({REF: bytes(ref), LST: bytes(lst), NODES: bytes(nodes)})
    return Call(0x408f00, [], regions, ret='a')


OWNER, WSTR = RAM + 0x4000, 0x7861c0


def ctor(r):
    n = r.choice([0, 1, 5, 0x10, 0x23])
    wide = bytearray(r.bytes(0x14))
    put(wide, 0, 'III', 0x7030c8, n, TEXTS)
    regions = {OWNER: r.bytes(0x40), WSTR: bytes(wide), TEXTS: r.bytes(0x60), HEAP: heap(fail=r.choice([0, 0, 0, 0, 0, 1, 2, 3])), **global_heap()}
    return Call(0x407b90, [OWNER], regions, conv='t', ret='a')


def chain(r, regions, at, count, objects, vt):
    """count nodes {object, next, prev} at `at` (16 bytes apart), each object (or none) with vtable vt; returns the head and tail."""
    nodes = bytearray(r.bytes(0x10 * max(count, 1)))
    for k in range(count):
        held = objects + 0x40 * k if r.random() < 0.85 else 0
        put(nodes, 0x10 * k, 'III', held, at + 0x10 * (k + 1) if k + 1 < count else 0, at + 0x10 * (k - 1) if k else r.getrandbits(32))
        if held:
            regions[held] = put(bytearray(r.bytes(0x40)), 0, 'I', vt)
    regions[at] = bytes(nodes)
    return (at, at + 0x10 * (count - 1)) if count else (0, 0)


def dtor(r, address):
    owner = 0x786b68 if address == 0x407b80 else OWNER
    regions = {HEAP: heap(), LOG: log_region(), VT: vtable({0: 1, 3: 1}), **global_heap()}
    o = bytearray(r.bytes(0x40))
    put(o, 0, 'III', 0x703090, r.randrange(8), TEXTS + 0x100)
    n = r.choice([0, 1, 2, 3, 5])
    head, tail = chain(r, regions, NODES, n, OBJS, VT)
    put(o, 0x28, 'III', head, tail, r.choice([n, n, n, r.randrange(n + 1)]))
    if r.random() < 0.8:  # the registry: {list, cached, ?}; the list {vtable, head, tail, count}
        m = r.choice([0, 1, 3])
        lst = bytearray(r.bytes(0x20))
        put(lst, 0, 'IIII', VT, *chain(r, regions, NODES + 0x100, m, OBJS + 0x400, VT), m)
        ref = bytearray(r.bytes(0xc))
        put(ref, 0, 'I', LST if r.random() < 0.85 else 0)
        regions.update({REF: bytes(ref), LST: bytes(lst)})
        put(o, 0x34, 'I', REF)
    else:
        put(o, 0x34, 'I', 0)
    put(o, 0x38, 'I', r.choice([0, 0, TEXTS + 0x200]))
    regions[owner] = bytes(o)
    if address == 0x407b80:
        return Call(address, [], regions, ret='v')
    return Call(address, [owner], regions, conv='t', ret='v')


def string(regions, at, text):
    regions[at] = put(bytearray(0x14), 0, 'IIIII', 0x703090, len(text), at + 0x20, len(text) + 1, 8) + bytes(0xc) + text + bytes(0x10)


def add(r):
    regions = {HEAP: heap(fail=r.choice([0] * 12 + [1, 2])), LOG: log_region(), **global_heap()}
    o = bytearray(r.bytes(0x40))
    n = r.choice([0, 1, 2, 3, 5, 5])
    words = [varied(r, r.choice(WORDS + [b'Fonts', b'Sk', b''])).rstrip(bytes(1)) for _ in range(n)]
    if r.random() < 0.6:
        words.sort(key=bytes.lower)
    nodes = bytearray(r.bytes(0x10 * max(n, 1)))
    for k, word in enumerate(words):
        held = OBJS + 0x40 * k
        string(regions, held, word)
        held = HEAP + 0x10 if r.random() < 0.05 else held   # (the block the new String gets)
        put(nodes, 0x10 * k, 'III', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else r.getrandbits(32))
    regions[NODES] = bytes(nodes)
    count = r.choice([n, n, n, n, 0, 1, 7]) if n else r.choice([0] * 20 + [1])
    put(o, 0x24, 'IIII', 0x6fd704, NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0, count)
    regions[OWNER] = bytes(o)
    string(regions, RAM + 0x4800, varied(r, r.choice(WORDS + [b'Fonts', b'Sk', b'', b'Zoo', b'Aa'])).rstrip(bytes(1)))
    return Call(0x409620, [OWNER, RAM + 0x4800], regions, conv='t', ret='v')


def case(r):
    op = r.choice(OPS)
    if op == 'Ctor':
        return ctor(r)
    if op == 'Dtor':
        return dtor(r, 0x407c90)
    if op == 'Static':
        return dtor(r, 0x407b80)
    if op == 'Add':
        return add(r)
    return remove(r)


OPS = [k for k in os.environ.get('OPS', 'Remove,Ctor,Dtor,Static,Add').split(',') if k]

if __name__ == '__main__':
    B.main(case)
