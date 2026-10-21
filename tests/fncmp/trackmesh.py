# The coaster track's mesh built along a piece (src/game/coaster/trackmesh: 0x43f4e0) against the original. The coaster [0x792218] and
# its mesh as coastermesh.py lays them out: Emit cases its bare animator (+0x10, +0x14, +0x18 the vertex buffers; its holder's mesh's
# part's +0x6c the list slots), the piece's +0 set and restart 0 (so no mesh is finished unless a count runs out); Restart cases
# coastermesh.py's Reset surroundings (CoasterMesh_Finish(0) clones the build's animator), the piece's +0 0 or restart set (any dword,
# -0.0 among them). The piece and its next (the same one half the time): flags +8 (0x800, 0x4000, 0x1000000; the next's 0x2000,
# 0x800000), +0xb0 0 or not, +0x100's byte +4 bit 0 now and then, +0x90 a spline of 1-5 0x58-byte nodes (coaster.py's node fields;
# their +8 the blend's weights) and up nodes (+8 a coaster node number, the track's flag 8 or not), the coaster's twelve 0x38-byte
# nodes (+0x18 the banks). The samples (0-6) start [0x79240c] + i / (n - 1) * span, as Keys_MergeSamples makes them: over the
# piece's nodes (the last node's whole number among them) or 0..1 between pieces, now and then any. The records: [0x792414] 1-6 of
# rising positions about the samples' (the last 1e5), each a section of 0-10 points (xsection.py's point floats; second points or
# not) and 0-6 index entries (lists 0-4, points within the count or one past), or the fixed ones at 0x7923f0 / 0x7925e8 (1e7, the
# build's sections +0x84 / +0x80; the build's +0x20 and [0x7925d0] small so that their lists exist). The flag 0x7925f8, the toggle
# 0x7923e4 and the vertex count [0x79c600] small or any; the rooms 0x7923e0 / 0x792410 large, small or negative.
# usage: trackmesh.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Emit,Restart to pick)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402
import animatormesh  # noqa: E402
import coastermesh  # noqa: E402
import holder  # noqa: E402
from texdirs import merged  # noqa: E402

B = Bench(merged([(0x43f150, 0x43f4db), (0x45bef0, 0x45bf20), (0x455690, 0x4557bf), (0x454a10, 0x455689), (0x44b140, 0x44b220)]
                 + animatormesh.RANGES + holder.RANGES + [
    (0x425440, 0x4254a0), (0x43e1b0, 0x43e490), (0x43e590, 0x43e990), (0x43f150, 0x4402e0), (0x44b140, 0x44b230), (0x454a10, 0x4554e0),
    (0x455690, 0x4557d0), (0x45b9d0, 0x45ba10), (0x45bbc0, 0x45bc70), (0x45bd00, 0x45bdb0), (0x45bef0, 0x45bf30), (0x46f210, 0x46f250),
    (0x54dcc0, 0x54dd00), (0x56af20, 0x56afb0), (0x56e3f0, 0x56e4f0), (0x575b90, 0x575c00), (0x575c60, 0x575e20), (0x575f90, 0x575fb0),
    (0x576270, 0x576320), (0x576410, 0x576550), (0x5775d0, 0x577610), (0x579b40, 0x579c30), (0x579cd0, 0x57a430), (0x587c80, 0x587d80),
    (0x67a740, 0x67a800)]), hooks={**holder.HOOKS, **animatormesh.HOOKS}, sines=256)
X = RAM + 0x80000
PIECE, NEXT, PSPL, NSPL, FLAG100, PNODES, PUP, NNODES, NUP, SPIECES, CNODES = (X + 0x200 * k for k in range(11))
SECS, SECTIONS, PTS, QTS, ENTRIES, POOL, VPOS, VNORM, VUV, SLOTS2 = (X + 0x2000 + 0x600 * k for k in range(10))


def nodes58(r, count, pieces, up):
    """count 0x58-byte spline nodes (and room for one more read past them)."""
    b = bytearray(r.bytes(0x58 * (count + 1) + 0x10))
    for k in range(count + 1):
        o = 0x58 * k
        r.floats(b, range(o + 0x34, o + 0x58, 4))
        put(b, o + 8, 'f', r.choice([r.uniform(0, 7.99), r.randrange(8), -0.5]) if up else r.choice([r.uniform(-2, 2), 0.0, 1.0]))
        if not up and r.random() < 0.1:
            put(b, o + 8, 'I', r.f())
        put(b, o + 0xc, 'I', r.f(5))
        put(b, o + 0x10, 'I', pieces + (k % 2) * 0x100)
    return b


