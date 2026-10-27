# A coaster's pending piece placed (src/game/coaster/train: Coaster_PlacePendingPiece 0x435d50) against the original, with all it
# calls: CoasterLinks_FindKey, CoasterPiece_CanPlaceAt (and the CRT), Track_AppendPiece, Train_PlaceCarAtBrace (bracecar.py's world:
# the coaster [0x790fec] with its cars and train, the type, the grid on [0x7a0854], 0x790afc set), TrackPieces_Renumber, Car_TravelCost
# and Global81af4c_Add. The point 0x7a1c70 / 0x7a1c78 over the grid's 24 x 18 cells (now and then past them; the tiles' taken bits
# any), the track's last piece (+0x24, a car of the train) within 300 of it mostly; the links (0x790fe0: the coaster, its +0x4c chain)
# the train (its +4 bits 0 / 2 clear mostly, its key +8 the cell mostly, its +0x14 the coaster or not, the type's +0xcc 1-7) or none.
# The track's flags 1 / 2, its +0x28 piece any car (in the train or not), its +0x1c none or a car; 0x79c658 flag 0x40 any (0x100 never:
# the editor is not moving a piece); without a link the track's +0x2c block an empty pylon put before its +0x14 one (with a +0x28 one
# or none); 0x785a2c set now and then.
# usage: placepiece.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import bracecar  # noqa: E402
import insertcar  # noqa: E402
import lengthfromnode  # noqa: E402
import swapcars  # noqa: E402

B = Bench(sorted(set(bracecar.RANGES + [(0x435d50, 0x436150), (0x436150, 0x436190), (0x436190, 0x436230), (0x444700, 0x444740),
                                         (0x434740, 0x434f08), (0x67a400, 0x690000), (0x447830, 0x4479a0), (0x52fba0, 0x52fbb3)])),
          hooks={0x473c70: _recorder(27, 3, cdecl=True)})
HEIGHTS, TAKEN, TK, PYLON, HEAD, HEAD2, DUMMY = RAM + 0x8000, RAM + 0x9000, RAM + 0x9800, RAM + 0x9840, RAM + 0x98c0, RAM + 0x9900, RAM + 0x9940
BASE = RAM + 0xa000   # the coaster's +4: its two types at +0xa8 and +0x1c8 (0x120 bytes each)
GW, GH = 24, 18


