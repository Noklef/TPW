# The named objects registry of src/game/core/namedobjects against the original: the removal of the 14 named globals (0x408f00) from a registry ({list,
# cached} at [0x786b9c]) of 0 to 6 objects named after them (case varied) or not, duplicates included, or without a list; the objects are deleted through
# a recorder (slot 0), NamedList_Find 0x421770, ListRef_Remove 0x421690 and the name comparison run (the STL lock left out as in vobj3.py).
# usage: namedobjects.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Remove)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, heap, log_region, put, vtable  # noqa: E402


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(0x408f00, 0x40914f), (0x421770, 0x4217e0), (0x421690, 0x421720), (0x590730, 0x590750), (0x424cf0, 0x424d10), (0x67df40, 0x67e040),
           (0x67faf0, 0x67fb20), (0x682720, 0x682780), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
NAMES = [0x786b68, 0x785e78, 0x785f98, 0x7862c8, 0x785f68, 0x7862f8, 0x786d10, 0x7866e8, 0x7862e0, 0x785ed8, 0x785f80, 0x786aa8, 0x786040, 0x7867d8]
WORDS = [b'Font', b'Sky', b'Rides', b'Shops', b'Staff', b'Water', b'Lake', b'Paths', b'Fence', b'Trees', b'Rock', b'Hint', b'Map', b'Sound']
REF, LST, NODES, OBJS, TEXTS, VT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000


def varied(r, base):
    return bytes(c ^ 0x20 if r.random() < 0.3 else c for c in base) + bytes(0x10)


def case(r):
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


OPS = [k for k in os.environ.get('OPS', 'Remove').split(',') if k]

if __name__ == '__main__':
    B.main(case)
