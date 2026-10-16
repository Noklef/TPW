# The members-to-virtual-methods functions of src/game/core/vobj2 against the originals: the objects they call have vtables of recorders (see tools/fncmp.py
# recorder()), so the comparison covers the slot, the object it is called on, the arguments (the log) and what is done with the result; the lists are of varied
# length, counted objects at the edge counts, a global vtable pointer at 0x7021f0 stands for a class vtable.
# usage: vobj2.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, set_return, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Rel': (0x47e460, 't', 'v', 0x50), 'Five': (0x5e4140, 't', 'v', 0x80), 'Drain': (0x421520, 't', 'v', 0x70), 'Any': (0x421720, 't', 'a', 0x50),
         'Words': (0x5b7990, 't', 'v', 0x60), 'D14': (0x5cea40, 't', 'v', 0x50), 'D4f': (0x4f0650, 't', 'v', 0x50), 'Mark': (0x579c80, 't', 'a', 0x50),
         'Del': (0x58c370, 't', 'a', 0x50), 'Pick': (0x5e1fa0, 't', 'v', 0x70), 'D2da': (0x5e4020, 't', 'v', 0x60)}


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x67df40, 0x67e040), (0x67faf0, 0x67fb20), (0x682720, 0x682780)],
          hooks={0x691de0: lock, 0x691ea0: lambda e: None})
OBJ, LST, NODES, OBJS, VT, VTL, VTM = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
NAMES = [b'Sloth', b'sloth', b'SLOTH', b'Slot', b'Sloths', b'', b'b']


def varied(r, base):
    s = bytearray(base)
    for i in range(len(s)):
        if r.random() < 0.3:
            s[i] ^= 0x20
    return bytes(s)


