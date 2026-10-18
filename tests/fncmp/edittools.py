# The coaster editor's adjust tool toggles (src/game/coaster/edittools: 0x441cd0, 0x441e80, 0x442010) against the originals: the editor's
# flags (no tool, tool 2 or 4, another, with 0x200 / 0x1e00 / 0x2000 or random high bits), a piece being edited or none (its definition's
# flag 2 or not, its type's flags 0x40 / 0x80 / 0x100 / 0x200 / 0x180 or others, values about the thresholds, odd floats), the game's +0x40
# set or not, the action recorder at 0x785a08 recording into a record stream or not, a current highlight or none.
# usage: edittools.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=F8,E0,Angles)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, STUBS, Bench, Call, put  # noqa: E402

CALLS = {'F8': 0x441cd0, 'E0': 0x441e80, 'Angles': 0x442010}
B = Bench([(0x441cd0, 0x441e78), (0x441e80, 0x442002), (0x442010, 0x4421d6), (0x403400, 0x403fc6), (0x45d630, 0x45d660), (0x5da3c0, 0x5da3d0)])
PIECE, DEF, TYPE, GAME, VT, BUF = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600
TOOLS = [0, 0, 0, 2, 2, 4, 4, 4, 1, 6, 0x20, 0x3f]
TYPES = [0x40, 0x80, 0x100, 0x180, 0x200, 0x240, 0x380, 0]


def value(r):
    return struct.unpack('<f', struct.pack('<I', r.getrandbits(32)))[0] if r.random() < 0.1 else \
        r.choice([0.5, 0.75, 1.5, 0.25, -0.3, 0.0, 3.0, 1e-38, -1e-38, 5e-39, 3e38, r.uniform(-4, 4), r.uniform(-4, 4)])


def case(r):
    op = r.deal(OPS)
    flags = r.choice(TOOLS) | r.choice([0, 0, 0x200, 0x1e00, 0x2000, 0x400]) | (r.getrandbits(32) & 0xffffc000 if r.random() < 0.2 else 0)
    glob = bytearray(r.bytes(0x110))
    put(glob, 0x40, 'I', PIECE if r.random() < 0.85 else 0)
    put(glob, 0x58, 'I', flags)
    piece = bytearray(r.bytes(0x110))
    put(piece, 8, 'I', r.getrandbits(32) & ~0x80 | r.choice([0, 0x80]))
    for off in (0xd4, 0xd8, 0xdc, 0xe0, 0xf8):
        put(piece, off, 'f', value(r))
    put(piece, 0x100, 'I', DEF)
    definition = bytearray(r.bytes(0x20))
    put(definition, 4, 'B', r.getrandbits(8) & ~2 | (2 if r.random() < 0.2 else 0))
    put(definition, 0x18, 'I', TYPE)
    kind = bytearray(r.bytes(0x98))
    put(kind, 0, 'I', r.choice(TYPES) | (r.getrandbits(32) & ~0x3c0 if r.random() < 0.2 else 0))
    put(kind, 0x94, 'f', value(r))
    game = bytearray(r.bytes(0x44))
    put(game, 0x40, 'I', r.choice([0, 1, 1, r.getrandbits(32)]))
    rec = bytearray(r.bytes(0x28))   # (recording, mode, playback; its first stream a record stream at +0xc)
    put(rec, 0, 'I', r.choice([0, 1, 1, 1, r.getrandbits(32)]))
    put(rec, 0x24, 'I', r.choice([0, 0, 0, 1]))
    put(rec, 0xc, '3I', VT, BUF, r.choice([0, 4, 9, 0x20, 0x20]))
    vt = bytearray(0x10)
    put(vt, 0xc, 'I', STUBS + 0x10)
    regions = {0x79c600: glob, PIECE: piece, DEF: definition, TYPE: kind, GAME: game, 0x785a08: rec, VT: vt, BUF: r.bytes(0x20),
               0x790fec: struct.pack('<I', GAME if r.random() < 0.95 else 0), 0x790ac8: r.bytes(8),
               0x7a1ce0: struct.pack('<I', r.choice([0, RAM + 0x700])), 0x7a1c7c: r.bytes(4)}
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
