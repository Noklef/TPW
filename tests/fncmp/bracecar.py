# A car put into a pylon at a brace (src/game/coaster/train: Train_PlaceCarAtBrace 0x436230) against the original, with all it calls:
# Car_LengthFromNode (lengthfromnode.py's world: swapcars.py's coaster [0x790fec] and insertcar.py's train, cars and type; the train is
# the node, its +0x2c the next node), Pylon_FindBraceLevel, Train_InsertCar and Terrain_FootprintHeight (footprint.py's grid on the
# world [0x7a0854]; 0x790afc set so that Train_PlaceCars, whose Model_FitToGrid would fit the cars to that grid, does nothing). The car
# one not in the train (its +0x68 piece's +0 an index along the coaster); the coaster's +0x3c flag 2 picks +0x64 (a car, or another
# block on the train or not) over +0x54 as the other car, each with an index (+0x74 ->) into its frames (+0x90 -> +4, 0x58 apart;
# +0x44 now and then a signalling NaN); the coaster's +4 the type less 0xa8 / 0x1c8 (as 0x79c658 flag 0x40 picks) mostly; its +0x5c
# a car or none; 0x79c664 2 or not; the train's position (+0x1c, +0x24) over the grid's cells mostly; save or not.
# usage: bracecar.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import insertcar  # noqa: E402
import lengthfromnode  # noqa: E402
import swapcars  # noqa: E402

B = Bench(sorted(set([(0x436230, 0x43648c), (0x433300, 0x433790), (0x433bd0, 0x433f64), (0x433280, 0x433300), (0x433790, 0x4338e0),
                      (0x433a90, 0x433bd0), (0x454050, 0x4541a0), (0x466d70, 0x466fa0), (0x469930, 0x469950), (0x435340, 0x4354fe),
                      (0x470940, 0x470990), (0x4424e0, 0x4425bb), (0x443f40, 0x444035), (0x444150, 0x44418e), (0x432d50, 0x432df0),
                      (0x434650, 0x434740), (0x43e520, 0x43e540), (0x5da3c0, 0x5da3c1), (0x432df0, 0x433220), (0x433220, 0x433240),
                      (0x430ca0, 0x430ee0), (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100),
                      (0x4702d0, 0x4704d0)])), hooks={0x473c70: _recorder(27, 3, cdecl=True)})
GRID, HEIGHTS, OTHER, IDX, CPL, NODES = RAM + 0x6000, RAM + 0x6040, RAM + 0x6800, RAM + 0x6a00, RAM + 0x6a10, RAM + 0x6a40
GW = 24


def frames(r, buf, o, regions):   # the other car's index and frames (both other cars share them)
    put(buf, o + 0x74, 'I', IDX)
    put(buf, o + 0x90, 'I', CPL)
    regions[IDX] = struct.pack('<I', r.randrange(4))
    regions[CPL] = put(bytearray(r.bytes(0x10)), 4, 'I', NODES)
    nodes = bytearray(r.bytes(4 * 0x58))
    for k in range(4):
        put(nodes, 0x58 * k + 0x44, 'I', r.choice([fb(r.uniform(-5, 40))] * 8 + [r.f(), 0x7f800001, 0xff812345]))
    regions[NODES] = nodes


def case(r):
    regions = lengthfromnode.case(r).regions
    train, carbuf, coaster = regions[insertcar.TRAIN], regions[insertcar.CARS], regions[swapcars.COASTER]
    n = struct.unpack_from('<I', train, 0x30)[0]
    cars = [insertcar.CARS + insertcar.CAR * k for k in range(n)]
    k = 5 if n == 5 else r.randrange(n, 6)
    car = insertcar.CARS + insertcar.CAR * k
    put(carbuf, insertcar.CAR * k + 0x68, 'I', insertcar.PIECES + 0x10 * k if r.random() < 0.95 else 0)
    pieces = bytearray(regions[insertcar.PIECES])
    for j in range(6):
        put(pieces, 0x10 * j, 'I', r.randrange(n))
    regions[insertcar.PIECES] = pieces
    put(train, 0x2c, 'I', lengthfromnode.NODE2)
    for o in (0x1c, 0x24):
        put(train, o, 'I', fb(r.uniform(-9, 105)) if r.random() < 0.97 else r.choice([r.f(), 0x7fc00000]))
    flags = r.getrandbits(32) & ~0x40 | r.choice([0, 0x40])
    tp = insertcar.TYPE - (0x1c8 if flags & 0x40 else 0xa8)
    put(coaster, 4, 'I', tp if r.random() < 0.9 else insertcar.TYPE - (0xa8 if flags & 0x40 else 0x1c8))
    coaster[0x3c] = coaster[0x3c] & ~2 | r.choice([0, 2])
    put(coaster, 0x5c, 'I', r.choice(cars + [0]))
    for j in range(6):   # (cars[0] the coaster's +0x54)
        frames(r, carbuf, insertcar.CAR * j, regions)
    block = bytearray(r.bytes(0x110))
    put(block, 0x100, 'I', r.choice([insertcar.TRAIN, insertcar.TRAIN, 0, insertcar.TYPE]))
    frames(r, block, 0, regions)
    regions[OTHER] = block
    put(coaster, 0x64, 'I', r.choice([OTHER, OTHER, r.choice(cars)]))
    world = regions[insertcar.WORLD]
    put(world, 0x6c, 'I', GRID)
    grid = put(bytearray(r.bytes(0x30)), 0x18, 'I', GW)
    put(grid, 0x28, 'I', HEIGHTS)
    heights = bytearray(4 * (GW + 1) * 20)
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', r.uniform(-50, 50) if r.random() < 0.98 else r.choice([float('nan'), float('inf')]))
    regions.update({GRID: grid, HEIGHTS: heights, 0x790afc: struct.pack('<I', r.getrandbits(32) | 1), 0x79c658: struct.pack('<I', flags),
                    0x79c664: struct.pack('<I', r.choice([2, 2, 0, 1, r.getrandbits(32)])), 0x790ff0: r.bytes(0x60),
                    0x791cd8: r.bytes(0x28 + 0x104 * 6)})
    save = r.choice([0, 1, 1, r.getrandbits(32)])
    return Call(0x436230, [swapcars.COASTER, car, insertcar.TRAIN, r.f(), save], regions)


if __name__ == '__main__':
    B.main(case)
