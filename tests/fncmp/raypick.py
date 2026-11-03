# The ray from the camera through the mouse against the ground and what stands on it (src/game/terrain/raypick: RayPick_World 0x45bf90) against the original: RayPick_Column (k 1, its answers in
# turn) and Highlight_SetTarget (k 2) recorders; Matrix_TransformVector, Vec3_Distance and the CRT as they are. A camera matrix that always looks down (a ray that does not goes on for ever, sic);
# the grid object [0x7a0854] or none, its flag 2; grid cells' heights and each cell's flag 2 (which way the cell is cut), a ray that comes down inside the grid or not, along x or z; the outs null
# or not, the column answering at once, after some steps or never. (--wild changes little here: a ground point far down or a ray going up makes the walk endless, sic.)
# usage: raypick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, log_region, put  # noqa: E402

B = Bench([(0x45bf90, 0x45cdc0), (0x46fca0, 0x46fd40), (0x470940, 0x470990), (0x67a400, 0x690000)], sines=256, hooks={0x45cdc0: _recorder(1, 10, cdecl=True), 0x469340: _recorder(2, 1, cdecl=True)})
OBJECT, GRID, HEIGHTS, FLAGS, TABLE = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x4000, RAM + 0x6000
OUT1, OUT2, HITFLAG = RAM + 0x6100, RAM + 0x6120, RAM + 0x6140
RIDE = RAM + 0x6200


def case(r):
    log = log_region()
    blob(log, 1, 7, 7)   # (the ray's direction, on the stack)
    # the column: the first dwords in turn none, the last ride; or never
    nth = r.choice([0, 1, 2, 3, 5, 8, 12, 40, 40])
    table = [0] * nth + [RIDE] * (40 - nth)
    answers(log, 1, TABLE)

    # the camera's matrix: its z axis (almost) straight down, with x and z across
    m = [r.uniform(-1, 1), r.uniform(-0.2, 0.2), r.uniform(-1, 1), 0,
         r.uniform(-1, 1), r.uniform(-0.2, 0.2), r.uniform(-1, 1), 0,
         r.uniform(-0.4, 0.4), -1.0, r.uniform(-0.4, 0.4), 0,
         r.uniform(-500, 500), r.uniform(-500, 500), r.uniform(-500, 500), 1]
    nx, ny = r.randrange(1, 17), r.randrange(1, 17)
    # (small cells, of different sizes, and high relief: the sides of the cell's tests, a plane's hit outside its cell)
    cell_x, cell_z = r.choice([10.0, 20.0, r.uniform(2, 30), r.uniform(0.5, 3), r.uniform(0.5, 3)]), r.choice([10.0, 20.0, r.uniform(2, 30), r.uniform(0.5, 3), r.uniform(0.5, 3)])
    relief = r.choice([3.0, 3.0, 20.0])
    x0, z0 = r.uniform(-50, 50), r.uniform(-50, 50)
    base = r.uniform(0, 20)
    # the camera over the grid mostly
    cam = (x0 + r.uniform(-20, nx * cell_x + 20), base + r.uniform(5, 70), z0 + r.uniform(-20, ny * cell_z + 20))
    heights = struct.pack('<%df' % ((nx + 1) * (ny + 2)), *[base + r.uniform(-relief, relief) for _ in range((nx + 1) * (ny + 2))])
    flags = bytearray(r.bytes(4 * (nx + 2) * (ny + 1)))
    for k in range(0, len(flags), 4):
        flags[k] = flags[k] & ~2 | r.choice([0, 2])
    grid = bytearray(r.bytes(0x40))
    put(grid, 8, '4f', x0, z0, cell_x, cell_z)
    put(grid, 0x18, '2I', nx, ny)
    put(grid, 0x20, '2f', base + r.uniform(-5, 0) - relief, base + r.uniform(0, 5) + relief)
    put(grid, 0x28, '2I', HEIGHTS, FLAGS)
    obj = bytearray(r.bytes(0x70))
    put(obj, 0x30, 'I', r.getrandbits(32) & ~2 | r.deal([2, 2, 2, 2, 0]))
    put(obj, 0x6c, 'I', GRID)

    regions = {LOG: log, TABLE: struct.pack('<40I', *table), OBJECT: obj, GRID: grid, HEIGHTS: heights, FLAGS: flags, RIDE: r.bytes(4),
               0x7a0854: struct.pack('<I', OBJECT if r.random() < 0.9 else 0),
               0x790990: struct.pack('<16f', *m), 0x790aa0: struct.pack('<3f', *cam),
               0x790ac8: struct.pack('<2f', r.uniform(0, 640), r.uniform(0, 480)), 0x7a0c9c: struct.pack('<f', r.uniform(300, 640)), 0x7a162c: struct.pack('<f', r.uniform(200, 480)),
               0x74c9b8: struct.pack('<f', r.uniform(0.5, 1.2)), 0x74d13c: struct.pack('<f', r.uniform(0.3, 1.0)),
               0x7a1cd4: r.choice([bytes(4), RIDE.to_bytes(4, 'little')]), 0x7a1cb8: struct.pack('<3f', *[r.uniform(-300, 300) for _ in range(3)]),
               OUT1: r.bytes(12), OUT2: r.bytes(12), HITFLAG: r.bytes(4)}
    args = [r.choice([OUT1, OUT1, OUT1, 0]), r.getrandbits(32), r.choice([OUT2, OUT2, 0]), r.choice([HITFLAG, HITFLAG, 0])]
    return Call(0x45bf90, args, regions, ret='v')


if __name__ == '__main__':
    B.main(case)
