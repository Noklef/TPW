# Track_Apply (0x471860, src/game/motion/track) against the original: one random track (any mix
# of visibility, vertex, rotation, scale, path and ride-path channels) on a node, which runs the
# key search, quaternions, curves, vertex animation and matrix code under it.
# usage: track.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x46f150, 0x475010), (0x67a400, 0x690000)], sines=256)
STATE, MODEL, NODE, TRACK, DATA, POS = (RAM + 0x1000 * k for k in range(6))
GLOBALS = 0x7b4980  # gMotionFlags (+0), gRotationKeyCount (+8), gCurvePointCount (+0x68)


def nan(r):
    return 0x7f800000 | (r.getrandbits(23) or 1) | r.getrandbits(1) << 31


def fl(r, scale=None):
    """A float's bits: mostly near the scale, sometimes extreme or a NaN."""
    if r.random() < 0.03:
        return nan(r)
    return r.f(scale or r.choice([1, 1, 10, 300, None, 1e38, 1e-38]))


class Heap:
    """Sub-structures bumped into the DATA region."""

    def __init__(self, r):
        self.buf, self.at = bytearray(r.bytes(0x1000)), 0

    def put(self, fmt, *values):
        address = DATA + self.at
        put(self.buf, self.at, fmt, *values)
        self.at = (self.at + struct.calcsize('<' + fmt) + 3) & ~3
        return address


def frames(r, n):
    """n ascending u16 key frames."""
    out, f = [], r.randrange(3)
    for _ in range(n):
        out.append(f)
        f += r.choice([0, 1, 1, 2, 3, r.randrange(10)])
    return out


def quat(r):
    if r.random() < 0.25:
        return [fl(r) for _ in range(4)]
    q = [r.uniform(-1, 1) for _ in range(4)]
    n = sum(x * x for x in q) ** 0.5 or 1
    return [struct.unpack('<I', struct.pack('<f', x / n))[0] for x in q]


