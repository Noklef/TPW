# The MAP chunk handler (src/game/terrain/attrmap: 0x519390, 0x4d8cf0; the vtable 0x700aa0's slots 2 and 3) against the originals.
# Match: the handler's tag +4 and the tag asked for equal or not. Read: a chunk (+0 width, +4 height: 0-20 by 0-140 now and then 0 or
# negative; its bytes from +0x1c) into the map cells' bytes +0x26 (the handler's +8 the cells, 0x44 bytes, 128 to a row; the first 16
# rows a region, the offsets 0x785584 / 0x785588 such that the rows written stay in them, now and then one past: a fault in both), the
# columns and rows past the map's 128 skipped.
# Chunks (0x5192d0, the TP2M reader: a file on the stack with one MAP handler, ChunkFile_Open of the name): chunkfile.py's opened() file
# (the heap never failing: a failed node faults in both) over a TP2M file (the magic mostly; a header; 0-3 chunks, "MAP " mostly (one
# width and height for the case, the offsets as Read's; its length the body's mostly), "END ", "DATA" or any), the handler's cells Read's.
# usage: attrmapchunk.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Match,Read,Chunks)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import chunkfile  # noqa: E402
import vfs  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, log_region, put  # noqa: E402
from texdirs import merged  # noqa: E402

CALLS = {'Match': 0x519390, 'Read': 0x4d8cf0, 'Chunks': 0x5192d0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench(merged(chunkfile.RANGES + [(0x519390, 0x5193a3), (0x4d8cf0, 0x4d8d98), (0x5192d0, 0x519387), (0x4d3850, 0x4d3860), (0x424cf0, 0x424d20),
                                     (0x5c5480, 0x5c5520), (0x5e2a30, 0x5e2a90), (0x5f6040, 0x5f6160), (0x5f61a0, 0x5f6210), (0x5f7180, 0x5f71a0),
                                     (0x5f76c0, 0x5f7740), (0x5f7950, 0x5f7990), (0x5f7d60, 0x5f7e70)]))
HANDLER, CHUNK, CELLS = RAM, RAM + 0x1000, RAM + 0x10000
ROWS = 16


def offsets(r, w):
    x0 = r.choice([0, 0, 1, 5, -3, 12, r.randrange(-10, 20)])   # (0x785584: a row's x is the row less it)
    y0 = r.choice([max(w, 0) - ROWS, 0, 2, max(w, 0) - 1, max(w, 0) + 2, r.randrange(max(w, 0) - ROWS, max(w, 0) + 4)])   # (0x785588)
    if r.random() < 0.03:
        y0 = max(w, 0) - ROWS - 1
    return struct.pack('<ii', x0, y0)


def chunks_case(r):
    """Chunks (the module comment)"""
    w = r.choice([0, 1, 2, 5, 16, r.randrange(17), -1])
    h = r.choice([0, 1, 3, 40, 128, 129, r.randrange(141), -3])
    raw = bytearray(struct.pack('<I', r.choice([chunkfile.MAGIC] * 8 + [chunkfile.MAGIC ^ 1])) + r.bytes(0x20))
    for _ in range(r.choice([0, 1, 1, 2, 3])):
        tag = r.choice([0x2050414d] * 4 + [0x20444e45, 0x41544144, r.getrandbits(32)])
        if tag == 0x2050414d:
            body = struct.pack('<ii', w, h) + r.bytes(0x14 + max(w, 0) * max(h, 0))
        else:
            body = r.bytes(r.choice([0, 0, 4, r.randrange(0x20)]))
        length = r.choice([len(body)] * 10 + [max(len(body) - 1, 0), 0xffff])
        raw += struct.pack('<Ii', tag, length) + body
    log = log_region()
    regions = {LOG: log}
    chunkfile.opened(r, log, regions, raw, fail=False)
    regions.update({CELLS: r.bytes(0x44 * (128 * ROWS + 1)), 0x785584: offsets(r, w)})
    # (the heap's first block is the list's node: its item the handler's stack address, which the two sides' stacks cannot share)
    return Call(CALLS['Chunks'], [CELLS + 0x44, vfs.NAME], regions, conv='t', ret='a', ignore=[(HEAP + 0x10, 4)])


def case(r):
    op = r.deal(OPS)
    if op == 'Chunks':
        return chunks_case(r)
    handler = bytearray(r.bytes(0x10))
    put(handler, 8, 'I', CELLS + 0x44)   # (the cells' base: cell id 1 the first)
    regions = {HANDLER: handler}
    if op == 'Match':
        tag = r.choice([0x5041414d, 0x4d4150, 0, r.getrandbits(32)])
        put(handler, 4, 'I', tag)
        asked = r.choice([tag, tag, tag ^ 1, tag ^ 0x80000000, r.getrandbits(32)])
        return Call(CALLS['Match'], [HANDLER, asked], regions, conv='t', ret='a')
    w = r.choice([0, 1, 2, 5, 16, 20, r.randrange(21), -1])
    h = r.choice([0, 1, 3, 100, 128, 129, 140, r.randrange(141), -3])
    chunk = bytearray(r.bytes(0x1c + max(w, 0) * max(h, 0) + 4))
    put(chunk, 0, 'ii', w, h)
    regions.update({CHUNK: chunk, CELLS: r.bytes(0x44 * (128 * ROWS + 1)), 0x785584: offsets(r, w)})
    return Call(CALLS['Read'], [HANDLER, CHUNK, r.getrandbits(32)], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
