# The chunked file's stream reader (src/game/files/chunkfile: 0x4d3860) against the original: a stream recorder whose size (slot 5) and read (slot 2)
# answer the data's size, -1, 0, short or long ones; the block from the heap (failing now and then) holding the data as read (the read recorder writes
# nothing): the magic (the file's +0x10 or not), a header, chunks of 2-3 tags ("END " among them) whose lengths are small, 0, past the end or far
# below the block; 0-3 handlers (a recorder pair each, their answers 0 or not) on the list +4. Open (0x4d3a60): the file opened by name
# through vfs.py's mounts (as tct.py's), its memory source over such data mostly (else the mounts' own answers), the size recorder (k 8) the
# data's mostly; the heap one block for both (failing now and then), the handlers' recorders from k 20 (vfs.py's take 2-11). opened()
# builds Open's file environment for other benches (attrmap.py's TP2M reader), RANGES its listing.
# usage: chunkfile.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Read,Open)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, global_heap, heap, log_region, put, recorder, set_return, vtable  # noqa: E402
import vfs  # noqa: E402
from gamestring import LOCALE  # noqa: E402
from texdirs import merged  # noqa: E402

RANGES = merged(vfs.RANGES + [(0x4d3860, 0x4d3a9e), (0x405710, 0x405760), (0x5f5660, 0x5f56a0), (0x5c4d50, 0x5c5020), (0x5da3c0, 0x5da3c1),
                              (0x67a400, 0x690000)])
B = Bench(RANGES)
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


def handlers(r, log, base):
    """0-3 handlers on the list (nodes, handler table, their vtables), recorders base + 2i (slot 0) and base + 2i + 1 (slot 1)"""
    n = r.choice([0, 1, 2, 3, 3])
    nodes, table, hvt = bytearray(8 * max(n, 1)), bytearray(4 * max(n, 1)), bytearray(8 * max(n, 1))
    for i in range(n):
        put(nodes, 8 * i, 'II', HANDLERS + 4 * i, NODES + 8 * (i + 1) if i + 1 < n else 0)
        put(table, 4 * i, 'I', HVT + 8 * i)
        put(hvt, 8 * i, 'II', recorder(base + 2 * i, 1), recorder(base + 1 + 2 * i, 2))
        set_return(log, base + 2 * i, r.choice([0, 1, 1, 0x100, 0x80000000]))
    return n, {NODES: nodes, HANDLERS: table, HVT: hvt}


def opened(r, log, regions, raw, fail=True):
    """Open's file (the module comment): the heap (failing now and then when fail), vfs.py's mounts and memory source over raw, the name
    at vfs.NAME, the error record 0x785de8."""
    regions.update({HEAP: heap(0x8000, fail=r.choice([0] * 12 + [1]) if fail else 0), LOCALE: bytes(4), **global_heap()})
    root = vfs.mounts(r, log, regions)
    if r.random() < 0.85:
        vfs.memory_source(r, log, regions, raw)
        set_return(log, 8, r.choice([len(raw)] * 10 + [0, 0x2b, len(raw) - 1, len(raw) + 5, -1]))
    regions[vfs.NAME] = vfs.name_under(r, root) + r.choice([b'.tp2', b'', b'.TP2']) + bytes(1) + r.bytes(7)
    regions[0x785de8] = r.bytes(0x5c)


def open_case(r):
    """Open (above)"""
    log = log_region()
    regions = {LOG: log}
    opened(r, log, regions, data(r))
    n, parts = handlers(r, log, 20)
    regions.update(parts)
    f = bytearray(r.bytes(0x14))
    put(f, 4, 'I', NODES if n else 0)
    put(f, 0x10, 'I', MAGIC)
    regions[FILE] = f
    return Call(0x4d3a60, [FILE, vfs.NAME], regions, conv='t', ret='a')


def case(r):
    if r.deal(OPS) == 'Open':
        return open_case(r)
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


OPS = [k for k in os.environ.get('OPS', 'Read,Open').split(',') if k]


if __name__ == '__main__':
    B.main(case)
