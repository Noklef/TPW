# The observer lists, handle swap, definition kind check and ring push of src/game/core/small15 against the originals: objects with and without
# their list (the allocator is the heap region), handles in and out of the thing table, definitions of any kind.
# usage: small15.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thingenv import park  # noqa: E402

CALLS = {'L4c': (0x41eaa0, 't', 'a'), 'L0': (0x421590, 't', 'a'), 'Swap': (0x4d91f0, 't', 'v'), 'Kind': (0x4e2b80, 't', 'a'), 'Ring': (0x50c100, 't', 'v')}
B = Bench([(0x41eaa0, 0x41eb40), (0x421590, 0x421630), (0x4d91f0, 0x4d9280), (0x4e2b80, 0x4e2c10), (0x50c100, 0x50c190), (0x5da3c0, 0x5da3c1),
           (0x412e90, 0x412eea)])
LIST, ITEM, THINGS = RAM + 0x800, RAM + 0x1000, RAM + 0x2000


def case(r):
    name = r.choice(OPS)
    address, conv, ret = CALLS[name]
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj, HEAP: heap()}
    if name in ('L4c', 'L0'):
        off = 0x4c if name == 'L4c' else 0
        node = RAM + 0x900
        have = r.random() < 0.6
        put(obj, off, 'I', LIST if have else 0)
        lst = bytearray(r.bytes(0x10))
        empty = r.random() < 0.4
        put(lst, 4, 'II', 0 if empty else node, 0 if empty else node)
        put(lst, 0xc, 'I', r.choice([0, 1, 5]))
        regions.update({LIST: lst, ITEM: r.bytes(0x60), node: r.bytes(0x20)})
        return Call(address, [RAM, ITEM], regions, conv='t', ret=ret)
    if name == 'Swap':
        slots = bytearray(r.bytes(20 * 12))
        for h in range(12):
            put(slots, 20 * h, 'I', THINGS + 0x40 * h)
        regions.update({0x7cfb90: slots, THINGS: r.bytes(0x40 * 12)})
        put(obj, 0x24, 'H', r.choice([0, 1, 2, 5, 11]))
        return Call(address, [RAM, r.choice([0, 1, 2, 5, 11, 0x10001])], regions, conv='t', ret=ret)
    if name == 'Kind':
        defs, _h, _a = park(r, count=0, defs=True)
        regions.update({k: v for k, v in defs.items() if k in (0x7890a0, 0x30008000)})
        put(obj, 0xe, 'H', struct.unpack_from('<H', defs[0x30008000], 0x500 * r.randrange(16))[0])
        return Call(address, [RAM], regions, conv='t', ret=ret)
    put(obj, 0x88, 'I', r.choice([0, 1, 5, 30, 31]))
    ident, value = r.choice([0, 1, 5, 0xffff, 0x10005]), r.choice([0, 1, 7, 0xffff, 0x10007])
    for k in range(33):  # the pairs: mostly the id and value about to be pushed
        put(obj, 4 + 4 * k, 'HH', ident & 0xffff if r.random() < 0.5 else r.getrandbits(16), value & 0xffff if r.random() < 0.5 else r.getrandbits(16))
    return Call(address, [RAM, ident, value], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
