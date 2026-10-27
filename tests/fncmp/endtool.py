# The coaster editor's tool ended and its mode set (src/game/coaster/edittools: CoasterEdit_EndTool 0x442220, CoasterEdit_SetMode
# 0x444360) against the originals, with every tool they turn off. The tool in use dealt: an adjuster or the drag tool (1, 2, 4) over
# edittools.py's world (drag()'s for 1), a move tool (8, 0x10) over movecar.py's, the selection (0x20: a chain of pieces with rising
# handles, both ends of the selection on it in either order, or none; flag 0x80 or not), or none / another (0, 3, 6, 0x3f). The game
# [0x790fec] a block of its own over the world's (its first bytes copied): +0x3c bits 0 / 1, +4 a flags byte (bit 1) unless drag's,
# +0x54 / +0x58 / +0x60 / +0x64 a car of ride 3 or none (SetMode (1) then reads a none's flag: crash in both), with flag 0x80 (+8) or
# not, +0x144 none or a car of ride 4; the rides' holders flashing or not (Highlight_Flash / _Unflash on their shortest paths); the mode
# 0x79c624 bit 0 or not. SetMode's mode 0, 1, 2, any or any with low byte 1 (1 with a game). A tool on with no car selected faults
# in both (sic).
# usage: endtool.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, put  # noqa: E402
import edittools  # noqa: E402
import movecar  # noqa: E402

