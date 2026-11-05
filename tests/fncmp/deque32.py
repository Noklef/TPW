# The sound code's deques of dwords (src/game/core/dequeiter: Deque32_*) against the originals: a deque at DEQ of 0 to ~2600 dwords in
# blocks of 0x1000 bytes at BLOCKS in any order (its first element anywhere in the first block), the blocks' slots somewhere in a map of up to 12 at MAP
# (other slots garbage); its map allocation (0x6c2990), the block release at the front and back (0x6c2910 / 0x6c2ca0, the count 0 or not),
# pop_front (0x6c1550) and pop_back (0x632150), the new back block (0x6c1940: an empty deque, a slot left after the end's, or the map grown), copy and copy_backward
# (0x6c3010 / 0x6c2f80) between positions in it, and erase(first, last) (0x6c15f0) of any stretch, nearer the front or the back.
# usage: deque32.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, IMPORT_TABLE, LOG, RAM, Bench, Call, heap, import_table, log_region, set_return, vtable  # noqa: E402

CALLS = {'allocmap': 0x6c2990, 'freefront': 0x6c2910, 'freeback': 0x6c2ca0, 'popfront': 0x6c1550, 'growback': 0x6c1940, 'copy': 0x6c3010,
         'copyback': 0x6c2f80, 'erase': 0x6c15f0, 'popback': 0x632150, 'clear': 0x632080, 'ownerpop': 0x631ec0,
         'qctor': 0x631870, 'qlock': 0x631fc0, 'qunlock': 0x632020, 'qpush': 0x631d10, 'qthunk': 0x631d00, 'qdtor': 0x6319a0, 'qdel': 0x631980, 'qbasedel': 0x631950}
B = Bench([(0x6c2990, 0x6c29ab), (0x6c2910, 0x6c298a), (0x6c2ca0, 0x6c2d20), (0x6c1550, 0x6c15ef), (0x6c1940, 0x6c1a87), (0x6c3010, 0x6c309c),
           (0x6c2f80, 0x6c300b), (0x6c15f0, 0x6c1932), (0x632310, 0x632343), (0x6c31a0, 0x6c31d3), (0x632150, 0x632200), (0x632080, 0x632142), (0x631ec0, 0x631fbc), (0x67a400, 0x690000),
           (0x631870, 0x6318f3), (0x631900, 0x63196e), (0x631980, 0x63199e), (0x6319a0, 0x631cfa), (0x631d00, 0x631d0c), (0x631d10, 0x631eb6), (0x631fc0, 0x63201e), (0x632020, 0x63207a),
           (0x6338b0, 0x6338c0), (0x633900, 0x633994), (0x632200, 0x632222), (0x632230, 0x632290), (0x632290, 0x6322dc), (0x6322e0, 0x632310)])
OWNER, DEQ, OUT, MAP, BLOCKS = RAM, RAM + 0x10, RAM + 0x100, RAM + 0x200, RAM + 0x10000
OWNER_VT, FRONT = RAM + 0x8000, RAM + 0x8100
LOCK, SECTION = RAM + 0x40, RAM + 0x8200
SECTIONS = {'Init': (1, 0x6fd144, 1, 0, 0), 'Delete': (2, 0x6fd0d0, 1, 0, 0), 'Enter': (3, 0x6fd140, 1, 0, 0), 'Leave': (4, 0x6fd130, 1, 0, 0)}
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
    if name in ('freefront', 'freeback', 'popfront', 'popback'):
        regions, it, n = build(r)
        if name not in ('popfront', 'popback') and r.random() < 0.4:
            struct.pack_into('<I', regions[DEQ], 0x2c, 0)
        return Call(CALLS[name], [DEQ], regions, conv='t', ret='v')
    if name == 'clear':
        regions, it, n = build(r, r.choice([0, 1, 2, 10, 0x3ff, 0x400, 0x401, 0x900]))
        return Call(CALLS[name], [DEQ], regions, conv='t', ret='v')
    if name == 'ownerpop':
        # The owner: the vtable at the deque's -0x10, a recorder's answer; the front dword an object with slot 0 a recorder.
        regions, it, n = build(r, r.choice([1, 2, 3, 0x3ff, 0x400, 0x401]))
        log = log_region()
        set_return(log, 7, r.choice([0, 1, 1, 1, 0x80000000]))
        regions.update({LOG: log, OWNER: struct.pack('<I', OWNER_VT) + r.bytes(0xc), OWNER_VT: vtable({0: 1, 7: 0}), FRONT: struct.pack('<I', OWNER_VT) + r.bytes(0xc)})
        blocks = bytearray(regions[BLOCKS])
        struct.pack_into('<I', blocks, it(0)[2] - BLOCKS, FRONT)
        regions[BLOCKS] = blocks
        return Call(CALLS[name], [OWNER, OUT, r.choice([0, r.getrandbits(32)])], regions, conv='t', ret='a')
    if name == 'qctor':
        log = log_region()
        return Call(CALLS[name], [OWNER], {LOG: log, IMPORT_TABLE: import_table(log, SECTIONS), OWNER: r.bytes(0x48), HEAP: heap(0x100)}, conv='t', ret='a', ignore=[(OWNER + 0x10, 1)])
    if name[0] == 'q':
        # The queue: the deque as above, the object at +8 or none, the section's calls recorded.
        count = r.choice([0, 1, 2, 3, 0x3ff, 0x400, 0x401, 0x900])
        regions, it, n = build(r, count)
        deq = regions[DEQ]
        if not count and r.random() < 0.5:   # (as the constructor leaves it)
            struct.pack_into('<8I', deq, 4, *[0] * 8)
        if name in ('qpush', 'qthunk') and count and r.random() < 0.6:   # (the last block full)
            struct.pack_into('<I', deq, 0x1c, struct.unpack_from('<I', deq, 0x18)[0])
        if name in ('qdtor', 'qdel') and r.random() < 0.2:   # (a count the iterators disagree with)
            struct.pack_into('<I', deq, 0x2c, count + r.choice([1, 2, 5]))
        log = log_region()
        lock = bytearray(r.bytes(8))
        struct.pack_into('<I', lock, 0, SECTION)
        owner = bytearray(struct.pack('<I', OWNER_VT) + r.bytes(0xc))
        struct.pack_into('<I', owner, 8, r.choice([0, FRONT]))
        regions.update({LOG: log, IMPORT_TABLE: import_table(log, SECTIONS), OWNER: owner, LOCK: lock, SECTION: r.bytes(0x18), OWNER_VT: vtable({0: 0}), FRONT: struct.pack('<I', OWNER_VT) + r.bytes(0xc)})
        if name in ('qpush', 'qthunk'):
            return Call(CALLS[name], [OWNER, r.getrandbits(32)], regions, conv='t', ret='v')
        if name in ('qdel', 'qbasedel'):
            return Call(CALLS[name], [OWNER, r.choice([0, 1, 2, 3])], regions, conv='t', ret='a')
        return Call(CALLS[name], [OWNER], regions, conv='t', ret='v')
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
