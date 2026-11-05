# The action recorder's text record (src/game/core/actionrec: 0x403fd0) against the original, its stream a record stream, the wide strings real, the global save's instance already made, the swear filter a recorder (the text hashed, its length sometimes cut).
# usage: actionrectext.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, STUBS, Bench, Call, _recorder, blob_bytes, fill, heap, log_region  # noqa: E402

FILTER = 1
B = Bench([(0x403fd0, 0x404136), (0x5da3c0, 0x5da3d0), (0x405000, 0x406000), (0x67a400, 0x690000), (0x409410, 0x40947a),
           (0x479150, 0x4791b0), (0x47b750, 0x47bcf0), (0x4f23f0, 0x4f2450), (0x4f6080, 0x4f60a0), (0x4ea740, 0x4ea7e0), (0x47ae60, 0x47b060), (0x47b600, 0x47b750), (0x479230, 0x479256),
           (0x47cc10, 0x47cd00), (0x47b460, 0x47b5b0), (0x47d9d0, 0x47db00), (0x47d620, 0x47d780), (0x47b320, 0x47b460), (0x4f0170, 0x4f0237)], hooks={0x5d3230: _recorder(FILTER, 2)})
THIS, VT, BUF, TEXT, INST = RAM, RAM + 0x400, RAM + 0x500, RAM + 0x800, RAM + 0xa00


def case(r):
    o = bytearray(r.bytes(0x30))
    struct.pack_into('<2I', o, 0, r.choice([0, 1, 1, 1, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    struct.pack_into('<I', o, 0x24, r.choice([0, 0, 0, 0, 0, 1, r.getrandbits(32)]))
    struct.pack_into('<3I', o, 0xc, VT, BUF, r.choice([0, 1, 2, 3, 4, 5, 6, 0x200, 0x200, 0x200, 0x200, 0x200]))
    vt = bytearray(r.bytes(0x10))
    struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
    n = r.choice([0, 1, 2, 7, 15, 16, 40])
    text = struct.pack('<%dH' % n, *[r.randrange(1, 0x10000) for _ in range(n)]) + b'\0\0'
    log = log_region()
    blob_bytes(log, FILTER, 0, 0xff00, inner=(4, 0xffff))
    if n and r.random() < 0.3:
        fill(log, FILTER, 0, struct.pack('<I', r.randrange(0, n + 1)), offset=8)
    regions = {THIS: o, VT: vt, BUF: r.bytes(0x200), TEXT: text, LOG: log, HEAP: heap(0x1000, r.choice([0] * 9 + [1, 2])), INST: r.bytes(0x50), 0xf7c58c: struct.pack('<I', INST)}   # (the instance always made: the original's inline making is GlobalSave_Instance's, tests/fncmp/globalsave.py)
    return Call(0x403fd0, [THIS, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), TEXT], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