def sections(r, regions):
    """Up to six sections; returns their addresses."""
    n = r.randrange(1, 7)
    secs = bytearray(r.bytes(0x18 * n))
    ents = bytearray(r.bytes(0x60 * n))
    out = []
    for j in range(n):
        pts = r.choice([0, 1, 2, 4, 8, 10]) if r.random() < 0.97 else r.choice([-1, 11])
        idx = r.choice([0, 1, 2, 4, 6]) if r.random() < 0.97 else r.choice([-1, -3])
        second = r.choice([0, 0, QTS]) if pts >= 0 else 0   # (xsection.h: a count below 0 with second points blends ~4G points, sic)
        put(secs, 0x18 * j, 'iiIIII', pts, idx, PTS, ENTRIES + 0x60 * j, second, r.f())
        for i in range(8):
            put(ents, 0x60 * j + 12 * i + 4, 'HHH', r.randrange(5), r.randrange(max(pts, 0) + 1), r.randrange(max(pts, 0) + 1))
        out.append(SECTIONS + 0x18 * j)
    regions[SECTIONS] = secs
    regions[ENTRIES] = ents
    pts = lambda: bytearray(b''.join(r.choice([r.f(), r.f(), r.f(10), r.f(0.01), 0]).to_bytes(4, 'little') for _ in range(5 * 12)))  # noqa: E731
    regions[PTS], regions[QTS] = pts(), pts()
    return out


