# The sound code's deques of dwords (src/game/core/dequeiter: Deque32_*) against the originals: a deque at DEQ of 0 to ~2600 dwords in
# blocks of 0x1000 bytes at BLOCKS in any order (its first element anywhere in the first block), the blocks' slots somewhere in a map of up to 12 at MAP
# (other slots garbage); its map allocation (0x6c2990), the block release at the front and back (0x6c2910 / 0x6c2ca0, the count 0 or not),
# pop_front (0x6c1550), the new back block (0x6c1940: an empty deque, a slot left after the end's, or the map grown), copy and copy_backward
# (0x6c3010 / 0x6c2f80) between positions in it, and erase(first, last) (0x6c15f0) of any stretch, nearer the front or the back.
# usage: deque32.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap  # noqa: E402

CALLS = {'allocmap': 0x6c2990, 'freefront': 0x6c2910, 'freeback': 0x6c2ca0, 'popfront': 0x6c1550, 'growback': 0x6c1940, 'copy': 0x6c3010,
         'copyback': 0x6c2f80, 'erase': 0x6c15f0}
B = Bench([(0x6c2990, 0x6c29ab), (0x6c2910, 0x6c298a), (0x6c2ca0, 0x6c2d20), (0x6c1550, 0x6c15ef), (0x6c1940, 0x6c1a87), (0x6c3010, 0x6c309c),
           (0x6c2f80, 0x6c300b), (0x6c15f0, 0x6c1932), (0x632310, 0x632343), (0x6c31a0, 0x6c31d3), (0x67a400, 0x690000)])
DEQ, OUT, MAP, BLOCKS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x10000
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def build(r, count=None, room=None):
    """A deque of count dwords (random if None) whose first is at a random offset in its first block; room: slots left after the last block's.
    Returns (regions, it) where it(p) is the iterator to position p (0..count) as four dwords."""
    count = r.choice([1, 2, 3, 10, 0x3ff, 0x400, 0x401, 0x7ff, 0x800, 0x900, r.randrange(1, 0xa00)]) if count is None else count
    off = r.choice([0, 1, 0x3ff, 0x200, r.randrange(0x400)])
    if count == 0x401 and r.random() < 0.5:
        off = 0x3ff
    blocks = (off + count) // 0x400 + 1
    room = r.choice([0, 0, 1, 2]) if room is None else room
    first = r.randrange(0, 3)
    slots = first + blocks + room
    m = bytearray(r.bytes(4 * slots))
    place = r.sample(range(blocks + 1), blocks + 1)   # the blocks in any order, one spare among them (so no block is its neighbour's next)
    for i in range(blocks):
        struct.pack_into('<I', m, 4 * (first + i), BLOCKS + 0x1000 * place[i])

    def it(p):
        q = off + p
        b = BLOCKS + 0x1000 * place[q // 0x400]
        return [b, b + 0x1000, b + 4 * (q % 0x400), MAP + 4 * (first + q // 0x400)]

    deq = bytearray(r.bytes(0x30))
    struct.pack_into('<4I4I3I', deq, 4, *it(0), *it(count), MAP, slots, count)
    regions = {DEQ: deq, MAP: m, BLOCKS: r.bytes(0x1000 * (blocks + 1)), OUT: r.bytes(0x10), HEAP: heap(0x6000)}
    return regions, it, count


def case(r):
    name = r.deal(OPS)
    if name == 'allocmap':
        regions, _it, _n = build(r)
        struct.pack_into('<I', regions[DEQ], 0x28, r.choice([0, 1, 2, 8, 0x20000000, 0x3fffffff, 0x40000000, 0x80000000, r.getrandbits(32)]))
        return Call(CALLS[name], [DEQ], regions, conv='t', ret='v')
    if name in ('freefront', 'freeback', 'popfront'):
        regions, it, n = build(r)
        if name != 'popfront' and r.random() < 0.4:
            struct.pack_into('<I', regions[DEQ], 0x2c, 0)
        return Call(CALLS[name], [DEQ], regions, conv='t', ret='v')
    if name == 'growback':
        regions, it, n = build(r, room=r.choice([0, 0, 1, 2]))
        if r.random() < 0.25:
            struct.pack_into('<I', regions[DEQ], 0x2c, 0)
        return Call(CALLS[name], [DEQ], regions, conv='t', ret='v')
    regions, it, n = build(r)
    a = r.randrange(n + 1)
    b = r.randrange(a, n + 1)
    if name == 'erase':
        return Call(CALLS[name], [DEQ, OUT] + it(a) + it(b), regions, conv='t', ret='a')
    length = b - a
    if name == 'copy':  # to a destination whose stretch stays in the deque
        d = r.randrange(n - length + 1)
    else:  # copy_backward: the destination is where the stretch ends
        d = r.randrange(length, n + 1)
    return Call(CALLS[name], [OUT] + it(a) + it(b) + it(d), regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
