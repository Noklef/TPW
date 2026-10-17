# The live objects list (src/game/core/liveobjects: 0x5775d0, 0x577600, and the constructor 0x579cd0 and deleting destructor 0x57ca00
# that use it) against the originals: lists from empty to full (and beyond), objects present once, twice or not at all; objects most
# derived or not, their virtual base tables the exe's or made up (other offsets, inside the object, one rewriting the table), any
# flags (the heap region given, whose free logs the pointer).
# usage: liveobjects.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Add,Remove,Construct,Delete)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, heap, put  # noqa: E402

B = Bench([(0x5775d0, 0x5775f3), (0x577600, 0x57764b), (0x579cd0, 0x579df1), (0x57ca00, 0x57ca5a)])
CALLS = {'Add': 0x5775d0, 'Remove': 0x577600, 'Construct': 0x579cd0, 'Delete': 0x57ca00}
OBJECT, TABLE = 0x30001000, 0x30002000
LIST = 0x87a6d8


def case(r):
    op = r.deal(OPS)
    n = r.choice([0, 1, 2, 5, 0x1ff, 0x200, 0x201, r.randrange(0x200)] + ([-1] if op != 'Add' else []))
    objs = [r.choice([0x1000, 0x2000, 0x3000, r.getrandbits(32)]) for _ in range(0x200)]
    mem = bytearray(0x808)
    for i, o in enumerate(objs):
        put(mem, i * 4, 'I', o)
    put(mem, 0x800, 'I', n & 0xffffffff)
    if op in ('Construct', 'Delete'):
        base_offset = 0x154 if op == 'Construct' else 0xa4
        offset = r.choice([base_offset, base_offset, 0x100, 0x40, 0x8])
        regions = {LIST: mem, OBJECT: r.bytes(0x200), TABLE: put(bytearray(r.bytes(8)), 4, 'I', offset), HEAP: heap()}
        table = TABLE
        if op == 'Construct' and r.random() < 0.3:
            table = 0x701a08
        elif r.random() < 0.2:  # the table inside the object, its offset where the dword before the base goes (so it is read again)
            table, offset = OBJECT + 0x100, 0x108
            put(regions[OBJECT], 0x104, 'I', offset)
        put(regions[OBJECT], 0, 'I', table)
        if r.random() < 0.3:  # the base already listed (a destructor finds it)
            put(mem, 4 * r.randrange(0x200), 'I', OBJECT + offset)
        if op == 'Construct':
            return Call(0x579cd0, [OBJECT, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
        return Call(0x57ca00, [OBJECT + 0xa4, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')
    obj = r.choice([0x1000, 0x2000, 0x3000, objs[r.randrange(0x200)], r.getrandbits(32)])
    return Call(CALLS[op], [obj], {LIST: mem}, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
