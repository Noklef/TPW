# The animator (src/game/motion/animator: 0x4726d0, 0x472bc0-0x4740d0) against the original: a
# random animator (layers, banks of motion blocks, holder, model and rest pose) and the motion
# clocks. Tracks drive rotation, scale and visibility only (no vertices, UVs or paths: track.py
# has those), and the animator never wakes, idles or attaches its holder (calls out of range).
# usage: animator.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from thunks_static import clock as timer  # noqa: E402

B = Bench([(0x46f150, 0x475010), (0x67a400, 0x690000), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202),
           (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)], sines=256)
GCLOCK = 0x785970      # the game clock (Motion_UpdateClocks reads its time and real time)
ANIM, LAYERS, HOLDER, MODEL, NODES, REST, RNODES, DATA, TFLAGS, OUT = (RAM + 0x1000 * k for k in range(10))
CLOCKS = 0x7b4960     # gMotionTime (+0xc), gMotionRealTime (+0x14), step (+0x1c), gMotionFlags (+0x20), ...
TRACKFLAGS = 0x7aecd0  # gTrackFlags (+4)
SCENE = 0x8bd500       # the scene flags (+8)
NPRIMARY, NSECOND = 4, 2


def nan(r):
    return 0x7f800000 | (r.getrandbits(23) or 1) | r.getrandbits(1) << 31


def snan(r):
    """A signalling NaN: the original moves speeds as dwords, an fld would quiet it."""
    return 0x7f800000 | (r.getrandbits(22) or 1) | r.getrandbits(1) << 31


def speed_bits(r):
    return r.choice([f32(1.0), f32(1.0), f32(0.5), f32(2.0), 0, f32(-1.0), f32(r.uniform(0, 5)), f32(r.uniform(0, 5)),
                     snan(r), fl(r), f32(3.3e38)])


def fl(r, scale=None):
    if r.random() < 0.03:
        return nan(r)
    return r.f(scale or r.choice([1, 1, 10, 300, None, 1e38, 1e-38]))


