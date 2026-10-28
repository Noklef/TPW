# The coaster editor's moved piece put back (src/game/coaster/edittools: CoasterEdit_CancelSelected 0x446050) against the original. Its
# calls through the driver's EditToolsEnv are cdecl recorders: Coaster_DropCar 0x442f40 (31 of 9), PathDrag_Finish 0x52c1c0 (18 of 0),
# Coaster_FlashCar 0x441840 (21 of 2), the tools CoasterEdit_ToggleDrag 0x441a00 (24 of 0), _ToggleAdjustE0 0x441e80 (25 of 0),
# _ToggleAdjustF8 0x441cd0 (22 of 0), _ToggleAdjustAngles 0x442010 (23 of 0), _MoveCarBack 0x4423f0 (26 of 0) and _MoveCarForward 0x4425c0
# (27 of 0); Flags_SetChanged, Highlight_SetShown and the inlined TrackSelection_Clear run. The editor's globals (0x79c600 on) random but
# for the flags 0x79c658 (0x100 mostly, a tool in bits 0-5 or none, 0x80 either way), the selection's ends 0x79c628 / 0x79c62c (on a chain
# of pieces with rising handles (+0), by +0x64, in either order; the first none now and then), the moved piece 0x79c6a4 with its definition
# at 0x79c660 or not, the selected car 0x79c640 (none now and then; +8 flag 0x40, its definition's +4 flag 2, either way) and the mode
# 0x79c624 bit 0; 0x785a2c set or not; the game 0x790fec (none now and then) with +0x3c bits 0 / 1 and +0x40 either way.
# usage: cancelselected.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

HOOKS = {0x442f40: _recorder(31, 9, cdecl=True), 0x52c1c0: _recorder(18, 0, cdecl=True), 0x441840: _recorder(21, 2, cdecl=True),
         0x441a00: _recorder(24, 0, cdecl=True), 0x441e80: _recorder(25, 0, cdecl=True), 0x441cd0: _recorder(22, 0, cdecl=True),
         0x442010: _recorder(23, 0, cdecl=True), 0x4423f0: _recorder(26, 0, cdecl=True), 0x4425c0: _recorder(27, 0, cdecl=True)}
RANGES = [(0x446050, 0x4462c2), (0x434650, 0x434740), (0x5da3c0, 0x5da3d0), (0x45d620, 0x45d660), (0x445f30, 0x446042),
          (0x444040, 0x444090)]
B = Bench(RANGES, hooks=HOOKS)
GAME, PIECE, DEF, SEL, CAR = (RAM + k * 0x1000 for k in range(5))


def case(r):
    log = log_region()
    glob = bytearray(r.bytes(0x100))
    flags = r.getrandbits(32) & ~0x1bf | r.choice([0x100] * 4 + [0]) | r.choice([0, 0x80])
    put(glob, 0x58, 'I', flags | r.choice([0, 1, 2, 4, 4, 8, 0x10, 0x20, 0x20, 3, 0x3f, r.getrandbits(6)]))
    put(glob, 0x24, 'I', r.getrandbits(32))
    m = r.randrange(1, 7)
    sel = bytearray(r.bytes(0x70 * m))
    handle = r.randrange(0x100)
    for k in range(m):
        handle += r.randrange(1, 4)
        put(sel, 0x70 * k, 'I', handle)
        put(sel, 0x70 * k + 0x64, 'I', SEL + 0x70 * (k + 1) if k + 1 < m else 0)
    i = r.randrange(m)
    j = r.randrange(i, m)
    ends = [SEL + 0x70 * i, SEL + 0x70 * j]
    r.shuffle(ends)
    put(glob, 0x28, 'II', *(ends if r.random() < 0.85 else [0, ends[1]]))
    piece = bytearray(r.bytes(0x110))
    put(piece, 0x100, 'I', DEF)
    put(glob, 0xa4, 'I', PIECE)
    put(glob, 0x60, 'I', r.choice([DEF, DEF + 0x40, 0]))
    df = bytearray(r.bytes(0x80))
    df[4] = r.getrandbits(8) & ~2 | r.choice([0, 2])
    car = bytearray(r.bytes(0x110))
    car[8] = r.getrandbits(8) & ~0x40 | r.choice([0, 0x40])
    put(car, 0x100, 'I', DEF)
    put(glob, 0x40, 'I', CAR if r.random() < 0.85 else 0)
    game = bytearray(r.bytes(0x60))
    put(game, 0x40, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions = {LOG: log, 0x79c600: glob, PIECE: piece, DEF: df, SEL: sel, CAR: car, GAME: game,
               0x790fec: struct.pack('<I', GAME if r.random() < 0.9 else 0),
               0x785a2c: struct.pack('<I', r.choice([0, 1, 1, r.getrandbits(32)])), 0x7925d0: r.bytes(4), 0x7a1c70: r.bytes(0x80)}
    return Call(0x446050, [], regions)


if __name__ == '__main__':
    B.main(case)
