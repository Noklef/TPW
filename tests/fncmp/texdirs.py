# The texture directories (src/game/gfx/texdirs: 0x46e610-0x46ea80) against the originals:
# the setter's paths built (sprintf, run by the driver's stand-in) and compared with the old
# table's strings, the new block chained (global_heap()); the first path, the current pointer
# and the reset freeing the chain.
# usage: texdirs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x46e610, 0x46e627), (0x46e630, 0x46e636), (0x46e640, 0x46e64a), (0x46e670, 0x46ea7b),
           (0x46ea80, 0x46eacb), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30), (0x67a400, 0x690000)])
STRINGS, TABLE, CHAIN = RAM, RAM + 0x800, RAM + 0x900
BASES = [b'', b'C:\\TP\\', b'data\\', b'x', b'..\\levels\\']


def text(r):
    return bytes(r.choice(b'ab\\.') for _ in range(r.choice([0, 1, 2, r.randrange(30)])))


def case(r):
    op = r.choice(OPS)
    ram = bytearray(r.bytes(0xa00))
    count = r.choice([0, 1, 3, 4, r.getrandbits(32)])
    globals_ = bytearray(r.bytes(0x14))
    put(globals_, 8, 'I', r.getrandbits(32))  # the chain (only the reset follows it)
    put(globals_, 0xc, 'I', TABLE)
    put(globals_, 0x10, 'I', count)
    indices = bytearray(r.bytes(12))
    for k in range(3):
        put(indices, 4 * k, 'i', r.choice([-1, 0, 1, 2, 3, 3]))
    regions = {RAM: ram, 0x7b0620: globals_, 0x74d980: indices, HEAP: heap(0x2000), **global_heap()}
    if op == 'Set':
        at = STRINGS
        base = r.choice(BASES)
        names = [r.choice([None, text(r), text(r) + b'\\', b'']) for _ in range(3)]
        ptrs = []
        for s in [base] + names:
            if s is None:
                ptrs.append(0)
                continue
            if s == b'':
                put(ram, at - RAM, 'B', r.choice([0x5c, 0x41]))  # the byte before an empty name
                at += 1
            p = at
            put(ram, at - RAM, '%ds' % (len(s) + 1), s + b'\0')
            at += len(s) + 1
            ptrs.append(p)
        olds = []
        for k in range(4):  # the old table: often the very path a name builds
            name = names[r.randrange(3)]
            s = base + (name or b'') + (b'' if name and name.endswith(b'\\') else b'\\')
            s = r.choice([s, s, text(r), base + text(r)])
            put(ram, at - RAM, '%ds' % (len(s) + 1), s + b'\0')
            olds.append(at)
            at += len(s) + 1
        put(ram, TABLE - RAM, '4I', *olds)
        if r.random() < 0.1:  # no table yet (only read through an index)
            put(globals_, 0xc, 'I', 0)
            put(indices, 0, '3i', -1, -1, -1)
        return Call(0x46e670, ptrs, regions)
    if op == 'Reset':
        n = r.choice([0, 1, 2, 4])
        put(globals_, 8, 'I', CHAIN if n else 0)
        for i in range(n):
            put(ram, CHAIN - RAM + 0x10 * i, 'I', CHAIN + 0x10 * (i + 1) if i + 1 < n else 0)
        return Call(0x46ea80, [], regions)
    if op == 'SetCurrent':
        return Call(0x46e640, [r.getrandbits(32)], regions)
    return Call(0x46e610 if op == 'First' else 0x46e630, [], regions, ret='a')


OPS_ALL = ['Set', 'Set', 'Set', 'Reset', 'First', 'Current', 'SetCurrent']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
