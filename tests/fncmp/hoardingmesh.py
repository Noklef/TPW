# A hoarding's boards built as a mesh (src/game/model/hoardingmesh: 0x452e70) against the original, with Matrix_Identity and the stable sort running as they are. GlobalMem_Alloc (k 1, its answers in turn), the sprintf
# (k 2: the format and the level compared as text, the frame's buffer not; nothing reads the text but the recorder of Animator_Create's record) and Animator_Create (k 3) are recorders. The record's pointer to the frame's
# buffer (0x74d010) is left out. A mesh has at least 2 points but on a grid of 1 by 1: without, the original's corner moves are unset stack.
# usage: hoardingmesh.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fb, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x452e70, 0x454043), (0x67b110, 0x67b13f), (0x46f150, 0x46f190), (0x470630, 0x470790)],
          hooks={0x45b9d0: _recorder(1, 2, cdecl=True), 0x67a650: _recorder(2, 3, cdecl=True), 0x424ef0: _recorder(3, 4, cdecl=True)})
LEVEL, OBJECT, SUB, MESH, ANIMATOR, PART, INFO, RANGE, MATRIX, TABLE = (RAM + 0x100 * k for k in range(10))
GRID = RAM + 0x1000
POINTS, VERTS, NORMALS, UVS, TRIS, EDGES = RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x6000, RAM + 0x7000, RAM + 0x8000
SIZES = [(1, 1)] * 3 + [(2, 1), (1, 2), (2, 2), (3, 2), (2, 3), (3, 3), (4, 4), (20, 1), (1, 20), (0, 2), (2, 0)]
NAN = 0x7fc12345


def point(r, lo, hi, nan):
    return NAN if r.random() < nan else fb(r.uniform(lo, hi))


def mesh_points(r, count, whole):
    """x, y, z by fours of a block, whole numbers now and then (so that distances tie), a NaN rarely."""
    blocks = bytearray(max((count + 3) // 4, 1) * 0x30)
    nan = r.choice([0, 0, 0.03])
    for k in range(0, len(blocks), 4):
        put(blocks, k, 'I', fb(float(r.randrange(-3, 6) * 5)) if whole else point(r, -40, 60, nan))
    return blocks


def case(r):
    log = log_region()
    width, height = r.deal(SIZES)
    density = r.deal([0, 0.2, 0.6, 1, 1])
    cells = bytearray(max(height, 1) * 0xa0 + 0xc)
    put(cells, 0, 'iiI', width, height, r.getrandbits(32))
    for k in range(0xc, len(cells), 8):
        put(cells, k, 'II', r.getrandbits(32), r.getrandbits(32))
    boards = 0
    for y in range(height):
        for x in range(width):
            byte = r.getrandbits(8) & 0xaa
            for bit in (1, 4, 0x10, 0x40):
                if r.random() < density:
                    byte |= bit
                    boards += 1
            cells[0xc + (y * 0x14 + x) * 8] = byte

    # The mesh: none, or its points and matrix.
    single = width == 1 and height == 1
    count = r.deal([0, 1, 2, 3, 5, 9]) if single else r.deal([2, 2, 3, 3, 5, 9])
    whole = r.random() < 0.3
    mesh = bytearray(r.bytes(0x68))
    if whole or r.random() < 0.2:
        put(mesh, 0x10, '12f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0)
        put(mesh, 0x40, 'fff', 0, 0, 0)
    else:
        nan = r.choice([0, 0, 0.01])
        for k in range(0x10, 0x4c, 4):
            put(mesh, k, 'I', point(r, -2, 2, nan))
    put(mesh, 0x58, 'H', count)
    put(mesh, 0x60, 'I', POINTS)
    sub = bytearray(r.bytes(0x30))
    put(sub, 0xc, 'H', r.deal([2] * 15 + [3, 0xffff, 1, 0]))
    put(sub, 0x2c, 'I', r.deal([MESH] * 12 + [0]))
    obj = bytearray(r.bytes(0xc0))
    put(obj, 8, 'I', SUB)

    animator = bytearray(r.bytes(0x40))
    put(animator, 0x10, 'IIII', VERTS, NORMALS, UVS, TRIS)
    part = bytearray(r.bytes(0x30))
    put(part, 0x28, 'II', MATRIX, RANGE)

    table = [ANIMATOR, PART, EDGES, INFO] + [0] * 36
    answers(log, 1, TABLE)
    strings(log, 2, 0b110, out=1)
    set_return(log, 2, r.getrandbits(32))
    set_return(log, 3, r.getrandbits(32))
    name = bytes(r.choice(b'abcdefgh') for _ in range(r.randrange(1, 12))) + b'\0'
    regions = {LOG: log, LEVEL: name, OBJECT: obj, SUB: sub, MESH: mesh, ANIMATOR: animator, PART: part, INFO: r.bytes(0x10), RANGE: r.bytes(0x10), MATRIX: r.bytes(0x40), TABLE: struct.pack('<40I', *table),
               GRID: cells, POINTS: mesh_points(r, count, whole), VERTS: r.bytes(boards * 0x30 + 8), NORMALS: r.bytes(boards * 0x60 + 8), UVS: r.bytes(boards * 0x20 + 8), TRIS: r.bytes(boards * 0x20 + 8),
               EDGES: r.bytes(boards * 8 + 8), 0x74d010: bytes(4), 0x74d020: r.bytes(4)}
    return Call(0x452e70, [LEVEL, OBJECT, GRID], regions, ignore=[(0x74d010, 4)])


if __name__ == '__main__':
    B.main(case)
