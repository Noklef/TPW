# A track piece selected for editing (src/game/coaster/edittools: 0x446d50 CoasterEdit_SelectPiece) against the original, over
# edittools.py's Drag world (the piece, its definition (flag 2 now and then), train, type and costs, the game, the action recorder, the
# editor's globals 0x79c600..): the pieces OTHER -> the piece -> OTHER2 chained by +0x64 with rising handles (+0), the piece's +0x68 OTHER
# mostly (OTHER2 with a +0x64, or none, now and then: none faults in CoasterEdit_SetBounds, in both), its +0x64 OTHER2 or none, OTHER2's
# +0x68 the piece; OTHER's definition (+0x100) the piece's, OTHER2's mostly (else none), OTHER's flag 0x10000 (SetBounds then takes
# the piece's +0x64) only with OTHER2; the game always there;
# the old other end 0x79c628 none or a piece the span reaches. The definition's cell (+0xc, +0x10) about the map's sizes (0x7a0314 /
# 0x7a0374, 0 or 1 now and then) and below 0; PathDrag_Begin's map cell and entry for the clamped cell (pathdrag.py's, under the world
# [0x7cf83c] the money's), 0x5367a0 its thiscall recorder (28). CoasterEdit_SetBounds as coaster.py's EditBounds: the game's +0x3c bit 1
# either, its type (+4, the Drag world's fallback type moved and extended) with small squared reaches (+0x148 / +0x268), the map's origins.
# usage: selectpiece.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import edittools as et  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops
et.OPS = ['Drag']

B = Bench([(0x446d50, 0x446f9e), (0x441cd0, 0x441e78), (0x403400, 0x403fc6), (0x45d630, 0x45d660), (0x5da3c0, 0x5da3d0), (0x433280, 0x433310),
           (0x434650, 0x434740), (0x43e160, 0x43e1c0), (0x447830, 0x4479a0), (0x52c150, 0x52c1b7), (0x4d0af0, 0x4d0b10), (0x434f10, 0x43501c),
           (0x46f150, 0x46f180), (0x6ad810, 0x6ad830)],
          hooks={0x5367a0: _recorder(28, 2)})
CELLS = RAM + 0x100000   # ([0x8023a0]: the map entries, 0x28 bytes each from + 0x10ffd8)
FALL = RAM + 0x7000   # (the Drag world's fallback type moved here and extended past + 0x268)


def ident(x, y):
    return ((y << 7) + x + 1) & 0xffff


def s32(v):
    return (v & 0xffffffff) - ((v & 0x80000000) << 1)


def case(r):
    regions = dict(et.case(r).regions)
    piece, other, other2 = (bytearray(regions[a]) for a in (et.PIECE, et.OTHER, et.OTHER2))
    h = r.randrange(0x100)
    for block in (other, piece, other2):
        h += r.choice([1, 1, 2, 5])
        put(block, 0, 'I', h)
    put(other, 0x64, 'I', et.PIECE)
    nxt = r.choice([et.OTHER2, et.OTHER2, 0])
    put(piece, 0x64, 'I', nxt)
    put(piece, 0x68, 'I', r.choice([et.OTHER] * 18 + ([et.OTHER2] if nxt else []) + [0]))
    put(other, 8, 'I', struct.unpack_from('<I', other, 8)[0] & ~0x10000 | (r.choice([0, 0x10000]) if nxt else 0))
    put(other, 0x100, 'I', et.DEF)
    put(other2, 0x100, 'I', r.choice([et.DEF] * 9 + [0]))
    put(other2, 0x64, 'I', 0)
    put(other2, 0x68, 'I', et.PIECE)
    glob = bytearray(regions[0x79c600])
    put(glob, 0x28, 'I', r.choice([0, 0, et.OTHER, et.PIECE] + ([et.OTHER2] if nxt else [])))
    regions.update({et.PIECE: piece, et.OTHER: other, et.OTHER2: other2, 0x79c600: glob, 0x790fec: struct.pack('<I', et.GAME)})
    wx, wz = (r.choice([0x80, 0x80, 0x40, 1, 0, r.randrange(0x100)]) for _ in range(2))
    definition = bytearray(regions[et.DEF])
    cx = r.choice([r.randrange(max(wx, 1)), r.randrange(max(wx, 1)), -1, wx, wx + 3, r.getrandbits(32)])
    cz = r.choice([r.randrange(max(wz, 1)), r.randrange(max(wz, 1)), -1, wz, wz + 3, r.getrandbits(32)])
    put(definition, 0xc, 'II', cx & 0xffffffff, cz & 0xffffffff)
    regions[et.DEF] = definition
    x, z = max(s32(cx), 0), max(s32(cz), 0)
    x, z = x if x < wx else wx - 1, z if z < wz else wz - 1
    world = struct.unpack('<I', regions[0x7cf83c])[0]
    regions[world + 0x294 + ident(x, z) * 0x44] = put(bytearray(r.bytes(0x44)), 0, 'H', r.randrange(8))
    regions.update({0x7a0314: struct.pack('<I', wx), 0x7a0374: struct.pack('<I', wz), 0x8023a0: struct.pack('<I', CELLS),
                    CELLS + 0x10ffd8: r.bytes(0x28 * 8), 0x763a00: r.bytes(8), 0x818c50: r.bytes(0x10), LOG: log_region(),
                    0x7925d0: r.bytes(4), 0x7a01bc: struct.pack('<i', r.choice([0, 0, 1, -1, r.randrange(-8, 9)])),
                    0x7a029c: struct.pack('<i', r.choice([0, 0, 1, -1, r.randrange(-8, 9)]))})
    for g in (0x791cf8, 0x79104c, 0x79221c, 0x790fd8):
        regions[g] = r.bytes(4)
    game = bytearray(regions[et.GAME]) + bytearray(r.bytes(0x40))
    game[0x3c] = r.choice([0, 2, 3, 0xfd, r.getrandbits(8)])
    put(game, 0x58, 'II', et.PIECE, et.PIECE)
    put(game, 0x60, 'I', et.PIECE)
    put(game, 4, 'I', FALL)
    regions[et.GAME] = game
    fall = bytearray(regions.pop(et.FALL)) + bytearray(r.bytes(0x170))
    for o in (0x148, 0x268):
        put(fall, o, 'I', r.choice([0, 1, 4, 9, 16, 100, r.randrange(400), r.getrandbits(32)]))
    regions[FALL] = fall
    return Call(0x446d50, [et.PIECE], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
