# The input messages' pools (src/game/input/messagepool) and the list<u32> methods of src/game/online/mailbox with the same list shape (0x634bf0, 0x634c60) against the originals: a circular list in RAM,
# the messages free or not, the cursor anywhere; the real heap's alloc and free.
# usage: messagepool.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402

ADDRESS = {'KeyDtor': 0x634a50, 'MouseDtor': 0x634b40, 'Clear': 0x634bf0, 'Insert': 0x634c60, 'KeyNext': 0x634d00, 'MouseNext': 0x634de0, 'KeyDel': 0x634b00, 'MouseDel': 0x634b20, 'Deliver': 0x634900, 'Keys': 0x6350f0,
           'Ctor2': 0x6345d0, 'Dtor2': 0x634760, 'Del2': 0x634740, 'Ctor1': 0x634ee0, 'Dtor1': 0x635000, 'Del1': 0x634fe0}
OPS = [n for n in ADDRESS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]
B = Bench([(0x634a50, 0x634af1), (0x634b40, 0x634be1), (0x634bf0, 0x634c55), (0x634c60, 0x634cb7), (0x634d00, 0x634ddb), (0x634de0, 0x634ec1), (0x47db00, 0x47db20), (0x67a400, 0x690000), (0x634b00, 0x634b1f), (0x634b20, 0x634b3f), (0x634900, 0x6349f8), (0x6350f0, 0x635174), (0x67acb0, 0x67acb1),
           (0x6345d0, 0x63471e), (0x634740, 0x63475f), (0x634760, 0x6348d3), (0x634ee0, 0x634fdf), (0x634fe0, 0x634fff), (0x635000, 0x6350ca), (0x634cc0, 0x634cf7), (0x5d0690, 0x5d06c0),
           (0x63e7c0, 0x63e815), (0x63e840, 0x63e8b9), (0x633120, 0x633140), (0x633160, 0x6331c0), (0x633200, 0x63321a), (0x633a50, 0x633a64), (0x633a00, 0x633a30), (0x6338b0, 0x6338c0)])
SELF, OUT, VALUE, LIST, MESSAGES = RAM, RAM + 0x80, RAM + 0x90, RAM + 0x100, RAM + 0x400
LIST2, MESSAGES2, RECORDS, SINK, VT = RAM + 0x200, RAM + 0x600, RAM + 0x800, RAM + 0x900, RAM + 0x980
SLOT_KEY, SLOT_MOUSE = 5, 6


def ring(r, count, list_=LIST, block=MESSAGES):
    """A circular list: the sentinel, then count nodes in any order, each with its message. Returns (nodes' bytes, messages' bytes, the sentinel's address, the nodes' addresses)."""
    nodes = bytearray(r.bytes(0x10 * (count + 1)))
    messages = bytearray(r.bytes(0x20 * (count + 1)))
    order = r.sample(range(1, count + 1), count)
    ring_ = [0] + order
    at = [list_ + 0x10 * i for i in ring_]
    for i, slot in enumerate(ring_):
        put(nodes, 0x10 * slot, 'III', at[(i + 1) % len(at)], at[i - 1], block + 0x20 * slot)
        put(messages, 0x20 * slot + 8, 'I', r.choice([0, 1, 1, 0x80000000]))
    return nodes, messages, at[0], at[1:]


def case(r):
    op = r.deal(OPS)
    count = r.choice([0, 1, 2, 3, 5])
    nodes, messages, sentinel, others = ring(r, count)
    self_ = bytearray(r.bytes(0x70))
    regions = {SELF: self_, LIST: nodes, MESSAGES: messages, OUT: r.bytes(8), VALUE: r.bytes(4), HEAP: heap(0x1000)}
    if op in ('Deliver', 'Keys'):
        log = log_region()
        regions[LOG] = log
        records = bytearray(r.bytes(0x60))
        for i in range(6):
            put(records, 0x10 * i, 'I', r.choice([0, 4, 8, 0xc, 0xd, 0xe, 0xf, 1, 5, 7, 9, 0xb, 0x10, 0x1ff, 0xffffffff]))
        regions[RECORDS], regions[SINK], regions[VT] = records, struct.pack('<I', VT), vtable({SLOT_KEY: 1, SLOT_MOUSE: 1})
        put(self_, 0xc, 'I', SINK)
        put(self_, 0x30, 'I', RECORDS)
        put(self_, 0x3c, 'III', sentinel, count, r.choice(others) if others else sentinel)
        nodes2, messages2, sentinel2, others2 = ring(r, r.choice([0, 1, 2, 3]), LIST2, MESSAGES2)
        regions[LIST2], regions[MESSAGES2] = nodes2, messages2
        put(self_, 0x50, 'III', sentinel2, (len(nodes2) >> 4) - 1, r.choice(others2) if others2 else sentinel2)
        return Call(ADDRESS[op], [SELF, r.choice([0, 1, 2, 3, 6])], regions, conv='t', ret='v')
    if op in ('Ctor2', 'Ctor1'):
        regions[HEAP] = heap(0x1000, r.choice([0, 0, 0, 2, 5]))
        return Call(ADDRESS[op], [SELF], regions, conv='t', ret='a', ignore=[(SELF + 0x38, 1), (SELF + 0x4c, 1)])
    if op in ('Dtor2', 'Del2', 'Dtor1', 'Del1'):
        for at, value in ((8, 0), (0xc, 0), (0x1c, 0), (0x30, r.choice([0, RAM + 0xf00]))):
            put(self_, at, 'I', value)
        put(self_, 0x3c, 'II', sentinel, count)
        nodes2, messages2, sentinel2, others2 = ring(r, r.choice([0, 1, 2, 3]), LIST2, MESSAGES2)
        regions[LIST2], regions[MESSAGES2] = nodes2, messages2
        put(self_, 0x50, 'II', sentinel2, len(others2))
        if op in ('Del2', 'Del1'):
            return Call(ADDRESS[op], [SELF, r.choice([0, 1, 2, 3, 0xfffffffe])], regions, conv='t', ret='a')
        return Call(ADDRESS[op], [SELF], regions, conv='t', ret='v')
    if op in ('KeyDel', 'MouseDel'):
        put(self_, 8, 'I', sentinel)
        put(self_, 0xc, 'I', count)
        return Call(ADDRESS[op], [SELF, r.choice([0, 1, 2, 3, 0xfffffffe])], regions, conv='t', ret='a')
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
