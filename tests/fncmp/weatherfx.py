# The weather renderer's setters, wind and texture release (src/game/render/weatherfx: 0x57d910, 0x57d920, 0x57dc40, 0x57dc70, 0x57d720)
# against the originals: any counts and speeds; the wind's angle whole and odd degrees, any, past 2^63 radians or not finite, its
# strength 0, small, any, tiny, huge or not finite (the landscape's points at 0x8c1828 + 0x1f70). Angles from 5e8 degrees (1e7
# radians) to 6e20 (past 2^63 radians) are left out: there fsin / fcos reduce by their own 66-bit pi (see tests/fncmp/quad.py); for the release its three texture arrays (+0x3348..+0x3350, as texturemem.py, counts 0-3) set or not; the renderer's deletion (0x57d1a0) with the same, or none at 0x8bcbac.
# usage: weatherfx.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetDropCount,SetWind,SetFallSpeed,RequestReload,FreeTextures,Delete)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import SPECIAL, Bench, Call, HEAP, LOG, RAM, bf, fb, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x57d910, 0x57d91d), (0x57d920, 0x57dc3f), (0x5856e0, 0x585705), (0x57dc40, 0x57dc78), (0x57d720, 0x57d7ed), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13),
           (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x57d1a0, 0x57d1d0)])
CALLS = {'SetDropCount': 0x57d910, 'SetWind': 0x57d920, 'SetFallSpeed': 0x57dc40, 'RequestReload': 0x57dc70, 'FreeTextures': 0x57d720, 'Delete': 0x57d1a0}
FX, ARRAYS = RAM + 0x10000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)


def free_textures(r, op):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    tail = bytearray(r.bytes(0x18))
    for k in range(3):
        if r.random() < 0.75:
            m = r.choice([0, 1, 2, 3])
            base = ARRAYS + 0x400 * k
            regions[base] = struct.pack('<I', m) + texturemem.textures(r, regions, base + 4, max(m, 1))
            put(tail, 8 + 4 * k, 'I', base + 4)
        else:
            put(tail, 8 + 4 * k, 'I', 0)
    regions[FX + 0x3340] = tail
    if op == 'Delete':   # (the renderer at 0x8bcbac, or none)
        regions[0x8bcbac] = struct.pack('<I', FX if r.random() < 0.85 else 0)
        return Call(0x57d1a0, [], regions, conv='c', ret='a')
    return Call(0x57d720, [FX], regions, conv='t', ret='a')


def wind(r):
    angle = r.choice([fb(r.choice([0, 90, 180, 270, -90, 45, 359, 720])), fb(r.randrange(-360, 361)), r.f(720), r.f(), r.getrandbits(32),
                      fb(r.choice([1e8, -3e21, 1e30])), r.choice(SPECIAL)])
    if 5e8 < abs(bf(angle)) < 6e20:
        angle = r.choice([fb(r.uniform(-5e8, 5e8)), fb(r.uniform(6e20, 3e38)) | (angle & 0x80000000)])
    strength = r.choice([0, fb(0.3), r.pos(5), r.f(), fb(r.choice([1e-40, 3e-39, 1e30, 3e38])), r.choice(SPECIAL), r.getrandbits(32)])
    regions = {RAM: r.bytes(0x3340), 0x8c1828 + 0x1f70: r.bytes(0x30)}
    return Call(0x57d920, [RAM, angle, strength], regions, conv='t', ret='v')


def case(r):
    op = r.deal(OPS)
    if op == 'SetWind':
        return wind(r)
    if op in ('FreeTextures', 'Delete'):
        return free_textures(r, op)
    fx = bytearray(r.bytes(0x3340))
    put(fx, 0, 'I', r.choice([0, 1, 2, 3, 0xfffffffc, r.getrandbits(32)]))
    args = [RAM]
    if op != 'RequestReload':
        args.append(r.choice([0, 0x3f4ccccd, r.getrandbits(32)]))
    return Call(CALLS[op], args, {RAM: fx}, conv='t', ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
