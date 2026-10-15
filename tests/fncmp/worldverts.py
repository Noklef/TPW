# The world vertex buffer at 0x7aecd4 (src/game/model/worldverts: 0x462870, 0x4628a0, 0x4628b0,
# 0x463a10, 0x466c80)
# against the original: its alloc / free (global_heap()) and the frame transform - a frame with
# a parent chain of matrices (Matrix_Mul) and SoA vertices, written as world positions into it;
# the fetch of one of those vertices; and a node's vertices through a given matrix.
# usage: worldverts.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x462870, 0x4629c4), (0x463a10, 0x463a42), (0x466c80, 0x466d62), (0x4700f0, 0x4702d0), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30)])
FRAMES, VERTS, OUT = RAM, RAM + 0x1000, RAM + 0x3000


def case(r):
    op = r.choice(OPS)
    regions = {HEAP: heap(), **global_heap(), 0x7aecd4: bytearray(r.bytes(4))}
    if op == 'Alloc':
        regions[0x7a42ac] = put(bytearray(4), 0, 'I', r.choice([0, 1, 7, r.randrange(300)]))
        return Call(0x462870, [r.choice([0, 0, 1, r.getrandbits(32)])], regions)
    if op == 'Fetch':  # raw words (NaNs too) from 8 groups
        regions.update({VERTS: bytearray(r.bytes(0x180)), OUT: bytearray(r.bytes(16))})
        return Call(0x463a10, [VERTS, r.randrange(32), OUT], regions)
    if op == 'Node':  # SoA vertices through one matrix (its last column any)
        n = r.choice([0, 1, 3, 4, 5, r.randrange(1, 24)])
        node = put(bytearray(r.bytes(0x140)), 0x58, 'H', n)
        put(node, 0x60, 'I', VERTS)
        r.floats(node, range(0x100, 0x140, 4), r.choice([1, 2, 100, None]))  # the matrix
        verts = r.floats(bytearray(0x30 * ((n + 3) // 4 + 1)), range(0, 0x30 * ((n + 3) // 4 + 1), 4))
        put(regions[0x7aecd4], 0, 'I', OUT)
        regions.update({FRAMES: node, VERTS: verts, OUT: bytearray(r.bytes(12 * n + 16))})
        return Call(0x466c80, [FRAMES, FRAMES + 0x100], regions)
    if op == 'Free':
        put(regions[0x7aecd4], 0, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x4628a0, [], regions)
    depth, n = r.choice([0, 0, 1, 2, 3]), r.choice([0, 1, 3, 4, 5, r.randrange(1, 24)])
    frames = bytearray(r.bytes(0x80 * (depth + 1)))
    for k in range(depth + 1):  # frame k: +4 its parent (k + 1), +0x10 a matrix
        put(frames, 0x80 * k + 4, 'I', FRAMES + 0x80 * (k + 1) if k < depth else 0)
        scale = r.choice([1, 2, 100, None])
        r.floats(frames, range(0x80 * k + 0x10, 0x80 * k + 0x50, 4), scale)
        if r.random() < 0.5:  # an affine matrix: 0, 0, 0, 1 in the last column
            put(frames, 0x80 * k + 0x1c, 'f', 0.0)
            put(frames, 0x80 * k + 0x2c, 'f', 0.0)
            put(frames, 0x80 * k + 0x3c, 'f', 0.0)
            put(frames, 0x80 * k + 0x4c, 'f', 1.0)
    put(frames, 0x58, 'H', n)
    put(frames, 0x60, 'I', VERTS)
    verts = r.floats(bytearray(0x30 * ((n + 3) // 4 + 1)), range(0, 0x30 * ((n + 3) // 4 + 1), 4))
    put(regions[0x7aecd4], 0, 'I', OUT)
    regions.update({FRAMES: frames, VERTS: verts, OUT: bytearray(r.bytes(12 * n + 16))})
    return Call(0x4628b0, [FRAMES], regions)


OPS_ALL = ['Alloc', 'Free', 'Fetch'] + ['Transform', 'Node'] * 6
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