def case(r):
    c = bracecar.case(r)
    regions, piece = c.regions, c.args[1]
    train, carbuf, coaster = regions[insertcar.TRAIN], regions[insertcar.CARS], regions[swapcars.COASTER]
    n = struct.unpack_from('<I', train, 0x30)[0]
    cars = [insertcar.CARS + insertcar.CAR * k for k in range(n)]
    others = [insertcar.CARS + insertcar.CAR * k for k in range(6) if insertcar.CARS + insertcar.CAR * k != piece]
    grid = regions[bracecar.GRID]
    put(grid, 0x1c, 'I', GH)
    put(grid, 0x28, 'I', HEIGHTS)
    heights = bytearray(4 * (GW + 1) * (GH + 8))
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', r.uniform(-50, 50) if r.random() < 0.98 else r.choice([float('nan'), float('inf')]))
    taken = bytearray(4 * GW * GH)
    for i in range(GW * GH):
        put(taken, 4 * i, 'I', r.getrandbits(32) & ~1 | (1 if r.random() < 0.3 else 0))
    off = r.random() < 0.08
    x = r.uniform(0, 10 * (GW + 4) if off else 10 * GW)
    z = r.uniform(0, 10 * (GH + 4) if off and r.random() < 0.5 else 10 * GH)
    gx, gz = int(x * 0.1), int(z * 0.1)
    cell = gz * GW + gx
    last = r.choice(cars)
    o = last - insertcar.CARS
    near = r.random() < 0.9
    put(carbuf, o + 0x4c, 'f', gx * 10 + (r.uniform(-200, 200) if near else r.choice([-1, 1]) * r.uniform(301, 900)))
    put(carbuf, o + 0x54, 'f', gz * 10 + (r.uniform(-200, 200) if near else r.uniform(-900, 900)))
    if r.random() < 0.03:
        put(carbuf, o + r.choice([0x4c, 0x54]), 'I', r.choice([0x7fc00000, 0x7f800001]))
    put(coaster, 0, 'I', struct.unpack_from('<I', coaster, 0)[0] & ~8)
    coaster[0x3c] = coaster[0x3c] & ~3 | r.choice([0, 1, 2, 3])
    put(coaster, 0x18, 'I', 0)
    linked = r.random() < 0.6
    put(coaster, 0x4c, 'I', r.choice([insertcar.TRAIN] * 4 + [DUMMY]) if linked else r.choice([0, DUMMY]))
    put(coaster, 0x3c + 0x14, 'I', HEAD if r.random() < 0.97 else 0)
    put(coaster, 0x3c + 0x1c, 'I', r.choice([0, 0, r.choice(others)]))
    put(coaster, 0x3c + 0x24, 'I', last)
    put(coaster, 0x3c + 0x28, 'I', r.choice(cars + others))
    put(coaster, 0x3c + 0x2c, 'I', PYLON)
    put(coaster, 0x3c + 0x30, 'I', piece)
    train[4] = train[4] & ~5 | (r.choice([1, 4]) if r.random() < 0.1 else 0)
    put(train, 8, 'I', cell if r.random() < 0.9 else r.randrange(GW * GH))
    put(train, 0x14, 'I', swapcars.COASTER if r.random() < 0.7 else DUMMY)
    put(train, 0x28, 'I', 0)
    for k in range(6):   # (the cars' previous pieces: CoasterPiece_CanPlaceAt reads their nodes, +0x100)
        put(carbuf, insertcar.CAR * k + 0x68, 'I', insertcar.CARS + insertcar.CAR * (k - 1) if k else 0)
    tp = bytearray(regions[insertcar.TYPE][:0x120])
    put(tp, 0xcc, 'I', r.randrange(1, 8))
    base = bytearray(r.bytes(0x2e8))
    base[0xa8:0x1c8] = tp
    put(tp, 0, 'I', struct.unpack_from('<I', tp, 0)[0] ^ r.choice([0, 0x4000]))
    base[0x1c8:0x2e8] = tp
    flags = struct.unpack_from('<I', regions[0x79c658], 0)[0] & ~0x100
    rules = BASE + (0x1c8 if flags & 0x40 else 0xa8)
    put(coaster, 4, 'I', BASE)
    put(train, 0x18, 'I', rules if r.random() < 0.8 else BASE + (0xa8 if flags & 0x40 else 0x1c8))
    put(regions[lengthfromnode.NODE2], 0x18, 'I', rules)   # (Car_TravelCost's next owner type)
    pylon = bytearray(r.bytes(0x60))
    put(pylon, 0x30, 'I', 0)
    head = bytearray(r.bytes(0x40))
    put(head, 0x18, 'I', rules)
    put(head, 0x28, 'I', r.choice([HEAD2, 0]))
    dummy = put(bytearray(r.bytes(0x40)), 0x28, 'I', 0)
    dummy[4] |= 1
    regions.update({BASE: base, HEIGHTS: heights, TAKEN: taken, TK: put(bytearray(r.bytes(0x40)), 0x2c, 'I', TAKEN), PYLON: pylon, HEAD: head,
                    HEAD2: r.bytes(0x40), DUMMY: dummy, 0x79c658: struct.pack('<I', flags), 0x79fc54: struct.pack('<I', TK),
                    0x7a1c70: struct.pack('<f', x), 0x7a1c74: r.bytes(4), 0x7a1c78: struct.pack('<f', z),
                    0x790fe0: struct.pack('<I', swapcars.COASTER), 0x790fd4: r.bytes(4), 0x785a2c: struct.pack('<I', 0 if r.random() < 0.9 else 1),
                    0x81af4c: r.bytes(4)})
    return Call(0x435d50, [swapcars.COASTER], regions)


if __name__ == '__main__':
    B.main(case)
