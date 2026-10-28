# The coasters' pending piece dropped (src/game/coaster/train: Coasters_DropPendingPiece 0x436490) against the original, with all it
# calls: Contacts_ClearPair, Track_RemovePiece, TrackPieces_Renumber, Train_RestoreSaved, Train_SetCouplings (Car_PickCoupling) and
# Train_PlaceCars (insertcar.py's train, cars, type and rides; Animator_Update the cdecl recorder k 27). The six cars are the track's
# pieces (+0 index, +0x64 next, +0x68 previous), each with up to three contacts (+0xb0, +0xb4) among them and flags 0x400000 / 0x8000
# now and then; the pending piece (the track's +0x30) one of them, mostly the track's last (+0x24), its train (+0x100) the train or none
# (a count of 1-5, now and then 0). The list 0x790fe0: the coaster (flags 1, 4, 8 any) and a second one (flag 1 any, 4 never) in
# either order; the game's coaster [0x790fec] mostly the first; 0x790fd4 any; the track's +0x2c block with flag 8 now and then and a
# next block or none. The saved train (0x790ff0, 0x791d00 on, 0x791038 on) a copy of the train and its cars with their lengths and rates changed now and then.
# usage: droppiece.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import insertcar  # noqa: E402

B = Bench([(0x436490, 0x43674a), (0x43ad50, 0x43afd0), (0x4367a0, 0x436820), (0x444700, 0x444740), (0x4338e0, 0x433a90),
           (0x432d50, 0x432df0), (0x436750, 0x4367a0), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0),
           (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)],
          hooks={0x473c70: _recorder(27, 3, cdecl=True)})
COAST, COAST2, BLOCK, BLOCK2 = RAM + 0x5000, RAM + 0x5200, RAM + 0x5400, RAM + 0x5440
CAR = insertcar.CAR


def case(r):
    while True:
        regions = insertcar.case(r).regions
        n = struct.unpack_from('<I', regions[insertcar.TRAIN], 0x30)[0]
        if n or r.random() < 0.03:
            break
    carbuf = regions[insertcar.CARS]
    cars = [insertcar.CARS + CAR * k for k in range(6)]
    for k in range(6):   # the track's pieces, in order
        o = CAR * k
        put(carbuf, o, 'I', k)
        put(carbuf, o + 0x64, 'I', cars[k + 1] if k < 5 else 0)
        put(carbuf, o + 0x68, 'I', cars[k - 1] if k else 0)
        flags = struct.unpack_from('<I', carbuf, o + 8)[0] & ~0x408000
        put(carbuf, o + 8, 'I', flags | r.choice([0, 0, 0x400000]) | r.choice([0, 0x8000, 0x8000]))
        c = r.randrange(4)
        put(carbuf, o + 0xb0, 'I', c)
        for j in range(8):
            put(carbuf, o + 0xb4 + 4 * j, 'I', r.choice(cars) if j < c else r.choice([0, r.choice(cars)]))
    p = r.choice([5, 5, r.randrange(1, 6), r.randrange(6)])
    piece = cars[p]
    if r.random() < 0.15:
        put(carbuf, CAR * p + 0x100, 'I', 0)
    coast = bytearray(r.bytes(0x200))
    put(coast, 0, 'I', r.getrandbits(32) & ~0xd | r.choice([1, 1, 1, 0]) | r.choice([4, 4, 4, 0]) | r.choice([0, 8]))
    track = 0x3c
    put(coast, track + 0x18, 'I', cars[0])
    put(coast, track + 0x30, 'I', piece)
    put(coast, track + 0x24, 'I', piece if r.random() < 0.8 else r.choice(cars))
    put(coast, track + 0x1c, 'I', r.choice([piece, r.choice(cars), 0]))
    put(coast, track + 0x28, 'I', r.choice(cars))
    put(coast, track + 0x2c, 'I', BLOCK)
    put(coast, track + 8, 'I', r.randrange(1, 7))
    put(coast, 0x140, 'I', r.randrange(30))
    block = bytearray(r.bytes(0x40))
    block[4] = block[4] & ~8 | r.choice([0, 8, 8])
    put(block, 0x2c, 'I', r.choice([BLOCK2, 0]))
    coast2 = bytearray(r.bytes(0x200))
    put(coast2, 0, 'I', r.getrandbits(32) & ~5 | r.choice([0, 1]))   # (no flag 4: its track is not built)
    first = r.choice([COAST, COAST, COAST2])
    put(coast, 0x18, 'I', COAST2 if first == COAST and r.random() < 0.7 else 0)
    put(coast2, 0x18, 'I', COAST if first == COAST2 else 0)
    train = regions[insertcar.TRAIN]
    header = bytearray(train[:0x48])
    put(header, 0x30, 'I', r.choice([n, n, r.randrange(6)]))
    saved = bytearray(r.bytes(6 * 0x104))
    for k in range(6):
        copy = bytearray(carbuf[CAR * k:CAR * k + 0x104])
        if r.random() < 0.3:
            put(copy, 0xe8, 'I', fb(r.random()))
        if r.random() < 0.2:
            put(copy, 8, 'I', struct.unpack_from('<I', copy, 8)[0] ^ r.choice([0x2000, 0x600000, 0x100, 0x40]))
        saved[0x104 * k:0x104 * (k + 1)] = copy
    regions.update({COAST: coast, COAST2: coast2, BLOCK: block, BLOCK2: r.bytes(0x40), 0x790fe0: struct.pack('<I', first),
                    0x790fec: struct.pack('<I', COAST if r.random() < 0.85 else COAST2),
                    0x790fd4: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)])), 0x790fe4: r.bytes(4), 0x792218: r.bytes(4),
                    0x790ff0: bytes(header), 0x791038: b''.join(struct.pack('<I', r.f(20)) for _ in range(6)), 0x791cd0: r.bytes(0x10),
                    0x791d00: bytes(saved)})
    return Call(0x436490, [], regions)


if __name__ == '__main__':
    B.main(case)
