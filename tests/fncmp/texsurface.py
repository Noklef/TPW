# The texture cache surface getters (src/game/gfx/texsurface: 0x58b3e0, 0x58c610, 0x58c2c0, 0x58c2e0, 0x58c330, 0x58c8a0,
# 0x58c8b0) and the renderers' surface factories (0x5738d0, 0x573e70) against the originals: any object bytes, the vbtable pointers set to a table whose
# offset reaches back and forward; the constructors most derived or not, over vtables with usual and
# odd base offsets (0 included: the stores then reach the vtable pointer); the factories on a heap whose allocation fails now and then.
# Class A's surface release (0x58b790, and through its thunk 0x58c2a0): counts 0-5 and the two arrays of COM objects (a recorder's slot 2) or nulls.
# usage: texsurface.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x58b300, 0x58b330), (0x58b3e0, 0x58b464), (0x58c610, 0x58c66a), (0x58c2c0, 0x58c2c4), (0x58b790, 0x58b805), (0x58c2a0, 0x58c2a8), (0x58c2e0, 0x58c2f2), (0x58c330, 0x58c334), (0x58c8a0, 0x58c8a4), (0x58c8b0, 0x58c8c2), (0x5738d0, 0x57393c), (0x573e70, 0x573edc), (0x67acc0, 0x67acc1)])
CALLS = {'CtorA': 0x58b3e0, 'CtorB': 0x58c610, 'A38': 0x58c2c0, 'AArea': 0x58c2e0, 'A10': 0x58c330, 'BC': 0x58c8a0, 'BArea': 0x58c8b0, 'NewA': 0x5738d0, 'NewB': 0x573e70, 'ARelease': 0x58b790, 'WRelease': 0x58c2a0}
SUB = RAM + 0x100
VB = RAM + 0x1000
COM, VTC = RAM + 0x400, RAM + 0x800


def release(r, op):
    obj = bytearray(r.bytes(0x200))
    for k in range(12):   # ECX - 0x38 .. - 0x14: the two arrays (with the dwords past them the larger counts reach)
        put(obj, 0x100 - 0x38 + 4 * k, 'I', COM + 0x10 * k if r.random() < 0.6 else 0)
    put(obj, 0x100 - 0x10, 'I', r.choice([0, 1, 2, 3, 4, 5]))
    regions = {RAM: obj, COM: b''.join(struct.pack('<I', VTC) + r.bytes(0xc) for _ in range(12)), VTC: vtable({}, com={2: 1}), LOG: log_region()}
    if op == 'ARelease':
        return Call(0x58b790, [SUB], regions, conv='t', ret='a')
    adj = r.choice([0x40, 0x50, 0x80])
    put(obj, 0x100 + adj - 4, 'I', adj)
    return Call(0x58c2a0, [SUB + adj], regions, conv='t', ret='a')


def case(r):
    op = OPS[r.deal(range(len(OPS)))]
    if op.endswith('Release'):
        return release(r, op)
    if op.startswith('New'):
        return Call(CALLS[op], [RAM], {RAM: r.bytes(0x10), HEAP: heap(0x100, 1 if r.random() < 0.2 else 0)}, conv='t', ret='a')
    if op.startswith('Ctor'):
        a = op == 'CtorA'
        md = r.choice([0, 1, 1])
        obj = bytearray(r.bytes(0x200))
        off = r.choice([0x3c, 0x3c, 0x40, 0x50, 0] if a else [0x10, 0x10, 0x14, 0x20, 0])
        put(obj, 0, 'I', VB)
        return Call(CALLS[op], [RAM, md], {RAM: obj, VB: struct.pack('<ii', 0, off)}, conv='t', ret='a')
    obj = bytearray(r.bytes(0x200))
    put(obj, 0x100 - 0x3c, 'I', VB)
    put(obj, 0x100 - 0x10, 'I', VB)
    vb = struct.pack('<ii', 0, r.choice([0, 0x10, 0x30, 0x38, 0x3c, 0x40, 0x50, -0x10, 0x80]))
    return Call(CALLS[op], [SUB], {RAM: obj, VB: vb}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