def case(r):
    op = r.deal(OPS)
    if op == 'Restart':
        c = coastermesh.reset(r)
        regions = dict(c.regions)
    else:
        regions = coastermesh.finish(r, 'bare')
        anim = coastermesh.ANIM
        regions[anim] = put(bytearray(regions[anim]), 0x10, 'III', VPOS, VNORM, VUV)
        regions[VPOS], regions[VNORM], regions[VUV] = r.bytes(0x600), r.bytes(0x600), r.bytes(0x400)
        regions[coastermesh.PART] = put(bytearray(regions[coastermesh.PART]), 0x6c, 'I', SLOTS2)
        regions[SLOTS2] = r.bytes(0x400)
    secs = sections(r, regions)
    lists = bytearray(regions[coastermesh.LISTS])
    for i in range(5):   # (the tails: none, or an item of coastermesh.py's)
        put(lists, 0x1c * i + 0xc, 'I', r.choice([0, 0, coastermesh.ITEMS + 0xc * r.randrange(16)]))
    regions[coastermesh.LISTS] = lists
    coaster = bytearray(regions[coastermesh.COASTER]) + bytearray(r.bytes(0x60))
    put(coaster, 0x74, 'I', r.f())
    put(coaster, 0x9c, 'I', CNODES)
    regions[coastermesh.COASTER] = coaster
    build = bytearray(regions[coastermesh.BUILD]) + bytearray(r.bytes(0x68))
    build[0] = r.choice([0, 0x40, r.getrandbits(8)])
    put(build, 0x20, 'I', r.choice([0, 1, 2]) if r.random() < 0.97 else r.getrandbits(32))
    put(build, 0x80, 'II', r.choice(secs), r.choice(secs))
    regions[coastermesh.BUILD] = build
    track = bytearray(regions[coastermesh.TRACK]) + bytearray(r.bytes(0x20))
    track[0] = r.choice([0, 8, r.getrandbits(8)])
    put(track, 0x34, 'I', r.f())
    put(track, 0x3c, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[coastermesh.TRACK] = track
    cn = bytearray(r.bytes(12 * 0x38))
    for k in range(12):
        put(cn, k * 0x38 + 0x18, 'I', r.f(4))
    regions[CNODES] = cn
    spieces = bytearray(r.bytes(0x200))
    for k in range(2):
        put(spieces, 0x100 * k, 'I', r.choice([0, 1, 5, r.randrange(100), r.getrandbits(32)]))
        put(spieces, 0x100 * k + 0x6c, 'I', r.f(5))
        put(spieces, 0x100 * k + 0xe4, 'I', r.f(4))
    regions[SPIECES] = spieces

    same = r.random() < 0.5
    n = r.choice([2, 2, 3, 4, 5] if same else [1, 2, 3, 4, 5])
    m = r.choice([1, 2, 3, 4, 5])
    regions[PNODES], regions[PUP] = nodes58(r, n, SPIECES, False), nodes58(r, n, SPIECES, True)
    regions[PSPL] = struct.pack('<iII', n, PNODES, PUP)
    piece = bytearray(r.bytes(0x110))
    flags = r.choice([0, 0x800, 0x800, 0x4000, 0x1000000, 0x1004000, 0x4800]) | (r.getrandbits(32) & ~0x1804800 if r.random() < 0.2 else 0)
    put(piece, 0, 'I', 0 if op == 'Restart' and r.random() < 0.5 else r.choice([1, r.getrandbits(32) | 1]))
    put(piece, 8, 'I', flags)
    put(piece, 0x90, 'I', PSPL)
    put(piece, 0xb0, 'I', 0 if r.random() < 0.75 else r.getrandbits(32) | 1)
    put(piece, 0x100, 'I', FLAG100)
    regions[FLAG100] = bytes([0] * 4 + [r.choice([0, 0, 0, 0, 0, 0, 1, 0xff, 0xfe])] + [0] * 3)
    regions[PIECE] = piece
    if same:
        nxt = PIECE
    else:
        nxt = NEXT
        regions[NNODES], regions[NUP] = nodes58(r, m, SPIECES, False), nodes58(r, m, SPIECES, True)
        regions[NSPL] = struct.pack('<iII', m, NNODES, NUP)
        other = bytearray(r.bytes(0x110))
        put(other, 8, 'I', r.choice([0, 0, 0x2000, 0x800000, 0x4000, 0x4000]))
        put(other, 0x90, 'I', NSPL)
        regions[NEXT] = other

    # the samples and the records
    g = r.choice([0.0, 0.0, r.uniform(0, 50), r.uniform(-20, 0)])
    count = r.choice([0, 1, 2, 3, 4, 6])
    span = (n - 1) if same else r.choice([1.0, r.random()])
    if same and r.random() < 0.2:
        span = r.uniform(0, n - 1)
    pos = [g + (i / (count - 1) * span if count > 1 else 0.0) for i in range(count)]
    if r.random() < 0.1:
        pos = sorted(r.uniform(g - 1, g + span + 1) for _ in range(count))
    nrec = r.randrange(1, 7)
    thr = sorted(r.uniform(g - 0.5, g + span + 0.5) for _ in range(nrec - 1)) + [1e5]
    recs = bytearray(r.bytes(0x10 + 0x10 * nrec))
    for j in range(nrec):
        put(recs, 0x10 + 0x10 * j, 'fIII', thr[j], r.f(), r.f(), r.choice(secs))
    regions[SECS] = recs
    lo = bytearray(r.bytes(0x40 + 4 * 8))
    put(lo, 0, 'i', r.choice([1000, 1000, r.randrange(0, 40), r.randrange(-5, 0)]) if op == 'Restart' else 1000)   # 0x7923e0
    put(lo, 4, 'I', r.choice([0, 1, r.getrandbits(32)]))   # 0x7923e4
    put(lo, 0x10, 'ffII', 1e7, 1e7, 0, struct.unpack_from('<I', build, 0x84)[0])   # 0x7923f0
    put(lo, 0x28, 'I', POOL)   # 0x792408
    put(lo, 0x2c, 'f', g)   # 0x79240c
    put(lo, 0x30, 'iI', r.choice([1000, 1000, r.randrange(0, 30), r.randrange(-5, 0)]) if op == 'Restart' else 1000, SECS + 0x10)   # 0x792410, 0x792414
    for i, p in enumerate(pos):
        put(lo, 0x38 + 4 * i, 'f', p)
    regions.pop(0x792408, None)
    regions[0x7923e0] = lo
    hi = bytearray(regions[0x7925a8]) + bytearray(r.bytes(0x68 - len(regions[0x7925a8])))
    put(hi, 0x20, 'I', r.choice([0, 1, 2]))   # 0x7925c8 (the list slots taken)
    put(hi, 0x28, 'I', r.choice([0, 1, 2]))   # 0x7925d0
    put(hi, 0x40, 'ffII', 1e7, 1e7, 0, struct.unpack_from('<I', build, 0x80)[0])   # 0x7925e8
    put(hi, 0x50, 'I', r.choice([0, 1]))   # 0x7925f8
    regions[0x7925a8] = hi
    regions[POOL] = r.bytes(0x400)
    c600 = bytearray(regions[0x79c600])
    put(c600, 0, 'I', r.choice([0, 1, 4, 7]) if op != 'Restart' or r.random() < 0.9 else r.getrandbits(16))
    put(c600, 4, 'I', POOL)
    regions[0x79c600] = c600
    restart = 0 if op == 'Emit' else r.choice([1, fb(1.0), 0x80000000, r.getrandbits(32) | 1])
    if op == 'Restart' and struct.unpack_from('<I', piece, 0)[0] == 0 and r.random() < 0.5:
        restart = 0
    if op == 'Restart':   # (the rooms Finish(0) gives: the sizes' words +4, +6)
        regions[coastermesh.SIZES] = put(bytearray(regions[coastermesh.SIZES]), 4, 'HH', r.choice([1000, r.randrange(0, 30)]), r.choice([1000, r.randrange(0, 40)]))
    return Call(0x43f4e0, [PIECE, nxt, count if r.random() < 0.95 else r.choice([0xffffffff, 0x80000000]), 0x792418, restart], regions)


OPS = [k for k in os.environ.get('OPS', 'Emit,Emit,Emit,Restart').split(',') if k]


if __name__ == '__main__':
    B.main(case)
