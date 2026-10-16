# The objects' definition lookups, counters and setters (src/game/world/object: 0x4dd4e0, 0x4e1670,
# 0x4e1690, 0x4e1e00, 0x4e2190, 0x4e21c0, 0x4e2710, 0x4e2b50) against the originals: ids found in
# gObjectDefs or its cache (the pushed high word anything), counts and averages about the wraps.
# usage: object.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=GetDef,AddCustomer,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4dd4e0, 0x4dd4fb), (0x4e1670, 0x4e168b), (0x4e1690, 0x4e16ab), (0x4e1e00, 0x4e1e2b),
           (0x4e2190, 0x4e21a2), (0x4e21c0, 0x4e21d2), (0x4e2710, 0x4e273b), (0x4e2b50, 0x4e2b7b),
           (0x412e90, 0x412eea)])
DEFS, OBJ, ITEMS, NITEMS = 0x7890a0, RAM, RAM + 0x400, 16
CALLS = {'GetDef': (0x4dd4e0, 'f', 'a'), 'AddWalkAway': (0x4e1670, 'f', 'v'), 'AddCustomer': (0x4e1690, 'f', 'v'),
         'Happiness': (0x4e1e00, 't', 'v'), 'Quality': (0x4e2190, 't', 'v'), 'Chance': (0x4e21c0, 't', 'v'),
         'Word284': (0x4e2710, 't', 'a'), 'IsKind3': (0x4e2b50, 'f', 'a')}


def word(r):
    return r.choice([0, 1, 0x7fffffff, 0x80000000, 0xffffffff, r.randrange(256), r.getrandbits(32)])


def case(r):
    op = r.choice(OPS)
    addr, conv, ret = CALLS[op]
    obj = bytearray(r.bytes(0x400))
    for off in (0x1a0, 0x1a4, 0x1a8, 0x230, 0x340):
        put(obj, off, 'I', word(r))
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(NITEMS * 0x300))
    n = r.randrange(1, NITEMS + 1)
    ids = [r.choice([i, r.getrandbits(16)]) for i in range(NITEMS)]
    for i in range(NITEMS):
        put(defs, i * 4, 'I', ITEMS + 0x300 * i)
        put(items, i * 0x300, 'H', ids[i])
        put(items, i * 0x300 + 0x9c, 'I', r.choice([3, 3, 0, 2, 0x103, r.getrandbits(32)]))
    put(defs, 0x528, 'i', n)
    k = r.randrange(n)
    id_ = ids[k]
    hit = r.random() < 0.3
    put(defs, 0x730, 'HxxI', id_ if hit else id_ ^ r.randrange(1, 0x10000),
        ITEMS + 0x300 * r.randrange(NITEMS))
    put(obj, 0xe, 'H', id_)
    args = [OBJ]
    if op in ('Happiness', 'Quality', 'Chance'):
        args.append(word(r))
    elif op == 'Word284':
        args.append(OBJ + 0x3f0 + r.randrange(4))
    return Call(addr, args, {DEFS: defs, OBJ: obj, ITEMS: items}, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
