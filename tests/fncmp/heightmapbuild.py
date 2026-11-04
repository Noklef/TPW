# An instance's height grid built (src/game/model/heightmap: 0x451880) against the original. Layer_TimeLeft and __ftol real; the clone, the blocks' calls and the rasterizer recorders (driver.cpp's kHeightMapCalls; the rasterizer's 23 arguments on two recorders).
# usage: heightmapbuild.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_fold, log_region, set_return, strings  # noqa: E402

CLONE, RESET, ALLOC, EVENT, SETTIME, START, ADVANCE, PLAY, IDENTITY, RASTER1, RASTER2, APPLY, COPY, CHANNELS, DISPOSE = range(1, 16)
CALLS = {0x4557c0: (CLONE, 5), 0x473e30: (RESET, 4), 0x45b9d0: (ALLOC, 2), 0x468600: (EVENT, 5), 0x4709d0: (SETTIME, 1), 0x472bc0: (START, 4), 0x472c60: (ADVANCE, 3), 0x472f60: (PLAY, 6),
         0x46f150: (IDENTITY, 1), 0x4721f0: (APPLY, 3), 0x472270: (COPY, 3), 0x472310: (CHANNELS, 3), 0x455ad0: (DISPOSE, 3)}
HOOKS = {addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CALLS.items()}
_first, _second = _recorder(RASTER1, 15, cdecl=True), _recorder(RASTER2, 8, cdecl=True)


def rasterize(e):   # (as the driver's heightmap_rasterize_recorder)
    _first(e)
    e.r['ESP'] += 15 * 4
    _second(e)
    e.r['ESP'] -= 15 * 4


HOOKS[0x451e20] = rasterize
B = Bench([(0x451880, 0x451e1c), (0x472d50, 0x472d61), (0x67a400, 0x690000), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
INSTANCE, HELD1, MODEL1, CLONED, HELD2, MODEL2, PARTS, BLOCKS, TRACKS, HEADER, CELLS, FLAGS, TABLE = (RAM + k * 0x400 for k in range(13))
BOUND = 0x770000


def dwords(*values):
    return struct.pack('<%dI' % len(values), *[v & 0xffffffff for v in values])


def floats(*values):
    return struct.pack('<%df' % len(values), *values)


def case(r):
    log = log_region()
    log_fold(log)
    width, height = r.choice([0, 1, 1, 2, 3]), r.choice([0, 1, 1, 2, 3])
    regions = {LOG: log, 0x79f7d4: r.bytes(4), 0x79fb08: r.bytes(8), 0x79fb20: r.bytes(4)}
    regions[INSTANCE] = r.bytes(8) + dwords(HELD1)
    regions[HELD1] = r.bytes(4) + dwords(MODEL1)
    regions[MODEL1] = r.bytes(0x98)
    count = r.choice([0, 1, 1, 2, 3])
    blocks = [r.choice([0, BOUND + 0x10 * i, BOUND + 0x10 * i, BOUND + 0x10 * i]) for i in range(count)]
    regions[BLOCKS] = dwords(*blocks, 0)
    tracks = bytearray(r.bytes(0x100))
    slots = [r.choice([0, 0, 0, 1, r.getrandbits(32)]) if r.random() < 0.5 else 0 for _ in range(0xc)]
    if r.random() < 0.7:
        slots[r.randrange(2, 0xc)] = 1
    at = [0] * 0xc
    for i in range(2, 0xc):
        at[i] = TRACKS + 0x20 * (i - 2)
    for i in (0, 1):
        at[i] = r.choice([0, 0, TRACKS + 0xf0 - 0x10 * i])
        struct.pack_into('<I', tracks, 0xf4 - 0x10 * i, r.choice(blocks) if blocks and r.random() < 0.7 else BOUND + 0xff0)
    regions[TRACKS] = tracks
    clone = bytearray(r.bytes(0xb4))
    struct.pack_into('<I', clone, 8, HELD2)
    for i in range(0xc):
        struct.pack_into('<II', clone, 0x1c + 0xc * i, slots[i], at[i])
    struct.pack_into('<II', clone, 0xa8, count, BLOCKS)
    regions[CLONED] = clone
    regions[HELD2] = r.bytes(4) + dwords(MODEL2) + r.bytes(4)
    parts = r.choice([0, 1, 3])
    model = bytearray(r.bytes(0x98))
    struct.pack_into('<H', model, 0x44, parts)
    struct.pack_into('<I', model, 0x70, PARTS)
    struct.pack_into('<I', model, 0x90, r.f())
    regions[MODEL2] = model
    regions[PARTS] = r.bytes(0xa0 * 3)
    regions[HEADER] = bytes(0x30)
    regions[CELLS] = r.bytes(27 * max(width * height, 1) + 8)
    regions[FLAGS] = b''.join(dwords(r.choice([0, 0, 1, 0x100]), 0) for _ in range(1 + 3 + 0x14 * 3 + 4))
    set_return(log, CLONE, r.choice([1] * 15 + [0]))
    fill(log, CLONE, 3, dwords(CLONED), which=0)
    regions[TABLE] = dwords(HEADER, CELLS, *([0] * 40))
    answers(log, ALLOC, TABLE)
    strings(log, CLONE, 0, out=1 << 3)
    strings(log, EVENT, 0, out=7 << 2)
    fill(log, EVENT, 2, dwords(r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)), which=1, records=3)
    fill(log, EVENT, 3, dwords(r.getrandbits(32), r.getrandbits(32), r.getrandbits(32)), which=2, records=3)
    blob(log, START, 0, 0xe)
    fill(log, START, 0, dwords(*[x for _ in range(3) for x in (r.randrange(2, 0xc), r.randrange(3))]), offset=4, which=3, records=3)
    end = r.choice([(1.0, 2.0), (0.0, 1e-3), (1.0, float('nan')), (float('nan'), 0.0)])
    steps = [(10.0, r.uniform(0, 10)), (10.0, 10.0), (5.0, r.uniform(-3, 4)), end]
    # (a record's number comes from the log's count, stuck when the log is full: every case ends within its 40 entries, or the loop would not)
    steps = [end] if count > 2 else r.choice([[end], steps[:1] + [end]]) if count == 2 else r.choice([[end], steps[:1] + [end], steps[1:], steps])
    fill(log, ADVANCE, 0, b''.join(floats(*s) for s in steps), offset=0x1c, which=4, records=len(steps))
    strings(log, ADVANCE, 0, out=1)
    blob(log, ADVANCE, 1, 1)
    blob(log, PLAY, 3, 0xe)
    strings(log, IDENTITY, 0, out=1)
    fill(log, IDENTITY, 0, b''.join(dwords(r.f()) for _ in range(0x10)), which=5)
    strings(log, APPLY, 0, out=1)
    fill(log, APPLY, 0, dwords(r.f()), offset=0x20, which=6)
    strings(log, RASTER2, 0, out=1 << 4)
    blob(log, RASTER2, 7, 1)
    fill(log, RASTER2, 7, dwords(r.getrandbits(32)), which=7)
    return Call(0x451880, [INSTANCE, width, height, r.choice([0, FLAGS, FLAGS]), r.choice([0, 0, 1, 0x100])], regions, ret='a')


if __name__ == '__main__':
    if '--table' in sys.argv:
        for addr, (k, n) in sorted(CALLS.items()):
            print('    {0x%06x, jpeg_recorder(%d, %d)},' % (addr, k, n))
        print('    {0x451e20, (u32)(uintptr_t)&heightmap_rasterize_recorder},')
        sys.exit(0)
    B.main(case)
