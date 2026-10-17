# The format record filler, list node splice and global object setter of src/game/core/format_splice against the originals: format records by index, circular lists of one to
# three nodes with the splice point and the moved node anywhere (same list or not), bytes at +0x24 around the signed range.
# usage: format_splice.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Fmt': (0x4925d0, 'c', 'v', 0x80), 'Splice': (0x5d0510, 't', 'v', 0x80), 'Apply': (0x4b0aa0, 't', 'v', 0x40)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x4e21e0, 0x4e21f0), (0x4e2190, 0x4e21b0), (0x4e2770, 0x4e2780)])


def ring(r, regions, base, n):
    """A circular list of a sentinel (base) and n nodes of 0x10 bytes, as RAM addresses: returns the list of node addresses (the sentinel first)."""
    at = [base + 0x10 * i for i in range(n + 1)]
    data = bytearray(r.bytes(0x10 * (n + 1)))
    for i in range(n + 1):
        put(data, 0x10 * i, 'II', at[(i + 1) % (n + 1)], at[i - 1])
    regions[base] = data
    return at


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    regions = {}
    if name == 'Fmt':
        table = bytearray(r.bytes(0x90))
        put(table, 0, 'I', r.choice([0, 1, 2, 3]))
        regions[0x8bd554] = table
        regions[RAM] = r.bytes(0x30)
        return Call(address, [r.getrandbits(32), RAM if r.random() < 0.9 else 0, r.choice([0, 1, 3, 640, r.getrandbits(32)]), r.getrandbits(32)], regions, conv=conv, ret=ret)
    if name == 'Splice':
        regions[RAM] = r.bytes(0x20)
        regions[RAM + 0x40] = r.bytes(0x20)
        a_nodes = ring(r, regions, RAM + 0x200, r.choice([1, 2, 3]))
        x_nodes = a_nodes if r.random() < 0.3 else ring(r, regions, RAM + 0x400, r.choice([1, 2, 3]))
        return Call(address, [RAM, r.choice(a_nodes), RAM + 0x40, r.choice(x_nodes[1:])], regions, conv=conv, ret=ret)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x24, 'b', r.choice([0, 1, -1, 127, -128, 5, -7, r.randrange(-128, 128)]))
    regions[RAM] = obj
    regions[RAM + 0x400] = r.bytes(0x300)
    regions[0x7c2658] = (RAM + 0x400).to_bytes(4, 'little')
    return Call(address, [RAM], regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
