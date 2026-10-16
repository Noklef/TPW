# The constructors, destructors and releases that call a virtual method of another object (src/game/core/vdtor) against the originals: the other objects
# (a global one, a member, a virtual base) have vtables of recorders (see tools/fncmp.py recorder()), so the comparison covers the slot, the object it is called on,
# the arguments (the log) and what is done with the result.
# usage: vdtor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'C48C': (0x5e3b90, 't', 'a', 0x20), 'C48R': (0x5e3c00, 't', 'a', 0x20), 'C48D': (0x5e3bd0, 't', 'a', 0x30), 'C9C': (0x5d5700, 't', 'a', 0x20),
         'C9R': (0x5d5750, 't', 'a', 0x20), 'C9D': (0x5d5720, 't', 'a', 0x30), 'Ae8': (0x5d9b20, 't', 'a', 0x30), 'Bb4': (0x5dc3b0, 't', 'a', 0x30),
         'Bb8': (0x5dc550, 't', 'a', 0x30), 'S20': (0x5dd7f0, 't', 'a', 0x20), 'S20b': (0x5ddaa0, 't', 'a', 0x20), 'VB': (0x564a60, 't', 'a', 0x20),
         'D80': (0x5b5600, 't', 'v', 0xc0), 'D90': (0x5b56c0, 't', 'v', 0xc0), 'Dd0': (0x5bcef0, 't', 'v', 0xc0), 'Df8': (0x5bd330, 't', 'v', 0xc0),
         'D20': (0x5bd740, 't', 'v', 0xc0), 'X80': (0x5b98a0, 't', 'a', 0xd0), 'X90': (0x5b9970, 't', 'a', 0xd0), 'Xd0': (0x5be810, 't', 'a', 0xd0),
         'Xf8': (0x5be8e0, 't', 'a', 0xd0)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x67acb0, 0x67acb1), (0x67afa0, 0x67b060)])
OBJ, OTHER, SUB, VT, VT2, VT3 = RAM, RAM + 0x200, RAM + 0x400, RAM + 0x1000, RAM + 0x1100, RAM + 0x1200


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region(), HEAP: heap()}
    flags = [r.choice([0, 1, 2, 3, 0xffffffff])]
    if name in ('C48C', 'C48R', 'C48D'):
        regions[0xf85614] = struct.pack('<I', OTHER)
        regions[OTHER] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[VT] = vtable({6: 1, 7: 1})
        return Call(address, [OBJ] + (flags if name == 'C48D' else []), regions, conv=conv, ret=ret)
    if name in ('C9C', 'C9R', 'C9D'):
        regions[0xf82884] = struct.pack('<I', OTHER)
        regions[OTHER] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[VT] = vtable({0: 1, 1: 1})
        return Call(address, [OBJ] + (flags if name == 'C9D' else []), regions, conv=conv, ret=ret)
    if name in ('Ae8', 'Bb4', 'Bb8'):
        field, slot = {'Ae8': (4, 2), 'Bb4': (8, 15), 'Bb8': (8, 14)}[name]
        put(obj, field, 'I', OTHER)
        regions[OTHER] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[VT] = vtable({slot: 1})
        return Call(address, [OBJ] + flags, regions, conv=conv, ret=ret)
    if name[0] in 'DX' and len(name) == 3:  # the 0x7028xx destructors: a shared array record, a member list of objects with deleting destructors
        put(obj, 0x2c, 'I', OTHER)
        n = r.choice([0, 1, 2, 3])
        lst = bytearray(r.bytes(0x20))
        put(lst, 0, 'I', VT3)
        put(lst, 4, 'II', SUB if n else 0, SUB + 0x10 * (n - 1) if n else 0)
        put(lst, 0xc, 'I', n)
        nodes = bytearray(r.bytes(0x60))
        for k in range(n):
            put(nodes, 0x10 * k, 'III', RAM + 0x700 + 0x40 * k if r.random() < 0.85 else 0, SUB + 0x10 * (k + 1) if k + 1 < n else 0, SUB + 0x10 * (k - 1) if k else 0)
            regions[RAM + 0x700 + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
        regions.update({OTHER: lst, SUB: nodes, VT: vtable({0: 1}), VT3: vtable({0: 1})})
        if r.random() < 0.7:  # the shared record: its use count, an array whose element count (in front of it) is 0 so that no element function is called
            put(obj, 0x30, 'I', RAM + 0x800)
            rec = bytearray(r.bytes(0x20))
            put(rec, 4, 'I', RAM + 0x904 if r.random() < 0.7 else 0)
            put(rec, 0xc, 'i', r.choice([1, 1, 2, 0]))
            regions[RAM + 0x800] = rec
            regions[RAM + 0x900] = struct.pack('<I', 0) + r.bytes(0x40)
        else:
            put(obj, 0x30, 'I', 0)
        return Call(address, [OBJ] + (flags if name[0] == 'X' else []), regions, conv=conv, ret=ret)
    if name in ('S20', 'S20b'):
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({8: 0})
        regions[0xf85614] = struct.pack('<I', OTHER)
        regions[OTHER] = struct.pack('<II', VT2, SUB) + r.bytes(0x20)
        regions[SUB] = struct.pack('<I', VT3) + r.bytes(0x20)
        regions[VT3] = vtable({7: 0, 18: 0})
        regions[VT2] = vtable({})
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    # VB: the object is 0x18 into a class whose first dword is a vtable pointer; dword +4 of that vtable is the virtual base's offset
    put(obj, 0, 'I', VT)
    offset = r.choice([0x40, 0x80, 0xc0])
    regions[VT] = bytearray(vtable({})) 
    struct.pack_into('<I', regions[VT], 4, offset)
    put(obj, offset, 'I', VT2)
    regions[VT2] = vtable({2: 1})
    return Call(address, [OBJ + 0x18, r.getrandbits(32)], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
