# The tiled backdrop's setters (src/game/render/backdrop: 0x5777a0, 0x577940, 0x577970) against the
# originals: any object bytes, any values. Its tiles' uncaching (0x577990): rectangles in pixels (tiles -1..4 and a few pixels
# either side: signed division by 64, empty ones too) over 1-4 tiles a row and a first tile 0-3, the tiles as texturemem.py's.
# Its making from an image (0x5777b0): sizes -70..300 (signed tile counts, 0 and negative too), any pitch and pixel size (0 now and then:
# faulting), the textures' block got or not (heap()).
# usage: backdrop.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear5c,SetRect,SetSourceRect,UncacheRect,Create)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
import struct  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x5777a0, 0x5777a8), (0x577940, 0x577965), (0x577970, 0x57798f), (0x577990, 0x577a21), (0x58dc50, 0x58dd5e),
           (0x5777b0, 0x57793d), (0x590a00, 0x590a70), (0x590eb0, 0x590f7e), (0x67a830, 0x67a860), (0x67d830, 0x67d8be)])
CALLS = {'Clear5c': 0x5777a0, 'SetRect': 0x577940, 'SetSourceRect': 0x577970, 'UncacheRect': 0x577990, 'Create': 0x5777b0}
BACKDROP, TILES = RAM + 0x10000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)


def pixel(r, first):
    """a coordinate in pixels whose tile (rounded toward 0) is first-1 .. 4."""
    t = r.choice([0, 0, 1, 2, 3, 4] + ([-1] if first else []))
    if t < 0:
        return -64 - r.randrange(64)
    if t == 0 and r.random() < 0.3:
        return -r.randrange(1, 64)
    return 64 * t + r.choice([0, 1, 63, r.randrange(64)])


def uncache(r):
    regions = {}
    width, first = r.choice([1, 2, 3, 4]), r.choice([0, 1, 2, 3])
    regions[TILES] = texturemem.textures(r, regions, TILES, 4 * 4 + 4 + 4)
    bd = bytearray(r.bytes(0x60))
    put(bd, 0x10, 'I', width)
    put(bd, 0x1c, 'I', TILES)
    put(bd, 0x48, 'I', first)
    regions[BACKDROP] = bd
    x0, y0 = pixel(r, first), pixel(r, False)
    x1, y1 = pixel(r, False), pixel(r, False)
    return Call(0x577990, [BACKDROP, x0 & 0xffffffff, y0 & 0xffffffff, x1 & 0xffffffff, y1 & 0xffffffff], regions, conv='t')


def side(r):
    return r.choice([r.randrange(1, 300), r.randrange(1, 130), 64 * r.randrange(1, 5) + r.choice([-1, 0, 1]), r.randrange(-70, 1)])


def create(r):
    width, height = side(r), side(r)
    tiles = max(0, (width + 63) // 64 if width > 0 else 0) * max(0, (height + 63) // 64 if height > 0 else 0)
    n = r.choice([1, 2, 3, 4])
    pitch = r.choice([width * n, width * n + r.choice([1, 4, 64]), r.getrandbits(32) - (1 << 31), -width * n])
    sizes = bytearray(r.bytes(0x20))
    kind = r.randrange(8)
    put(sizes, 4 * kind, 'i', n if r.random() < 0.97 else 0)
    regions = {RAM: r.bytes(0x60), HEAP: heap(0x60 * tiles + 0x100, fail=1 if r.random() < 0.05 else 0),
               0x701820: bytes(sizes), 0x8bd554: struct.pack('<I', kind)}
    return Call(0x5777b0, [RAM, r.getrandbits(32), width & 0xffffffff, height & 0xffffffff, pitch & 0xffffffff], regions, conv='t')


def case(r):
    op = r.deal(OPS)
    if op == 'UncacheRect':
        return uncache(r)
    if op == 'Create':
        return create(r)
    args = [RAM] + ([] if op == 'Clear5c' else [r.getrandbits(32) for _ in range(4)])
    return Call(CALLS[op], args, {RAM: r.bytes(0x60)}, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
