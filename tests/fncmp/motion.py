# Vector helpers (0x431480-0x431660), packed vertex animation (0x470e90-0x471860),
# quaternions (0x474120-0x4745c0) and path curves (0x474840-0x475010) against the originals.
# usage: motion.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x431480, 0x431660), (0x470e90, 0x471860), (0x474120, 0x4745c0), (0x474840, 0x475010), (0x46f650, 0x46f6f0),
           (0x67a400, 0x690000)], sines=256)
A, PTS, OUT = RAM, RAM + 0x1000, RAM + 0x2000  # small inputs, curve points / keys, outputs
BLK, POS, GRP = RAM + 0x3000, RAM + 0x4000, RAM + 0x5000  # packed block, position blocks, groups
GLOBALS = 0x7b4980  # gMotionFlags (+0), gRotationKeyCount (+8), gCurvePointCount (+0x68)


def nan(r):
    """A NaN of either sign and any payload: of two, the x87 keeps the larger significand."""
    return 0x7f800000 | (r.getrandbits(23) or 1) | r.getrandbits(1) << 31


def floats(r, buf, off, n, scale=None):
    scale = scale or r.choice([1, 1, 10, 300, None, 1e38, 1e-38])
    for k in range(n):
        put(buf, off + 4 * k, 'I', nan(r) if r.random() < 0.03 else r.f(scale))


def quat(r, buf, off):
    """A unit-ish quaternion, sometimes a raw one, sometimes the negation or a copy of the one
    before it (the slerp's nearly-equal and nearly-opposite branches)."""
    if r.random() < 0.3:
        floats(r, buf, off, 4)
        return
    if r.random() < 0.2:  # mixed scales: products of a tiny component reach the denormals
        for k in range(4):
            put(buf, off + 4 * k, 'I', r.f(r.choice([1, 1e-20, 1e-38, 1e-42])))
        return
    q = [r.uniform(-1, 1) for _ in range(4)]
    n = sum(x * x for x in q) ** 0.5 or 1
    put(buf, off, '4f', *[x / n for x in q])
    if off >= 0x14 and r.random() < 0.3:
        prev = struct.unpack_from('<4f', buf, off - 0x14)
        sign = r.choice([1, -1])
        put(buf, off, '4f', *[sign * x + r.uniform(-1e-4, 1e-4) * r.choice([0, 1]) for x in prev])


def packed(r, op, regs, t):
    """A packed vertex-animation block (flags, group count, groups +0xc, offsets +0x14, scales
    +0x20) whose groups (count, slots, frames, keys, key) share one ascending frames array."""
    blk, pos, grp = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x400)), bytearray(r.bytes(0x1000))
    regs.update({BLK: blk, POS: pos, GRP: grp})
    ngroups, nkeys, flags = r.randrange(1, 5), r.randrange(2, 8), r.choice([0, 2, r.randrange(256)])
    if flags & 2 and r.random() < 0.9:  # else the group count runs out past the raw group: crashes
        ngroups = max(ngroups, 2)
    put(blk, 0, 'BxH', flags, ngroups)
    put(blk, 0xc, 'I', GRP)
    floats(r, blk, 0x14, 6)
    frames, f = [], r.randrange(50)
    for _ in range(nkeys):
        frames.append(f)
        f += r.choice([0, 1, 5, 20]) if r.random() < 0.1 else r.randrange(1, 20)
    put(grp, 0xf00, '%dH' % nkeys, *frames)
    for g in range(ngroups):
        n, base = r.randrange(2, 9), GRP + 0x100 + 0x300 * g
        put(grp, 20 * g, 'HHIIII', r.randrange(65536), n, base, GRP + 0xf00, base + 0x40, r.randrange(nkeys - 1))
        put(grp, base - GRP, '%dH' % n, *[r.randrange(32) for _ in range(n)])
        put(grp, base + 0x40 - GRP, '%dI' % (n * nkeys), *[r.getrandbits(32) for _ in range(n * nkeys)])
    add = r.choice([0, 1])
    if op == 'LerpVerts':
        return Call(0x470e90, [BLK, POS, GRP + 20 * r.randrange(ngroups), t, add], regs)
    if op == 'LerpBox':
        return Call(0x4711d0, [BLK, OUT, GRP + 20 * r.randrange(ngroups), t, add], regs)
    node = regs[A]
    put(node, 0, 'I', r.getrandbits(32) & ~0x800000 | r.choice([0, 0x800000]))
    put(node, 0x60, 'I', POS)
    frame = r.uniform(frames[0] - 5, frames[-1]) if r.random() < 0.8 else r.choice([frames[0], frames[-1]])
    return Call(0x4714a0, [A, BLK, struct.unpack('<I', struct.pack('<f', frame))[0], r.getrandbits(32), add], regs)


