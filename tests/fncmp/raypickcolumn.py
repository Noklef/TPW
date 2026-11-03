# A ray against what stands on a map cell (src/game/terrain/raypick: RayPick_Column 0x45cdc0) against the original: Terrain_SurfaceHeight (k 1, the height answered and the slot filled), Ride_Train
# (k 2, a train or none, the count filled), Train_CarValue (k 3, ride ids in turn), Model_UpdateNormals (k 4) and RayPick_Node (k 5, answers in turn) recorders; Matrix_Identity and the CRT as they
# are. The cell out of the map either way; no slot; a ride with or without a train; its model's flags for the normals; the ray's way along x (a set) or z, stepped or not (the step above zero), the
# span round the height either way and against the model's +0x84.
# usage: raypickcolumn.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, set_float_return, set_return, strings  # noqa: E402

B = Bench([(0x45cdc0, 0x45d08a), (0x46f150, 0x46f190), (0x67a400, 0x690000)],
          hooks={0x452ae0: _recorder(1, 3, cdecl=True), 0x447790: _recorder(2, 2, cdecl=True), 0x4477d0: _recorder(3, 2, cdecl=True), 0x473c30: _recorder(4, 1, cdecl=True), 0x45d090: _recorder(5, 15, cdecl=True)})
HOLDERS, TABLES = RAM, RAM + 0x1000
TRAIN = RAM + 0x3000
RIDES = 8


def fb(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def case(r):
    log = log_region()
    width, height = r.randrange(10, 200), r.randrange(10, 200)
    height_answer = r.uniform(0, 80)
    set_float_return(log, 1, height_answer)
    ride = r.deal([0, 2, 2, 2, 2, 5, 5, 5])
    # the slot from the first Terrain_SurfaceHeight, then anything (the original's own out): only the first is read
    fill(log, 1, 2, struct.pack('<4I', ride, 1, 2, 3), records=4, which=0)
    strings(log, 1, 0, out=1 << 2)
    cars = r.deal([0, 1, 2, 3, 3])
    has_train = r.random() < 0.5
    set_return(log, 2, TRAIN if has_train else 0)
    fill(log, 2, 1, struct.pack('<I', cars), which=1)
    strings(log, 2, 0, out=1 << 1)
    ids = struct.pack('<40I', *[r.randrange(1, RIDES) for _ in range(40)])
    answers(log, 3, TABLES)
    results = struct.pack('<40I', *[r.choice([0, 0, 1, 2]) for _ in range(40)])
    answers(log, 5, TABLES + 0x100)

    table = bytearray(4 * RIDES)
    holders = bytearray(r.bytes(0x100 * RIDES))
    for k in range(RIDES):
        at = HOLDERS + 0x100 * k
        put(table, 4 * k, 'I', at)
        put(holders, 0x100 * k + 8, 'I', at + 0x10)
        put(holders, 0x100 * k + 0x14, 'I', at + 0x20)
        put(holders, 0x100 * k + 0x20 + 0x30, 'I', r.getrandbits(32) & ~0x60000 | r.deal([0, 0x20000, 0x40000, 0x60000]))
        put(holders, 0x100 * k + 0x20 + 0x78, 'I', RAM + 0x4000 + k)
        put(holders, 0x100 * k + 0x20 + 0x84, 'f', height_answer + r.uniform(-20, 20))

    span = r.choice([0.5, r.uniform(1, 30)])
    regions = {LOG: log, 0x7a0314: struct.pack('<i', width), 0x7a0374: struct.pack('<i', height), 0x7a4610: table, HOLDERS: holders, TABLES: ids, TABLES + 0x100: results, TRAIN: r.bytes(0x10),
               0x7a1cd8: r.bytes(4), 0x7a1cb0: r.bytes(4), 0x7a1cc8: r.bytes(12), 0x7a1c80: r.bytes(12), 0x7a1cb8: r.bytes(12), 0x7a1ca0: r.bytes(12)}
    x = r.choice([r.randrange(-5, width + 5), r.randrange(width)])
    y = r.choice([r.randrange(-5, height + 5), r.randrange(height)])
    args = [x & 0xffffffff, y & 0xffffffff, r.choice([0, 1, r.getrandbits(32)]), fb(r.uniform(0, 2000)), fb(height_answer + r.uniform(-25, 25)), fb(r.uniform(0, 2000)), fb(r.choice([0.0, span, -span])),
            r.getrandbits(32), r.getrandbits(32), fb(r.choice([10.0, -10.0, 0.0]))]
    return Call(0x45cdc0, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
