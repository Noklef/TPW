# GameMode_StepBack (src/game/world/gamemode: 0x52fe50, cdecl (thing, relink)) against the original: the game mode [0x81ae2c] dealt from
# the modes it tells apart (and others), the point stack's count [0x820a8c] -1 to 4 with its points at 0x81b738 + 8 * count, the thing 0 or
# not, the track run [0x8186ac] 0 or not, relink's low byte 0 or not; the new top point's 0x44-byte cell (the world pointer [0x7cf83c] set
# so it lands in a small region) of +8 1 or not, for mode 3. Its calls are recorders (tools/fncmp/driver.cpp's GameModeStepEnv): 0x444f30
# cdecl 1 of 0, 0x4de1f0 thiscall 2 of 0, GameMode_MapClick 0x524960 cdecl 3 of 4, GameMode_Set / SetOnly cdecl 4 / 5 of 2,
# TrackCell_Relink cdecl 6 of 1, TrackRun_UpdateA / B cdecl 7 / 8 of 4 (UpdateA answering 0 or 1); Global816d48_Set runs.
# (The original pushes TrackCell_Relink's corners as a dword whose upper bytes are ECX's at its entry (PUSH ECX made the slot): TrackCell_Relink
# reads only its low byte, and the port passes 0 there; with ECX 0 at entry in both runs the two agree here.)
# usage: gamemodestep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, set_return  # noqa: E402

B = Bench([(0x52fe50, 0x5300c0), (0x527940, 0x52794a)],
          hooks={0x444f30: _recorder(1, 0, cdecl=True), 0x4de1f0: _recorder(2, 0), 0x524960: _recorder(3, 4, cdecl=True),
                 0x52f200: _recorder(4, 2, cdecl=True), 0x52f580: _recorder(5, 2, cdecl=True), 0x52fc80: _recorder(6, 1, cdecl=True),
                 0x52c6f0: _recorder(7, 4, cdecl=True), 0x52ccb0: _recorder(8, 4, cdecl=True)})
THING, RUN, CELL, WORLD = RAM, RAM + 0x100, RAM + 0x200, 0x7cf83c
MODES = [0x15, 0xe, 0x13, 4, 0x3b, 0x16, 1, 3, 3, 0x34, 0xb, 0xb, 0x10, 0x10, 0x35, 0x35, 0x19, 0x11, 2, 0, 5, 0x36, 0x40, 0xffffffff]


def case(r):
    mode = r.deal(MODES)
    count = r.choice([-1, 0, 1, 1, 2, 2, 3, 4])
    points = bytearray()
    for _ in range(6):
        points += struct.pack('<2i', r.randrange(-2, 130) if r.random() < 0.1 else r.randrange(128), r.randrange(128))
    log = log_region()
    set_return(log, 7, r.choice([0, 1, 0x100, 0xffffff01]))
    regions = {0x81ae2c: struct.pack('<I', mode), 0x820a8c: struct.pack('<i', count), 0x81b738: points, 0x81adb0: r.bytes(8),
               0x81ede4: r.bytes(8), 0x763b00: r.bytes(8), 0x816d48: r.bytes(4), LOG: log,
               0x8186ac: struct.pack('<I', RUN if r.random() < 0.7 else 0), THING: r.bytes(0x40), RUN: r.bytes(0x40)}
    # the top point after the pop (count - 1): its cell put at CELL
    top = count - 1
    x, y = struct.unpack_from('<2i', points, 8 * top) if 0 < top < 6 else (0, 0)
    cell = bytearray(r.bytes(0x44))
    struct.pack_into('<i', cell, 8, r.choice([1, 1, 0, 2, 0x101]))
    regions[CELL] = cell
    regions[WORLD] = struct.pack('<I', (CELL - 0x294 - ((((y << 7) + x + 1) & 0xffff) * 0x44)) & 0xffffffff)
    relink = r.choice([0, 1, 0x100, 0xffffff01])
    return Call(0x52fe50, [THING if r.random() < 0.6 else 0, relink], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