def case(r):
    op = r.choice(OPS)
    a, pts, out = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x1000)), bytearray(r.bytes(0x100))
    floats(r, a, 0, 16)
    regs = {A: a, PTS: pts, OUT: out}
    t = r.choice([r.pos(1), r.f(1), r.pos(r.choice([4, 12]))])
    if op == 'Modf':
        return Call(0x431480, [r.f(), OUT], regs, ret='d')
    if op == 'RotYCS':
        return Call(0x4314b0, [A, r.f(1), r.f(1)], regs)
    if op in ('Vec3RotX', 'Vec3RotY', 'MatRotY'):
        return Call({'Vec3RotX': 0x4314f0, 'Vec3RotY': 0x431570, 'MatRotY': 0x4315f0}[op], [A, r.f(7)], regs)
    if op == 'QuatToMatrix':
        quat(r, a, 0)
        return Call(0x474120, [A, OUT], regs)
    if op in ('Slerp', 'RotationAt'):
        n = r.randrange(1, 8)
        for k in range(n + 1):
            put(pts, 0x14 * k, 'Hh', r.randrange(100), r.randrange(-3, 4))
            quat(r, pts, 0x14 * k + 4)
        g = bytearray(r.bytes(0x70))
        put(g, 0, 'I', r.choice([0, 2, r.getrandbits(32)]))
        put(g, 8, 'I', n)
        regs[GLOBALS] = g
        if op == 'Slerp':
            k = r.randrange(n)
            return Call(0x4742a0, [PTS + 0x14 * k + 4, PTS + 0x14 * (k + 1) + 4, t, OUT], regs)
        return Call(0x474490, [PTS, r.randrange(n), t, OUT], regs)
    if op == 'Percent':
        n = r.randrange(1, 20)
        for k in range(n + 1):
            put(pts, 4 * k, 'I', r.choice([r.pos(100), r.pos(100), r.f(), r.f(1e38), nan(r)]))
        tt = r.choice([t, r.pos(n + 1), float(n), float(n - 1)])
        if isinstance(tt, float):
            tt = struct.unpack('<I', struct.pack('<f', tt))[0]
        return Call(0x474ea0, [PTS, n, tt, OUT, r.choice([0, 1])], regs)
    if op in ('LerpVerts', 'LerpBox', 'Unpack'):
        return packed(r, op, regs, t)
    # the curves: own points at curve + 8 or the given ones; count from gCurvePointCount
    n = r.randrange(1, 12)
    floats(r, pts, 0, 3 * 40)
    g = bytearray(r.bytes(0x70))
    put(g, 0x68, 'I', n)
    regs[GLOBALS] = g
    put(a, 8, 'I', PTS + 0x10)
    base = r.randrange(0, 4)
    points = r.choice([0, PTS])
    address = {'Bezier': 0x474840, 'BezierTan': 0x474a20, 'Linear': 0x474bf0, 'LinearTan': 0x474c70,
               'CatmullRom': 0x474cc0}[op]
    tt = r.choice([t, r.pos(n - 0.001)])
    if isinstance(tt, float):
        tt = struct.unpack('<I', struct.pack('<f', tt))[0]
    return Call(address, [A, points, base, tt, OUT], regs)


OPS_ALL = ['Modf', 'RotYCS', 'Vec3RotX', 'Vec3RotY', 'MatRotY', 'QuatToMatrix', 'Slerp', 'RotationAt', 'Percent',
           'Bezier', 'BezierTan', 'Linear', 'LinearTan', 'CatmullRom', 'LerpVerts', 'LerpBox', 'Unpack']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
