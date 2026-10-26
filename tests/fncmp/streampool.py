# The sound streams' block pools (src/game/core/streampool) and the streams' destructors that delete into them (core/streamfilter,
# loopstream, soundstream) against the originals: each pool's globals (its list and lock) with a free list of 0-2 blocks in RAM (linked at
# the pool's next offset; now and then a list whose last block points on, to a wild address for the ops that walk it), the lock any dword;
# the critical section imports recorders, the heap (Mem_Alloc / Mem_Free) failing now and then. Init, Shutdown and Purge; New with 0, the
# pool's size or another (another pool's too); Delete with null, a block of RAM or a wild dword, the pool's size or not; the destructors
# over a block of RAM, flags any.
# usage: streampool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region  # noqa: E402

# pool: (delete, destructor, size, next offset, list global, lock global); init, shutdown, purge, new
POOLS_MORE = {'filter': (0x6c6340, 0x6c6370, 0x6c63a0, 0x6c63e0), 'loop': (0x6c68a0, 0x6c68d0, 0x6c6900, 0x6c6940),
              'f70abf8': (0x6c6bd0, 0x6c6c00, 0x6c6c30, 0x6c6c70), 'f70a6c8': (0x6c7300, 0x6c7330, 0x6c7360, 0x6c73a0),
              'voice': (0x6c79c0, 0x6c79f0, 0x6c7a20, 0x6c7a60), 'voicestream': (0x6c7e10, 0x6c7e40, 0x6c7e70, 0x6c7eb0)}
KINDS = ('delete', 'dtor', 'init', 'shutdown', 'purge', 'new')
POOLS = {'filter': (0x6c6450, 0x6bb170, 0x18, 0x10, 0xfb2068, 0xfb206c), 'loop': (0x6c69b0, 0x6c6a40, 0x24, 0x18, 0xfb2080, 0xfb2084),
         'f70abf8': (0x6c6ce0, 0x6c6d50, 0x14, 0x10, 0xfb208c, 0xfb2090), 'f70a6c8': (0x6c7410, 0x6baab0, 0x1c, 0x14, 0xfb20a4, 0xfb20a8),
         'voice': (0x6c7ad0, None, 0x40, 0x28, 0xfb20bc, 0xfb20c0), 'voicestream': (0x6c7f20, None, 0x5c, 0x58, 0xfb20c8, 0xfb20cc)}
OPS = [(p, kind) for p in POOLS for kind in KINDS
       if POOLS[p][1] or kind != 'dtor'
       if not os.environ.get('OPS') or p + '_' + kind in os.environ['OPS'].split(',')]
B = Bench([(0x6c6340, 0x6c6495), (0x6c68a0, 0x6c69f5), (0x6c6bd0, 0x6c6d25), (0x6c7300, 0x6c7455), (0x6c79c0, 0x6c7b15), (0x6c7e10, 0x6c7f65), (0x6bb170, 0x6bb190),
           (0x6c6a40, 0x6c6a60), (0x6c6d50, 0x6c6d70), (0x6baab0, 0x6baad0), (0x6c7090, 0x6c7097)])
IMPORTS = {'EnterCriticalSection': (20, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (21, 0x6fd130, 1, 0, 0),
           'InitializeCriticalSection': (22, 0x6fd144, 1, 0, 0), 'DeleteCriticalSection': (23, 0x6fd0d0, 1, 0, 0)}
BLOCKS = 3   # RAM blocks: RAM + 0x100 * i (the last the deleted block or the stream)


def block_at(i):
    return RAM + 0x100 * i


def case(r):
    pool, kind = r.deal(OPS)
    delete, dtor, size, nxt, list_at, lock_at = POOLS[pool]
    log = log_region()
    blocks = [bytearray(r.bytes(0x60)) for _ in range(BLOCKS)]
    n = r.choice([0, 0, 1, 2])
    for i in range(n - 1):
        struct.pack_into('<I', blocks[i], nxt, block_at(i + 1))
    if n and r.random() < 0.8:
        struct.pack_into('<I', blocks[n - 1], nxt, 0)
    elif n and kind in ('shutdown', 'purge'):
        struct.pack_into('<I', blocks[n - 1], nxt, r.getrandbits(32) | 0x80000000)   # (walked: a wild address, not the exe's image)
    regions = {list_at: struct.pack('<I', block_at(0) if n else 0), lock_at: struct.pack('<I', r.choice([r.getrandbits(32), 0])), LOG: log,
               IMPORT_TABLE: import_table(log, IMPORTS), HEAP: heap(0x100, fail=r.choice([0, 0, 0, 1, 2]))}
    for i, b in enumerate(blocks):
        regions[block_at(i)] = b
    last = block_at(BLOCKS - 1)
    if kind == 'delete':
        other = r.choice([p[2] for p in POOLS.values()])
        args = [r.choice([last, last, last, 0, r.getrandbits(32) | 0x80000000]), r.choice([size, size, 0, size + 1, other, r.getrandbits(32)])]
        return Call(delete, args, regions, conv='c', ret='v')
    if kind == 'dtor':
        return Call(dtor, [last, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    init, shutdown, purge, new = POOLS_MORE[pool]
    if kind == 'new':
        other = r.choice([p[2] for p in POOLS.values()])
        args = [r.choice([size, size, size, 0, 1, other, size + 1, r.randrange(0x100), r.getrandbits(16)])]
        return Call(new, args, regions, conv='c', ret='a')
    return Call({'init': init, 'shutdown': shutdown, 'purge': purge}[kind], [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
