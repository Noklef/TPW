# The methods of src/game/core/vobj3 that hand their members or list entries to virtual methods, against the originals: the objects they call have vtables of
# recorders (see tools/fncmp.py recorder()), so the comparison covers the slot, the object it is called on, the arguments (the log) and what is done with the result.
# usage: vobj3.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, fill, heap, log_region, put, recorder, set_return, strings, vtable  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Slot80': (0x5644c0, 't', 'a', 0x20), 'VidMem': (0x5645a0, 't', 'a', 0x40), 'Crel': (0x4ef7b0, 't', 'a', 0x40), 'Cref': (0x4ef790, 't', 'v', 0x20), 'Seven': (0x5640a0, 't', 'a', 0xa0),
         'Six': (0x58d380, 't', 'a', 0x80), 'Rem': (0x421690, 't', 'a', 0x90), 'Add': (0x576270, 't', 'a', 0xa0)}


def lock(e):  # the STL lock (0x691de0, thiscall, returns its argument; 0x691ea0 releases): the port leaves them out
    e.r['EAX'] = e.r['ECX']


B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x590730, 0x590750)], hooks={0x691de0: lock, 0x691ea0: lambda e: None})
OBJ, LST, NODES, OBJS, VT, VTA, VTM = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200


def case(r):
    name = r.deal(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x500))
    log = log_region()
    regions = {OBJ: obj, LOG: log, HEAP: heap()}

    def call(*args):
        return Call(address, [OBJ] + list(args), regions, conv=conv, ret=ret)

    if name == 'Slot80':
        put(obj, 0x14, 'I', RAM + 0x300)
        regions[RAM + 0x300] = struct.pack('<I', VTM) + r.bytes(0x20)
        regions[VTM] = vtable({}, size=48, com={32: 2})
        regions[0x879cd4] = r.bytes(4)
        set_return(log, 32, r.getrandbits(32))
        return call()
    if name == 'VidMem':   # GetAvailableVidMem (COM slot 23: this, caps, &total, &free): the stack pointers logged as 0, the outs filled
        put(obj, 8, 'I', RAM + 0x300)
        regions[RAM + 0x300] = struct.pack('<I', VTM) + r.bytes(0x20)
        regions[VTM] = vtable({}, size=48, com={23: 4})
        strings(log, 23, 0, out=0b1110)
        fill(log, 23, 3, r.bytes(4))
        fill(log, 23, 2, r.bytes(4), which=1)
        set_return(log, 23, r.choice([0, 0x887600e1]))
        return call()
    if name in ('Crel', 'Cref'):
        counted = RAM + 0x200
        regions[counted] = struct.pack('<II', VT, r.choice([0, 1, 1, 2, 3, 0xffffffff, 0xfffffffe])) + r.bytes(0x20)
        regions[VT] = vtable({0: 1})
        if name == 'Crel':
            return Call(address, [counted], regions, conv=conv, ret=ret)
        put(obj, 0, 'I', counted if r.random() < 0.85 else 0)
        return call()
    if name == 'Seven':
        obj = bytearray(r.bytes(0x800))
        regions[OBJ] = obj
        regions[VTM] = vtable({}, size=48, com={2: 1, 19: 1})
        put(obj, 0x780, 'B', r.choice([0, 1, 2, 3, 0xff]))
        for k, field in enumerate((0, 4, 8, 0xc, 0x10, 0x14, 0x18)):
            at = RAM + 0x800 + 0x40 * k
            put(obj, field, 'I', at if r.random() < 0.7 else 0)
            regions[at] = struct.pack('<I', VTM) + r.bytes(0x20)
        return call()
    if name == 'Six':
        obj = bytearray(r.bytes(0x1c00))
        regions[OBJ] = obj
        regions[VT] = vtable({9: 1})
        for g in range(6):
            n = r.choice([0, 1, 2, 3, 3, -1])
            put(obj, 0x1800 + 4 * g, 'i', n)
            for i in range(3):
                idx = 3 * g + i
                rec = RAM + 0x4000 + 0x20 * idx
                put(obj, 0x400 * g + 4 * i, 'I', rec if r.random() < 0.85 else 0)
                held = RAM + 0x6000 + 0x40 * idx
                regions[rec] = struct.pack('<I', held if r.random() < 0.8 else 0) + r.bytes(0x1c)
                regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
        return call()
    if name == 'Rem':
        n = r.choice([0, 1, 2, 3])
        lst = bytearray(r.bytes(0x20))
        put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
        put(lst, 0xc, 'I', n)
        nodes = bytearray(r.bytes(0x60))
        for k in range(n):
            held = OBJS + 0x40 * k
            put(nodes, 0x10 * k, 'III', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
            regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
        target = OBJS + 0x40 * r.randrange(n) if n and r.random() < 0.8 else r.choice([0, OBJS + 0x300, 0x1234])
        put(obj, 0, 'I', LST if r.random() < 0.92 else 0)
        put(obj, 4, 'I', target if r.random() < 0.5 else r.choice([0, 0x777]))
        regions.update({LST: lst, NODES: nodes, VT: vtable({0: 1})})
        return call(target)
    # Add: a pool with a free node (or none), a head variable, listed objects of two classes (slot 2 for one, 3 for the other)
    n = r.choice([0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x60))
    classes = [r.choice([VT, VTA]) for _ in range(n + 1)]
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'II', held, NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        regions[held] = struct.pack('<I', classes[k]) + r.bytes(0x20)
    new = OBJS + 0x40 * r.randrange(n) if n and r.random() < 0.2 else OBJS + 0x300
    if new == OBJS + 0x300:
        regions[new] = struct.pack('<I', classes[n]) + r.bytes(0x20)
    vta = bytearray(vtable({}, size=8))
    struct.pack_into('<I', vta, 8, recorder(3, 0))
    regions.update({NODES: nodes, VT: vtable({}, size=8), VTA: vta, RAM + 0x600: struct.pack('<I', NODES if n else 0), RAM + 0x5000: r.bytes(0x20),
                    0x8bd508: r.bytes(4)})
    put(obj, 4, 'I', RAM + 0x5000 if r.random() < 0.85 else 0)
    for k, v in {2: r.choice([1, 1, 0, 5]), 3: r.choice([1, 1, 0, 5])}.items():
        if r.random() < 0.7:
            set_return(log, k, v)
    return call(RAM + 0x600, new)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
