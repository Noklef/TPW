# The chunked file's stream reader (src/game/files/chunkfile: 0x4d3860) against the original: a stream recorder whose size (slot 5) and read (slot 2)
# answer the data's size, -1, 0, short or long ones; the block from the heap (failing now and then) holding the data as read (the read recorder writes
# nothing): the magic (the file's +0x10 or not), a header, chunks of 2-3 tags ("END " among them) whose lengths are small, 0, past the end or far
# below the block; 0-3 handlers (a recorder pair each, their answers 0 or not) on the list +4.
# usage: chunkfile.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

B = Bench([(0x4d3860, 0x4d3a60), (0x405710, 0x405760), (0x5f5660, 0x5f56a0), (0x67ae00, 0x67af10), (0x5da3c0, 0x5da3c1)])
FILE, STREAM, NODES, HANDLERS, NAME = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x200, RAM + 0x300
SVT, HVT = RAM + 0x400, RAM + 0x500
MAGIC, TAGS = 0x4d325054, [0x2050414d, 0x20444e45, 0x41544144]
SIZE, CLOSE, READ = 5, 1, 2


def data(r):
    """the file's bytes: the magic (or a near miss), a header of 0x20, up to 5 chunks, now and then trailing bytes"""
    out = bytearray(struct.pack('<I', r.choice([MAGIC] * 8 + [MAGIC ^ 1, 0])) + r.bytes(0x20))
    for _ in range(r.choice([0, 1, 2, 3, 5])):
        tag = r.choice(TAGS + [r.getrandbits(32)])
        n = r.choice([0, 0, 1, 4, 7, r.randrange(0x40)])
        body = r.bytes(n)
        length = r.choice([n] * 12 + [0x7fffffff, -0x100000, n + 9, 0xffff])
        out += struct.pack('<Ii', tag, length) + body
    if r.random() < 0.3:
        out += r.bytes(r.randrange(1, 12))
    return out


def case(r):
    log = log_region()
    raw = data(r)
    size = r.choice([len(raw)] * 10 + [-1, 0, 0x2b, 0x2c, len(raw) - 1, len(raw) + 5, 0x80000000])
    set_return(log, SIZE, size)
    set_return(log, READ, r.choice([size] * 8 + [size - 1, 0, -1]))
    block = heap(0x800, fail=r.choice([0] * 12 + [1]))
    block[0x10:0x10 + len(raw)] = raw
    n = r.choice([0, 1, 2, 3, 3])
    nodes, handlers, hvt = bytearray(8 * max(n, 1)), bytearray(4 * max(n, 1)), bytearray(8 * max(n, 1))
    for i in range(n):
        put(nodes, 8 * i, 'II', HANDLERS + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
        put(handlers, 4 * i, 'I', HVT + 8 * i)
        put(hvt, 8 * i, 'II', recorder(10 + 2 * i, 1), recorder(11 + 2 * i, 2))
        set_return(log, 10 + 2 * i, r.choice([0, 1, 1, 0x100, 0x80000000]))
    f = bytearray(r.bytes(0x14))
    put(f, 4, 'I', NODES if n else 0)
    put(f, 0x10, 'I', MAGIC)
    regions = {LOG: log, HEAP: block, FILE: f, STREAM: struct.pack('<I', SVT), SVT: vtable({READ: 2}, size=8), NODES: nodes, HANDLERS: handlers, HVT: hvt,
               NAME: b'maps\\park01.tp2' + bytes(1) + r.bytes(0x10), 0x785de8: r.bytes(0x5c)}
    regions.update(global_heap())
    return Call(0x4d3860, [FILE, STREAM, NAME], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
