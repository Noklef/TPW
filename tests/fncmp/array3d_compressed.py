# The array's compressed read (src/game/gfx/array3d: 0x5e7990) against the original: an array with an old block or none, a record stream of a
# header (width and height 0-3, now and then larger; depth 1, 3 or 4, or another) and the depth's part headers and bytes, sometimes cut short
# in the part bytes (never in a header: the original would then use uninitialised stack); the allocations failing now and then (fncmp's
# heap); the codec a recorder (slot 3, eight arguments) that now and then writes a few bytes into its output.
# usage: array3d_compressed.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, fill, heap, log_region, put, record_stream, vtable  # noqa: E402

B = Bench([(0x5e7990, 0x5e7d62), (0x5c4f60, 0x5c4f90)])
OBJ, STREAM, CODEC, CVT, DATA = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000


def case(r):
    obj = bytearray(r.bytes(0x20))
    put(obj, 0xc, 'I', r.choice([0, HEAP + 0x3f00]))
    if r.wild and r.random() < 0.3:
        w, h = r.randrange(4, 17), r.randrange(4, 17)
    else:
        w, h = r.randrange(0, 4), r.randrange(0, 4)
    depth = r.choice([1, 1, 3, 3, 4, 4, 4, 0, 2, 5])
    head = struct.pack('<3I', w, h, depth)
    lengths = []
    if depth == 1:
        total = r.randrange(0, 0x30)
        lengths = [r.choice([total, r.randrange(0, total + 1)])]
        head += struct.pack('<2I', total, lengths[0])
    elif depth == 3:
        lengths = [r.randrange(0, 0x30)]
        head += struct.pack('<4I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), lengths[0])
    elif depth == 4:
        lengths = [r.randrange(0, 0x30), r.randrange(0, 0x30)]
        head += struct.pack('<6I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.randrange(0, 0x40), *lengths)
    body = b''.join(r.bytes(n) for n in lengths)
    left = len(head) + len(body) + r.choice([0, 0, 4])
    if body and r.random() < 0.2:
        left = len(head) + r.randrange(len(body))
    log = log_region()
    if r.random() < 0.5:
        fill(log, 3, 4, r.bytes(r.choice([1, 3, 4])))
    fail = r.choice([0] * 12 + [1, 2, 3, 4, 5])
    regions = {OBJ: obj, STREAM: record_stream(STREAM, DATA, left), DATA: head + body + r.bytes(4), HEAP: heap(0x2000, fail), LOG: log,
               CODEC: struct.pack('<I', CVT) + r.bytes(0xc), CVT: vtable({3: 8})}
    return Call(0x5e7990, [OBJ, STREAM, CODEC], regions, conv='t', ret='b')


if __name__ == '__main__':
    B.main(case)
