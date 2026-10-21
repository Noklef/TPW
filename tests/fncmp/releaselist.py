# The release list (src/game/core/releaselist: 0x6d7370-0x6d7520) against the originals: the instance at 0xfb2730 (its flag at 0xfb2728)
# with a chain of 0-5 nodes at NODES whose count is the chain's length or less (a larger one spins in both), the critical-section imports
# and atexit as recorders, the heap failing now and then (a push then writes through null in both).
# usage: releaselist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, _cdecl_log, heap, import_table, log_region, set_return  # noqa: E402

# name: (address, returns)
CALLS = {'dtor': (0x6d7370, 'v'), 'basedtor': (0x6d7420, 'v'), 'get': (0x6d7460, 'a'), 'atexit': (0x6d74a0, 'v'), 'ctor': (0x6d74b0, 'a'),
         'deleting': (0x6d7500, 'a'), 'push': (0x6d7520, 'a')}
B = Bench([(0x6d7370, 0x6d75b7)], hooks={0x67a7d0: _cdecl_log})
NODES = RAM
FLAG, LIST = 0xfb2728, 0xfb2730
ENTER_K, LEAVE_K, INIT_K, DELETE_K = 40, 41, 42, 43
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    address, ret = CALLS[name]
    log = log_region()
    imports = import_table(log, {'enter': (ENTER_K, 0x6fd140, 1, 0, 0), 'leave': (LEAVE_K, 0x6fd130, 1, 0, 0),
                                 'init': (INIT_K, 0x6fd144, 1, 0, 0), 'delete': (DELETE_K, 0x6fd0d0, 1, 0, 0)})
    for k in (ENTER_K, LEAVE_K, INIT_K, DELETE_K):
        set_return(log, k, r.choice([0, 1, r.getrandbits(32)]))
    n = r.randrange(6)
    nodes = bytearray(r.bytes(0xc * 6))
    for i in range(n):
        struct.pack_into('<3I', nodes, 0xc * i, r.getrandbits(32), NODES + 0xc * (i + 1) if i + 1 < n else 0,
                         NODES + 0xc * (i - 1) if i else r.choice([0, r.getrandbits(32)]))
    lst = bytearray(r.bytes(0x28))
    struct.pack_into('<4I', lst, 0, r.getrandbits(32), NODES if n else 0, NODES + 0xc * (n - 1) if n and r.random() < 0.8 else r.choice([0, NODES]),
                     r.randrange(n + 1) if name == 'dtor' or name == 'atexit' else r.choice([n, r.randrange(8), r.getrandbits(32)]))
    flag = bytearray(r.bytes(4))
    if r.random() < 0.5:
        flag[0] ^= 1
    regions = {LIST: lst, FLAG: flag, NODES: nodes, IMPORT_TABLE: imports, LOG: log, HEAP: heap(0x40, fail=r.choice([0, 0, 0, 1]))}
    if name in ('get', 'atexit'):
        return Call(address, [], regions, ret=ret)
    if name in ('dtor', 'basedtor', 'ctor'):
        return Call(address, [LIST], regions, conv='t', ret=ret)
    return Call(address, [LIST, r.choice([0, 1, 2, 3, r.getrandbits(32)]) if name == 'deleting' else r.getrandbits(32)], regions, conv='t', ret=ret)


if __name__ == '__main__':
    B.main(case)
