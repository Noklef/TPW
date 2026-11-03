# A car's mesh bent along the track (src/game/coaster/bendmesh: Coaster_BendMesh 0x438800) against the original: Coaster_NodeAngles (k 1) a recorder that fills 28 floats of each table and the
# count, PartStates_Update (k 2) a recorder, the rotations and the CRT as they are. A car with no ride (returns at once), a holder of none (a fault in both, sic); the model's frames: the car's
# frame number below the model's first count or in its second table (+0x74), flag 0x200 (skipped); packs of 4 vertices (0x30 bytes each) of 0 to 9 vertices. The heights keep the whole part of
# (count * spacing + z) / spacing inside the filled tables.
# usage: bendmesh.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, strings  # noqa: E402

B = Bench([(0x438800, 0x438ab0), (0x4314f0, 0x4315f0), (0x67a400, 0x690000)], sines=256, hooks={0x438ab0: _recorder(1, 8, cdecl=True), 0x44ab30: _recorder(2, 1, cdecl=True)})
PATH, CAR, TYPE, HOLDER, INSTANCE, MODEL, BASE = RAM, RAM + 0x100, RAM + 0x180, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x400
FRAMES, EXTRA, BASEFRAMES, SRC, DST = RAM + 0x1000, RAM + 0x11c0, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
FIRST = 2
RIDE = 3


def case(r):
    log = log_region()
    count = r.randrange(11, 15)
    spacing = r.choice([0.5, 1.0, 2.0, 8.0, r.uniform(0.25, 10.0)])
    for which, arg in enumerate((2, 3, 4)):
        fill(log, 1, arg, struct.pack('<28I', *[r.f(7) for _ in range(28)]), which=which)
    fill(log, 1, 7, struct.pack('<I', count), which=3)
    strings(log, 1, 0, out=1 << 2 | 1 << 3 | 1 << 4 | 1 << 7)

    frame_number = r.deal([0, 1, FIRST, FIRST + 1])
    verts = r.deal([0, 1, 3, 4, 5, 9])
    flags = r.getrandbits(32) & ~0x200 | r.deal([0, 0, 0, 0, 0x200])
    frames = bytearray(r.bytes(EXTRA - FRAMES + 0x58 * 3))
    at = frame_number * 0xa0 if frame_number < FIRST else EXTRA - FRAMES + (frame_number - FIRST) * 0x58
    put(frames, at, 'I', flags)
    put(frames, at + 0x58, 'H', verts)
    put(frames, at + 0x60, 'I', DST)
    index = at // 0xa0
    base = bytearray(r.bytes(0xa0 * 4))
    put(base, index * 0xa0 + 0x60, 'I', SRC)

    src = bytearray(r.bytes(0x30 * 3))
    for k in range(9):
        pack, i = k // 4, k % 4
        put(src, pack * 0x30 + i * 4, 'I', r.f(30))
        put(src, pack * 0x30 + 0x10 + i * 4, 'I', r.f(30))
        put(src, pack * 0x30 + 0x20 + i * 4, 'f', r.uniform(-10.5, 10.5) * spacing)

    holder = r.deal([HOLDER, HOLDER, HOLDER, HOLDER, HOLDER, HOLDER, HOLDER, 0])
    table = bytearray(4 * (RIDE + 1))
    put(table, 4 * RIDE, 'I', holder)
    car = bytearray(r.bytes(0x60))
    put(car, 4, 'I', TYPE)
    put(car, 0xc, 'I', r.deal([RIDE, RIDE, RIDE, RIDE, RIDE, 0]))
    put(car, 0x40, 'I', r.f(100))
    path = put(bytearray(r.bytes(0xb0)), 0xac, 'f', spacing)
    regions = {LOG: log, PATH: path, CAR: car, TYPE: put(bytearray(r.bytes(0x40)), 0x20, 'I', frame_number), 0x7a4610: table,
               HOLDER: put(bytearray(r.bytes(0x10)), 8, 'I', INSTANCE), INSTANCE: put(bytearray(r.bytes(0x10)), 4, 'I', MODEL),
               MODEL: put(put(put(put(bytearray(r.bytes(0xa0)), 0x44, 'H', FIRST), 0x70, 'I', FRAMES), 0x74, 'I', EXTRA), 0x9c, 'I', BASE),
               BASE: put(bytearray(r.bytes(0x80)), 0x70, 'I', BASEFRAMES), FRAMES: frames, BASEFRAMES: base, SRC: src, DST: r.bytes(0x30 * 3)}
    return Call(0x438800, [PATH, CAR], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
