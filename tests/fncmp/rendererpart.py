# The renderer part's constructor and destructors (src/game/render/renderer: 0x48f150, 0x48f300, 0x48f2e0) against the originals: a part
# of random bytes; the live objects list from empty to full (the set's virtual base listed or not), the tracked blocks' table (as
# spritequeue.py), the heap (now and then failing the set's array: a fault in both); for the destructors the set's table the exe's
# (0x6ff318) or made up (other virtual base offsets), its array's block tracked or not, the image's textures none or an empty array and its
# pixels none or a tracked block, any flags.
# usage: rendererpart.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ctor,Dtor,ScalarDtor)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, global_heap, heap, put  # noqa: E402

CALLS = {'Ctor': 0x48f150, 'Dtor': 0x48f300, 'ScalarDtor': 0x48f2e0}
B = Bench([(0x48f150, 0x48f290), (0x48f2e0, 0x48f3d0), (0x5775d0, 0x577660), (0x579b40, 0x579c30), (0x588320, 0x588420), (0x588d70, 0x588e50),
           (0x58a4f0, 0x58a560), (0x58dc50, 0x58dd70), (0x590b90, 0x590c50), (0x590a70, 0x590a75), (0x588ca0, 0x588d6e), (0x6597b0, 0x6598ed),
           (0x659994, 0x6599bc), (0x659d00, 0x659d53), (0x659d90, 0x659dbc), (0x65a9d0, 0x65aa00), (0x66bff7, 0x66c01b), (0x67afa0, 0x67b0d0)])
PART, TABLE, EMPTY = RAM + 0x100, RAM, RAM + 0x300
LIVE, LIVE_COUNT, COUNT, PAIRS = 0x87a6d8, 0x87aed8, 0x8bb214, 0x87b1f8
BLOCKS = [HEAP + 0x40, HEAP + 0x80]


def case(r):
    op = r.deal(OPS)
    regions = {HEAP: heap(fail=1 if r.random() < 0.03 else 0), **global_heap(), EMPTY: bytes(8)}
    part = bytearray(r.bytes(0x100))
    off = 0x20
    if op != 'Ctor':
        table = 0x6ff318
        if r.random() < 0.4:
            off = r.choice([0x24, 0x28, 0x30])
            table = TABLE
            regions[TABLE] = struct.pack('<2I', r.getrandbits(32), off)
        put(part, 0xb4, 'I', table)
        put(part, 0xb4 + 0x14, 'I', r.choice(BLOCKS + [0, r.getrandbits(32) | 0x80000000]))
        put(part, 0x9c, 'I', r.choice([0, 0, EMPTY + 4]))
        put(part, 0x9c + 4, 'I', r.choice([0, 0] + BLOCKS))
        put(part, 0x9c + 0x16, 'H', 0)
    regions[PART] = part
    count = r.choice([0, 1, 2, 3, 0x10])
    regions[COUNT] = struct.pack('<I', count)
    pairs = bytearray(r.bytes(0x80))
    for k in range(min(count, 2)):
        if r.random() < 0.6:
            put(pairs, 8 * k, '2I', BLOCKS[k], BLOCKS[k])
    regions[PAIRS] = pairs
    live_count = r.choice([0, 1, 2, 5, 0x1ff, 0x200, r.randrange(0x200)])
    live = bytearray(r.bytes(0x800))
    if live_count and op != 'Ctor' and r.random() < 0.6:
        put(live, 4 * r.randrange(min(live_count, 0x200)), 'I', PART + 0xb4 + off)
    regions[LIVE] = live
    regions[LIVE_COUNT] = struct.pack('<I', live_count)
    if op == 'Ctor':
        return Call(CALLS[op], [PART, r.getrandbits(32)], regions, conv='t', ret='a')
    if op == 'Dtor':
        return Call(CALLS[op], [PART], regions, conv='t')
    return Call(CALLS[op], [PART, r.choice([0, 1, 1, 2, 3, 0xffffffff])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
