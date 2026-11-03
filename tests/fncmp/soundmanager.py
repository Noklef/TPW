# The sound manager handle's methods (src/game/core/soundmanager) against the originals: the handle at MGR (a recorder vtable, slot 3
# the deleting destructor; null now and then for Release, which tests it), its engine at ENG (a recorder vtable whose slots return a
# random value; null now and then for the destructor, which tests it, else a fault in both), the out dword at OUT, arguments random.
# The manager made (0x6b5dc0, 'create'): its block failing now and then (heap fail 1), the engine ENG, the slot and out random.
# usage: soundmanager.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, fill, heap, log_region, set_return, strings, vtable  # noqa: E402

# name: (address, stack arguments after out (None: no out), returns)
CALLS = {'dtor': (0x6b5ab0, None, 'v'), 'release': (0x6b5af0, 0, 'a'), 'query': (0x6b5b10, 3, 'a'), 'post': (0x6b5b40, None, 'a'),
         'call3': (0x6b5b80, None, 'a'), 'call5': (0x6b5b60, None, 'a'), 'call1': (0x6b5ad0, 1, 'a'), 'deldtor': (0x6b5bf0, None, 'a'),
         'create': (0x6b5dc0, None, 'a')}
NARGS = {'post': 2, 'call3': 3, 'call5': 2, 'deldtor': 1}
B = Bench([(0x6b5ab0, 0x6b5ba0), (0x6b5bf0, 0x6b5c20), (0x6b5dc0, 0x6b5e14), (0x6b5d00, 0x6b5db1)], hooks={0x6b8380: _recorder(20, 0)})
MGR, OUT, ENG, MVT, EVT = RAM, RAM + 0x10, RAM + 0x100, RAM + 0x200, RAM + 0x300
OPS = [n for n in list(CALLS) + ['make'] if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def make(r):
    """The manager and its engine made (0x6b5d00): the engine's constructor a recorder (20), the two vtables recorders in the game's own places (the manager's slot 4 the real 0x6b5ad0, slot 0 logged
    with its out a stack address), the status the engine's slot 1 writes either way. The heap fails now and then: a fault in both (a null engine called, a null manager released)."""
    log = log_region()
    fill(log, 1, 0, struct.pack('<I', r.choice([0, 0, 1, 0x7fffffff, 0xffffffff, 0x80000000, 0x88780078])))
    strings(log, 1, 0, out=1)
    strings(log, 0, 0, out=1)
    manager_vt = vtable({0: 1}, size=6)
    struct.pack_into('<I', manager_vt, 0x10, 0x6b5ad0)
    regions = {LOG: log, HEAP: heap(0x100, r.choice([0] * 18 + [1, 2])), OUT: r.bytes(4), MGR: r.bytes(4), 0x70a288: manager_vt, 0x70a2a0: vtable({1: 2})}
    return Call(0x6b5d00, [OUT, r.getrandbits(32), MGR], regions, conv='c', ret='a')


def case(r):
    name = r.deal(OPS)
    if name == 'make':
        return make(r)
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
    if name == 'create':   # (stdcall (out, &manager, engine, a))
        regions[HEAP] = heap(0x40, fail=r.choice([0, 0, 0, 1]))
        return Call(address, [OUT, MGR, ENG, r.getrandbits(32)], regions, ret=ret)
    this = 0 if name == 'release' and r.random() < 0.15 else MGR
    if after_out is None:
        args = [r.choice([0, 1, r.getrandbits(32)]) if name == 'deldtor' else r.getrandbits(32) for _ in range(NARGS.get(name, 0))]
    else:
        args = [OUT] + [r.getrandbits(32) for _ in range(after_out)]
    return Call(address, [this] + args, regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
