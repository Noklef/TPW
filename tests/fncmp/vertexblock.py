# A block of vertices kept four abreast (src/game/render/vertexblock: 0x53ddd0, 0x53e530) against the originals: a block whose count is 0-11
# (any group and lane), a vertex of any floats (the ground's map and point as terrainheight.py makes them, for x and z), with the wave
# (the sine table, a phase at 0x874fc0 of any float, 0, large ones and NaNs), a normal of any floats, or neither. Quad (0x53e530): four
# corners, texture coordinates and normals of any floats, the index list's entries and count random, the vertex count 0-11 or about
# the word indices' wrap. Face (0x53df30): a face (0, 1, 4, 0x10, 0x40 or any byte) of a cube at a ground point as Add's, turned
# (0, 4, 0x10, 0x40 or any), its type -1-15 or any, big or not, fixed or not; [0x874fc4] 0 or not, the scene (0x81ae2c) 4, 0xe, 0x13
# or another, the offsets at 0x8186b4 / 0x8186b8 and 0x820af8 / 0x820afc any, the texture table at 0x871ee8 any floats.
# usage: vertexblock.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, Bench, Call, fb, global_heap, heap, log_region, put  # noqa: E402
import animatormesh  # noqa: E402
import holder  # noqa: E402
import terrainheight  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x53ddd0, 0x53df30), (0x4527f0, 0x452a70), (0x53e530, 0x53e61c), (0x53df30, 0x53e4d0), (0x52f840, 0x52f860), (0x53d790, 0x53d9db),
                  (0x54dcc0, 0x54dcf0), (0x5c5520, 0x5c5530)] + animatormesh.RANGES + holder.RANGES), sines=4096,
          hooks={**holder.HOOKS, **animatormesh.HOOKS})
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


def face(r):
    c = case(r, ['add'])
    regions = dict(c.regions)
    x, z = c.args[1], c.args[3]
    put(regions[BLOCK], 0x94, 'II', INDICES + 8 * r.randrange(3), r.choice([0, 1, 7, r.getrandbits(32)]))
    regions[INDICES] = r.bytes(0x40)
    regions[0x874fc0] = regions[0x874fc0] + struct.pack('<I', r.choice([0, 0, 0, 1]))
    regions[0x81ae2c] = struct.pack('<I', r.choice([4, 0xe, 0x13, 4, r.randrange(0x20)]))
    regions[0x8186b4] = struct.pack('<2i', *(r.choice([0, r.randrange(-64, 64), r.getrandbits(31)]) for _ in range(2)))
    regions[0x820af8] = r.floats(bytearray(8), [0, 4])
    regions[0x871ee8] = r.floats(bytearray(0x9c), range(0, 0x9c, 4))
    face = r.choice([0, 1, 4, 0x10, 0x40, 0, 1, 4, 0x10, 0x40, r.getrandbits(8), r.getrandbits(32)])
    turn = r.choice([0, 4, 0x10, 0x40, r.getrandbits(32)])
    kind = r.choice([-1, r.randrange(-1, 16), r.randrange(-1, 16), r.choice([12, 13, 14, 15, 7, 8]), r.getrandbits(32) - (1 << 31)])
    args = [x, r.f(), z, face, turn, kind & 0xffffffff, r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)]), r.getrandbits(32), r.choice([0, 0, 1])]
    return Call(0x53df30, [BLOCK] + args, regions, conv='t', ret='a')


CBLOCK, CNAME, CTEXT, CRESULT = RAM + 0x60000, RAM + 0x60200, RAM + 0x60300, RAM + 0x60400


def construct(r):
    """0x53d790 (thiscall; cubes, quads, result, name, head, five dwords): a block of 0-3 cubes and 0-5 quads built (now and then a cube
    or quad count with high bits); the node's word +0xc (block +0x30) small; the text at +8 of 0x786b68 any; the render flags (0x78d968,
    0x785874, 0x79d86c) as rendermode.py's; the face scene as face()'s; holder.py's Wake environment (its renderer) for the wake."""
    regions = dict(holder.wake(r).regions)
    block = bytearray(r.bytes(0xa4))
    put(block, 0x30, 'H', r.choice([0, 1, 2, r.randrange(3, 13)]))
    regions.update({CBLOCK: block, CNAME: animatormesh.text(r), CTEXT: animatormesh.text(r), CRESULT: r.bytes(4), HEAP: heap(0x2000),
                    LOG: log_region(), 0x871f68: r.bytes(0x80)})
    regions.update(global_heap())
    regions[0x786b68] = put(bytearray(r.bytes(0xc)), 8, 'I', CTEXT)
    regions[0x78d968] = struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))
    regions[0x785874] = struct.pack('<I', r.choice([0, 1, 2, r.getrandbits(32)]))
    regions[0x79d86c] = struct.pack('<I', r.getrandbits(32))
    regions[0x874fc0] = struct.pack('<2I', r.f(), r.choice([0, 0, 0, 1]))
    regions[0x81ae2c] = struct.pack('<I', r.choice([4, 0xe, 0x13, 4, r.randrange(0x20)]))
    regions[0x8186b4] = struct.pack('<2i', *(r.choice([0, r.randrange(-64, 64), r.getrandbits(31)]) for _ in range(2)))
    regions[0x820af8] = r.floats(bytearray(8), [0, 4])
    regions[0x871ee8] = r.floats(bytearray(0x80), range(0, 0x80, 4))
    cubes = r.choice([0, 1, 1, 2, 3, 0x10002 if r.random() < 0.1 else 1])
    quads = r.choice([0, 1, 2, 5, 0x10001 if r.random() < 0.1 else 2])
    name = r.choice([CNAME, CNAME, CNAME, 0])
    args = [cubes, quads, CRESULT, name] + [r.getrandbits(32) for _ in range(7)]
    return Call(0x53d790, [CBLOCK] + args, regions, conv='t', ret='a')


def case(r, ops=('add', 'quad', 'face', 'construct')):
    op = r.deal(list(ops))
    if op == 'construct':
        return construct(r)
    if op == 'quad':
        return quad(r)
    if op == 'face':
        return face(r)
    ground = terrainheight.case(r, ['Height'])
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
