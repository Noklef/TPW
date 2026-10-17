# The texture cache surface getters (src/game/gfx/texsurface: 0x58b3e0, 0x58c610, 0x58c2c0, 0x58c2e0, 0x58c330, 0x58c8a0,
# 0x58c8b0) and the renderers' surface factories (0x5738d0, 0x573e70) against the originals: any object bytes, the vbtable pointers set to a table whose
# offset reaches back and forward; the constructors most derived or not, over vtables with usual and
# odd base offsets (0 included: the stores then reach the vtable pointer); the factories on a heap whose allocation fails now and then.
# usage: texsurface.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x58b300, 0x58b330), (0x58b3e0, 0x58b464), (0x58c610, 0x58c66a), (0x58c2c0, 0x58c2c4), (0x58c2e0, 0x58c2f2), (0x58c330, 0x58c334), (0x58c8a0, 0x58c8a4), (0x58c8b0, 0x58c8c2), (0x5738d0, 0x57393c), (0x573e70, 0x573edc), (0x67acc0, 0x67acc1)])
CALLS = {'CtorA': 0x58b3e0, 'CtorB': 0x58c610, 'A38': 0x58c2c0, 'AArea': 0x58c2e0, 'A10': 0x58c330, 'BC': 0x58c8a0, 'BArea': 0x58c8b0, 'NewA': 0x5738d0, 'NewB': 0x573e70}
SUB = RAM + 0x100
VB = RAM + 0x1000


def case(r):
    op = r.choice(list(CALLS))
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


if __name__ == '__main__':
    B.main(case)
