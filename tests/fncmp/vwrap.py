# The small virtual-call wrappers of src/game/core/vwrap against the originals: objects whose vtables point at the recorders (see tools/fncmp.py recorder()),
# so the comparison covers which slot is called, on which object, with which arguments (the log) and what is done with the result (the hash the recorder returns).
# usage: vwrap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'S18': (0x5c5510, 't', 'a', 0x20), 'Pass': (0x617dd0, 't', 'a', 0x20), 'R24': (0x5d60d0, 't', 'a', 0x30), 'C24': (0x5d60f0, 't', 'a', 0x30),
         'G14': (0x46c330, 's', 'a', 0x30), 'G1e0': (0x498b90, 'c', 'v', 0x30), 'G1c8': (0x4987d0, 'c', 'v', 0x30), 'G1d8': (0x578390, 'c', 'v', 0x30),
         'Sub14': (0x5c5530, 't', 'a', 0x30), 'Sub28': (0x5c5560, 't', 'a', 0x30), 'M14': (0x617a30, 't', 'a', 0x30), 'M18': (0x617b00, 't', 'a', 0x30),
         'M10': (0x617b30, 't', 'a', 0x30), 'Fin': (0x5c4f80, 'c', 'a', 0x30), 'Prod': (0x5c4f60, 'c', 'a', 0x30), 'Sized': (0x565560, 't', 'a', 0x30),
         'Shared': (0x565930, 't', 'a', 0x50), 'G2c': (0x48ccd0, 't', 'a', 0x30), 'D9acc': (0x569ef0, 'c', 'a', 0x30), 'D9ac0': (0x5667a0, 'c', 'a', 0x30),
         'D4b84': (0x47e990, 'c', 'v', 0x20), 'R2fc': (0x4b3f20, 'c', 'v', 0x40), 'T26b0': (0x48eaf0, 'c', 'v', 0x30), 'VB48': (0x57c4a0, 'c', 'a', 0x30),
         'Drain': (0x576230, 't', 'v', 0x30), 'Rel': (0x591a80, 't', 'a', 0x30), 'R1bc': (0x497b60, 'c', 'v', 0x12), 'M20': (0x48d860, 't', 'a', 0x10),
         'GThing': (0x4af9f0, 't', 'v', 0x4f), 'Keys': (0x5e2310, 't', 'a', 0x50)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
OBJ, OBJ2, VT, VT2 = RAM, RAM + 0x200, RAM + 0x1000, RAM + 0x1100


