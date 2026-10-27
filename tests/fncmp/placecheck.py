# Whether a coaster piece may go on a tile (src/game/coaster/placecheck: 0x434740) against the original, with the CRT (its acos).
# The coaster [0x790fec] with its type's two rule blocks (squared distances 0-3 / 1-20, interpolation distances, angle limits in
# 0..pi, now and then a degenerate span or a NaN), eight pieces chained before / after on a walk over a 3-10 tile map (now and
# then a chain's end, kinds 0-2, flags 0x40 / 0x10000 / 0x20000), their nodes and a few more on two coaster links (bits 0 / 2 now
# and then, the coaster or another, the rule blocks or another, counts 0-4 against 0-4, objects' flags 0x200 / 0x400), the
# tiles' taken bits, the editor's flags 0x100 / 0x40, 0x79c664 1 or not and its selected piece; the tile near the piece, or
# anywhere (off the map too); half the cases with a piece given.
# Kept away (the original reads an unset slot): no piece given, moving (0x100), and the piece chosen without a next one: that
# piece gets flags 0x30000, so the angle test, the only reader of the slot, is not reached.
# usage: placecheck.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x434740, 0x434f08), (0x67a400, 0x690000)])
COASTER, TYPE, PIECES, NODES = RAM, RAM + 0x100, RAM + 0x400, RAM + 0xd00
OBJS, LINKS, OTHER, WORLD, TFLAGS = RAM + 0x1200, RAM + 0x1800, RAM + 0x1700, RAM + 0x1400, RAM + 0x1500
NP, NN = 8, 12   # pieces, nodes (a piece's node is its own index; four more)
PIECE, NODE = 0x110, 0x40


def piece_at(k):
    return PIECES + PIECE * k


