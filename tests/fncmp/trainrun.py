# A coaster's trains moved on one step (src/game/coaster/trainrun: Coaster_RunTrains 0x43ce20) against the original: Motion_Time (k 1, answering the time), CoasterCar_Bump (k 2), Coaster_FrameAt
# (k 3, filling the point), RideObject_Orient (k 4, filling the offset when given one), Layer_SetSpeed (k 5), Coaster_BendMesh (k 6), CoasterCar_SetSoundMode (k 7) recorders; Vec3_Distance and the
# CRT as they are. One to three trains of one to three cars, each car on or not, its model's flags and +0x18 zero or not; the track's kinds' flag 0x800, the nodes' flag 8; speeds above 0, so
# that the run ends. Flags 0x10 / 0x20 / 0x40 / 0x80 of a train either way.
# usage: trainrun.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, set_return, strings  # noqa: E402

B = Bench([(0x43ce20, 0x43d8e8), (0x470940, 0x470990), (0x67a400, 0x690000)],
          hooks={0x4709e0: _recorder(1, 1, cdecl=True), 0x43cd60: _recorder(2, 2, cdecl=True), 0x43c0a0: _recorder(3, 4, cdecl=True), 0x43d8f0: _recorder(4, 3, cdecl=True),
                 0x474050: _recorder(5, 3, cdecl=True), 0x438800: _recorder(6, 2, cdecl=True), 0x439780: _recorder(7, 3, cdecl=True)})
COAST, TYPE, SEATTYPE, TRAINTYPE, HOLD = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x900, RAM + 0xa00
NODES, KINDS, PIECES, TRAINS, SORTED, CARS, MATRIX = RAM + 0x1000, RAM + 0x2000, RAM + 0x2100, RAM + 0x3000, RAM + 0x3200, RAM + 0x4000, RAM + 0xb00
NODE = 0x38
NODE_COUNT = 18
RIDE = 3


