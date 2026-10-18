# The GUI library's message queue (src/game/ui/msgqueue: 0x669735, 0x6697e3) against the originals: a queue of 1-8 entries (each free or
# not), its fill index anywhere in them, its count and the four values random; the window now and then 0. The free: entries set or not
# (the heap region given: its free count and hash).
# usage: msgqueue.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Free,Push)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

CALLS = {'Free': 0x669735, 'Push': 0x6697e3}
B = Bench([(0x669735, 0x669767), (0x6697e3, 0x6698a4)])
QUEUE, ENTRIES = RAM, RAM + 0x100


def queue(r, regions):
    cap = r.choice([1, 2, 3, 4, 8])
    entries = bytearray(r.bytes(16 * cap))
    for k in range(cap):
        if r.random() < 0.5:
            put(entries, 16 * k, 'I', 0)
    regions[ENTRIES] = entries
    q = bytearray(r.bytes(0x14))
    put(q, 0, 'I', ENTRIES)
    put(q, 4, 'I', cap)
    put(q, 8, 'I', r.randrange(cap))
    put(q, 0xc, 'I', r.choice([0, 0, cap - 1, r.randrange(cap)]))
    regions[QUEUE] = q


def case(r):
    op = r.deal(OPS)
    regions = {}
    queue(r, regions)
    if op == 'Free':
        regions[HEAP] = heap(0x20)
        put(regions[QUEUE], 0, 'I', r.choice([HEAP + 0x10, HEAP + 0x10, 0]))
        return Call(CALLS[op], [QUEUE], regions, conv='t')
    window = r.choice([0x1234, r.getrandbits(32), r.getrandbits(32), 0])
    return Call(CALLS[op], [QUEUE, window] + [r.getrandbits(32) for _ in range(3)], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
