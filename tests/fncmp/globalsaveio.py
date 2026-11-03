# The global save info written and read (src/game/core/globalsave: 0x5aff50) against the original. Its fields, then the theme map's names and records, the sound options and the ride ids; the strings, the two trees' iterators, the field readers and the
# stream's two front ends real, the stream's slots recorders (a write answered from a table, now and then short; a read a sequenced read, now and then cut), the record's serialise, the options' and the two inserts recorders (driver.cpp's kGlobalSaveCalls).
# usage: globalsaveio.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Save,Load)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, answers, blob, blob_bytes, fill, heap, log_fold, log_region, put, seq_read, set_return, strings, vtable  # noqa: E402
import gamestring  # noqa: E402
import globalsave  # noqa: E402
import stdtree  # noqa: E402

READ, WRITE, OPTIONS, RECORD, INSERT_THEME, INSERT_RIDE = 2, 3, 4, 5, 6, 7
CALLS = {0x423e90: (OPTIONS, 3), 0x5b0d70: (RECORD, 3), 0x5b1620: (INSERT_THEME, 2), 0x5b1650: (INSERT_RIDE, 2)}
HOOKS = {addr: _recorder(k, n) for addr, (k, n) in CALLS.items()}
HOOKS.update({0x691de0: globalsave.lock, 0x691ea0: lambda e: None})
B = Bench([(0x5aff50, 0x5b089b), (0x5f5e20, 0x5f5e90), (0x4d37e0, 0x4d384e), (0x4fae10, 0x4fae7e), (0x416e10, 0x416e15), (0x5b2480, 0x5b2530), (0x5b2e90, 0x5b2f40), (0x5da3c0, 0x5da3c1)] + gamestring.B_RANGES, hooks=HOOKS)
OPS = os.environ.get('OPS', 'Save,Load').split(',')
INFO, STREAM, VT, DATA, TABLE, TABLE2 = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x400, RAM + 0x800, RAM + 0x900
THEME_HEAD, RIDE_HEAD, THEME_NIL, RIDE_NIL = RAM + 0xa00, RAM + 0xa40, RAM + 0xa80, RAM + 0xac0
THEMES, RIDES, TEXTS, MAPPED = RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
OPTIONS_OBJECT = 0x78d8d8


