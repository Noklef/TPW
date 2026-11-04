# The list members of src/game/core/std_lists against the originals: lists of 0 to 6 counted strings with runs of equal ones (the count bytes at the edges, null
# characters for empty strings, lengths that differ); sorted lists of dwords of 0 to 6 elements merged (and a list merged with itself); lists of strings merged (0x5d03e0); lists sorted (0x5ce190, 0x5ce5a0).
# usage: std_lists.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Unique,Merge,MergeStrings,SortStrings,SortDwords)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, Bench, Call, HEAP, RAM, _recorder, heap, log_region, mask_this, put  # noqa: E402

CALLS = {'Unique': 0x5ce080, 'Merge': 0x5d0590, 'MergeStrings': 0x5d03e0, 'SortStrings': 0x5ce190, 'SortDwords': 0x5ce5a0, 'Add64': 0x5cc640, 'Add70': 0x5cc770, 'AddString': 0x5cd510}
DWORD_CLEAR = 3
B = Bench([(0x5ce080, 0x5ce190), (0x5d0590, 0x5d0690), (0x5d03e0, 0x5d0510), (0x5d1560, 0x5d15c9), (0x67acb0, 0x67acb1), (0x67acc0, 0x67acc1), (0x5ce190, 0x5ce530), (0x5ce5a0, 0x5ce910), (0x5ce530, 0x5ce540),
           (0x5d0a50, 0x5d0a90), (0x5d03b0, 0x5d03e0), (0x5d1330, 0x5d1390), (0x5cf7e0, 0x5cf840), (0x5cde80, 0x5cdf00), (0x67acf0, 0x67ada0), (0x67afa0, 0x67b0d0),
           (0x5cc640, 0x5cc700), (0x5cc770, 0x5cc830), (0x5cd510, 0x5cd660), (0x5d0330, 0x5d03b0), (0x5cdf70, 0x5cdfdd), (0x45b340, 0x45b9a0)],
          hooks={0x634bf0: _recorder(DWORD_CLEAR, 0)})
LIST, HEAD, NODES, TEXT, LIST2, HEAD2, NODES2 = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x4000, RAM + 0x200, RAM + 0x300, RAM + 0x2000
OWNER, KEY, X = RAM + 0x400, RAM + 0x500, RAM + 0x600


def circular(head, nodes, count, size_of=0x20):
    """next/prev words for a head and count nodes at nodes (size_of apart): returns (head bytes, node addresses)."""
    addrs = [nodes + size_of * k for k in range(count)]
    ring = [head] + addrs
    return ring


