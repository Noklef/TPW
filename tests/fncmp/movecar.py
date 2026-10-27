# The coaster editor's move-car tools (src/game/coaster/edittools: CoasterEdit_MoveCarBack 0x4423f0, CoasterEdit_MoveCarForward 0x4425c0)
# against the originals, with Train_SwapCars and what it calls, the action recorder's word writer (0x403d00) and Highlight_SetShown. The
# trains, rides and type are insertcar.py's (two cars or more), the game [0x790fec] a block with +0x40 set or not; the selected car
# [0x79c640] any of them (now and then none, with the tool off); the editor's flags 0x79c658 with no tool half the time, else the tool's
# bit, the other's or any of bits 0-5; the train's flag 2 (+4) now and then; the recorder recording or not (its first stream a record
# stream, as actionrec.py's writer), and the highlight's holder 0x7a1ce0 or none.
# usage: movecar.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, STUBS, Bench, Call, _recorder, put  # noqa: E402
import insertcar  # noqa: E402

B = Bench([(0x4423f0, 0x4424d8), (0x4425c0, 0x44269b), (0x403400, 0x403fc6), (0x45d630, 0x45d660), (0x4424e0, 0x4425bb), (0x432d50, 0x432df0),
           (0x434650, 0x434740), (0x5da3c0, 0x5da3c1), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0), (0x469940, 0x46a560),
           (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
GAME, VT, BUF = RAM + 0x4000, RAM + 0x4240, RAM + 0x4260
OPS = {'Back': (0x4423f0, 8), 'Forward': (0x4425c0, 0x10)}


def case(r):
    op = r.deal(PICK)
    address, bit = OPS[op]
    while True:   # (insertcar.py's world, with two cars or more)
        regions = insertcar.case(r).regions
        n = struct.unpack_from('<I', regions[insertcar.TRAIN], 0x30)[0]
        if n >= 2:
            break
    train = regions[insertcar.TRAIN]
    train[4] = train[4] & ~2 | (2 if r.random() < 0.15 else 0)
    game = bytearray(r.bytes(0x60))
    put(game, 0x40, 'I', 0 if r.random() < 0.1 else r.getrandbits(32) | 1)
    low = r.choice([0, 0, 0, 0, bit, bit, 0x18, r.getrandbits(6)])
    flags = r.getrandbits(32) & ~0x3f | low
    car = insertcar.CARS + insertcar.CAR * r.randrange(n)
    if not low & bit and r.random() < 0.05:
        car = 0
    rec = bytearray(r.bytes(0x30))
    put(rec, 0, 'II', r.choice([0, 1, 1, 1]), r.choice([0, 0, 1, 2]))
    put(rec, 0xc, 'III', VT, BUF, r.choice([0, 1, 2, 3, 4, 0x20, 0x20]))
    put(rec, 0x24, 'I', r.choice([0, 0, 0, 1]))
    vt = bytearray(r.bytes(0x10))
    put(vt, 0xc, 'I', STUBS + 0x10)
    regions.update({GAME: game, VT: vt, BUF: r.bytes(0x20), 0x790fec: GAME.to_bytes(4, 'little'),
                    0x785a08: bytes(rec), 0x79c640: car.to_bytes(4, 'little'), 0x79c658: flags.to_bytes(4, 'little'),
                    0x7a1ce0: r.choice([bytes(4), r.bytes(4)]), 0x7a1c7c: r.bytes(4)})
    return Call(address, [], regions)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
