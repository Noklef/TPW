# The list searches and string destructor of src/game/core/named_lists against the originals: lists of 0 to 4 objects whose names (case varied) are the key's or not, and five
# game strings (gamestring.py's). WFind: the wide-text list search (0x5accb0) on the same lists, the texts made wide.
# usage: named_lists.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Find': (0x421770, 't', 'a', 0x70), 'Has': (0x41ede0, 't', 'a', 0x50), 'Dtor': (0x5c4c60, 't', 'v', 0x80), 'WFind': (0x5accb0, 't', 'a', 0x36)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x424cf0, 0x424d40),(0x67df40, 0x67e040), (0x67faf0, 0x67fb20), (0x682720, 0x682780),
          (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
NAMES = [b'Sloth', b'sloth', b'SLOTH', b'Slot', b'', b'b']
LIST, NODES, OBJS, TEXTS = RAM + 0x800, RAM + 0x900, RAM + 0x1000, RAM + 0x2000


def varied(r, base):
    s = bytearray(base)
    for i in range(len(s)):
        if r.random() < 0.3:
            s[i] ^= 0x20
    return bytes(s) + bytes(0x10)


def wide(text):
    return b''.join(bytes([c, 0]) for c in text)


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {RAM: obj, HEAP: heap()}
    if name == 'Dtor':
        for k in range(5):
            gamestring.string(r, 0, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions)
            obj[0x14 * k:0x14 * k + 0x14] = regions.pop(RAM + 0x4000 + 0x40 * k)[:0x14]
        return Call(address, [RAM], regions, conv=conv, ret=ret)
    n = r.choice([0, 1, 2, 3, 4])
    wanted = r.choice(NAMES)
    lst = bytearray(r.bytes(0x10))
    put(lst, 4, 'I', NODES if n else 0)
    nodes = bytearray(r.bytes(0x10 * 5))
    for k in range(n):
        put(nodes, 0x10 * k, 'II', OBJS + 0x40 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0)
        o = bytearray(r.bytes(0x40))
        put(o, 0xc, 'I', TEXTS + 0x40 * k)
        regions[OBJS + 0x40 * k] = o
        regions[TEXTS + 0x40 * k] = varied(r, wanted if r.random() < 0.5 else r.choice(NAMES))
    key = bytearray(r.bytes(0x20))
    put(key, 8, 'I', TEXTS + 0x400)
    if name == 'WFind':   # the items' wide texts at +8
        for k in range(n):
            put(regions[OBJS + 0x40 * k], 8, 'I', TEXTS + 0x40 * k)
            regions[TEXTS + 0x40 * k] = wide(regions[TEXTS + 0x40 * k])
        regions.update({LIST: lst, NODES: nodes, RAM + 0x400: key, TEXTS + 0x400: wide(varied(r, wanted))})
        return Call(address, [LIST, RAM + 0x400], regions, conv=conv, ret=ret)
    regions.update({LIST: lst, NODES: nodes, RAM + 0x400: key, TEXTS + 0x400: varied(r, wanted), RAM + 0x500: r.bytes(0x10)})
    if name == 'Find':
        put(obj, 0, 'I', LIST if r.random() < 0.9 else 0)
        return Call(address, [RAM, RAM + 0x400, RAM + 0x500], regions, conv=conv, ret=ret)
    put(obj, 0x4c, 'I', LIST if r.random() < 0.9 else 0)
    return Call(address, [RAM, RAM + 0x400], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
