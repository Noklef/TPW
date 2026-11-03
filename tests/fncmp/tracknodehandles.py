# A track piece's node handles (src/game/coaster/handles: 0x4321c0, 0x432430) against the original, with TrackPiece_NodeGaps and _isnan. A piece of 0-5 nodes with a
# previous and a next piece (or none); each node's owner one of those, its links dealt empty; a source or none; straight up or down by the flags; a railed info or none. Now and then a NaN.
# usage: tracknodehandles.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Piece,Node)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x4321c0, 0x432b60), (0x67b410, 0x67b450)], sines=256)
OPS = [k for k in os.environ.get('OPS', 'Piece,Node').split(',') if k]
PIECES, LISTS, NODES, VECS, INFO, TYPE, RULES, G, SOURCE, TABLE, WAY = (RAM + k * 0x1000 for k in range(11))
PIECE, NODE, LIST, VEC = 0x120, 0x58, 0x40, 0x40
NANS = [0x7fc00000, 0xffc00000, 0x7fa00000, 0x7f800000, 0xff800000]


def f(r, nan, lo=-30.0, hi=30.0):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def info(r):
    return put(bytearray(r.bytes(4)), 0, 'I', r.getrandbits(32) & ~0x4000 | r.deal([0, 0x4000]))


def piece_at(k):
    return PIECES + PIECE * k


def pieces(r, nan):
    """Three pieces: 0 the piece, 1 the previous, 2 the next; each with lists of nodes; the links of the others dealt too."""
    blob, lists, nodes = bytearray(r.bytes(3 * PIECE)), bytearray(r.bytes(3 * LIST)), bytearray(r.bytes(36 * NODE))
    for k in range(3):
        at = k * PIECE
        put(blob, at + 8, 'I', r.getrandbits(32) & ~0xc0 | r.deal([0, 0x40]) | r.deal([0, 0x80]))
        put(blob, at + 0x4c, 'III', f(r, nan), f(r, nan), f(r, nan))
        put(blob, at + 0x64, 'II', piece_at(r.choice([1, 2])) if r.random() < 0.8 else 0, piece_at(r.choice([1, 2])) if r.random() < 0.8 else 0)
        put(blob, at + 0x90, 'I', LISTS + k * LIST)
        put(blob, at + 0x100, 'I', TYPE)
        put(blob, at + 0xec, 'I', f(r, nan, -7.0, 7.0))
        put(blob, at + 0xf8, 'I', f(r, nan, -3.0, 3.0))
        n = r.deal([0, 1, 2, 3, 5]) if k == 0 else r.choice([1, 2, 3])
        first, second = NODES + 2 * k * 6 * NODE, NODES + (2 * k + 1) * 6 * NODE
        put(lists, k * LIST, 'III', n, first, second)
        for start in (first, second):
            for j in range(6):
                base = start - NODES + j * NODE
                put(nodes, base + 4, 'I', VECS + VEC * r.randrange(4))
                put(nodes, base + 0x10, 'I', piece_at(r.randrange(3)))
                put(nodes, base + 0x40, 'III', f(r, nan), f(r, nan), f(r, nan))
    put(blob, 0x64, 'II', piece_at(2) if r.random() < 0.8 else 0, piece_at(1) if r.random() < 0.8 else 0)
    return blob, lists, nodes


def regions_common(r, nan):
    vecs = bytearray(r.bytes(4 * VEC))
    for j in range(4):
        put(vecs, j * VEC + 0x20, 'III', f(r, nan), f(r, nan), f(r, nan))
        put(vecs, j * VEC + 0x28, 'III', f(r, nan), f(r, nan), f(r, nan))
    rules = put(bytearray(r.bytes(0x10)), 8, 'I', f(r, nan, 0.0, 5.0))
    return {VECS: vecs, RULES: rules, G: put(bytearray(r.bytes(8)), 4, 'I', RULES), INFO: info(r)}


def node_case(r, nan):
    """TrackNode_Handles alone: any before and after."""
    blob, lists, nodes = pieces(r, nan)
    regions = regions_common(r, nan)
    regions.update({PIECES: blob, LISTS: lists, NODES: nodes})
    put(nodes, 0x10, 'I', piece_at(r.randrange(3)))
    regions[WAY] = struct.pack('<III', f(r, nan), f(r, nan), f(r, nan)) if r.random() < 0.9 else bytes(12)
    return Call(0x432430, [RULES, WAY, NODES, f(r, nan, 0.0, 9.0), f(r, nan, 0.0, 9.0), r.choice([INFO, INFO, INFO, 0]), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)], regions)


def case(r):
    nan = r.deal([0, 0, 0, 0, 0.03])
    if r.deal(OPS) == 'Node':
        return node_case(r, nan)
    blob, lists, nodes = pieces(r, nan)
    regions = regions_common(r, nan)
    regions.update({PIECES: blob, LISTS: lists, NODES: nodes, TYPE: put(bytearray(r.bytes(0x20)), 0x18, 'I', r.choice([INFO, INFO, INFO, 0])), 0x792218: struct.pack('<I', G)})
    source = 0
    if r.random() < 0.4:
        source = SOURCE
        table = bytearray(r.bytes(20 * 8))
        for j in range(8):
            put(table, 20 * j + 4, 'I', VECS + VEC * r.randrange(4))
        regions[TABLE] = table
        # (source +8 -> SOURCE + 0x10, whose +0x28 -> SOURCE + 0x30, whose +4 -> the table)
        regions[SOURCE] = put(put(bytearray(r.bytes(0x40)), 8, 'I', SOURCE + 0x10), 0x38, 'I', SOURCE + 0x30)
        put(regions[SOURCE], 0x34, 'I', TABLE)
        for j in range(5):
            put(nodes, j * NODE + 4, 'I', r.randrange(8))
    return Call(0x4321c0, [PIECES, source], regions)


if __name__ == '__main__':
    B.main(case)