def case(r):
    name = r.choice(OPS)
    if name in ('Add64', 'Add70'):   # (the key at [+0x7c]+8 pushed, sorted, duplicates cut)
        n = r.choice([0, 1, 2, 3, 4, 5, 8, 13])
        log = log_region()
        mask_this(log, DWORD_CLEAR)
        key = r.choice([0, 1, 2, 3, r.getrandbits(32)])
        owner = bytearray(r.bytes(0x80))
        put(owner, 0x7c, 'I', KEY)
        offset = 0x64 if name == 'Add64' else 0x70
        put(owner, offset + 4, 'II', HEAD, n)
        regions = {LOG: log, OWNER: owner, KEY: r.bytes(8) + struct.pack('<I', key), HEAD: bytearray(r.bytes(0x20)), NODES: bytearray(r.bytes(0x20 * 13)), HEAP: heap()}
        ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
        for i, addr in enumerate(ring):
            buf, off = (regions[HEAD], 0) if addr == HEAD else (regions[NODES], addr - NODES)
            put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
            if addr != HEAD:
                put(buf, off + 8, 'I', r.choice([0, 1, 2, 3, key, r.getrandbits(32)]))
        return Call(CALLS[name], [OWNER], regions, conv='t', ret='v')
    if name == 'AddString':   # (x's characters null when empty, or x a node's own string: erased as a duplicate it reads empty, not found: -1)
        words = [None, b'', b'a', b'a', b'ab', b'b', b'abc', b'B']
        n = r.choice([0, 1, 2, 3, 4, 6, 9])
        log = log_region()
        regions = {LOG: log, LIST: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x20)), NODES: bytearray(r.bytes(0x20 * 9)), TEXT: bytearray(r.bytes(0x400)), HEAP: heap()}
        ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
        for i, addr in enumerate(ring):
            buf, off = (regions[HEAD], 0) if addr == HEAD else (regions[NODES], addr - NODES)
            put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
            if addr != HEAD:
                w = r.choice(words)
                at = 0x20 * (i - 1)
                regions[TEXT][at] = r.choice([0, 1, 2, 0xfe, 0xff])
                if w:
                    regions[TEXT][at + 1:at + 1 + len(w)] = w
                put(buf, off + 0xc, 'III', 0 if w is None else TEXT + at + 1, len(w or b''), len(w or b''))
        put(regions[LIST], 4, 'II', HEAD, n)
        if n and r.random() < 0.3:
            return Call(CALLS[name], [LIST, NODES + 0x20 * r.randrange(n) + 8], regions, conv='t', ret='a')
        w = r.choice(words)
        x = bytearray(r.bytes(0x10))
        regions[TEXT][0x200] = r.choice([0, 1, 2, 0xfe, 0xff])
        if w:
            regions[TEXT][0x201:0x201 + len(w)] = w
        put(x, 4, 'III', 0 if w is None else TEXT + 0x201, len(w or b''), len(w or b'') + r.choice([0, 0, 4]))
        regions[X] = x
        return Call(CALLS[name], [LIST, X], regions, conv='t', ret='a')
    if name == 'SortStrings':   # (with ties; null characters for empty strings; the heads from the heap)
        words = [None, b'', b'a', b'a', b'ab', b'b', b'abc', b'B', b'\xff', b'c', b'ca']
        n = r.choice([0, 1, 2, 3, 4, 5, 8, 13, 17, 24])
        regions = {LIST: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x20)), NODES: bytearray(r.bytes(0x20 * 24)), TEXT: bytearray(r.bytes(0x400)), HEAP: heap()}
        ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
        for i, addr in enumerate(ring):
            buf, off = (regions[HEAD], 0) if addr == HEAD else (regions[NODES], addr - NODES)
            put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
            if addr != HEAD:
                w = r.choice(words)
                text = 0 if w is None else TEXT + 8 * i
                if w:
                    regions[TEXT][8 * i:8 * i + len(w)] = w
                put(buf, off + 0xc, 'III', text, len(w or b''), len(w or b''))
        put(regions[LIST], 4, 'II', HEAD, n)
        return Call(CALLS[name], [LIST], regions, conv='t', ret='v')
    if name == 'SortDwords':   # (with ties; the lists' clear a recorder on locals: their blocks stay)
        n = r.choice([0, 1, 2, 3, 4, 5, 8, 13, 17, 24])
        log = log_region()
        mask_this(log, DWORD_CLEAR)
        regions = {LOG: log, LIST: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x20)), NODES: bytearray(r.bytes(0x20 * 24)), HEAP: heap()}
        ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
        for i, addr in enumerate(ring):
            buf, off = (regions[HEAD], 0) if addr == HEAD else (regions[NODES], addr - NODES)
            put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
            if addr != HEAD:
                put(buf, off + 8, 'I', r.choice([0, 1, 2, 3, 5, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)]))
        put(regions[LIST], 4, 'II', HEAD, n)
        return Call(CALLS[name], [LIST], regions, conv='t', ret='v')
    if name == 'MergeStrings':   # (sorted or not; null characters for empty strings, this list's and the other's)
        words = [None, b'', b'a', b'a', b'ab', b'b', b'abc', b'B', b'\xff']
        regions = {LIST: bytearray(r.bytes(0x10)), LIST2: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x20)), HEAD2: bytearray(r.bytes(0x20)), NODES: bytearray(r.bytes(0x20 * 8)), NODES2: bytearray(r.bytes(0x20 * 8)), TEXT: bytearray(r.bytes(0x400))}
        text_at = 0
        for lst, head, nodes in ((LIST, HEAD, NODES), (LIST2, HEAD2, NODES2)):
            vals = [r.choice(words) for _ in range(r.choice([0, 1, 2, 4, 6]))]
            if r.random() < 0.7:
                vals.sort(key=lambda v: v or b'')
            ring = [head] + [nodes + 0x20 * k for k in range(len(vals))]
            for i, addr in enumerate(ring):
                buf, off = (regions[head], 0) if addr == head else (regions[nodes], addr - nodes)
                put(buf, off, 'II', ring[(i + 1) % len(ring)], ring[i - 1])
                if addr != head:
                    w = vals[i - 1]
                    text = 0 if w is None else TEXT + text_at
                    if w:
                        regions[TEXT][text_at:text_at + len(w)] = w
                    text_at += 8
                    put(buf, off + 0xc, 'III', text, len(w or b''), len(w or b''))
            put(regions[lst], 4, 'II', head, len(vals))
        other = LIST if r.random() < 0.05 else LIST2
        return Call(CALLS[name], [LIST, other], regions, conv='t', ret='v')
    if name == 'Merge':
        n1, n2 = r.choice([0, 1, 2, 4, 6]), r.choice([0, 1, 3, 6])
        a, b = sorted(r.choice(range(0, 12)) for _ in range(n1)), sorted(r.choice(range(0, 12)) for _ in range(n2))
        regions = {LIST: bytearray(r.bytes(0x10)), LIST2: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x10)), HEAD2: bytearray(r.bytes(0x10)), NODES: bytearray(0x20 * 8), NODES2: bytearray(0x20 * 8)}
        for lst, head, nodes, vals in ((LIST, HEAD, NODES, a), (LIST2, HEAD2, NODES2, b)):
            ring = [head] + [nodes + 0x20 * k for k in range(len(vals))]
            for i, addr in enumerate(ring):
                nxt, prv = ring[(i + 1) % len(ring)], ring[i - 1]
                buf = regions[head] if addr == head else regions[nodes]
                off = 0 if addr == head else addr - nodes
                put(buf, off, 'II', nxt, prv)
                if addr != head:
                    put(buf, off + 8, 'I', vals[i - 1])
            put(regions[lst], 4, 'II', head, len(vals))
        other = LIST if r.random() < 0.08 else LIST2
        return Call(CALLS[name], [LIST, other], regions, conv='t', ret='v')
    n = r.choice([0, 1, 2, 3, 4, 6])
    words = [b'', b'a', b'a', b'ab', b'b', b'abc']
    regions = {LIST: bytearray(r.bytes(0x10)), HEAD: bytearray(r.bytes(0x10)), NODES: bytearray(r.bytes(0x20 * 8)), TEXT: bytearray(r.bytes(0x400)), HEAP: heap()}
    ring = [HEAD] + [NODES + 0x20 * k for k in range(n)]
    text_at = 0
    for i, addr in enumerate(ring):
        nxt, prv = ring[(i + 1) % len(ring)], ring[i - 1]
        buf = regions[HEAD] if addr == HEAD else regions[NODES]
        off = 0 if addr == HEAD else addr - NODES
        put(buf, off, 'II', nxt, prv)
        if addr != HEAD:
            w = r.choice(words)
            if w == b'' and r.random() < 0.7:
                put(buf, off + 0xc, 'III', 0, 0, 0)
            else:
                text = TEXT + 0x20 * (i - 1) + 1
                regions[TEXT][0x20 * (i - 1)] = r.choice([0, 1, 2, 0xfe, 0xff])
                regions[TEXT][0x20 * (i - 1) + 1:0x20 * (i - 1) + 1 + len(w)] = w
                put(buf, off + 0xc, 'III', text, len(w), len(w))
    put(regions[LIST], 4, 'II', HEAD, n)
    return Call(CALLS[name], [LIST], regions, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
