# The sweeps over the things of src/game/world/ridesweep (0x482040-0x482400) against the originals: a park of things (thingenv.py) whose objects have definitions of every kind,
# definition words and upgrade types, and flag bytes; the callback is the driver's logging stub (the objects it is called with end up in the log).
# Things_ForEachSelected (0x481bc0): a mask of random groups over a park of staff, model 1 and object things, the staff split among the five
# lists (heads in the world [0x7cf83c] +0x1da73c, chained through +0x210) or in none.
# usage: ridesweep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, CDECL_LOG, Call, LOG, log_region, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

# name: (address, with a kind argument)
CALLS = {'Kind': (0x482040, True), 'Word': (0x482100, False), 'Up': (0x4821b0, False), 'F1': (0x482270, False), 'F0': (0x482340, False), 'F2': (0x482400, False), 'Select': (0x481bc0, False), 'Count': (0x48ca00, False), 'One': (0x48cad0, False)}
SWEEP_RANGES = ([(a, a + 0xd0) for (a, _k) in CALLS.values()] + [(0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360), (0x412e90, 0x412eea), (0x4dd4e0, 0x4dd500),
          (0x4e2710, 0x4e273a), (0x5da3c0, 0x5da3c1), (0x481bc0, 0x481f7e), (0x481f80, 0x482040), (0x4cd300, 0x4cd331),
          (0x48ca00, 0x48caac), (0x48cad0, 0x48cae1)])
B = Bench(SWEEP_RANGES)
OWNER, COUNTS = RAM + 0x21000, 0x7c2658   # the count owner (its +4 the group); 0x7c2658 the value, 0x7c2660 the count
GROUPS = [-3, -2, -1, 0, 1, 2, 3, 4, 0x3e9, 0x3ea, 0x3eb, 0x3ec, 0x3ed]
HEADS = RAM + 0x20000


def kinds(r, env):
    """The park's definitions given kinds (+0x4ac) 1 and 2 as well (thingenv's are 0, 3 and 5), their words +0x284 more often 0."""
    items = env[0][ITEMS]
    for i in range(NITEMS):
        put(items, i * ITEM + 0x4ac, 'I', r.choice([0, 1, 2, 3, 3, 5]))
        put(items, i * ITEM + 0x284, 'H', r.choice([0, 0, 0, 1, 5]))
    return env


def select(r, count=False, pool=POOL):
    regions, handles, addrs = kinds(r, park(r, count=r.choice([0, 2, 3, 4, 6, 6]), kinds=(3, 3, 3, 3, 3, 1, 4, 5, 6, 7, 8, 0xff, 0), defs=True, pool=pool))
    regions[LOG] = log_region()
    lists = [[] for _ in range(5)]
    for i in range(len(handles)):
        k = r.randrange(6)
        if k < 5:
            lists[k].append(i)
    heads = [0] * 5
    pool = regions[pool]
    for k, members in enumerate(lists):
        r.shuffle(members)
        for j, i in enumerate(members):
            struct.pack_into('<H', pool, THING * i + 0x210, handles[members[j + 1]] if j + 1 < len(members) else 0)
        heads[k] = handles[members[0]] if members else 0
    regions[0x7cf83c] = struct.pack('<I', HEADS - 0x1da73c)
    regions[HEADS] = struct.pack('<5H', *heads) + r.bytes(6)
    if count:   # Things_CountGroup: the group the owner's +4 picks counted (into 0x7c2660)
        regions[OWNER] = put(bytearray(r.bytes(8)), 4, 'i', r.choice(GROUPS + [r.getrandbits(32) - (1 << 31)]))
        regions[COUNTS] = r.bytes(0x10)
        return Call(0x48ca00, [OWNER, r.getrandbits(32)], regions, conv='t', ret='a')
    mask = r.choice([0xffffffff, 0xffff, r.getrandbits(16), r.getrandbits(16), 1 << r.randrange(16), r.getrandbits(8), 0])
    return Call(0x481bc0, [mask, CDECL_LOG], regions, ret='a')


def case(r):
    name = r.choice(OPS)
    if name == 'Select':
        return select(r)
    if name == 'Count':
        return select(r, count=True)
    if name == 'One':   # Things_CountOne: the count raised
        return Call(0x48cad0, [r.getrandbits(32)], {COUNTS: r.bytes(0x10)}, ret='a')
    address, with_kind = CALLS[name]
    regions, handles, addrs = kinds(r, park(r, count=r.choice([0, 1, 2, 3, 4, 6]), defs=True))
    regions[LOG] = log_region()
    args = ([r.choice([0, 1, 3, 3, 5, 0x7fffffff])] if with_kind else []) + [CDECL_LOG]
    return Call(address, args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