def f32(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


class Heap:
    def __init__(self, r, base, size):
        self.base, self.buf, self.at = base, bytearray(r.bytes(size)), 0

    def put(self, fmt, *values):
        address = self.base + self.at
        put(self.buf, self.at, fmt, *values)
        self.at = (self.at + struct.calcsize('<' + fmt) + 3) & ~3
        return address


def frames(r, n):
    out, f = [], r.randrange(3)
    for _ in range(n):
        out.append(f)
        f += r.choice([0, 1, 1, 2, 3, r.randrange(10)])
    return out


def track(r, h, nodes):
    """A 64-byte track on node index k: visibility (0x20000), rotation (8), scale (0x80)."""
    t = bytearray(r.bytes(0x40))
    flags = 0
    for bit in (0x20000, 8, 0x80):
        if r.random() < 0.4:
            flags |= bit
    put(t, 4, 'I', flags | (r.getrandbits(32) & ~0x3f7ff & ~0x10000))
    index = r.randrange(NPRIMARY + NSECOND)
    put(t, 0x14, 'H', index)
    put(t, 0x3c, 'I', nodes[index])
    n = r.randrange(0, 5)
    put(t, 0x16, 'H', n)
    put(t, 0x30, 'I', h.put('%dh' % max(n, 1), *[r.choice([1, -1]) * f for f in frames(r, max(n, 1))]))
    n = r.randrange(1, 5)
    keys = b''
    for f in frames(r, n):
        q = [r.uniform(-1, 1) for _ in range(4)]
        norm = sum(x * x for x in q) ** 0.5 or 1
        keys += struct.pack('<Hh4f', f, r.choice([-1, -1, 0, 1]), *[x / norm for x in q])
    put(t, 0x10, 'H', n)
    put(t, 0x1c, 'I', h.put('%ds' % len(keys), keys))
    put(t, 0x34, 'I', h.put('16B', *[r.randrange(256) for _ in range(16)]))
    n = r.randrange(1, 4)
    put(t, 0x12, 'H', n)
    put(t, 0x20, 'I', h.put('HH3I' * n, *[v for f in frames(r, n) for v in (f, 0, fl(r, 2), fl(r, 2), fl(r, 2))]))
    return bytes(t)


def block(r, h, nodes):
    """flags (2: material animation), start/end frames (+4, +8), tracks (count +0x12, at +0x2c),
    material animations (count +0x18, at +0x30), listed nodes (count +0x1a, at +0x38)."""
    ntracks = r.randrange(0, 4)
    tracks = h.put('%ds' % (0x40 * max(ntracks, 1)), b''.join(track(r, h, nodes) for _ in range(max(ntracks, 1))))
    nmat = r.randrange(0, 3)
    mats = []
    for _ in range(nmat):
        nk = r.randrange(0, 4)
        f = r.randrange(20)
        keys = []
        for _ in range(nk):
            keys += [f, r.randrange(4)]
            f += r.randrange(0, 20)
        mats += [r.randrange(4), nk, h.put('%dH' % max(2 * nk, 2), *(keys or [0, 0]))]
    mat = h.put('HHI' * max(nmat, 1), *(mats or [0, 0, 0]))
    nlisted = r.randrange(0, 3)
    listed = h.put('%dH' % max(nlisted, 1), *[r.randrange(NPRIMARY + NSECOND) for _ in range(max(nlisted, 1))])
    start = r.randrange(0, 5)
    end = start + r.choice([0, 1, 5, 20, r.randrange(100), r.randrange(100), 0x1000001 + 2 * r.randrange(4), r.getrandbits(32)])
    end &= 0xffffffff  # > 2^24 frames: the counts round as floats
    b = bytearray(r.bytes(0x3c))
    put(b, 0, 'I', r.getrandbits(32) & ~2 | r.choice([0, 2]))
    put(b, 4, 'II', start, end)
    put(b, 0x12, 'H', ntracks)
    put(b, 0x18, 'HH', nmat, nlisted)
    put(b, 0x2c, 'I', tracks)
    put(b, 0x30, 'I', mat)
    put(b, 0x38, 'I', listed if r.random() < 0.9 else 0)
    return h.put('%ds' % len(b), bytes(b))


def model(r, nodes_at, nodes):
    m, n = bytearray(r.bytes(0x100)), bytearray(r.bytes(0x1000))
    put(m, 0x30, 'I', r.getrandbits(32) & ~4 | r.choice([0, 4]))
    put(m, 0x36, 'H', 4)
    put(m, 0x42, 'HH', NPRIMARY + NSECOND, NPRIMARY)
    put(m, 0x50, 'I', nodes_at + 0xf00)  # materials (8 bytes each)
    put(m, 0x70, 'II', nodes_at, nodes_at + 0xa0 * NPRIMARY)
    for k, at in enumerate(nodes):
        o = at - nodes_at
        put(n, o, 'I', r.getrandbits(32) & ~0x10000)
        for off in range(0x10, 0x50, 4):
            put(n, o + off, 'f', r.uniform(-1, 1))
        put(n, o + 0x50, 'H', r.randrange(NPRIMARY + NSECOND))
        put(n, o + 0x58, 'H', 0)
        put(n, o + 0x5c, 'H', 0)
        put(n, o + 0x60, 'I', 0)
        put(n, o + 0x68, 'I', 0)
    return m, n


def world(r):
    """the animator ANIM (its layers, banks of motion blocks, holder, model and rest pose) and the motion clocks: (regions, layer count, blocks)."""
    nodes = [NODES + 0xa0 * k for k in range(NPRIMARY)] + [NODES + 0xa0 * NPRIMARY + 0x58 * k for k in range(NSECOND)]
    rnodes = [RNODES + a - NODES for a in nodes]
    mdl, nds = model(r, NODES, nodes)
    rest, rnds = model(r, RNODES, rnodes)
    h = Heap(r, DATA, 0x1000)
    blocks = [block(r, h, nodes) for _ in range(r.randrange(1, 5))]
    anim = bytearray(r.bytes(0x200))
    put(anim, 0, 'I', ANIM + 0x100)  # -> +8 -> +4: the rest model
    put(anim, 0x108, 'I', ANIM + 0x120)
    put(anim, 0x124, 'I', REST)
    flags = r.getrandbits(32) & ~(8 | 0x4000)
    for bit in (0x8000, 0x10, 0x400000, 0x40000000, 0x100000, 0x1000000, 0x20000000, 4, 0x80000000):
        flags = flags & ~bit | (bit if r.random() < 0.25 else 0)
    put(anim, 4, 'I', flags)
    put(anim, 8, 'I', HOLDER)
    nlayers = r.randrange(1, 5)
    put(anim, 0xe, 'H', nlayers)
    put(anim, 0x10, 'I', LAYERS)
    for bank in range(12):  # bank flags, entry count, table of 8-byte entries (block at +4)
        n = r.choice([0, 1, 2, 3])
        table = h.put('%dI' % max(2 * n, 2), *[v for _ in range(max(n, 1)) for v in (0, r.choice(blocks + [0]) if r.random() < 0.1 else r.choice(blocks))])
        put(anim, 0x18 + 12 * bank, 'III', r.choice([0, 1, 3, r.getrandbits(32)]) if n else r.choice([0, 1]), n, table)
    put(anim, 0xa8, 'I', r.choice([0, 1, 1]))
    layers = bytearray(r.bytes(0x38 * 4))
    now = r.getrandbits(32)
    for k in range(4):
        o = 0x38 * k
        bank = r.choice([0, 1, 2, 5, r.randrange(12), 0xc])
        lflags = r.getrandbits(32) & ~0x7f
        for bit in (1, 2, 4, 8, 0x10, 0x20, 0x40):
            if r.random() < 0.2:
                lflags |= bit
        length = float(r.choice([0, 1, 5, 20, r.randrange(100), r.randrange(100), 3.3e38]))
        start = (now - r.randrange(0, 5000)) & 0xffffffff
        put(layers, o, 'III', lflags, bank, r.randrange(3))
        put(layers, o + 0xc, 'I', speed_bits(r))
        put(layers, o + 0x10, 'III', start, (start + r.randrange(0, 5000)) & 0xffffffff, now)
        put(layers, o + 0x1c, 'f', length)
        put(layers, o + 0x20, 'I', r.choice([f32(r.uniform(-1, min(length, 1e6) + 3)), f32(r.uniform(-1, min(length, 1e6) + 3)),
                                             f32(length), f32(-3.3e38), fl(r)]))  # at the end exactly too
        put(layers, o + 0x24, 'III', r.choice([0xc, 0xc, 0, 1, 0xd, 0xe, r.randrange(12)]), r.randrange(3), r.randrange(16))
        put(layers, o + 0x30, 'I', r.choice([0, 0, f32(1.0), f32(0.5), fl(r), snan(r)]))
        put(layers, o + 0x34, 'I', r.choice(blocks + [0]))
    holder = bytearray(r.bytes(0x40))
    put(holder, 0, 'I', r.getrandbits(32) & ~1)
    put(holder, 4, 'I', MODEL)
    put(holder, 0x28, 'I', 0)
    clocks = bytearray(r.bytes(0x90))
    put(clocks, 0xc, 'I', (now + r.choice([0, 1, 33, r.randrange(10000)])) & 0xffffffff)
    put(clocks, 0x14, 'I', (now + r.randrange(10000)) & 0xffffffff)
    put(clocks, 0x20, 'I', r.choice([0, 1, 8, 0x10, r.getrandbits(32)]))
    tf = bytearray(r.bytes(0x40))
    regs = {ANIM: anim, LAYERS: layers, HOLDER: holder, MODEL: mdl, NODES: nds, REST: rest, RNODES: rnds,
            DATA: h.buf, TFLAGS: tf, OUT: bytearray(r.bytes(0x10)), CLOCKS: clocks,
            TRACKFLAGS: bytearray(struct.pack('<II', 0, TFLAGS)), SCENE: bytearray(r.bytes(0x10))}
    return regs, nlayers, blocks


def mark_tracks(r, regs, blocks):
    """Motion_MarkNodes only reads the tracks' flags: add the vertex (0x1000), UV (0x10000) and matrix (1, 0x200) bits."""
    data = regs[DATA]
    for b in blocks:
        o = b - DATA
        n, tracks = struct.unpack_from('<H', data, o + 0x12)[0], struct.unpack_from('<I', data, o + 0x2c)[0] - DATA
        for k in range(n):
            flags = struct.unpack_from('<I', data, tracks + 0x40 * k + 4)[0]
            for bit in (0x1000, 0x10000, 1, 0x200):
                if r.random() < 0.3:
                    flags |= bit
            put(data, tracks + 0x40 * k + 4, 'I', flags)


def case(r):
    op = r.deal(OPS)
    regs, nlayers, blocks = world(r)
    layer = LAYERS + 0x38 * r.randrange(nlayers)
    blk = r.choice(blocks + [0])
    speed = speed_bits(r)
    bank, entry, mode = r.choice([0, 1, 2, 5, r.randrange(12), 0xc, 0xd, 0xe, r.randrange(20)]), r.randrange(4), r.randrange(16)
    if op == 'Start':
        frame = r.choice([f32(r.uniform(-1, 30)), fl(r), f32(float(r.randrange(30))),
                          f32(float(0x1000000 + 4 * r.randrange(3)))])  # by the counts past 2^24
        return Call(0x472bc0, [layer, blk, r.choice([0, 1]), frame], regs)
    if op == 'Advance':
        return Call(0x472c60, [layer, r.choice([OUT, 0])], regs)
    if op == 'SetBlock':
        return Call(0x472cb0, [layer, blk], regs)
    if op == 'TimeLeft':
        return Call(0x472d50, [layer], regs, ret='d')
    if op == 'Play':
        return Call(0x472f60, [ANIM, bank, entry, layer, mode, speed], regs, ret='a')
    if op == 'AnimPlay':
        return Call(0x4732a0, [ANIM, bank, entry, mode, speed, r.randrange(nlayers)], regs, ret='a')
    if op == 'IsIdle':
        return Call(0x473490, [ANIM], regs, ret='a')
    if op == 'Update':
        return Call(0x4735d0, [ANIM, layer, blk], regs)
    if op == 'AnimUpdate':
        return Call(0x473c70, [ANIM, blk, mode], regs, ret='a')
    if op == 'Reset':
        return Call(0x473e30, [ANIM, r.choice([0, 1]), r.choice([0, 1]), r.choice([0, 1])], regs, ret='a')
    if op == 'SetTime':
        return Call(0x4709d0, [r.getrandbits(32)], regs)
    if op == 'Time':
        return Call(0x4709e0, [r.choice([0, 1, r.getrandbits(32)])], regs, ret='a')
    if op == 'ResetLayers':
        return Call(0x4726d0, [ANIM], regs)
    if op == 'QSpeed':
        return Call(0x474020, [ANIM, speed], regs)
    if op == 'SetSpeed':
        return Call(0x474050, [ANIM, speed, r.randrange(nlayers)], regs)
    if op == 'EndMs':
        return Call(0x474070, [ANIM, bank, entry], regs, ret='a')
    index = r.randrange(nlayers)
    if op == 'HasMotion':
        put(regs[OUT], 0xc, 'I', ANIM)
        return Call(0x472d10, [OUT + 0xc, bank, entry], regs, ret='a')
    if op == 'MarkNodes':
        mark_tracks(r, regs, blocks)
        return Call(0x472d70, [r.choice(blocks), HOLDER], regs)
    if op == 'CountLayers':
        return Call(0x472ee0, [ANIM], regs)
    if op == 'SetPlaying':
        return Call(0x472f40, [ANIM], regs)
    if op == 'ClearQueue':
        return Call(0x473270, [ANIM, index], regs, ret='a')
    if op == 'RefreshNodes':
        return Call(0x473550, [ANIM], regs)
    if op == 'GetFlags':
        return Call(0x473fb0, [ANIM, r.choice([OUT, 0]), r.choice([OUT + 4, 0]), index], regs, ret='a')
    if op == 'GetQueued':
        return Call(0x473ff0, [ANIM, r.choice([OUT, 0]), r.choice([OUT + 4, 0]), index], regs)
    if op == 'SetFlags':
        return Call(0x4740d0, [r.getrandbits(32)], regs)
    if op == 'UpdateClocks':  # the game clock as gameclock.py makes it: paused, frozen or not, on the game's Timer and its imports
        timer(r, regs, True)
        c = bytearray(r.bytes(0xa0))
        put(c, 0x30, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(c, 0x68, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        put(c, 0x38, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
        for o in (0x18, 0x58):
            put(c, o, 'd', r.choice([0.0, 1.0, 1000.5, float(r.getrandbits(32)), r.uniform(0, 2 ** 31)]))
        put(c, 0x20, 'd', r.choice([1.0, 1.0, 0.5, 2.0, 0.0, r.uniform(0, 4)]))
        regs[GCLOCK] = c
        return Call(0x473440, [], regs)
    if op == 'SlotAnimate':  # the model slots (0x790398, 0xb0 bytes each): the animator at +4, or none
        slots = bytearray(r.bytes(0xb0 * 4))
        for k in range(4):
            put(slots, 0xb0 * k + 4, 'I', r.choice([ANIM, ANIM, 0]))
        regs[0x790398] = slots
        return Call(0x429df0, [r.randrange(4)], regs)
    if op == 'BindNodes':
        return Call(0x470a00, [HOLDER, r.choice(blocks), r.choice([0, 1])], regs)
    if op == 'ResetChannels':
        return Call(0x472310, [ANIM, r.choice(blocks), r.choice(blocks + [0])], regs)
    if op == 'MotionApply':
        return Call(0x4721f0, [layer, HOLDER, blk], regs, ret='a')
    if op == 'CopyVertices':  # from the rest holder (ANIM + 0x120: model REST) to HOLDER; the primary nodes get vertices
        for nodes, base in ((regs[NODES], NODES), (regs[RNODES], RNODES)):
            for k in range(NPRIMARY):
                put(nodes, 0xa0 * k + 0x58, 'H', r.choice([0, 1, 3, 4, 5, 8]))
                put(nodes, 0xa0 * k + 0x60, 'I', r.choice([base + 0x400 + 0x80 * k] * 4 + [0]))
                if r.random() < 0.5:
                    put(nodes, 0xa0 * k, 'I', struct.unpack_from('<I', nodes, 0xa0 * k)[0] ^ 0x200000)
        return Call(0x472270, [ANIM + 0x120, HOLDER, r.choice([0, 1])], regs)
    return Call(0x473f50, [ANIM, OUT, r.choice([OUT + 4, 0]), OUT + 8, r.randrange(nlayers)], regs, ret='a')


OPS_ALL = ['Start', 'Advance', 'SetBlock', 'TimeLeft', 'Play', 'AnimPlay', 'IsIdle', 'Update', 'AnimUpdate', 'Reset',
           'ResetLayers', 'QSpeed', 'SetSpeed', 'EndMs', 'Playback', 'SetTime', 'Time', 'HasMotion', 'MarkNodes', 'CountLayers',
           'SetPlaying', 'ClearQueue', 'RefreshNodes', 'GetFlags', 'GetQueued', 'SetFlags', 'BindNodes', 'ResetChannels',
           'MotionApply', 'CopyVertices', 'UpdateClocks', 'SlotAnimate']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
