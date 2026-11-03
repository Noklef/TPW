# The drops as streaks (WeatherFx_DrawStreaks, src/game/render/weatherfx: 0x57efd0) against the original: the transformer's slot 1 is a cdecl recorder (k 1) writing the transformed places back; Batch_Begin (k 2),
# DrawBatch_AddPolygon (k 3), Batch_TransformVertices (k 4) and Batch_Close (k 5) are recorders. The world: 0-6 streaks, places on the wrap's edges, the transformer's answers at z 0, NaNs now and then, the
# texture none, without an id or with one. The vertices' dwords 5, 13, 21 and 29 are unset in the original too: the polygons' hash leaves them out.
# usage: weatherstreaks.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x57efd0, 0x57f490), (0x67a830, 0x67a860), (0x67b110, 0x67b13f)],
          hooks={0x565240: _recorder(2, 5), 0x565bb0: _recorder(3, 5), 0x565f50: _recorder(4, 1), 0x5658c0: _recorder(5, 1)})
FX, TEX, MASK, XF, XFT, BATCH = RAM, RAM + 0x10000, RAM + 0x10100, RAM + 0x10200, RAM + 0x10300, RAM + 0x10400
QUAD = 0xffffffff & ~(1 << 5 | 1 << 13 | 1 << 21 | 1 << 29)
NANS = [0x7fc12345, 0x7f812345, 0xffc00000, 0x7f800000]


def fnum(r, lo, hi, nan=0.01):
    return struct.pack('<I', r.choice(NANS)) if r.random() < nan else struct.pack('<f', r.uniform(lo, hi))


def case(r):
    log = log_region()
    count = r.choice([0, 1, 2, 3, 4, 5, 6])
    exact = r.random() < 0.2                                             # (no x, y, z: a place of exactly 4 or -4 meets the wrap's comparisons)
    places = b''.join(struct.pack('<f', r.choice([4.0, -4.0])) if exact and r.random() < 0.5 else fnum(r, -6, 6, 0.05 if exact else 0.01) for _ in range(3 * max(count, 1)))
    fields = bytearray(r.bytes(0x60))
    fields[0x14:0x18] = fnum(r, -2, 2)                                   # +0x3314: the scale
    put(fields, 0x18, 'i', count)                                        # +0x3318
    for off in (0x1c, 0x20, 0x24):
        fields[off:off + 4] = fnum(r, -3, 3)                             # +0x331c..: the wind
    kind = r.choice(['none', 'plain', 'id', 'id', 'id', 'id0'])
    put(fields, 0x50, 'I', 0 if kind == 'none' else TEX)                 # +0x3350
    texture = bytearray(r.bytes(0x40))
    flags = r.getrandbits(32)
    put(texture, 8, 'I', flags & ~0x1000 if kind == 'plain' else flags | 0x1000)
    if kind == 'id0':
        put(texture, 0x1c, 'I', 0)
    for off in (0x20, 0x24, 0x28, 0x2c):
        texture[off:off + 4] = fnum(r, -2, 2, 0.05)
    strings(log, 1, 0, out=1 << 2 if count else 3 << 2)                   # (the out and in places are one stack address)
    if count:
        blob(log, 1, 3, (1 << min(6 * count, 32)) - 1)
        out = [fnum(r, -8, 8, 0.03) if r.random() < 0.8 else struct.pack('<f', r.choice([0.0, -0.0, 6.0, 7.0, 40.0, 1.0])) for _ in range(6 * min(count, 4))]
        if r.random() < 0.6:                                             # (a fill goes in before the hash: without one the hash and the quads show the places as made)
            fill(log, 1, 2, b''.join(out))
    blob(log, 3, 0, QUAD)
    set_return(log, 2, 0 if r.random() < 0.1 else BATCH)
    if r.random() < 0.1:
        set_return(log, 3, 0)
    if r.random() < 0.5:
        set_return(log, 5, r.choice([0, 1, r.getrandbits(32)]))
    regions = {LOG: log, FX + 8: places, FX + 0x3300: fields, FX + 0x9d58: r.bytes(0x60), TEX: texture, BATCH: r.bytes(0x40),
               MASK: r.bytes(0x30), XF: struct.pack('<I', XFT), XFT: vtable({}, 8, cdecl={1: 8}), 0x87b1d8: struct.pack('<I', XF), 0x87a248: struct.pack('<I', MASK),
               0x879ab0: struct.pack('<I', r.getrandbits(32)), 0x8bcbc8: struct.pack('<I', r.getrandbits(32))}
    args = [0 if exact else struct.unpack('<I', fnum(r, -6, 6))[0] for _ in range(3)]
    return Call(0x57efd0, [FX] + args, regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