B = Bench([(0x442220, 0x4423ac), (0x444360, 0x444569), (0x441840, 0x4418cb), (0x4423f0, 0x4424d8), (0x4425c0, 0x44269b), (0x4424e0, 0x4425bb),
           (0x432d50, 0x432df0), (0x5da3c0, 0x5da3d0), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0), (0x469940, 0x46a560),
           (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0),
           (0x441cd0, 0x441e78), (0x441e80, 0x442002), (0x442010, 0x4421d6), (0x403400, 0x403fc6), (0x45d630, 0x45d660), (0x441a00, 0x441cc8),
           (0x433280, 0x433310), (0x434650, 0x434740), (0x43e160, 0x43e1c0), (0x447830, 0x4479a0), (0x4d0190, 0x4d0270), (0x519510, 0x519560),
           (0x5195d0, 0x519620), (0x6ad810, 0x6ad830), (0x469580, 0x46964c), (0x469650, 0x46983a)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
GAME, FLASH, KEPT, HOLD, TFLAGS, SEL = RAM + 0x5000, RAM + 0x5200, RAM + 0x5280, RAM + 0x5300, RAM + 0x5900, RAM + 0x6000
H3, H4 = HOLD, HOLD + 0x100


def case(r):
    op = r.deal(['End', 'Mode'])
    kind = r.deal(['adjust', 'adjust', 'move', 'move', 'select', 'none'])
    glob = None
    game_src = b''
    if kind == 'adjust':
        regions = edittools.case(r).regions
        tool = r.choice([1, 2, 4, 4])
        if tool == 1:
            edittools.drag(r, regions)
        glob = bytearray(regions[0x79c600])
        if regions[0x790fec] != bytes(4):
            game_src = regions[edittools.GAME]
    elif kind == 'move':
        regions = movecar.case(r).regions
        tool = r.choice([8, 0x10])
        glob = bytearray(r.bytes(0x110))
        put(glob, 0x40, 'I', struct.unpack('<I', regions.pop(0x79c640))[0])
        put(glob, 0x58, 'I', struct.unpack('<I', regions.pop(0x79c658))[0])
        game_src = regions[movecar.GAME]
    else:
        regions = {0x7a1ce0: r.choice([bytes(4), r.bytes(4)]), 0x7a1c7c: r.bytes(4), 0x7925d0: r.bytes(4)}
        tool = 0x20 if kind == 'select' else r.choice([0, 0, 3, 6, 0x3f])
        glob = bytearray(r.bytes(0x110))
        game_src = put(bytearray(r.bytes(0x44)), 0x40, 'I', r.choice([0, 1]))
        m = r.randrange(1, 7)
        sel = bytearray(r.bytes(0x70 * m))
        handle = r.randrange(0x100)
        for k in range(m):   # (a chain by +0x64, the handles (+0) rising along it)
            handle += r.randrange(1, 4)
            put(sel, 0x70 * k, 'I', handle)
            put(sel, 0x70 * k + 0x64, 'I', SEL + 0x70 * (k + 1) if k + 1 < m else 0)
        i = r.randrange(m)
        j = r.randrange(i, m)
        ends = [SEL + 0x70 * i, SEL + 0x70 * j]
        r.shuffle(ends)
        put(glob, 0x28, 'II', *(ends if r.random() < 0.85 else [0, ends[1]]))
        regions[SEL] = sel
    put(glob, 0x58, 'I', struct.unpack_from('<I', glob, 0x58)[0] & ~0x3f | tool)
    if kind == 'select':
        put(glob, 0x58, 'I', struct.unpack_from('<I', glob, 0x58)[0] & ~0x80 | r.choice([0, 0x80, 0x80]))
    glob[0x24] = r.getrandbits(8) & ~1 | r.choice([0, 1])
    regions[0x79c600] = glob
    # the game, and Coaster_FlashCar's cars
    if game_src:
        game = bytearray(game_src) + bytearray(r.bytes(0x150 - len(game_src)))
        put(game, 0x3c, 'I', r.getrandbits(32) & ~3 | r.getrandbits(2))
        if kind != 'adjust' or tool != 1:   # (drag's +4 is its fallback type's)
            put(game, 4, 'I', TFLAGS)
        for off in (0x54, 0x58, 0x60, 0x64):
            put(game, off, 'I', r.choice([FLASH, FLASH, 0]))
        put(game, 0x144, 'I', r.choice([0, KEPT]))
        regions[GAME] = game
        regions[0x790fec] = GAME.to_bytes(4, 'little')
    else:
        regions[0x790fec] = bytes(4)
    flash = put(put(bytearray(r.bytes(0x80)), 0x70, 'I', 3), 8, 'I', r.getrandbits(32) & ~0x80 | r.choice([0, 0x80]))
    kept = put(bytearray(r.bytes(0x80)), 0x70, 'I', 4)
    # the rides' holders (H3, H4): flashing (0x4000000, listed from 0x7afe14 by +0xd0) or not, never highlighted (0x20000); +8 an
    # object whose +4 a model without meshes, flags without 0x100000, its +0x5c not its base's (+0x9c): Highlight_Flash / _Unflash's
    # shortest paths
    hold = bytearray(r.bytes(0x600))
    base = HOLD + 0x500
    listed = []
    for k, h in enumerate((H3, H4)):
        o = h - HOLD
        flashing = r.random() < 0.6
        put(hold, o + 4, '2I', r.getrandbits(32) & ~0x4020000 | (0x4000000 if flashing else 0), HOLD + 0x200 + 0x40 * k)
        put(hold, 0x204 + 0x40 * k, 'I', HOLD + 0x300 + 0x100 * k)
        model = 0x300 + 0x100 * k
        put(hold, model + 0x30, 'I', r.getrandbits(32) & ~0x100000)
        put(hold, model + 0x44, 'H', 0)
        put(hold, model + 0x9c, 'I', base)
        if flashing:
            listed.append(h)
    r.shuffle(listed)
    for h, nxt in zip(listed, listed[1:] + [0]):
        put(hold, h - HOLD + 0xd0, 'I', nxt)
    for model in (0x35c, 0x45c):
        if struct.unpack_from('<I', hold, model)[0] == struct.unpack_from('<I', hold, 0x55c)[0]:
            hold[model] ^= 1
    table = bytearray(regions.get(0x7a4610, b'')) + bytearray(12 - len(regions.get(0x7a4610, b'')))
    if 0x7a4610 not in regions:
        table = bytearray(struct.pack('<3I', H4, H4, H4))
    table += struct.pack('<2I', H3, H4)
    regions.update({FLASH: flash, KEPT: kept, HOLD: hold, TFLAGS: r.bytes(4), 0x7a4610: table,
                    0x7afe14: struct.pack('<I', listed[0] if listed else 0)})
    if op == 'End':
        return Call(0x442220, [], regions)
    mode = r.choice([0, 1, 1, 1, 2, r.getrandbits(32), r.getrandbits(32) & ~0xff | 1]) if game_src else r.choice([0, 2])
    return Call(0x444360, [mode], regions)


if __name__ == '__main__':
    B.main(case)
