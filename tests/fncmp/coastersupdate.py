# All coasters updated for a frame (src/game/coaster/coastersupdate: 0x435a30) against the original: up to 3 coasters, each with its type, offsets, 2 listed cars with their shapes and 3 station cars; the calls recorders. ActionRec_WriteW's u16 goes on the stack in a whole ECX (the rest left by the code before): the recorder sees the low 16 bits, all the callee reads.
# usage: coastersupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

PLACE, WRITE, REBUILD, RUN, TIME, STATION, SOUNDS = range(1, 8)
WRITE_RECORDER = _recorder(WRITE, 2)


def _write(e):
    e.write(e.r['ESP'] + 4, struct.pack('<I', e.u32(e.r['ESP'] + 4) & 0xffff))   # (a hook's ESP: its first argument)
    WRITE_RECORDER(e)


B = Bench([(0x435a30, 0x435d4b)],
          hooks={0x435d50: _recorder(PLACE, 1, cdecl=True), 0x403d00: _write, 0x43eb70: _recorder(REBUILD, 1, cdecl=True), 0x43ce20: _recorder(RUN, 1, cdecl=True),
                 0x4709e0: _recorder(TIME, 1, cdecl=True), 0x43b8b0: _recorder(STATION, 2, cdecl=True), 0x439570: _recorder(SOUNDS, 2, cdecl=True)})
SIZE = 0x2000
TYPE, MATERIALS, TABLE, OFFSETS, STATIONS, CARS = 0x200, 0x240, 0x260, 0x300, 0x400, 0x600
SHAPES = (0x800, 0x1200)


def shape(r, buf, at, base):
    put(buf, at + 0x68, 'I', base + at + 0x100)
    put(buf, at + 0x6c, 'I', base + at + 0x80)
    put(buf, at + 0x70, 'I', base + at + 0xc0)
    groups = r.choice([0, 1, 2, 3])
    put(buf, at + 0x5a, 'H', groups)
    for g in range(3):
        put(buf, at + 0x80 + g * 0x10, 'I', 0x1000 + 8 * r.randrange(4))
        put(buf, at + 0x80 + g * 0x10 + 4, '2H', r.randrange(3), r.choice([0, 1, 2, 3, 5]))
    limit = r.choice([4, 4, 12, 0x100])
    for f in range(8):
        put(buf, at + 0xc0 + f * 8 + 2, '3H', r.randrange(limit), r.randrange(limit), r.randrange(limit))
    r.floats(buf, range(at + 0x100, at + 0x900, 4), r.choice([1.0, 8.0]))


def coaster(r, base, following):
    buf = bytearray(r.bytes(SIZE))
    put(buf, 0, 'I', r.getrandbits(32) & ~0x2005 | r.choice([0, 1, 1, 5, 0x2001, 0x2005, 0x2000]))
    put(buf, 4, 'I', base + TYPE)
    put(buf, 8, 'I', r.getrandbits(32) & ~0x4e | r.choice([0, 2, 4, 8, 0xc, 0xe, 0x44, 0x48]))
    put(buf, 0x18, 'I', following)
    r.floats(buf, [0x2c], r.choice([1.0, 5.0, 100.0]))
    put(buf, 0x38, 'I', r.choice([0, base + CARS, base + CARS, base + CARS + 0x40]))
    put(buf, 0xf4, 'I', r.choice([0, 1, 2, 3]))
    put(buf, 0x128, 'I', base + STATIONS)
    put(buf, 0x138, 'I', r.choice([0, 1000, 123456, 0xfffffff0]))
    put(buf, 0x13c, 'I', base + OFFSETS)
    put(buf, TYPE, 'I', r.getrandbits(32) & ~4 | r.choice([0, 4, 4]))
    put(buf, TYPE + 0x1c, 'I', base + MATERIALS)
    put(buf, TYPE + 0x20, 'I', r.choice([0, 1, 2, 4]))
    put(buf, MATERIALS + 0x10, 'I', base + TABLE)
    put(buf, MATERIALS + 0x18, 'I', 0x1000)
    r.floats(buf, range(OFFSETS, OFFSETS + 0x10, 4), r.choice([1.0, 3.0, 100.0]))
    if r.randrange(16) == 0:
        put(buf, OFFSETS + 4 * r.randrange(4), 'I', r.choice([0x7fc00001, 0x7f800000, 0x7f800001]))
    for i in range(3):
        put(buf, STATIONS + i * 0x80, 'I', r.getrandbits(32) & ~2 | r.choice([0, 2, 2]))
    for i, at in enumerate(SHAPES):
        car = CARS + i * 0x40
        put(buf, car + 0x24, 'I', base + car + 0x10)   # (an object in the car itself: its +4 a holder at `at`, whose +0x78 is the shape)
        put(buf, car + 0x14, 'I', base + at)
        put(buf, car + 0x38, 'I', base + CARS + 0x40 if i == 0 else 0)
        put(buf, at + 0x78, 'I', base + at + 0x80)
        shape(r, buf, at + 0x80, base)
    return buf


def case(r):
    log = log_region()
    count = r.choice([0, 1, 1, 2, 3])
    bases = [RAM + 0x4000 * i for i in range(count)]
    regions = {LOG: log, 0x792218: r.bytes(0xc)}
    for i, base in enumerate(bases):
        regions[base] = coaster(r, base, bases[i + 1] if i + 1 < count else 0)
    regions[0x790fe0] = struct.pack('<4I', bases[0] if bases else 0, r.getrandbits(32), r.getrandbits(32), r.choice(bases + [0]))
    set_return(log, TIME, r.choice([0, 1000, 1016, 200000, r.getrandbits(32)]))
    return Call(0x435a30, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
