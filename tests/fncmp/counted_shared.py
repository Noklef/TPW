# The counted-object helpers and constructors of src/game/core/small22 against the originals: use counts at the edges (0, 1, -2, -1), ids set or not, any object
# bytes (the allocator is the heap region; the STL lock is left out of both).
# usage: small22.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'AddRef': (0x4f5d10, 't', 'v', 0x30), 'Sh20': (0x4ef7f0, 't', 'a', 0x40), 'Sh34': (0x5d0dc0, 't', 'a', 0x40), 'Id': (0x4f21a0, 't', 'a', 0x30),
         'Init': (0x4f1520, 't', 'a', 0x40), 'Copy': (0x4f5d40, 't', 'a', 0x20), 'Node': (0x5b3110, 's', 'a', 0x20), 'Sent': (0x618130, 't', 'a', 0x30),
         'Name': (0x5b9020, 't', 'a', 0x20), 'G44': (0x4f7970, 'c', 'v', 0xa0), 'Gf8': (0x5d15d0, 'c', 'v', 0xa0)}


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases)
    e.r['EAX'] = e.r['ECX']


B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)], hooks={0x691de0: lock, 0x691ea0: lambda e: None})
COUNTS = [0, 1, 2, 5, 0xfffffffe, 0xffffffff, 0x7fffffff, 0x80000000]


def shared_count(r):
    return r.choice([0, 1, 1, 2, 3, 0xffffffff, 0xfffffffe])


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {RAM: obj, HEAP: heap()}
    shared = bytearray(r.bytes(0x20))
    put(shared, 4, 'I', r.choice(COUNTS))
    regions[RAM + 0x200] = shared
    if name in ('G44', 'Gf8'):  # the counted object the global points to (its slot 0 is a recorder)
        regions[LOG] = log_region()
        regions[0x7ced44 if name == 'G44' else 0xf827dc] = struct.pack('<I', RAM + 0x200) if r.random() < 0.95 else bytes(4)
        regions[RAM + 0x200] = struct.pack('<II', RAM + 0x1000, shared_count(r)) + r.bytes(0x20)
        regions[RAM + 0x1000] = vtable({0: 1})
        return Call(address, [], regions, conv=conv, ret=ret)
    if name == 'AddRef':
        return Call(address, [RAM + 0x200], regions, conv=conv, ret=ret)
    if name in ('Sh20', 'Sh34'):
        put(obj, 0x20 if name == 'Sh20' else 0x34, 'I', RAM + 0x200)
        regions[RAM + 0x300] = r.bytes(0x10)
        return Call(address, [RAM, RAM + 0x300], regions, conv=conv, ret=ret)
    if name == 'Id':
        regions[0xfb1dd0] = r.bytes(0x10)
        put(obj, 0, 'I', r.choice([0, 0, 1, 0x1234, 0xffffffff]))
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    if name == 'Init':
        regions[0x7099f0] = r.bytes(0x10)
        regions[0xfb34b0] = r.bytes(0x10)
        return Call(address, [RAM, r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Copy':
        put(obj, 0x40, 'I', RAM + 0x200)
        return Call(address, [RAM, RAM + 0x40], regions, conv=conv, ret=ret)
    if name == 'Node':
        return Call(address, [r.getrandbits(32), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Sent':
        return Call(address, [RAM, RAM + 0x40], regions, conv=conv, ret=ret)
    # Name: a wide text (or none) at +0x54, a string object not yet constructed
    n = r.choice([0, 1, 2, 5, 9, 17])
    text = struct.pack('<%dH' % (n + 1), *([r.choice([0x61, 0x62, 0x1234, 0xffff, 0x41]) for _ in range(n)] + [0]))
    regions[RAM + 0x1000] = text + r.bytes(0x10)
    put(obj, 0x54, 'I', RAM + 0x1000 if r.random() < 0.85 else 0)
    regions[RAM + 0x400] = r.bytes(0x20)
    return Call(address, [RAM, RAM + 0x400], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