def make_list(r, regions, n, nulls=0.0, count=None):
    """A list head at LST (vtable VTL, head, tail, count) of n nodes {object, next, prev} whose objects have the vtable VT."""
    lst = bytearray(r.bytes(0x20))
    put(lst, 0, 'I', VTL)
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n if count is None else count)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held if r.random() >= nulls else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({LST: lst, NODES: nodes})


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x500))
    log = log_region()
    regions = {OBJ: obj, LOG: log, HEAP: heap()}

    def call(*args):
        return Call(address, [OBJ] + list(args), regions, conv=conv, ret=ret)

    if name == 'Rel':
        regions[VTM] = vtable({}, size=48, com={2: 1})
        for field, at in ((0xac, RAM + 0x300), (0xb0, RAM + 0x340)):
            put(obj, field, 'I', at if r.random() < 0.7 else 0)
            regions[at] = struct.pack('<I', VTM) + r.bytes(0x20)
        return call()
    if name == 'Five':
        n = r.choice([0, 1, 2, 3])
        make_list(r, regions, n)
        put(obj, 0x18, 'I', LST)
        regions[VT] = vtable({4: 0, 0: 1})
        regions[VTL] = vtable({0: 1})
        for k, field in enumerate((4, 8, 0xc, 0x10, 0x14)):
            at = OBJS + 0x400 + 0x40 * k
            put(obj, field, 'I', at if r.random() < 0.7 else 0)
            regions[at] = struct.pack('<I', VT) + r.bytes(0x20)
        return call()
    if name in ('Drain', 'Any'):
        n = r.choice([0, 1, 2, 3])
        make_list(r, regions, n, nulls=0.15 if name == 'Drain' else 0.0)
        put(obj, 0, 'I', LST if r.random() < 0.9 else 0)
        regions[VTL] = vtable({0: 1})
        if name == 'Drain':
            regions[VT] = vtable({0: 1})
            return call()
        regions[VT] = vtable({2: 3})
        if r.random() < 0.7:
            set_return(log, 2, r.choice([1, 1, 0, 5]))
        return call(r.getrandbits(32), r.getrandbits(32))
    if name == 'Words':
        regions[VT] = vtable({5: 1})
        put(obj, 0x10c, 'I', VT)
        regions[RAM + 0x2000] = r.bytes(0x200)
        return call(r.choice([0, 1, 7, 0x80000000]), r.choice([0, 0, RAM + 0x2400, RAM + 0x2410]), RAM + 0x2000)
    if name in ('D14', 'D4f'):
        put(obj, 0x34, 'I', RAM + 0x200 if r.random() < 0.85 else 0)
        regions[RAM + 0x200] = struct.pack('<II', VT, r.choice([0, 1, 1, 2, 3, 0xffffffff, 0xfffffffe])) + r.bytes(0x20)
        regions[VT] = vtable({0: 1})
        return call()
    if name == 'Mark':
        n = r.choice([0, 1, 2, 3, -1])
        put(obj, 0x400, 'i', n)
        regions[VTL] = vtable({}, size=4)
        for i in range(3):
            thing, off = OBJS + 0x100 * i, r.choice([0x20, 0x40, 0x60])
            put(obj, 4 * i, 'I', thing)
            vth = bytearray(r.bytes(0x10))
            put(vth, 4, 'I', off)
            regions[VT + 0x40 * i] = vth
            body = bytearray(r.bytes(0x100))
            put(body, 0, 'I', VT + 0x40 * i)
            put(body, off, 'I', VTM)
            regions[thing] = body
        regions[VTM] = vtable({1: 0})
        return call()
    if name == 'Del':
        off = r.choice([0x40, 0x60])
        vto = bytearray(r.bytes(0x10))
        put(vto, 4, 'I', off)
        put(obj, 0, 'I', VT)
        regions[VT] = vto
        regions[0x7021f0] = vtable({1: 0}, size=8)
        return Call(address, [OBJ + 0x3c, r.choice([0, 1, 2, 3, 0xffffff00, 0xffffffff])], regions, conv=conv, ret=ret)
    if name == 'Pick':
        n = r.choice([1, 1, 2, 3, 4])
        lst = bytearray(r.bytes(0x20))
        nodes = bytearray(r.bytes(0x60))
        put(lst, 4, 'I', NODES)
        wanted = r.choice(NAMES)
        for k in range(n):
            entry = OBJS + 0x80 * k
            put(nodes, 0x10 * k, 'II', entry, NODES + 0x10 * (k + 1) if k + 1 < n else 0)
            e = bytearray(r.bytes(0x40))
            put(e, 0x30, 'I', RAM + 0x2800 + 0x20 * k)
            regions[entry] = e
            regions[RAM + 0x2800 + 0x20 * k] = varied(r, wanted if r.random() < 0.5 else r.choice(NAMES)) + bytes(0x10)
        put(obj, 8, 'I', LST)
        glob = bytearray(r.bytes(0x20))  # (the object at 0xf85480 and, in its tail, the name pointer at 0xf85488)
        put(glob, 0, 'I', VT)
        put(glob, 8, 'I', RAM + 0x2c00)
        regions.update({LST: lst, NODES: nodes, RAM + 0x2c00: wanted + bytes(0x10), 0xf85480: glob, VT: vtable({5: 1})})
        return call(r.choice([0, 0, 0, 1, 2]))
    # D2da: a chain of nodes (next at +4), a global object told about the object
    n = r.choice([0, 1, 2, 3])
    put(obj, 0xc, 'I', RAM + 0x1000 if n else 0)
    chain = bytearray(r.bytes(0x40))
    for k in range(n):
        put(chain, 0x10 * k + 4, 'I', RAM + 0x1000 + 0x10 * (k + 1) if k + 1 < n else 0)
    regions[RAM + 0x1000] = chain
    regions[0xf85614] = struct.pack('<I', RAM + 0x300)
    regions[RAM + 0x300] = struct.pack('<I', VT) + r.bytes(0x20)
    regions[VT] = vtable({7: 1})
    return call()


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
