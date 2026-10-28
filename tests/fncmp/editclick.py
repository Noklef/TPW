# The coaster editor's click (src/game/coaster/edittools: CoasterEdit_Click 0x446300) against the original. Its tools are recorders (cdecl,
# as the driver's EditToolsEnv has them): CoasterEdit_ToggleAdjustF8 0x441cd0 (22 of 0), CoasterEdit_ToggleAdjustAngles 0x442010 (23 of 0),
# CoasterEdit_ToggleDrag 0x441a00 (24 of 0), CoasterEdit_ToggleAdjustE0 0x441e80 (25 of 0), CoasterEdit_MoveCarBack 0x4423f0 (26 of 0),
# CoasterEdit_MoveCarForward 0x4425c0 (27 of 0), CoasterEdit_DeleteSpan 0x446740 (28 of 0), CoasterEdit_SelectPiece 0x446d50 (22 of 1);
# Highlight_Current and CoasterLink_FindCarOnRide run. The world: 0x785a2c set now and then; the mode 0x79c624's low byte one of the seven
# tools mostly (else none or any); 0x79c658 flags 1 / 2 / 4 either way; 0x7a1c7c set or not; the current holder 0x7a1ce0 one of the rides
# or not; the link (0x790fec, none now and then) with +0x40 / +0x54 set or not and up to four trains on +0x4c (+0x28 next, +4 flag 1 or
# not, cars +0x30 / +0x34) of cars (+0x70 a ride of eight, +8 flag 0x40 or not, +0x100 a definition whose +4 byte has flag 2 or not);
# 0x79c640 a car or none; the press 1, 0 or any.
# usage: editclick.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

HOOKS = {0x441cd0: _recorder(22, 0, cdecl=True), 0x442010: _recorder(23, 0, cdecl=True), 0x441a00: _recorder(24, 0, cdecl=True),
         0x441e80: _recorder(25, 0, cdecl=True), 0x4423f0: _recorder(26, 0, cdecl=True), 0x4425c0: _recorder(27, 0, cdecl=True),
         0x446740: _recorder(28, 0, cdecl=True), 0x446d50: _recorder(22, 1, cdecl=True)}
RANGES = [(0x446300, 0x446696), (0x443ee0, 0x443f31), (0x45d620, 0x45d626)]
B = Bench(RANGES, hooks=HOOKS)
LINK, TRAINS, CARS, DEFS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000
NCARS = 12


def case(r):
    rides = [r.choice([1, 2, 3, r.getrandbits(32)]) for _ in range(8)]
    link = bytearray(r.bytes(0x60))
    put(link, 0x40, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    put(link, 0x54, 'I', r.choice([1, 1, 1, 0, r.getrandbits(32)]))
    trains = bytearray(r.bytes(4 * 0x60))
    nt = r.choice([1, 2, 2, 3, 4, 0])
    put(link, 0x4c, 'I', TRAINS if nt else 0)
    for k in range(nt):
        put(trains, k * 0x60 + 0x28, 'I', TRAINS + (k + 1) * 0x60 if k + 1 < nt else 0)
        put(trains, k * 0x60 + 4, 'B', r.choice([0, 0, 0, 1, 2, r.getrandbits(8)]))
        nc = r.randrange(0, 5)
        put(trains, k * 0x60 + 0x30, 'I', nc)
        for j in range(nc):
            put(trains, k * 0x60 + 0x34 + 4 * j, 'I', CARS + r.randrange(NCARS) * 0x110)
    cars = bytearray(r.bytes(NCARS * 0x110))
    for k in range(NCARS):
        o = 0x110 * k
        put(cars, o + 8, 'I', r.getrandbits(32) & ~0x40 | r.choice([0, 0x40]))
        put(cars, o + 0x70, 'I', r.randrange(8))
        put(cars, o + 0x100, 'I', DEFS + 0x10 * r.randrange(4))
    defs = bytearray(r.bytes(0x40))
    for k in range(4):
        defs[0x10 * k + 4] = r.getrandbits(8) & ~2 | r.choice([0, 0, 0, 2])
    mode = r.choice([2, 4, 8, 0x10, 0x20, 0x40, 0x80] * 3 + [0, 1, 3, 0x81, r.getrandbits(8)])
    glob = bytearray(r.bytes(0x60))
    put(glob, 0x24, 'I', r.getrandbits(32) & ~0xff | mode)
    put(glob, 0x40, 'I', r.choice([CARS + r.randrange(NCARS) * 0x110] * 3 + [0]))
    put(glob, 0x58, 'I', r.getrandbits(32) & ~7 | r.getrandbits(3))
    regions = {LOG: log_region(), LINK: link, TRAINS: trains, CARS: cars, DEFS: defs, 0x79c600: glob,
               0x7a4610: struct.pack('<8I', *rides),
               0x790fec: struct.pack('<I', LINK if r.random() < 0.95 else 0),
               0x785a2c: struct.pack('<I', r.choice([0] * 9 + [1])),
               0x7a1c7c: struct.pack('<I', r.choice([0, 0, 0, 1])),
               0x7a1ce0: struct.pack('<I', r.choice(rides[:4] * 3 + [0, r.getrandbits(32)]))}
    return Call(0x446300, [r.choice([1, 1, 0, 0, r.getrandbits(32)])], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
