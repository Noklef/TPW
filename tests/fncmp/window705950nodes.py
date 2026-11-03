# The nodes of the list window (src/game/ui/window705950nodes) against the original: a tree of 0x54-byte nodes in depth-first order, mostly true to the count, the texts from a few short ones so that the
# comparisons go every way; the heap's first allocation failing now and then; the bar (child 1) a window whose slot 5 is a recorder; the post's queues as window.py's. A row with no children under flag-less
# InsertSorted, and a negative n other than -1 with no list to go in front of, fault in both.
# usage: window705950nodes.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=InsertSorted,InsertAt,RemoveNode,FreeNodes,Clear,SetNodeText,GetNodeText)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, log_region, put, vtable  # noqa: E402
from window import RANGES, queues  # noqa: E402

CALLS = {'InsertSorted': 0x65ae3e, 'InsertAt': 0x65b20b, 'RemoveNode': 0x65b486, 'FreeNodes': 0x65b66a, 'Clear': 0x65b6d6, 'SetNodeText': 0x65b79c, 'GetNodeText': 0x65b7db}
B = Bench(RANGES + [(0x65ae3e, 0x65b20b), (0x65b20b, 0x65b486), (0x65b486, 0x65b66a), (0x65b66a, 0x65b6d6), (0x65b6d6, 0x65b766), (0x65b79c, 0x65b7db), (0x65b7db, 0x65b818), (0x65ada4, 0x65ae3c),
                    (0x66b47d, 0x66b518), (0x66b53e, 0x66b567), (0x67b0d0, 0x67b140), (0x67bf10, 0x67bf80), (0x67c290, 0x67c2e2)])
LIST, BAR, OTHER, VT, TEXT, OUT = RAM + 0xa000, RAM + 0xa200, RAM + 0xa400, RAM + 0xa600, RAM + 0xa700, RAM + 0xa800
NODES = RAM + 0xb000
WORDS = ['', 'A', 'B', 'AB', 'AA', 'M', 'Z', 'a', 'MM', 'ZZZZ']


def wide(s):
    return s.encode('utf-16-le') + b'\0\0'


def tree(r, regions):
    """0-12 nodes of 0x54 bytes in depth-first order; their number."""
    n = r.choice([0, 1, 2, 4, 7, 12])
    bufs = [bytearray(r.bytes(0x54)) for _ in range(n)]
    addr = [NODES + 0x60 * i for i in range(n)]
    for b in bufs:
        text = wide(r.choice(WORDS))
        b[:len(text)] = text
        put(b, 0x44, 'IIII', 0, 0, 0, 0)
    path, last = [], [None]
    for i in range(n):
        depth = r.randrange(len(path) + 1)
        while len(path) > depth:
            path.pop()
            last.pop()
        if path:
            put(bufs[i], 0x4c, 'I', addr[path[-1]])
            if last[-1] is None:
                put(bufs[path[-1]], 0x48, 'I', addr[i])
        if last[-1] is not None:
            put(bufs[last[-1]], 0x44, 'I', addr[i])
            put(bufs[i], 0x50, 'I', addr[last[-1]])
        last[-1] = i
        path.append(i)
        last.append(None)
    for i, b in enumerate(bufs):
        regions[addr[i]] = b
    return n


def case(r):
    op = r.deal(OPS)
    log = log_region()
    regions = {LOG: log, VT: vtable({5: 1}), HEAP: heap(fail=r.choice([0] * 12 + [1]))}
    count = tree(r, regions)
    win = bytearray(r.bytes(0x160))
    put(win, 0x144, 'I', NODES if count else (r.getrandbits(1) * 0))
    put(win, 0x14c, 'i', r.choice([count] * 8 + [count + 1, max(count - 1, 0)]))
    put(win, 0x48, 'I', r.choice([0, 0x10, 0x10, 0x20, 0x20, 0x30, r.getrandbits(32)]))
    put(win, 0x148, 'i', r.choice([-1, 0, 1, 2, r.randrange(13), r.randrange(13)]))
    put(win, 0x150, 'i', r.choice([0, 0, 1, 3, r.randrange(13), -1]))
    put(win, 0x156, 'h', r.choice([0, 1, 2, 4, 8, 12, 20, -1]))
    put(win, 0x5c, 'I', r.choice([BAR, BAR, BAR, OTHER, 0]))
    regions[LIST] = win
    regions[BAR] = put(put(put(bytearray(r.bytes(0x160)), 0, 'I', VT), 0x8c, 'I', r.choice([1, 1, 1, 2, 0x10001])), 0x54, 'I', r.choice([0, OTHER]))
    regions[OTHER] = put(put(put(bytearray(r.bytes(0x160)), 0, 'I', VT), 0x8c, 'I', r.choice([1, 3])), 0x54, 'I', 0)
    regions[0xfaa5d4] = struct.pack('<I', r.choice([0, 1]))
    queues(r, regions)
    long = bytes(r.randrange(0x41, 0x5b) if i % 2 == 0 else 0 for i in range(0x60))   # (past the copy's 0x20 units)
    regions[TEXT] = r.choice([wide(w) for w in WORDS] + [long + b'\0\0'])
    regions[OUT] = r.bytes(0x100)
    n = r.choice([-1, -1, 0, 0, r.randrange(max(count, 1)), r.randrange(max(count, 1)), r.randrange(max(count, 1)), count - 1, count, count + 1, -2, r.getrandbits(32)]) & 0xffffffff
    if op == 'InsertSorted':
        return Call(CALLS[op], [LIST, n, TEXT, r.getrandbits(32)], regions, conv='t', ret='w')
    if op == 'InsertAt':
        position = r.choice([0, 0, 1, 2, 3, r.randrange(13), -1, count, r.getrandbits(32)]) & 0xffffffff
        return Call(CALLS[op], [LIST, n, position, TEXT, r.getrandbits(32)], regions, conv='t', ret='w')
    if op == 'RemoveNode':
        if count > 1 and r.random() < 0.4:   # (a row in the tree that is just past those shown or the first shown, with the post getting through)
            rows = struct.unpack_from('<h', win, 0x156)[0]
            n = r.randrange(1, count)
            put(win, 0x150, 'i', r.choice([n - rows, n]))
            put(win, 0x14c, 'i', count)
            win[4] = 0
            regions[0xfaa5d4] = struct.pack('<I', 1)
        return Call(CALLS[op], [LIST, n & 0xffffffff], regions, conv='t', ret='w')
    if op == 'FreeNodes':
        return Call(CALLS[op], [NODES if count and r.random() < 0.9 else 0], regions, ret='a')
    if op == 'Clear':
        return Call(CALLS[op], [LIST], regions, conv='t', ret='v')
    if op == 'SetNodeText':
        return Call(CALLS[op], [LIST, n, TEXT], regions, conv='t', ret='w')
    return Call(CALLS[op], [LIST, n, OUT], regions, conv='t', ret='w')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
