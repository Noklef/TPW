# A block of vertices kept four abreast (src/game/render/vertexblock: 0x53ddd0, 0x53e530) against the originals: a block whose count is 0-11
# (any group and lane), a vertex of any floats (the ground's map and point as terrainheight.py makes them, for x and z), with the wave
# (the sine table, a phase at 0x874fc0 of any float, 0, large ones and NaNs), a normal of any floats, or neither. Quad (0x53e530): four
# corners, texture coordinates and normals of any floats, the index list's entries and count random, the vertex count 0-11 or about
# the word indices' wrap.
# usage: vertexblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402
import terrainheight  # noqa: E402

B = Bench([(0x53ddd0, 0x53df30), (0x4527f0, 0x452a70), (0x53e530, 0x53e61c)], sines=4096)
BLOCK, POS, NORMALS, UVS, NORMAL = RAM + 0x8000, RAM + 0x8100, RAM + 0x8400, RAM + 0x8600, RAM + 0x8800
INDICES, CORNERS, CORNER_UVS, CORNER_NORMALS = RAM + 0x8a00, RAM + 0x8b00, RAM + 0x8c00, RAM + 0x8d00


def quad(r):
    count = r.randrange(12) if r.random() < 0.7 else r.choice([0xfffc, 0xfffd, 0xfffe, 0xffff, 0x1fffe, 0x7ffffffd])
    g = count >> 2  # (the arrays placed so that the count's group lands in the regions)
    block = put(bytearray(r.bytes(0xa0)), 0x64, '3I', (POS - 0x30 * g) & 0xffffffff, (NORMALS - 0xc * count) & 0xffffffff, (UVS - 0x20 * g) & 0xffffffff)
    put(block, 0x94, 'III', INDICES + 8 * r.randrange(3), r.choice([0, 1, 7, r.getrandbits(32)]), count)
    regions = {BLOCK: block, POS: r.bytes(0x300), NORMALS: r.bytes(0x200), UVS: r.bytes(0x200), INDICES: r.bytes(0x40)}
    for at in (CORNERS, CORNER_UVS, CORNER_NORMALS):
        regions[at] = r.floats(bytearray(0x30), range(0, 0x30, 4))
    return Call(0x53e530, [BLOCK, CORNERS, r.getrandbits(32), CORNER_UVS, CORNER_NORMALS], regions, conv='t', ret='a')


def case(r):
    if r.deal(['add', 'quad']) == 'quad':
        return quad(r)
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