def case(r):
    log = log_region()
    length = r.choice([12.0, 15.0, r.uniform(6.0, 16.0)])
    spacing = r.choice([1.0, 2.0, r.uniform(0.5, 4.0)])
    now = r.randrange(0, 1 << 20)
    set_return(log, 1, now)
    fill(log, 3, 2, struct.pack('<3f', *[r.uniform(-500, 500) for _ in range(3)]), which=0)
    fill(log, 4, 2, struct.pack('<3f', *[r.uniform(-5, 5) for _ in range(3)]), which=1)
    strings(log, 3, 0, out=1 << 2)
    strings(log, 4, 0, out=1 << 2)

    kinds = bytearray(r.bytes(0x40))
    pieces = bytearray(r.bytes(0x100))
    for k in range(2):
        put(kinds, 0x20 * k + 8, 'I', r.getrandbits(32) & ~0x800 | r.choice([0, 0x800]))
    nodes = bytearray(r.bytes(NODE * NODE_COUNT))
    for k in range(NODE_COUNT):
        o = NODE * k
        put(nodes, o, 'I', r.getrandbits(32) & ~8 | r.choice([0, 8]))
        put(nodes, o + 4, '3f', r.uniform(-30, 30), r.uniform(0, 30), r.uniform(-30, 30))
        put(nodes, o + 0x1c, 'f', r.uniform(1, 40))
        put(nodes, o + 0x2c, 'I', PIECES + 0x20 * (k % 8))
    for k in range(8):
        put(pieces, 0x20 * k + 0x10, 'I', KINDS + 0x20 * (k % 2))

    count = r.deal([1, 1, 2, 3])
    trains = bytearray(r.bytes(0x80 * count))
    cars_at = CARS
    regions = {}
    for t in range(count):
        o = 0x80 * t
        seats = r.deal([1, 2, 3, 3])
        mark, lead = r.randrange(seats), r.randrange(seats)
        flags = r.getrandbits(32) & ~0xf0 | r.deal([0, 0x10, 0x20, 0x40, 0x80, 0xa0, 0xc0, 0x90, 0xf0, 0])
        put(trains, o, 'I', flags)
        put(trains, o + 4, 'I', TRAINTYPE)
        put(trains, o + 8, 'I', mark)
        put(trains, o + 0xc, 'I', seats)
        put(trains, o + 0x28, 'I', cars_at)
        put(trains, o + 0x2c, 'I', lead)
        put(trains, o + 0x74, 'f', r.uniform(0, length))
        put(trains, o + 0x78, 'f', r.choice([0.0, 0.005, r.uniform(0, 1.5), r.uniform(1.5, 30)]))
        cars = bytearray(r.bytes(0x60 * 2 * seats))
        for k in range(2 * seats):
            c = 0x60 * k
            on = r.getrandbits(32) & ~1 | r.choice([1, 1, 1, 0])
            put(cars, c, 'I', on)
            put(cars, c + 4, 'I', SEATTYPE + 0x40 * r.randrange(2))
            put(cars, c + 0xc, 'I', r.choice([0, RIDE, RIDE]))
            put(cars, c + 0x38, 'f', r.uniform(-3, 3))
            put(cars, c + 0x40, 'f', r.uniform(0, length))
            put(cars, c + 0x48, '6f', *[r.uniform(-40, 40) for _ in range(6)])
        regions[cars_at] = cars
        cars_at += 0x60 * 2 * seats

    seat_type = bytearray(r.bytes(0x80))
    for k in range(2):
        put(seat_type, 0x40 * k, 'I', r.getrandbits(32) & ~1 | r.choice([0, 1]))
        put(seat_type, 0x40 * k + 0x18, 'f', r.choice([0.0, 0.0, r.uniform(-2, 2)]))
        put(seat_type, 0x40 * k + 0x1c, 'f', r.uniform(-2, 2))
        put(seat_type, 0x40 * k + 0x24, 'f', r.uniform(0, 3))
        put(seat_type, 0x40 * k + 0x28, 'f', r.uniform(0, 3))
    train_type = put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([1, 2, 3, 7, 100]))

    the_type = bytearray(r.bytes(0x2f0))
    for off, low, high in ((0x24, 0, 3), (0x28, 0, 3), (0x2c, 0, 3), (0x44, 1, 30), (0x48, 1, 30), (0x54, 0.2, 2)):
        put(the_type, off, 'f', r.uniform(low, high))
    put(the_type, 0x2ec, 'I', r.choice([0, 500, 2000, 10000, 50000]))

    last = (now - r.choice([0, 16, 33, 100, 300])) & 0xffffffff
    coast = bytearray(r.bytes(0x140))
    put(coast, 0, 'I', r.getrandbits(32) & ~0x10 | r.deal([0x10, 0x10, 0x10, 0]))
    put(coast, 4, 'I', TYPE)
    put(coast, 0x2c, 'f', r.choice([1.0, 1.0, 0.0, r.uniform(0.1, 2)]))
    put(coast, 0x34, 'f', r.choice([60.0, r.uniform(1, 40), r.uniform(1, 40)]))
    put(coast, 0x9c, 'I', NODES)
    put(coast, 0xa4, 'I', NODE_COUNT)
    put(coast, 0xa8, 'f', length)
    put(coast, 0xac, 'f', spacing)
    put(coast, 0xf4, 'I', count)
    put(coast, 0xfc, 'f', r.choice([0.0, r.uniform(0, length), float('nan')]))
    put(coast, 0x120, 'I', last)
    put(coast, 0x128, 'I', TRAINS)
    put(coast, 0x12c, 'I', SORTED)
    put(coast, 0x134, 'f', r.choice([r.uniform(0, length), length * 0.8, length * 0.8, float('nan')]))
    table = bytearray(4 * (RIDE + 1))
    put(table, 4 * RIDE, 'I', HOLD)
    regions.update({LOG: log, COAST: coast, TYPE: the_type, SEATTYPE: seat_type, TRAINTYPE: train_type, NODES: nodes, KINDS: kinds, PIECES: pieces, TRAINS: trains, SORTED: r.bytes(4 * count),
                    0x7a4610: table, HOLD: put(put(bytearray(r.bytes(0x20)), 8, 'I', HOLD + 0x10), 0x14, 'I', HOLD + 0x20), HOLD + 0x20: put(bytearray(r.bytes(0x80)), 0x78, 'I', MATRIX),
                    MATRIX: r.bytes(0x50)})
    return Call(0x43ce20, [COAST], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