def case(r):
    h = Heap(r)
    state, model, node, track = (bytearray(r.bytes(n)) for n in (0x40, 0x100, 0x100, 0x40))
    vscale = r.choice([10, 10, 3e38, 1e-38])  # extremes: the lerp's x leaves float range in st0
    pos = bytearray(struct.pack('<256I', *[fl(r, vscale) for _ in range(256)]))
    g = bytearray(r.bytes(0x70))
    put(g, 0, 'I', r.choice([0, 2, 0x10, r.getrandbits(32)]))
    flags = 0
    for bit, p in [(0x20000, 0.3), (0x1000, 0.35), (0x4000, 0.5), (0x8, 0.4), (0x80, 0.35), (0x1, 0.3),
                   (0x200, 0.2), (0x400, 0.4)]:
        if r.random() < p:
            flags |= bit
    put(track, 4, 'I', flags | (r.getrandbits(32) & ~0x3f7ff & ~0x10000))
    frame = r.uniform(-1, 12) if r.random() < 0.8 else r.choice([0.0, 1.0, 5.0, r.uniform(-1, 0)])
    put(state, 0x1c, 'f', r.choice([100.0, 100.0, frame + 1, frame - 1]))
    put(state, 0x20, 'f', frame)
    put(model, 0x30, 'I', r.getrandbits(32) & ~4 | r.choice([0, 4]))
    put(node, 0, 'I', r.getrandbits(32))
    nverts = r.randrange(0, 10)
    put(node, 0x58, 'H', nverts)
    put(node, 0x60, 'I', POS)
    for off in range(0x10, 0x50, 4):  # the 3x4 matrix and position
        put(node, off, 'I', nan(r) if off >= 0x40 and r.random() < 0.15 else fl(r, 1))
    for off in range(0x78, 0x90, 4):  # the box
        put(node, off, 'I', r.choice([0x7f800000, 0xff800000]) if r.random() < 0.15 else fl(r, 10))
    # visibility keys
    n = r.randrange(0, 6)
    put(track, 0x16, 'H', n)
    put(track, 0x30, 'I', h.put('%dh' % max(n, 1), *[r.choice([1, -1]) * f for f in frames(r, max(n, 1))]))
    # unpacked vertices: {remap or null, vertices a frame, vertices}
    per = max(nverts, 1)
    remap = h.put('%dI' % per, *[r.randrange(per) for _ in range(per)]) if r.random() < 0.5 else 0
    verts = h.put('%dI' % (3 * per * 14), *[fl(r, vscale) for _ in range(3 * per * 14)])
    unpacked = h.put('III', remap, per, verts)
    # packed vertices (Motion_UnpackVertices): groups share one frames array
    if not flags & 0x4000:
        ngroups, nkeys, bflags = r.randrange(2, 4), r.randrange(2, 6), r.choice([0, 2])
        fr = frames(r, nkeys - 1) + [40]
        fa = h.put('%dH' % nkeys, *fr)
        groups = []
        for k in range(ngroups):
            n = r.randrange(2, 6)
            slots = h.put('%dH' % n, *[r.randrange(32) for _ in range(n)])
            keys = h.put('%dI' % (n * nkeys), *[r.getrandbits(32) for _ in range(n * nkeys)])
            groups.append((r.randrange(65536), n, slots, fa, keys, r.randrange(nkeys - 1)))
        gaddr = h.put('HHIIII' * ngroups, *[v for grp in groups for v in grp])
        # flags, group count, groups +0xc, offsets +0x14, scales +0x20
        block = h.put('BxH4I6I', bflags, ngroups, 0, 0, gaddr, 0, *[fl(r) for _ in range(6)])
        put(track, 0x28, 'I', block)
    else:
        put(track, 0x28, 'I', unpacked)
    # rotation keys (20 bytes: frame, ease, quaternion), eased through 8-byte tables
    n = r.randrange(1, 6)
    rot = b''.join(struct.pack('<Hh4I', f, r.choice([-1, -1, r.randrange(3)]), *quat(r)) for f in frames(r, n))
    put(track, 0x10, 'H', n)
    put(track, 0x1c, 'I', h.put('%ds' % len(rot), rot))
    put(track, 0x34, 'I', h.put('24B', *[r.randrange(256) for _ in range(24)]))
    # scale keys (16 bytes: frame, x, y, z)
    n = r.randrange(1, 5)
    put(track, 0x12, 'H', n)
    put(track, 0x20, 'I', h.put('HH3I' * n, *[v for f in frames(r, n) for v in (f, 0, fl(r), fl(r), fl(r))]))
    # a path: {flags, u16 points, u16 keys, points, keys}; bezier (2), linear (8) or catmull-rom
    npts = r.randrange(4, 10)
    pflags = r.choice([0, 1, 2, 3, 8, 9])
    points = h.put('%dI' % (3 * 40), *[fl(r, 100) for _ in range(3 * 40)])
    n = r.randrange(1, 6)
    keys = h.put('HH' * n, *[v for f in frames(r, n) for v in (f, 0)])
    put(track, 0x18, 'I', h.put('IHHII', pflags, npts, n, points, keys))
    # a ride path: the model's paths (+0xac) by the node's object's index, the range at +0x24
    obj = h.put('B81xH', r.choice([0, 8, 8, 0xff]), r.randrange(3))  # flags, path index +0x52
    put(node, 4, 'I', r.choice([obj, obj, 0]))
    put(model, 0xac, 'I', h.put('IHHII' * 3, *[v for _ in range(3) for v in
                                               (r.choice([0, 1, 2, 3, 8]), npts, 0, points, 0)]))
    length = r.randrange(1, 12)
    table = h.put('%dI' % (length + 1), *[r.choice([r.pos(100), r.pos(100), fl(r)]) for _ in range(length + 1)])
    put(track, 0x24, 'I', h.put('4I', r.randrange(4), length, 0, table))
    # the third argument is only read at +8 (the block, in game: passed on unused)
    return Call(0x471860, [STATE, MODEL, DATA, NODE, TRACK],
                {STATE: state, MODEL: model, NODE: node, TRACK: track, DATA: h.buf, POS: pos, GLOBALS: g}, ret='a')


if __name__ == '__main__':
    B.main(case)