def case(r):
    name = r.deal(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region()}
    if name == 'S18':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({6: 1})
        return Call(address, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Pass':
        put(obj, 0, 'I', VT)
        other = bytearray(r.bytes(0x40))
        put(other, 0, 'I', VT + 0x100)
        regions.update({OBJ2: other, VT + 0x100: vtable({4: 1})})
        return Call(address, [OBJ, OBJ2], regions, conv=conv, ret=ret)
    if name in ('R24', 'C24'):
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({9: 0})
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    if name == 'G14':
        regions[0x7b05d8] = struct.pack('<I', OBJ)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({5: 2})
        return Call(address, [r.getrandbits(32), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name in ('G1e0', 'G1c8', 'G1d8'):
        glob = {'G1e0': 0x7ca1e0, 'G1c8': 0x7ca1c8, 'G1d8': 0x87b1d8}[name]
        slot = 0 if name == 'G1d8' else 2
        regions[glob] = struct.pack('<I', OBJ if r.random() < 0.7 else 0)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({slot: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name in ('M14', 'M18', 'M10'):
        slot = {'M14': 5, 'M18': 6, 'M10': 4}[name]
        arg = bytearray(r.bytes(0x40))
        put(obj, 8, 'I', OBJ2 if r.random() < 0.7 else 0)
        regions.update({OBJ2: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({slot: 1}), RAM + 0x400: arg})
        return Call(address, [OBJ, RAM + 0x400], regions, conv=conv, ret=ret)
    if name == 'Fin':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({1: 0, 0: 1})
        return Call(address, [OBJ], regions, conv=conv, ret=ret)
    if name == 'Prod':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({2: 2})
        return Call(address, [r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), OBJ], regions, conv=conv, ret=ret)
    if name in ('Sized', 'Shared'):
        put(obj, 0, 'I', VT)
        put(obj, 0xc, 'I', r.choice([0, 1, 4, 0x10, 0xff]))
        regions[VT] = vtable({4: 1})
        arg = bytearray(r.bytes(0x40))
        put(arg, 0x20, 'ii', r.choice([-1, 0, 1, 5]), r.choice([-1, 0, 1, 5]))
        put(arg, 4, 'I', r.choice([0, 1, 4, 0x10, 0xff]))
        regions[RAM + 0x400] = arg
        return Call(address, [OBJ, RAM + 0x400], regions, conv=conv, ret=ret)
    if name in ('D9acc', 'D9ac0', 'D4b84'):
        glob = {'D9acc': 0x879acc, 'D9ac0': 0x879ac0, 'D4b84': 0x7b4b84}[name]
        regions[glob] = struct.pack('<I', OBJ if r.random() < 0.7 else 0)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({0: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'R2fc':
        for g, o in ((0x7cc2fc, OBJ), (0x7cc300, OBJ2), (0x7cc304, RAM + 0x300)):
            regions[g] = struct.pack('<I', o if r.random() < 0.6 else 0)
            regions[o] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[VT] = vtable({2: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'T26b0':
        table = bytearray(r.bytes(0x1450))
        for k in range(10):
            put(table, 0x208 * k, 'I', RAM + 0x2000 + 0x40 * k if r.random() < 0.6 else 0)
            put(table, 0x208 * k + 4, 'H', r.choice([0, 0, 1, 7]))
            regions[RAM + 0x2000 + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[0x7c26b0] = table
        regions[VT] = vtable({2: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'VB48':
        regions[0x8bba48] = struct.pack('<I', OBJ if r.random() < 0.7 else 0)
        put(obj, 0, 'I', VT)
        regions[VT] = bytearray(vtable({}))
        off = r.choice([0x40, 0x80])
        struct.pack_into('<I', regions[VT], 4, off)
        put(obj, off, 'I', VT2)
        regions[VT2] = vtable({0: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'Drain':
        nodes = bytearray(r.bytes(0x80))
        n = r.choice([0, 1, 2, 3])
        for k in range(n):
            put(nodes, 8 * k, 'II', RAM + 0x2000 + 0x40 * k, RAM + 0x100 + 8 * (k + 1) if k + 1 < n else 0)
            regions[RAM + 0x2000 + 0x40 * k] = struct.pack('<I', VT) + r.bytes(0x20)
        regions[RAM + 0x100] = nodes
        regions[VT] = vtable({2: 0, 1: 0})
        return Call(address, [RAM + 0x100 if n else 0], regions, conv=conv, ret=ret)
    if name == 'Rel':
        # the field is 0x54 into the owner, whose dword +4 points to an object of a class whose vtable's dword +4 is the base's offset
        own = bytearray(r.bytes(0x300))
        put(own, 8, 'i', r.choice([1, 2, 5, 0, -1]))
        put(own, 4, 'I', RAM + 0x600)
        off = r.choice([0x40, 0x80])
        put(own, 4 + off, 'I', VT2)
        held = bytearray(struct.pack('<I', VT) + r.bytes(0x20))
        regions.update({RAM + 0x3000: own, RAM + 0x600: held, VT: bytearray(vtable({})), VT2: vtable({7: 1})})
        struct.pack_into('<I', regions[VT], 4, off)
        return Call(address, [RAM + 0x3000 + 0x54], regions, conv=conv, ret=ret)
    if name == 'R1bc':
        regions[0x7ca1bc] = struct.pack('<I', OBJ if r.random() < 0.7 else 0)
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({2: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'M20':
        put(obj, 0x20, 'I', OBJ2)
        regions.update({OBJ2: struct.pack('<I', VT) + r.bytes(0x20), VT: vtable({5: 1})})
        return Call(address, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'GThing':   # (things in the slots 0-3 and 0x100-0x103 or none; the global one of them or another; +0x10 set or not, its low byte too)
        put(obj, 0, 'I', VT)
        put(obj, 0x10, 'I', r.choice([0, 1, 0x100, 0x10000, r.getrandbits(32)]))
        regions[VT] = vtable({5: 0})
        slots = list(range(4)) + list(range(0x100, 0x104))
        things = {k: RAM + 0x400 + 0x20 * i if r.random() < 0.8 else 0 for i, k in enumerate(slots)}
        table = bytearray(20 * 0x104)
        for k, t in things.items():
            table[20 * k:20 * k + 20] = struct.pack('<I', t) + r.bytes(16)
        regions[0x7cfb90] = table
        regions[0x7c2658] = struct.pack('<I', r.choice(list(things.values()) + [things[1], things[0x101], RAM + 0x400, r.getrandbits(32)]))
        handle = r.choice(slots + [1, 0x101]) | r.choice([0, 0, r.getrandbits(16)]) << 16
        return Call(address, [OBJ, handle], regions, conv=conv, ret=ret)
    if name == 'Keys':  # messages 0x10005 / 0x1000b and others, keys 0xd, 0x2500, 0x2700 and others
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({14: 0, 15: 0, 16: 0})
        msg = r.choice([0x10005, 0x10005, 0x1000b, 0x1000b, 0x1000b, 0x10006, 0x1000a, 0, r.getrandbits(32)])
        key = r.choice([0, 0, 0xd, 0x2500, 0x2700, 0x2600, 1, r.getrandbits(32)])
        return Call(address, [OBJ, msg, key, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'G2c':
        put(obj, 0, 'I', VT)
        regions[VT] = vtable({11: 0})
        g = r.choice([0, 5, 0x1234, r.getrandbits(32)])
        regions[0x7c2658] = struct.pack('<I', g)
        return Call(address, [OBJ, g if r.random() < 0.6 else r.choice([0, 5, 0x1234])], regions, conv=conv, ret=ret)
    sub = 0x14 if name == 'Sub14' else 0x28
    put(obj, sub, 'I', VT)
    regions[VT] = vtable({6: 1})
    return Call(address, [OBJ, r.getrandbits(32)], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
