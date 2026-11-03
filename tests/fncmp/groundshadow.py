# A flat quad over the height grid (src/game/render/groundshadow: 0x57be90) against the original, with HeightGrid_HeightAt as it is. Batch_DrawQuad is a recorder (k 1; its spec compared but for dword 1, a stack address, 5, unset,
# and its points: the transformer's), the transformer's slot 1 a cdecl recorder (k 2; its in corners compared, its out argument and in pointer logged as 0). The polygon flags are stack garbage in the game: the bench's stack
# starts at 0, as the port's flags do. Places in the grid, or a little off it (the next row and column are clamped, not the cell).
# usage: groundshadow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, log_region, put, recorder, set_return, strings  # noqa: E402

B = Bench([(0x57be90, 0x57c190), (0x57c190, 0x57c25a), (0x590c40, 0x590ce8)], hooks={0x565f90: _recorder(1, 2)})
OBJECT, RECT, TEX, QUADS, MATRIX, XF, XFT, SAMPLER, GRID = (RAM + 0x200 * k for k in range(9))
HEIGHTS = RAM + 0x4000
SPEC = 0x7dd
NANS = [0x7fc12345, 0x7f812345]


def place(r, lo, hi, nan=0.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def case(r):
    log = log_region()
    nan = r.choice([0, 0, 0, 0.05])
    nx, nz = r.choice([1, 2, 4, 8, 16]), r.choice([1, 2, 4, 8, 16])
    ox, oz = r.uniform(-50, 50), r.uniform(-50, 50)
    width, depth = r.choice([0.5, 1, 8, 64, r.uniform(1, 9)]), r.choice([0.5, 1, 8, 64, r.uniform(1, 9)])
    grid = bytearray(r.bytes(0x30))
    put(grid, 8, 'ffff', ox, oz, width, depth)
    put(grid, 0x28, 'I', HEIGHTS)
    sampler = bytearray(r.bytes(0x20))
    put(sampler, 0, 'I', GRID)
    put(sampler, 8, 'iiff', nx, nz, 1 / width, 1 / depth)
    heights = b''.join(struct.pack('<I', place(r, -20, 20, nan)) for _ in range((nx + 1) * (nz + 2) + 2))

    def x(off=0.0):
        return ox + r.uniform(0, nx) * width * 0.999 + off

    def z():
        return oz + r.uniform(0, nz) * depth * 0.999
    rect = bytearray(r.bytes(0x20))
    put(rect, 0, 'f', x())
    put(rect, 8, 'f', z())
    put(rect, 0xc, 'f', x())
    put(rect, 0x14, 'f', z())
    put(rect, 0x18, 'I', r.deal([TEX, TEX, 0]))
    tex = bytearray(r.bytes(0x40))
    put(tex, 8, 'I', r.getrandbits(32) & ~0x101000 | r.deal([0, 0x1000]) | r.deal([0, 0, 0x100000]))
    put(tex, 0x20, 'IIII', place(r, 0, 2, nan), place(r, 0, 2, nan), place(r, -1, 1, nan), place(r, -1, 1, nan))
    obj = bytearray(r.bytes(0x140))
    put(obj, 0x13c, 'I', r.deal([RECT] * 12 + [0]))

    blob(log, 1, 0, SPEC)
    set_return(log, 1, r.getrandbits(32))
    blob(log, 2, 3, 0xfff)
    strings(log, 2, 0, out=1 << 2)
    regions = {LOG: log, OBJECT: obj, RECT: rect, TEX: tex, QUADS: r.bytes(0x40), MATRIX: r.bytes(0x40), XF: struct.pack('<I', XFT), XFT: struct.pack('<II', 0, recorder(2, 8, cdecl=True)), SAMPLER: sampler, GRID: grid, HEIGHTS: heights,
               0x879d60: struct.pack('<I', SAMPLER), 0x87b088: struct.pack('<I', MATRIX), 0x87b1d8: struct.pack('<I', XF), 0x879ab0: struct.pack('<I', QUADS), 0x8bd4ec: struct.pack('<I', place(r, -3, 3, nan)),
               0x8bd524: struct.pack('<I', r.deal([4, 1, 0, 2]))}
    return Call(0x57be90, [OBJECT], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
