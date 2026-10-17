# The weather renderer's setters and texture release (src/game/render/weatherfx: 0x57d910, 0x57dc40, 0x57dc70, 0x57d720) against the
# originals: any counts and speeds; for the release its three texture arrays (+0x3348..+0x3350, as texturemem.py, counts 0-3) set or not; the renderer's deletion (0x57d1a0) with the same, or none at 0x8bcbac.
# usage: weatherfx.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetDropCount,SetFallSpeed,RequestReload,FreeTextures,Delete)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

B = Bench([(0x57d910, 0x57d91d), (0x57dc40, 0x57dc78), (0x57d720, 0x57d7ed), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13),
           (0x58dc50, 0x58dd5e), (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1), (0x57d1a0, 0x57d1d0)])
CALLS = {'SetDropCount': 0x57d910, 'SetFallSpeed': 0x57dc40, 'RequestReload': 0x57dc70, 'FreeTextures': 0x57d720, 'Delete': 0x57d1a0}
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


def case(r):
    op = r.deal(OPS)
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
