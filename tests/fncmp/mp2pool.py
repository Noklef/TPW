# The MP2 decoders' block pools (src/game/core/mp2pool) and the decoders' destructors (core/mp2decoder) against the originals: the pools'
# globals (0xfb270c-0xfb2718: the buffer pool's list and lock, the stream pool's) with free lists of 0-3 blocks in RAM (each linked at its
# pool's next offset, each pool its own; now and then the other pool's offset, or a list whose last block points on), the lock any dword; the critical
# section imports recorders, the heap (Mem_Alloc / Mem_Free) failing now and then. New with 0, the pool's size or another (the other
# pool's too); delete with null, a block of RAM or the pool's size or not; the destructors over a decoder open or not, flags any.
# usage: mp2pool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region  # noqa: E402

# name: (address, convention, returns)
CALLS = {'init_b': (0x6c8f00, 'c', 'v'), 'init_s': (0x6c9090, 'c', 'v'), 'shutdown_b': (0x6c8f30, 'c', 'v'), 'shutdown_s': (0x6c90c0, 'c', 'v'),
         'purge_b': (0x6c8f60, 'c', 'v'), 'purge_s': (0x6c90f0, 'c', 'v'), 'new_b': (0x6c8fb0, 'c', 'a'), 'new_s': (0x6c9140, 'c', 'a'),
         'delete_b': (0x6c9030, 'c', 'v'), 'delete_s': (0x6c91c0, 'c', 'v'), 'dtor': (0x6c9210, 't', 'v'), 'ddtor_b': (0x6c9230, 't', 'a'),
         'ddtor_s': (0x6c8a20, 't', 'a')}
B = Bench([(0x6c8a20, 0x6c8a55), (0x6c8f00, 0x6c8f17), (0x6c8f30, 0x6c907b), (0x6c9090, 0x6c90a7), (0x6c90c0, 0x6c920b), (0x6c9210, 0x6c9253),
           (0x6d1a80, 0x6d1aa4), (0x5d60c0, 0x5d60c3), (0x5da3c0, 0x5da3c1)])
GLOBALS = 0xfb270c   # buffer list, buffer lock, stream list, stream lock
BLOCK, BLOCKS = 0x3000, 7   # RAM blocks: RAM + BLOCK * i (0-2 the buffer pool's, 3-5 the stream pool's, the last the deleted block or the decoder)
POOLS = {'b': (0x2478, 0x2404), 's': (0x2c80, 0x2478)}
IMPORTS = {'EnterCriticalSection': (20, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (21, 0x6fd130, 1, 0, 0),
           'InitializeCriticalSection': (22, 0x6fd144, 1, 0, 0), 'DeleteCriticalSection': (23, 0x6fd0d0, 1, 0, 0)}
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def block_at(i):
    return RAM + BLOCK * i


def case(r):
    name = r.deal(OPS)
    address, conv, ret = CALLS[name]
    log = log_region()
    blocks = [bytearray(r.bytes(0x2c90)) for _ in range(BLOCKS)]
    globals_ = bytearray(0x10)
    for g, (size, nxt) in enumerate(POOLS.values()):
        n = r.choice([0, 0, 1, 1, 2, 3])
        chain = r.sample(range(3 * g, 3 * g + 3), n)   # (each pool its own blocks: no cycles)
        if r.random() < 0.1:
            nxt = POOLS['s' if nxt == 0x2404 else 'b'][1]
        for i, j in zip(chain, chain[1:]):
            struct.pack_into('<I', blocks[i], nxt, block_at(j))
        if chain and r.random() < 0.8:
            struct.pack_into('<I', blocks[chain[-1]], nxt, 0)
        struct.pack_into('<II', globals_, 8 * g, block_at(chain[0]) if chain else 0, r.choice([r.getrandbits(32), r.getrandbits(32), 0]))
    regions = {GLOBALS: globals_, LOG: log, IMPORT_TABLE: import_table(log, IMPORTS), HEAP: heap(0x6000, fail=r.choice([0, 0, 0, 1, 2]))}
    for i, b in enumerate(blocks):
        regions[block_at(i)] = b
    last = block_at(BLOCKS - 1)
    if name.startswith('new'):
        size = POOLS[name[-1]][0]
        args = [r.choice([size, size, size, 0, 1, POOLS['s' if name[-1] == 'b' else 'b'][0], r.randrange(0x100), r.getrandbits(16)])]
    elif name.startswith('delete'):
        size = POOLS[name[-1]][0]
        args = [r.choice([last, last, last, 0, r.getrandbits(32)]), r.choice([size, size, 0, size + 1, POOLS['s' if name[-1] == 'b' else 'b'][0],
                                                                             r.getrandbits(32)])]
    elif name in ('dtor', 'ddtor_b', 'ddtor_s'):
        blocks[-1][0x243c] = r.choice([0, 0, 1, r.getrandbits(8)])
        args = [last] + ([] if name == 'dtor' else [r.choice([0, 1, 2, 3, r.getrandbits(32)])])
    else:
        args = []
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
