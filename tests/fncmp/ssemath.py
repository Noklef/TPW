# The SSE matrix routines (src/game/math/ssemath: 0x5f4eab, 0x5f4f81, 0x5f50c5, 0x5f5433) against the originals, the emulator's XMM registers (tools/x87emu.py) against the hardware. Sources any alignment where the original allows it,
# a 16-aligned destination off its boundary now and then (a fault in both); strides small and overlapping; a destination on top of the source; the batch's count wrapping past 0. Mode 1's source stays aligned: off its boundary the
# original faults (MOVAPS) and the port, whose _mm_load_ps the compiler does not fault on, runs on.
# No NaN goes into the sums, only into ExpandMatrix's moves: with two in a lane ADDPS and MULPS give the first operand, and the compiler may swap their operands. Every invalid operation gives the same NaN, so the order is not seen.
# usage: ssemath.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Expand,Batch,Vectors,Multiply)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5f4eab, 0x5f4f81), (0x5f4f81, 0x5f50c5), (0x5f50c5, 0x5f525f), (0x5f5433, 0x5f554d)])
OPS = [k for k in os.environ.get('OPS', 'Expand,Batch,Vectors,Multiply').split(',') if k]
TABLE = 0x7753d0
MATRIX, SOURCE, DEST = RAM, RAM + 0x100, RAM + 0x1000


def number(r):
    bits = r.f()
    if bits & 0x7f800000 == 0x7f800000:   # a NaN made an infinity (an infinity stays one)
        bits &= 0xff800000
    return bits


def floats(r, count):
    return b''.join(struct.pack('<I', number(r)) for _ in range(count))


def case(r):
    op = r.deal(OPS)
    regions = {MATRIX: r.bytes(0x100) if op == 'Expand' else floats(r, 0x40), SOURCE: floats(r, 0x300), DEST: floats(r, 0x300), TABLE: r.bytes(0x100)}
    this = r.getrandbits(32)
    matrix = MATRIX + r.choice([0, 0, 4, 8, 0xc])
    off = 0 if r.randrange(30) else 4   # (16-aligned, or now and then not: a fault in both)
    if op == 'Expand':
        return Call(0x5f4eab, [matrix, r.choice([DEST + off, DEST + off, MATRIX])], regions, conv='edxedi', ret='v')
    if op == 'Batch':
        count = r.choice([0, 1, 2, 3, 4, 5, 8, 9, 12, 0xfffffffd, 0xfffffffe, 0xffffffff, 0xfffffffc])
        return Call(0x5f4f81, [this, matrix, r.choice([DEST + off, DEST + off, DEST + off, SOURCE + 0x30, TABLE]), SOURCE + r.choice([0, 4, 0x10]), r.choice([4, 8, 12, 16, 16, 0]), r.randrange(10), count], regions, ret='v')
    if op == 'Vectors':
        mode = r.deal([1, 2, 3, 4, 0, 8])
        if mode == 8:
            mode = r.getrandbits(32) | 8
        source = SOURCE + r.choice([0, 0x10] if mode == 1 else [0, 4, 8, 0xc])
        stride = r.choice([0x10, 0x20, 0, 0x10]) if mode == 1 else r.choice([0xc, 0x10, 0x14, 0x20, 0])
        dest = r.choice([DEST, DEST + 4, DEST + 8, source, source])
        return Call(0x5f50c5, [this, matrix, dest, source, r.choice([0x10, 0xc, 8, 0x14, 0x20, 0]), stride, r.randrange(1, 7), mode], regions, ret='v')
    a, b = SOURCE + r.choice([0, 4, 0x40]), SOURCE + 0x100 + r.choice([0, 8])
    return Call(0x5f5433, [this, a, b, r.choice([DEST, DEST + 4, a, b, a + 0x10, a + 0x10, a + 0x10, b + 0x20])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
