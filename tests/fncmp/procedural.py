# The procedural terrain texture's column extents (src/game/terrain/procedural: 0x56e780) against the original: low and high bounds about the
# signed byte range and past it, a column range of 1 to 6 (now and then empty), the stored extents (+0x20 / +0x24) around the new ones.
# The grid's colours lit (0x56ef10): see light_grid().
# usage: procedural.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Extents,LightGrid)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import meshlist  # noqa: E402

B = Bench([(0x56e780, 0x56e7de), (0x56ef10, 0x56f086)] + meshlist.RANGES)
OPS = [k for k in os.environ.get('OPS', 'Extents,LightGrid').split(',') if k]
TEX, HEADER, HEIGHTS = RAM + 0x20000, RAM + 0x10000, RAM + 0x40000


def light_grid(r):
    """0x56ef10 (thiscall on the texture object): meshlist.py's lit() scene (its object [0x87a248]); the stamp (+0x12968) the frame
    number (0x8bd50c) or not, 0x8bd508's 0x40000000 either way; 0-128 points a row (now and then none or negative) and 0-6 rows (or
    negative), the heights near the lights' point (any floats, NaNs and infinities now and then) with room for the last row's read 128
    back; the header's x / z start and steps and the scale (+0x18) small floats or any."""
    regions, near, _normals = meshlist.lit(r)
    cols = r.choice([0, 1, 2, 3, 5, 8, 17, 40, 128, r.randrange(1, 129), -1])
    rows = r.choice([0, 1, 2, 3, 4, 6, -1])
    small = lambda lo, hi: r.choice([fb(r.uniform(lo, hi)), fb(r.uniform(lo, hi)), fb(r.uniform(lo, hi)), r.f(30)])  # noqa: E731
    n = 128 + max(rows, 1) * max(cols, 1) + max(cols, 1) + 2
    heights = [near(1) if r.random() < 0.95 else r.choice([0x7fc00000, 0xffc00000, 0x7f812345, 0x7f800000, 0xff800000, r.f(1e30)])
               for _ in range(n)]
    header = bytearray(r.bytes(0x40))
    put(header, 8, 'II', small(0.25, 4), small(-20, 20))
    put(header, 0x14, 'I', small(-4, 4))
    put(header, 0x28, 'I', HEIGHTS + 0x200)
    tex = bytearray(r.bytes(0x3600))   # (the colours, 0x200 a row from +0x2968; the stamp +0x12968 a region of its own)
    put(tex, 0, 'I', HEADER)
    put(tex, 8, 'ii', cols, rows)
    put(tex, 0x18, 'I', small(-2, 2))
    frames = bytearray(r.bytes(8))
    put(frames, 0, 'I', r.getrandbits(32) & ~0x40000000 | r.choice([0, 0, 0x40000000]))
    stamp = frames[4:8] if r.random() < 0.2 else r.bytes(4)
    regions.update({TEX: tex, TEX + 0x12968: stamp, HEADER: header, HEIGHTS: struct.pack('<%dI' % n, *heights), 0x8bd508: frames,
                    0x87a248: struct.pack('<I', meshlist.OBJ)})
    return Call(0x56ef10, [TEX], regions, conv='t')


def case(r):
    if r.deal(OPS) == 'LightGrid':
        return light_grid(r)
    obj = bytearray(r.bytes(0x400))
    arg = lambda: r.choice([0, 1, 5, 9, -1, -5, 100, 0x7f, 0x80, -0x80, 0x1ff, -0x1ff, r.randrange(-300, 300)]) & 0xffffffff
    first = r.randrange(0, 10)
    last = first + r.randrange(0, 6) - (2 if r.random() < 0.1 else 0)
    put(obj, 0x20, 'i', first + r.choice([-1, 0, 1, r.randrange(-20, 20)]))  # the stored extents about the new ones
    put(obj, 0x24, 'i', last + r.choice([-1, 0, 1, r.randrange(-20, 20)]))
    return Call(0x56e780, [RAM, arg(), first, arg(), last], {RAM: obj}, conv='t')


if __name__ == '__main__':
    B.main(case)
