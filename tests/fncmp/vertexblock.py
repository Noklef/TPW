# A block of vertices kept four abreast (src/game/render/vertexblock: 0x53ddd0) against the original: a block whose count is 0-11
# (any group and lane), a vertex of any floats (the ground's map and point as terrainheight.py makes them, for x and z), with the wave
# (the sine table, a phase at 0x874fc0 of any float, 0, large ones and NaNs), a normal of any floats, or neither.
# usage: vertexblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402
import terrainheight  # noqa: E402

B = Bench([(0x53ddd0, 0x53df30), (0x4527f0, 0x452a70)], sines=4096)
BLOCK, POS, NORMALS, UVS, NORMAL = RAM + 0x8000, RAM + 0x8100, RAM + 0x8400, RAM + 0x8600, RAM + 0x8800


def case(r):
    ground = terrainheight.case(r)
    x, z = ground.args
    if r.random() < 0.2:
        x, z = r.f(), r.f()
    count = r.randrange(12)
    block = put(bytearray(r.bytes(0xa0)), 0x64, '3I', POS, NORMALS, UVS)
    put(block, 0x9c, 'I', count)
    mode = r.deal(['wave', 'wave', 'normal', 'none'])
    regions = dict(ground.regions)
    regions.update({BLOCK: block, POS: r.bytes(0x300), NORMALS: r.bytes(0x200), UVS: r.bytes(0x200),
                    NORMAL: r.floats(bytearray(12), [0, 4, 8])})
    k = r.random()
    phase = r.f() if k < 0.7 else 0 if k < 0.8 else fb(r.uniform(-1e6, 1e6)) if k < 0.95 else r.choice([0x7fc00000, 0x7f800001, 0x4f000000])
    regions[0x874fc0] = struct.pack('<I', phase)
    args = [x, r.f(), z, r.f(), r.f(), 0 if mode != 'wave' else r.choice([1, 0xffffffff, 0x100]), 0 if mode == 'none' else NORMAL]
    return Call(0x53ddd0, [BLOCK] + args, regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
