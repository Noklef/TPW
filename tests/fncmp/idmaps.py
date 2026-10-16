# The mapping functions of src/game/core/idmaps against the originals: every kind around the tables and wild values.
# usage: idmaps.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, convention, size, field offset / width, domain)
CALLS = {'A': (0x4d3220, 't', 0x170, (0, 'I'), 0x24), 'B': (0x4d3390, 't', 0x170, (0, 'I'), 0x24), 'C': (0x4d3500, 't', 0x170, (0, 'I'), 0x24),
         'D': (0x4d3670, 't', 0x170, (0, 'I'), 0x24), 'E': (0x485ec0, 'c', 0x40, None, 8), 'F': (0x506490, 't', 0x60, (2, 'B'), 12), 'G': (0x5064f0, 't', 0x60, (0x19c, 'I'), 12)}
B = Bench([(a, a + n) for (a, _c, n, _f, _d) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])


def case(r):
    name = r.choice(OPS)
    address, conv, _n, field, domain = CALLS[name]
    value = r.choice([r.randrange(domain)] * 6 + [r.getrandbits(32) if field is None or field[1] == 'I' else r.getrandbits(8), 0xffffffff & (domain + r.randrange(4))])
    if field is None:
        return Call(address, [value], {}, conv=conv, ret='a')
    obj = bytearray(r.bytes(0x200))
    put(obj, field[0], field[1], value & (0xff if field[1] == 'B' else 0xffffffff))
    return Call(address, [RAM], {RAM: obj}, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
