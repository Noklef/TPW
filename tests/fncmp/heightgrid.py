# The height grid's sampler (src/game/render/heightgrid: 0x57c190) against the original: grids of 1-8 cells a side, points mostly on
# the grid (its edges included, where the next column or row is clamped), sometimes a cell beyond it (the heights are padded so the
# unchecked reads stay mapped), and any floats for the scales and heights now and then.
# usage: heightgrid.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x57c190, 0x57c25a)])
SAMPLER, GRID, HEIGHTS = RAM, RAM + 0x100, RAM + 0x1000
PAD = 0x200  # bytes mapped before and after the heights


def case(r):
    nx, nz = r.randrange(1, 9), r.randrange(1, 9)
    cell = [fb(r.uniform(0.5, 64)), fb(r.uniform(0.5, 64))]
    per = [fb(1 / struct.unpack('<f', struct.pack('<I', c))[0]) for c in cell]
    if r.random() < 0.05:
        per = [r.f(), r.f()]
    origin = [fb(r.uniform(-1000, 1000)), fb(r.uniform(-1000, 1000))]
    grid = bytearray(r.bytes(0x2c))
    struct.pack_into('<4I', grid, 8, origin[0], origin[1], cell[0], cell[1])
    struct.pack_into('<I', grid, 0x28, HEIGHTS)
    sampler = struct.pack('<I4s2i2I', GRID, r.bytes(4), nx, nz, per[0], per[1])
    point = []
    for k, n in ((0, nx), (1, nz)):
        o, c = (struct.unpack('<f', struct.pack('<I', v))[0] for v in (origin[k], cell[k]))
        u = r.choice([r.uniform(0, n), r.uniform(-1, n + 1), float(r.randrange(n + 1)), n - 1e-3])
        point.append(fb(o + u * c) if r.random() < 0.95 else r.f())
    heights = r.floats(bytearray(PAD + 4 * (nx * (nz + 1) + 1) + PAD), range(0, PAD + 4 * (nx * (nz + 1) + 1) + PAD, 4), r.choice([1, 50, None]))
    return Call(0x57c190, [SAMPLER] + point, {SAMPLER: sampler, GRID: grid, HEIGHTS - PAD: heights}, conv='t', ret='d')


if __name__ == '__main__':
    B.main(case)
