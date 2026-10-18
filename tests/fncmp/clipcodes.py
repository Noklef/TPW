# The clip codes of a run of points (src/game/render/clipcodes: 0x561bc0, and 0x570d60 that passes its ECX and its third argument on) against the original: 1-9 points (0x10 to 0x30 bytes apart) of floats of either
# sign whose magnitudes tie, lead or trail w's (or random patterns), a random 128-byte code table at 0x879970, masks of all bits or random, and the
# caller's ECX (whose high half the result's top bytes keep) random; and the table's set-up (0x561840: built or, with the block at 0x879a20
# there, not; the block allocated or the heap failing).
# usage: clipcodes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, Bench, Call, RAM, _cdecl_log, heap, log_region, put, strings  # noqa: E402

B = Bench([(0x561bc0, 0x561c3c), (0x570d60, 0x570d8a), (0x5da3c0, 0x5da3c1), (0x561840, 0x561a00), (0x56af20, 0x56af9c), (0x56a770, 0x56ae60), (0x67a400, 0x690000)],
          hooks={0x564650: _cdecl_log})
OUT, CODES, POINTS = RAM + 0x100, RAM + 0x200, RAM + 0x1000


def coord(r, w):
    if r.random() < 0.1:
        return r.getrandbits(32)
    m = r.choice([abs(w), abs(w) * 0.5, abs(w) * 2, 0.0, 1.0, r.uniform(0, 100)])
    return struct.unpack('<I', struct.pack('<f', m if r.random() < 0.5 else -m))[0]


def init(r):   # 0x561840: the table built when [0x879a20] is clear, then its 8-byte block (the heap failing now and then: the error pushed as in ptrarray_add.py)
    log = log_region()
    strings(log, 0x3f, 1)
    errors = bytearray(r.bytes(0x204))
    struct.pack_into('<i', errors, 0x200, r.choice([r.randrange(0x41), 0, 0x3f, 0x40]))
    flags = r.getrandbits(32) & ~0x1000000 | (0x1000000 if r.random() < 0.5 else 0)
    regions = {0x879970: r.bytes(0x80), 0x879a20: struct.pack('<I', r.choice([0, 0, 0, 0x1234])), HEAP: heap(0x100, 1 if r.random() < 0.2 else 0),
               0x879ad0: errors, 0x8bd508: struct.pack('<I', flags), LOG: log}
    return Call(0x561840, [], regions, ret='a')


def case(r):
    op = r.deal([0x561bc0, 0x570d60, 0x561840])
    if op == 0x561840:
        return init(r)
    count = r.choice([1, 1, 2, 3, 4, 9, r.randrange(1, 10)])
    stride = r.choice([0x10, 0x20, 0x20, 0x30])
    points = bytearray(r.bytes(stride * count + 0x10))
    for k in range(count):
        w = r.choice([1.0, 2.5, 0.0, 100.0, r.uniform(-50, 50)])
        wbits = struct.unpack('<I', struct.pack('<f', w if r.random() < 0.8 else -w))[0]
        struct.pack_into('<4I', points, stride * k, coord(r, w), coord(r, w), coord(r, w), wbits if r.random() < 0.95 else r.getrandbits(32))
    regions = {0x879970: r.bytes(0x80), OUT: r.bytes(8), CODES: r.bytes(0x10), POINTS: points}
    mask = r.choice([0xff, 0xff, 0x3f, r.getrandbits(32)])
    return Call(op, [r.getrandbits(32), r.getrandbits(32), OUT, CODES, POINTS, count, stride, mask], regions, conv='f', ret='a')


if __name__ == '__main__':
    B.main(case)
