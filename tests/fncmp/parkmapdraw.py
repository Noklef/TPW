# The park map's drawing (src/game/world/parkmap: ParkMap_Draw 0x5f2380) against the original: a map of up to 6 x 6 squares (now and then 0, negative, or 128 long) of a
# few kinds and things, so that neighbours match; the mode, view and flags any; a surface of a depth with a canvas driver of recorders (now and then none); the list's sprites with
# handles on or off the map. Object_Rating, Object_HistoryAverage, TexSprite_Update and 0x5f2fd0 are recorders (env in driver.cpp's kParkMapEnv).
# usage: parkmapdraw.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, blob_array, fb, log_fold, log_region, put, set_return, strings, vtable  # noqa: E402

RATING, AVERAGE, UPDATE, LINES = 10, 11, 12, 13   # (driver.cpp's kParkMapEnv)
HLINE, VLINE, BOX, FILL = 2, 3, 7, 8              # (the driver's slots)
B = Bench([(0x5f2380, 0x5f2f9c), (0x5da3c0, 0x5da3c1), (0x617370, 0x6175b4), (0x5fa680, 0x5fa6a0), (0x5f09a0, 0x5f0ae0), (0x4e0440, 0x4e044b), (0x67a830, 0x67a860)],
          hooks={0x4e0860: _recorder(RATING, 1), 0x4e1e30: _recorder(AVERAGE, 0), 0x5f44d0: _recorder(UPDATE, 3), 0x5f2fd0: _recorder(LINES, 0)})
MAP, DRIVER_TABLE, OBJECTS, LIST, NODES, SPRITES, HANDLES = RAM, RAM + 0x20200, RAM + 0x20300, RAM + 0x20800, RAM + 0x20820, RAM + 0x20840, RAM + 0x20900
MAP_SIZE = 0x20100
RECORDS = 0xf86a28


def case(r):
    wide, high = r.choice([(r.randrange(1, 7), r.randrange(1, 7))] * 8 + [(0, 3), (3, 0), (0xffffffff, 2), (0x80, 1), (1, 0x80)])
    m = bytearray(MAP_SIZE)
    m[:0x60] = r.bytes(0x60)
    put(m, 0x18, '3I', r.getrandbits(32), r.choice([0, 1, 2, 3, r.getrandbits(32)]), r.choice([0, 1, 2, 2, 3, 3, r.getrandbits(32)]))
    put(m, 0x24, '2I', wide, high)
    put(m, 0x2c, '6I', r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.choice([8, 16, 24, 32, 32, 32, 15]), r.choice([0, 0, 0, 0, 0, 1]))
    kinds = [r.randrange(0, 14) for _ in range(3)]
    for y in range(min(high, 0x80) if high < 0x100 else 3):
        for x in range(min(wide, 0x80) if wide < 0x100 else 3):
            put(m, 0x60 + (y * 0x80 + x) * 8, 'IH', r.choice(kinds), r.randrange(3))

    objects = bytearray(0x80 * 3)
    for i in range(3):
        put(objects, i * 0x80 + 0x44, 'I', r.choice([fb(r.uniform(0, 110)), fb(r.uniform(300, 400)), r.f()]))
        put(objects, i * 0x80 + 0x68, 'I', r.choice([0, 1]))

    count = r.randrange(4)
    links = bytearray(0x20)
    sprites = bytearray(r.bytes(0x30 * 3))
    handles = bytearray(0x210 * 3)
    for i in range(3):
        at = i * 0x30
        put(sprites, at + 0x14, 'I', r.choice([2, 2, 2, 1]))
        put(sprites, at + 0x18, 'I', HANDLES + i * 0x210)
        kind = r.choice([1, 2, 3, 0, 4])
        put(sprites, at + 0x1c, 'I', kind)
        map_value = struct.unpack_from('<I', m, 0x14 + 4 * max(1, min(kind, 3)))[0]
        put(sprites, at + 0x20, 'I', map_value if r.random() < 0.7 else r.getrandbits(32))
        put(handles, i * 0x210 + 0x20a, '2H', *[r.choice([0, 1, r.getrandbits(16), (r.randrange(1, 4) << 7) + r.randrange(1, 0x7f) + 1, (0x7f << 7) + 2, (3 << 7) + 0x80]) for _ in range(2)])
    for i in range(count):
        put(links, i * 8, '2I', SPRITES + i * 0x30, NODES + (i + 1) * 8 if i + 1 < count else 0)
    put(m, 0x20060, 'I', SPRITES + r.randrange(3) * 0x30)
    put(m, 0x20080, 'I', LIST)

    log = log_region()
    log_fold(log)
    set_return(log, RATING, r.choice([r.randrange(0, 101), 0x1200 + r.randrange(0, 101)]))
    set_return(log, AVERAGE, r.choice([r.randrange(0, 101), 0xffffffff, 0x1000]))
    for k in (HLINE, VLINE, BOX, FILL):
        strings(log, k, 0, out=0b11)
    blob(log, HLINE, 6, 0x1ff)
    blob(log, VLINE, 6, 0x1ff)
    blob(log, FILL, 4, 0x1ff)
    blob_array(log, FILL, 3, 2, 2)           # (the rectangle: 8, the flags, x 2 bytes)
    blob(log, BOX, 3, 0xf)
    blob_array(log, BOX, 4, 2, 3, which=1)   # (the record: 0xc, the flags, x 3 bytes)
    regions = {MAP: m, DRIVER_TABLE: vtable({HLINE: 7, VLINE: 7, BOX: 5, FILL: 5}), OBJECTS: objects, LIST: struct.pack('<4I', 0, NODES if count else 0, 0, 0),
               NODES: links, SPRITES: sprites, HANDLES: handles, RECORDS: r.bytes(0x21c), 0xfa7e30: struct.pack('<4I', *[DRIVER_TABLE] * 4), 0x7cfb90: struct.pack('<15I', *[OBJECTS + (i // 5) * 0x80 if i % 5 == 0 else 0 for i in range(15)]), LOG: log}
    return Call(0x5f2380, [MAP], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
