# The texture cache's item unlinking (src/game/gfx/texcache: 0x58dc50) against the original: the cache's flag 0x20000000 set or not,
# a null item now and then, its flag 0x1000 set or not, its owner's list (next at +4) of 0-4 items with it at any place or missing,
# the words at +0x10 and +0x12 at and around the multiples of 8 and the signed edges, the float at +0x30 around 0..1.5 or any bits,
# flags of any bits (the 0x80000 one often). And an item's key (0x58cb40): the same words at +0x14 and +0x16, float and flags (0x4000 and
# 0x80000 often).
# usage: texcacheunlink.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unlink,Key)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x58dc50, 0x58dd5e), (0x58cb40, 0x58cb92)])
CACHE, ITEM, OWNER, NODES = RAM, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200


def word(r):
    return r.choice([0, 1, 7, 8, 9, 0x7ff8, 0x7ff9, 0x7fff, 0xfff8, 0xfff9, 0xffff, r.getrandbits(16), r.randrange(0, 0x400)])


def key(r):
    flags = r.getrandbits(32)
    for bit in (0x4000, 0x80000):
        flags = flags | bit if r.random() < 0.5 else flags & ~bit
    item = bytearray(r.bytes(0x40))
    put(item, 8, 'I', flags)
    put(item, 0x14, 'HH', word(r), word(r))
    put(item, 0x30, 'I', r.choice([fb(r.uniform(0, 1.5)), fb(r.choice([0.0, 1.5, 1.0, -1.0, 3.0, 1e9, -1e9])), r.f()]))
    return Call(0x58cb40, [ITEM], {ITEM: item}, conv='t', ret='v')


def case(r):
    if r.deal(OPS) == 'Key':
        return key(r)
    cache = put(bytearray(r.bytes(0x1900)), 0x18cc, 'I', r.getrandbits(32) | 0x20000000 if r.random() < 0.9 else r.getrandbits(32) & ~0x20000000)
    flags = r.getrandbits(32)
    flags = flags | 0x1000 if r.random() < 0.9 else flags & ~0x1000
    if r.random() < 0.5:
        flags |= 0x80000
    item = bytearray(r.bytes(0x40))
    put(item, 8, 'I', flags)
    put(item, 0x10, 'HH', word(r), word(r))
    put(item, 0x18, 'I', OWNER)
    put(item, 0x30, 'I', r.choice([fb(r.uniform(0, 1.5)), fb(r.choice([0.0, 1.5, 1.0, -1.0, 3.0])), r.f()]))
    count = r.choice([0, 1, 2, 3, 4])
    nodes = [NODES + 0x40 * k for k in range(count)]
    if r.random() < 0.85:   # the item in the list (at any place), or missing
        nodes.insert(r.randrange(len(nodes) + 1), ITEM)
    regions = {CACHE: cache, NODES: r.bytes(0x100)}
    links = nodes[1:] + [0]
    put(item, 4, 'I', r.getrandbits(32))
    for node, link in zip(nodes, links):
        if node == ITEM:
            put(item, 4, 'I', link)
        else:
            regions[NODES] = put(bytearray(regions[NODES]), node - NODES + 4, 'I', link)
    regions[OWNER] = put(bytearray(r.bytes(0x10)), 4, 'I', nodes[0] if nodes else 0)
    regions[ITEM] = item
    return Call(0x58dc50, [CACHE, 0 if r.random() < 0.05 else ITEM], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Unlink,Key').split(',') if k]

if __name__ == '__main__':
    B.main(case)
