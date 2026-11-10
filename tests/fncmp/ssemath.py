# The SSE matrix routines (src/game/math/ssemath: 0x5f4eab, 0x5f4f81, 0x5f50c5, 0x5f5433, 0x5f525f) against the originals, the emulator's XMM registers (tools/x87emu.py) against the hardware.
# 0x5f525f's RCPPS is this CPU's own approximation in both (x87emu runs the instruction natively; the port is _mm_rcp_ps), so the cases hold on the machine that runs them. Sources any alignment where the original allows it,
# a 16-aligned destination off its boundary now and then (a fault in both); strides small and overlapping; a destination on top of the source; the batch's count wrapping past 0. Mode 1's source stays aligned: off its boundary the
# original faults (MOVAPS) and the port, whose _mm_load_ps the compiler does not fault on, runs on.
# No NaN goes into the sums, only into ExpandMatrix's moves: with two in a lane ADDPS and MULPS give the first operand, and the compiler may swap their operands. Every invalid operation gives the same NaN, so the order is not seen.
# usage: ssemath.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Expand,Batch,Vectors,Multiply,Project)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import x87emu  # noqa: E402
from fncmp import RAM, Bench, Call  # noqa: E402

# x87emu's SUBPS and MULSS against hand-worked lanes, and its RCPPS (this CPU's own instruction) within the 1.5 * 2^-12 Intel and AMD promise.
_bits = lambda f: struct.unpack('<I', struct.pack('<f', f))[0]
assert x87emu._sse_single('SUBPS', _bits(1.0), _bits(0.25)) == _bits(0.75) and x87emu._sse_single('SUBPS', _bits(2.0), _bits(2.0)) == 0
_rcp = struct.unpack('<4f', x87emu._native_rcpps()(struct.pack('<4f', 3.0, -0.5, 1e10, 0.0)))
assert all(abs(a * b - 1) < 1.5 * 2 ** -12 for a, b in zip(_rcp, (3.0, -0.5, 1e10))) and _rcp[3] == float('inf')

B = Bench([(0x5f4eab, 0x5f4f81), (0x5f4f81, 0x5f50c5), (0x5f50c5, 0x5f525f), (0x5f5433, 0x5f554d), (0x5f525f, 0x5f5433)])
OPS = [k for k in os.environ.get('OPS', 'Expand,Batch,Vectors,Multiply,Project').split(',') if k]
TABLE = 0x7753d0
MATRIX, SOURCE, DEST = RAM, RAM + 0x100, RAM + 0x1000
VIEW, ONES, SUM = 0x8bcbf0, 0x7754d0, 0x7754f0   # (the render view's +0x28..+0x77; the table (0, 1, 0, 0); the z sum's float)


def number(r):
    bits = r.f()
    if bits & 0x7f800000 == 0x7f800000:   # a NaN made an infinity (an infinity stays one)
        bits &= 0xff800000
    return bits


def floats(r, count):
    return b''.join(struct.pack('<I', number(r)) for _ in range(count))


def project(r):
    """0x5f525f: vertices at a stride of either sign; the view's z limit (+0x60) one of the vertices' z bits now and then (the compare's equal case)."""
    count, stride = r.randrange(1, 7), r.choice([0x10, 0x10, 0x14, 0x20, 0, -0x10])
    if not r.randrange(40):   # (2^32 runs: a fault in both)
        count, stride = 0, 0x10
    source = bytearray(floats(r, 0x300))
    v = SOURCE + (0x800 if stride < 0 else 0) + r.choice([0, 4, 8])
    zs = [struct.unpack_from('<I', source, v - SOURCE + k * stride + 8)[0] for k in range(max(count, 1))]
    view = bytearray(floats(r, 0x14))
    struct.pack_into('<I', view, 0x60 - 0x28, r.choice([r.choice(zs), r.choice(zs), number(r), 0, 0xffffffff, 0x7f7fffff]))
    ones = r.choice([bytes.fromhex('000000000000803f0000000000000000')] * 3 + [floats(r, 4)])
    regions = {SOURCE: bytes(source), VIEW: bytes(view), ONES: ones, SUM: r.bytes(4)}
    return Call(0x5f525f, [r.getrandbits(32), v, stride & 0xffffffff, count, r.deal([1, 2, 3, 0, 7])], regions, ret='d')


def case(r):
    op = r.deal(OPS)
    if op == 'Project':
        return project(r)
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
    return Call(0x5f5433, [this, a, b, r.choice([DEST, DEST + 4, a, b, a + 8, a + 8, a + 0x10, a + 0x10, a + 0x10, b + 0x20])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
