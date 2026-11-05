# The input messages' pools (src/game/input/messagepool) and the list<u32> methods of src/game/online/mailbox with the same list shape (0x634bf0, 0x634c60) against the originals: a circular list in RAM,
# the messages free or not, the cursor anywhere; the real heap's alloc and free.
# usage: messagepool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, heap, put  # noqa: E402

ADDRESS = {'KeyDtor': 0x634a50, 'MouseDtor': 0x634b40, 'Clear': 0x634bf0, 'Insert': 0x634c60, 'KeyNext': 0x634d00, 'MouseNext': 0x634de0}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
B = Bench([(0x634a50, 0x634af1), (0x634b40, 0x634be1), (0x634bf0, 0x634c55), (0x634c60, 0x634cb7), (0x634d00, 0x634ddb), (0x634de0, 0x634ec1), (0x47db00, 0x47db20), (0x67a400, 0x690000)])
SELF, OUT, VALUE, LIST, MESSAGES = RAM, RAM + 0x40, RAM + 0x50, RAM + 0x100, RAM + 0x400


def ring(r, count):
    """A circular list: the sentinel, then count nodes in any order, each with its message. Returns (nodes' bytes, messages' bytes, the sentinel's address, the nodes' addresses)."""
    nodes = bytearray(r.bytes(0x10 * (count + 1)))
    messages = bytearray(r.bytes(0x20 * (count + 1)))
    order = r.sample(range(1, count + 1), count)
    ring_ = [0] + order
    at = [LIST + 0x10 * i for i in ring_]
    for i, slot in enumerate(ring_):
        put(nodes, 0x10 * slot, 'III', at[(i + 1) % len(at)], at[i - 1], MESSAGES + 0x20 * slot)
        put(messages, 0x20 * slot + 8, 'I', r.choice([0, 1, 1, 0x80000000]))
    return nodes, messages, at[0], at[1:]


def case(r):
    op = r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 5])
    nodes, messages, sentinel, others = ring(r, count)
    self_ = bytearray(r.bytes(0x20))
    regions = {SELF: self_, LIST: nodes, MESSAGES: messages, OUT: r.bytes(8), VALUE: r.bytes(4), HEAP: heap(0x1000)}
    if op == 'KeyNext' or op == 'MouseNext':
        put(self_, 8, 'I', sentinel)
        put(self_, 0xc, 'I', count)
        put(self_, 0x10, 'I', r.choice(others) if others else sentinel)   # (the cursor at the sentinel of a list with nodes, all busy, never ends: on both sides)
        if r.random() < 0.4:   # (all busy: a new message)
            for i in range(count + 1):
                put(messages, 0x20 * i + 8, 'I', 1)
        return Call(ADDRESS[op], [SELF], regions, conv='t', ret='a')
    if op == 'Clear':
        put(self_, 4, 'I', sentinel)
        put(self_, 8, 'I', count)
        return Call(ADDRESS[op], [SELF], regions, conv='t', ret='v')
    if op == 'Insert':
        put(self_, 8, 'I', count)
        return Call(ADDRESS[op], [SELF, OUT, r.choice(others + [sentinel]), VALUE], regions, conv='t', ret='a')
    put(self_, 8, 'I', sentinel)
    put(self_, 0xc, 'I', count)
    return Call(ADDRESS[op], [SELF], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