def case(r):
    w, h = r.randrange(3, 11), r.randrange(3, 11)
    # the walk
    tiles = [(r.randrange(w), r.randrange(h))]
    for _ in range(NP - 1):
        x, z = tiles[-1]
        tiles.append((x + r.choice([-2, -1, -1, 0, 1, 1, 2]), z + r.choice([-2, -1, -1, 0, 1, 1, 2])))
    tiles += [(r.randrange(w), r.randrange(h)) for _ in range(NN - NP)]
    blocks = [TYPE + 0xa8, TYPE + 0x1c8]
    coaster = bytearray(r.bytes(0x80))
    other = bytearray(r.bytes(0x100))   # another coaster, and rules no node of this one has
    pieces = bytearray(r.bytes(PIECE * NP))
    nodes = bytearray(r.bytes(NODE * NN))
    objs = bytearray(r.bytes(0x100))
    for k in range(NP):
        o = PIECE * k
        put(pieces, o, 'I', r.choice([0, 1, 2, 2, r.getrandbits(32)]))
        put(pieces, o + 8, 'I', r.getrandbits(32) & ~0x30040 | r.choice([0, 0, 0, 0x40]) | r.choice([0, 0, 0, 0x10000, 0x20000, 0x30000]))
        put(pieces, o + 0x64, 'I', piece_at(k - 1) if k and r.random() < 0.9 else 0)
        put(pieces, o + 0x68, 'I', piece_at(k + 1) if k < NP - 1 and r.random() < 0.9 else 0)
        put(pieces, o + 0x100, 'I', NODES + NODE * k)
    for k in range(NN):
        o = NODE * k
        x, z = tiles[k]
        put(nodes, o + 4, 'I', r.getrandbits(32) & ~5 if r.random() < 0.8 else r.getrandbits(32))
        put(nodes, o + 8, 'I', (z * w + x) & 0xffffffff if r.random() < 0.95 else r.getrandbits(32))
        put(nodes, o + 0xc, 'I', x & 0xffffffff)
        put(nodes, o + 0x10, 'I', z & 0xffffffff)
        put(nodes, o + 0x14, 'I', COASTER if r.random() < 0.85 else OTHER)
        put(nodes, o + 0x18, 'I', r.choice(blocks + blocks + [OTHER]))
        put(nodes, o + 0x28, 'I', 0)
        put(nodes, o + 0x30, 'I', r.randrange(5))
        put(nodes, o + 0x34, 'I', OBJS + 0x10 * r.randrange(8))
    for k in range(8):
        put(objs, 0x10 * k + 8, 'I', r.getrandbits(32) & ~0x600 | r.choice([0, 0, 0, 0x200, 0x400]))
    put(other, 0xcc, 'I', r.randrange(5))
    # the links: the nodes in two chains, shuffled
    order = list(range(NN))
    r.shuffle(order)
    cut = r.randrange(NN + 1)
    links = bytearray(r.bytes(0xc0))
    chains = [order[:cut], order[cut:]]
    for c, chain in enumerate(chains):
        put(links, 0x60 * c + 0x4c, 'I', NODES + NODE * chain[0] if chain else 0)
        put(links, 0x60 * c + 0x18, 'I', LINKS + 0x60 if c == 0 else 0)
        for a, b in zip(chain, chain[1:] + [None]):
            put(nodes, NODE * a + 0x28, 'I', NODES + NODE * b if b is not None else 0)
    # the type's rule blocks
    tp = bytearray(r.bytes(0x2a0))
    for b in (0xa8, 0x1c8):
        for j in range(4):   # +0x74.. / +0x84..: the low limits (+0, +8), the high (+4, +0xc)
            for base in (0x74, 0x84):
                v = r.uniform(0, 1.6) if j in (0, 2) else r.uniform(1.4, 3.2)
                put(tp, b + base + 4 * [0, 2, 1, 3][j], 'I', fb(v) if r.random() < 0.97 else r.f())
        put(tp, b + 0x9c, 'I', r.randrange(4))
        put(tp, b + 0xa0, 'I', r.randrange(1, 21))
        near = r.uniform(0, 2)
        put(tp, b + 0xa4, 'f', near)
        put(tp, b + 0xa8, 'I', fb(near) if r.random() < 0.05 else fb(near + r.uniform(0.5, 5)))
        put(tp, b + 0xcc, 'I', r.randrange(5))
    put(coaster, 4, 'I', TYPE)
    put(coaster, 0x3c, 'I', r.getrandbits(32) & ~3 | r.getrandbits(2))
    for off in (0x54, 0x58, 0x60, 0x64):
        put(coaster, off, 'I', piece_at(r.randrange(NP)))
    flags = r.getrandbits(32) & ~0x140 | r.choice([0, 0x100]) | r.choice([0, 0x40])
    selected = piece_at(r.randrange(NP)) if r.random() < 0.97 else 0
    editor = bytearray(r.bytes(0x50))   # 0x79c658..
    put(editor, 0, 'I', flags)
    put(editor, 0xc, 'I', r.choice([0, 1, 1, 2]))
    put(editor, 0x4c, 'I', selected)
    tflags = bytearray(r.getrandbits(8) & (1 if r.random() < 0.3 else 0) for _ in range(4 * w * h))
    world = bytearray(0x100)
    put(world, 0x6c, 'I', WORLD + 0x80)
    put(world, 0x80 + 0x18, 'I', w)
    put(world, 0x80 + 0x1c, 'I', h)
    put(world, 0xc0 + 0x2c, 'I', TFLAGS)

    def p32(buf, off):
        return struct.unpack_from('<I', buf, off)[0]

    def piece_field(p, off):
        return p32(pieces, p - PIECES + off) if PIECES <= p < PIECES + PIECE * NP else None

    given = r.random() < 0.5
    if given:
        k = r.randrange(NP)
        piece = piece_at(k)
        px, pz = tiles[k]
        nk = k + 1 if k < NP - 1 else k - 1
        nx, nz = tiles[nk] if r.random() < 0.9 else (r.randrange(w), r.randrange(h))
        base = (px, pz)
    else:
        piece, px, pz, nx, nz = 0, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)
        # the piece the original chooses (as the port does), kept from the unset slot
        c3c = p32(coaster, 0x3c)
        if c3c & 2:
            if flags & 0x100:
                nxt = piece_field(selected, 0x68) if selected else None
                frm = None if nxt is None or (piece_field(nxt, 8) or 0) & 0x10000 else nxt
            else:
                frm = p32(coaster, 0x60)
        else:
            frm = piece_field(selected, 0x68) if (flags & 0x100) and selected else p32(coaster, 0x58) if not flags & 0x100 else None
        if frm and (flags & 0x100) and piece_field(frm, 0x68) == 0:
            o = frm - PIECES + 8
            put(pieces, o, 'I', p32(pieces, o) | 0x30000)
        base = tiles[(frm - PIECES) // PIECE] if frm else (r.randrange(w), r.randrange(h))
    if r.random() < 0.8:
        x, z = base[0] + r.choice([-2, -1, 0, 1, 1, 2]), base[1] + r.choice([-2, -1, 0, 1, 2])
    else:
        x, z = r.randrange(-1, w + 2), r.randrange(-1, h + 2)
    if r.random() < 0.3:   # a tile a node is on
        x, z = tiles[r.randrange(NN)]
    regions = {COASTER: coaster, TYPE: tp, PIECES: pieces, NODES: nodes, OBJS: objs, LINKS: links, OTHER: other, WORLD: world,
               TFLAGS: tflags, 0x790fe0: struct.pack('<4I', LINKS if r.random() < 0.95 else 0, 0, 0, COASTER), 0x79c658: editor,
               0x79fc54: struct.pack('<I', WORLD + 0xc0), 0x7a0854: struct.pack('<I', WORLD)}
    args = [x & 0xffffffff, z & 0xffffffff, piece, px & 0xffffffff, pz & 0xffffffff, nx & 0xffffffff, nz & 0xffffffff]
    return Call(0x434740, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