def dwords(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


def tree(r, regions, n, head, nil, nodes_at, stride, each):
    """A red-black tree of n nodes, stride bytes each, its links to nil and head; each(block, offset, index) fills a node's other fields."""
    nodes, root = stdtree.build(r, n) if n else ({}, None)
    keys = sorted(nodes)
    addr = {k: nodes_at + stride * i for i, k in enumerate(keys)}
    block = bytearray(r.bytes(stride * n))
    for i, k in enumerate(keys):
        left, parent, right, colour = nodes[k]
        put(block, stride * i, 'III', nil if left is None else addr[left], head if parent is None else addr[parent], nil if right is None else addr[right])
        put(block, stride * i + 0x24, 'I', colour)
        each(block, stride * i, i)
    if n:
        regions[nodes_at] = block
    regions[head] = struct.pack('<III', addr[keys[0]], addr[root], addr[keys[-1]]) if n else struct.pack('<III', head, nil, head)
    regions[nil] = bytes(0x24) + dwords(stdtree.BLACK)


def info_block(r):
    info = bytearray(r.bytes(0x60))
    put(info, 0x2c, 'I', RIDE_HEAD)
    put(info, 0x3c, 'I', THEME_HEAD)
    return info


def save(r, regions, log):
    n, keys = r.choice([0, 1, 2, 2]), r.choice([0, 1, 2, 2])
    lengths = [r.randrange(4) for _ in range(n)]
    texts = bytearray(r.bytes(0x10 * max(n, 1)))

    def theme(block, at, i):
        put(block, at + 0x10, 'I', lengths[i])
        put(block, at + 0x14, 'I', TEXTS + 0x10 * i)
        put(block, at + 0x20, 'I', MAPPED + 0x100 * i)
        texts[0x10 * i:0x10 * (i + 1)] = bytes(b | 1 for b in texts[0x10 * i:0x10 * (i + 1)])
        texts[0x10 * i + lengths[i]] = 0

    tree(r, regions, n, THEME_HEAD, THEME_NIL, THEMES, 0x40, theme)
    tree(r, regions, keys, RIDE_HEAD, RIDE_NIL, RIDES, 0x40, lambda block, at, i: put(block, at + 0xc, 'H', r.getrandbits(16)))
    regions[TEXTS] = texts
    regions[0xf7c968] = dwords(THEME_NIL)
    regions[0xf7c970] = dwords(RIDE_NIL)
    info = info_block(r)
    put(info, 0x34, 'I', keys)
    put(info, 0x44, 'I', n)
    regions[INFO] = info
    expected = [1] * 6 + [4, 4, 1, 1, 1, 4]
    for length in lengths:
        expected += [4] + [1] * length
    expected += [4] + [2] * keys
    if r.random() < 0.3:
        i = r.randrange(len(expected))
        expected[i] = r.choice([0, expected[i] - 1, 0xffffffff, expected[i] + 1])
    regions[TABLE] = dwords(*(expected + [0] * (64 - len(expected))))
    answers(log, WRITE, TABLE)
    blob_bytes(log, WRITE, 0, 0xf)
    regions[TABLE2] = dwords(*[r.choice([0, 0, 0, 0, 0, 1]) for _ in range(40)])
    answers(log, RECORD, TABLE2)
    return r.choice([1, 1, r.getrandbits(32) | 1])


def load(r, regions, log):
    count, keys = r.choice([0, 1, 2, 2, 0xffffffff]), r.choice([0, 1, 2, 2, 0xfffffffb])
    data = bytearray(r.bytes(6)) + r.bytes(8) + r.bytes(3) + dwords(count)
    for _ in range(count if count < 0x80 else 0):
        n = r.randrange(4)
        data += dwords(n) + r.bytes(n)
    data += dwords(keys)
    for _ in range(keys if keys < 0x80 else 0):
        data += r.bytes(2)
    if r.random() < 0.2:
        data = data[:r.randrange(len(data))]
    regions[DATA] = bytes(data) + bytes(4)
    seq_read(log, READ, DATA, len(data), buffer=0, count=1)
    strings(log, READ, 0, out=1)
    regions[INFO] = info_block(r)
    regions[TABLE2] = dwords(*[r.choice([0, 0, 0, 0, 0, 1]) for _ in range(40)])
    answers(log, RECORD, TABLE2)
    strings(log, INSERT_THEME, 0, out=1)
    blob(log, INSERT_THEME, 1, 0x3f)
    fill(log, INSERT_THEME, 0, dwords(r.getrandbits(32), r.choice([1, 1, 1, 0x101, 0]), r.getrandbits(32), r.choice([1, 1, 0xffffff01, 0x100])), records=2)
    strings(log, INSERT_RIDE, 0, out=1)
    blob_bytes(log, INSERT_RIDE, 1, 3)
    return 0


def case(r):
    op = r.deal(OPS)
    log = log_region()
    log_fold(log)
    regions = {LOG: log, STREAM: dwords(VT, 0, 0, 0), VT: vtable({2: 2, 3: 2})}
    set_return(log, OPTIONS, r.choice([0, 0, 0, 0, 0, 1]))
    saving = save(r, regions, log) if op == 'Save' else load(r, regions, log)
    regions[HEAP] = heap(0x1000, fail=r.choice([0] * 26 + [1, 2, 3, 4]))
    return Call(0x5aff50, [INFO, STREAM, saving, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (k, n) in sorted(CALLS.items()):
            print('    {0x%06x, recorder(%d, %d)},' % (addr, k, n))
        sys.exit(0)
    B.main(case)
