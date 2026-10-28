# The coaster editor's selected piece put down (src/game/coaster/edittools: CoasterEdit_PlaceSelected 0x445a70) against the original. Its
# calls through the driver's EditToolsEnv are cdecl recorders: TrackSelection_ClearFlags 0x444040 (29 of 0), CoasterLinks_FindTile 0x434000
# (30 of 3: a tile, now and then none), Coaster_DropCar 0x442f40 (31 of 9), PathDrag_Finish 0x52c1c0 (18 of 0), Map_SetRideCell 0x452d30
# (19 of 5), Car_TravelCost 0x447830 (20 of 7: a float in st0), Coaster_FlashCar 0x441840 (21 of 2), the tools CoasterEdit_ToggleDrag
# 0x441a00 (24 of 0), _ToggleAdjustE0 0x441e80 (25 of 0), _ToggleAdjust 0x4421e0 (17 of 0), _MoveCarBack 0x4423f0 (26 of 0),
# _MoveCarForward 0x4425c0 (27 of 0), TrackSelection_End 0x445fb0 (16 of 0) and _Clear 0x445f30 (15 of 0). Flags_SetChanged,
# Highlight_SetShown, Fn_52c2a0, Float_RoundUp, __ftol and the money (Money_Credit / _Debit of the world's record, Source_Field0c) run.
# The editor's globals (0x79c600 on) random but for the mode 0x79c624 (2 mostly), the selection 0x79c6a4 (a piece, none now and then),
# the old cell 0x79c6d8 / 0x79c6a0 (the new one now and then) and the kept costs 0x79c698 / 0x79c64c (floats); 0x785a2c set or not; the
# game 0x790fec's +0x3c bit 0 either way; the tile's +0x18 / +0x14 the piece's definition's or not, its +4 flag 2 either way.
# usage: placeselected.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_after, log_region, put, set_float_return, set_return, strings  # noqa: E402

HOOKS = {0x444040: _recorder(29, 0, cdecl=True), 0x434000: _recorder(30, 3, cdecl=True), 0x442f40: _recorder(31, 9, cdecl=True),
         0x52c1c0: _recorder(18, 0, cdecl=True), 0x452d30: _recorder(19, 5, cdecl=True), 0x447830: _recorder(20, 7, cdecl=True),
         0x441840: _recorder(21, 2, cdecl=True), 0x441a00: _recorder(24, 0, cdecl=True), 0x441e80: _recorder(25, 0, cdecl=True),
         0x4421e0: _recorder(17, 0, cdecl=True), 0x4423f0: _recorder(26, 0, cdecl=True), 0x4425c0: _recorder(27, 0, cdecl=True),
         0x445fb0: _recorder(16, 0, cdecl=True), 0x445f30: _recorder(15, 0, cdecl=True)}
RANGES = [(0x445a70, 0x445eae), (0x434650, 0x434740), (0x5da3c0, 0x5da3d0), (0x45d620, 0x45d660), (0x52c2a0, 0x52c2a8),
          (0x43e160, 0x43e1c0), (0x4d0190, 0x4d0270), (0x519510, 0x519560), (0x5195d0, 0x519620), (0x6ad810, 0x6ad830)]
B = Bench(RANGES, hooks=HOOKS)
GAME, PIECE, DEF, TILE, CAR, WORLD, MONEY, PARK = (RAM + k * 0x1000 for k in range(8))


def case(r):
    log = log_region()
    strings(log, 31, 0, out=0b1100000)        # (Coaster_DropCar's outX / outY: stack addresses)
    strings(log, 20, 0, out=0b1111000)        # (Car_TravelCost's positions: the same)
    set_float_return(log, 20, r.choice([0.0, 0.5, -1.5, r.uniform(-1e4, 1e4), r.uniform(-50, 50)]))
    if r.random() < 0.3:
        fail_after(log, 29, 0, 0)             # (TrackSelection_ClearFlags: nothing cleared)
    tile_kind = r.random()
    if tile_kind < 0.25:
        fail_after(log, 30, 0, 0)             # (no tile)
    else:
        set_return(log, 30, TILE)
    glob = bytearray(r.bytes(0x100))
    put(glob, 0x24, 'I', r.choice([2, 2, 2, 2, 0, 1, 3, r.getrandbits(32)]))
    put(glob, 0xa4, 'I', PIECE if r.random() < 0.9 else 0)
    put(glob, 0x58, 'I', r.getrandbits(32) & ~0x3f | r.choice([0, 1, 2, 4, 8, 0x10, 0x20, 3, 0x3f, r.getrandbits(6)]))
    put(glob, 0x98, 'f', r.uniform(-1e4, 1e4))
    put(glob, 0x4c, 'f', r.uniform(-1e4, 1e4))
    old = r.randrange(-3, 130), r.randrange(-3, 130)
    put(glob, 0xd8, 'i', old[0])
    put(glob, 0xa0, 'i', old[1])
    x, y = old if r.random() < 0.25 else (r.choice([old[0], r.randrange(-3, 130)]), r.choice([old[1], r.randrange(-3, 130)]))
    piece = bytearray(r.bytes(0x110))
    put(piece, 0x100, 'I', DEF)
    put(piece, 0x64, 'I', PIECE + 0x80)
    df = bytearray(r.bytes(0x40))
    tile = bytearray(r.bytes(0x40))
    if r.random() < 0.5:
        tile[0x14:0x1c] = df[0x14:0x1c]
    elif r.random() < 0.5:
        tile[0x18:0x1c] = df[0x18:0x1c]
    put(tile, 0x34, 'I', CAR)
    game = bytearray(r.bytes(0x60))
    world = bytearray(r.bytes(0x40))
    put(world, 0x20, 'H', 2)
    put(world, 0x26, 'H', 1)
    slots = bytearray(r.bytes(20 * 4))
    put(slots, 20 * 1, 'I', MONEY)
    put(slots, 20 * 2, 'I', PARK)
    money = bytearray(r.bytes(0x200))
    put(money, 0x114, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions = {LOG: log, 0x79c600: glob, PIECE: piece, DEF: df, TILE: tile, CAR: r.bytes(0x80), GAME: game,
               0x790fec: struct.pack('<I', GAME), 0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1, r.getrandbits(32)])),
               0x7925d0: r.bytes(4), 0x7a1c70: r.bytes(0x80), WORLD + 0x1da700: world, 0x7cfb90: slots, MONEY: money,
               PARK + 0x1f500: r.bytes(0x800), 0x80239c: struct.pack('<I', WORLD), 0x7cf83c: struct.pack('<I', WORLD)}
    return Call(0x445a70, [x & 0xffffffff, y & 0xffffffff], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
