# The sound manager handle's methods (src/game/core/soundmanager) against the originals: the handle at MGR (a recorder vtable, slot 3
# the deleting destructor; null now and then for Release, which tests it), its engine at ENG (a recorder vtable whose slots return a
# random value; null now and then for the destructor, which tests it, else a fault in both), the out dword at OUT, arguments random.
# usage: soundmanager.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, set_return, vtable  # noqa: E402

# name: (address, stack arguments after out (None: no out), returns)
CALLS = {'dtor': (0x6b5ab0, None, 'v'), 'release': (0x6b5af0, 0, 'a'), 'query': (0x6b5b10, 3, 'a'), 'post': (0x6b5b40, None, 'a'),
         'call3': (0x6b5b80, None, 'a'), 'call5': (0x6b5b60, None, 'a'), 'call1': (0x6b5ad0, 1, 'a'), 'deldtor': (0x6b5bf0, None, 'a')}
NARGS = {'post': 2, 'call3': 3, 'call5': 2, 'deldtor': 1}
B = Bench([(0x6b5ab0, 0x6b5ba0), (0x6b5bf0, 0x6b5c20)])
MGR, OUT, ENG, MVT, EVT = RAM, RAM + 0x10, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, after_out, ret = CALLS[name]
    log = log_region()
    for k in range(6):
        set_return(log, k, r.getrandbits(32))
    engine = ENG if r.random() < 0.9 or name not in ('dtor', 'deldtor') else 0
    if r.random() < 0.02:
        engine = r.getrandbits(32)
    manager = struct.pack('<II', MVT, engine) + r.bytes(8)
    regions = {MGR: manager, OUT: r.bytes(4), ENG: struct.pack('<I', EVT) + r.bytes(0x1c), MVT: vtable({3: 1}),
               EVT: vtable({0: 1, 1: 2, 2: 4, 3: 3, 4: 2, 5: 2}), LOG: log}
    if name == 'deldtor':   # (the manager freed for flags bit 0)
        regions[HEAP] = heap(0x40)
    this = 0 if name == 'release' and r.random() < 0.15 else MGR
    if after_out is None:
        args = [r.choice([0, 1, r.getrandbits(32)]) if name == 'deldtor' else r.getrandbits(32) for _ in range(NARGS.get(name, 0))]
    else:
        args = [OUT] + [r.getrandbits(32) for _ in range(after_out)]
    return Call(address, [this] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
