# The class destructors and shared-array resets of src/game/core/small29 against the originals: objects on their parent's list (first, middle, last, only or not listed), three
# game strings (narrow or wide; gamestring.py's), shared records with use counts at the edge (arrays without elements to destroy).
# usage: small29.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, wide strings, size)
CALLS = {'A': (0x5bdc10, 1, 0xc0), 'B': (0x5be320, 0, 0xc0), 'C': (0x5be750, 1, 0xc0), 'R26': (0x5b97a0, None, 0x80), 'R34': (0x5b9820, None, 0x80)}
B = Bench([(a, a + n) for (a, _w, n) in CALLS.values()] + [(0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x67afa0, 0x67b060)])
PARENT, LIST, NODES = RAM + 0x400, RAM + 0x500, RAM + 0x600


def case(r):
    name = r.choice(OPS)
    address, wide, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    regions = {RAM: obj, HEAP: heap()}
    if wide is None:
        put(obj, 0x30, 'I', RAM + 0x200 if r.random() < 0.85 else 0)
        rec = bytearray(r.bytes(0x20))
        put(rec, 4, 'I', RAM + 0x304 if r.random() < 0.7 else 0)
        put(rec, 0xc, 'i', r.choice([1, 1, 2, 0, -1]))
        regions[RAM + 0x200] = rec
        regions[RAM + 0x300] = struct.pack('<I', 0) + r.bytes(0x20)
        return Call(address, [RAM, r.getrandbits(32), r.getrandbits(32)], regions, conv='t', ret='v')
    n = r.choice([0, 1, 2, 3])
    mine = r.randrange(n) if n and r.random() < 0.85 else -1
    put(obj, 4, 'I', PARENT)
    parent = bytearray(r.bytes(0x40))
    put(parent, 0x2c, 'I', LIST)
    lst = bytearray(r.bytes(0x20))
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    nodes = bytearray(r.bytes(0x50))
    for k in range(n):
        put(nodes, 0x10 * k, 'III', RAM if k == mine else RAM + 0x1000 + 0x40 * k, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
    regions.update({PARENT: parent, LIST: lst, NODES: nodes})
    for k, off in enumerate((0xc, 0x20, {'A': 0x44, 'B': 0x34, 'C': 0x4c}[name])):
        gamestring.string(r, wide, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions)
        obj[off:off + 0x14] = regions.pop(RAM + 0x4000 + 0x40 * k)[:0x14]
    return Call(address, [RAM], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
