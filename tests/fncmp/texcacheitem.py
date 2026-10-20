# The texture cache item's constructor and flag word (src/game/gfx/texcache: 0x58b300-0x58b398, thiscall) against the originals: Ctor (vtable
# 0x702198, flags 0, the item returned), SetFlags (the new flags with the old word's bit 0x10000000 kept), the flag 1 / 2 / 4 tests and flag 1
# set and cleared, on items of random bytes.
# usage: texcacheitem.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x58b300, 0x58b399)])
CALLS = {'Ctor': 0x58b300, 'SetFlags': 0x58b330, 'Flag1': 0x58b350, 'Flag2': 0x58b360, 'Flag4': 0x58b370, 'SetFlag1': 0x58b380, 'ClearFlag1': 0x58b390}


def case(r):
    op = r.deal(OPS)
    item = bytearray(r.bytes(0x20))
    put(item, 4, 'I', r.choice([0, 0x10000000, 0x10000007, 7, 1, 2, 4, 0xffffffff, r.getrandbits(32)]))
    args = [RAM] + ([r.choice([0, 1, 0x10000000, 0xefffffff, r.getrandbits(32)])] if op == 'SetFlags' else [])
    return Call(CALLS[op], args, {RAM: item}, conv='t', ret='a' if op.startswith('Flag') or op == 'Ctor' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
