# The lightning's subdivision (src/game/render/weatherfx: WeatherFx_BoltSubdivide 0x57e810) against the original, with its midpoint and ribbon running as they are. The
# projection's slot 1 ([0x87b1d8], cdecl, k 1 of 8) and DrawBatch_AddPolygon 0x565bb0 (k 2, thiscall of 5) are recorders, as in tests/fncmp/ribbon.py. The world keeps
# the recursion finite: end points in a box, a displacement below 0.4 of the length, a limit above its wobble (end points with a NaN, or the same ones, never end in
# the original either: the same end points are given only with an open fork and a refused polygon, which end them first). Half the seeds are chosen to take the branch (9 in 512).
# usage: weatherbolt.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, fill, log_region, put, set_return, strings, vtable  # noqa: E402

B = Bench([(0x57dd10, 0x57ec50), (0x578d20, 0x579004)], hooks={0x565bb0: _recorder(2, 5)})
OBJECT, MATRIX, A, Z, BATCH, PROJECTION, TABLE = RAM, RAM + 0xb000, RAM + 0xb040, RAM + 0xb060, RAM + 0xb080, RAM + 0xb090, RAM + 0xb0a0
QUAD = 0xffffffff & ~(1 << 5 | 1 << 13 | 1 << 21 | 1 << 29)
M = 0xffffffff


def rol(x, n):
    return (x << n | x >> (32 - n)) & M


def step(s):
    return rol((rol(s * 0x13d573 & M, 3) + 0xe0423) & M, 4)


def branching_seed(r):
    """A seed whose sixth draw (the midpoint takes five) is below 9 of 512."""
    while True:
        seed = s = r.getrandbits(32)
        for _ in range(6):
            s = step(s)
        if s & 0x1ff < 9:
            return seed


def case(r):
    log = log_region()
    obj = bytearray(r.bytes(0xa1c0))
    a = [r.uniform(-20, 20) for _ in range(3)]
    b = [r.uniform(-20, 20) for _ in range(3)]
    if r.random() < 0.1:
        axis = r.randrange(3)
        b[axis] = a[axis]
    same = r.random() < 0.1   # the same end points: a length of no size in the branch, and only a refused polygon ends them
    if same:
        b = a[:]
    limit = r.choice([100.0, 400.0, 1600.0])
    put(obj, 0x3310, 'I', fb(r.uniform(-0.4, 0.4)))
    put(obj, 0xa180, 'I', fb(limit))
    put(obj, 0xa184, 'I', fb(r.uniform(0.0, limit / 2)))
    ribbon = r.randrange(13)
    put(obj, 0xa188, 'I', ribbon)
    put(obj, 0xa1a0, 'I', r.choice([ribbon + 1, ribbon + 2, 14] if same else [ribbon, ribbon + 1, ribbon + 2, 14, 14, 0xffffffff]))   # (same: the fork must be open, its midpoint a NaN)
    normal = r.randrange(4)
    put(obj, 0x9da0, 'I', normal)
    put(obj, 0x3020 + normal * 0x30, '3f', *[r.uniform(-1, 1) for _ in range(3)])
    # rng: the renderer's own, or another's whose seed lies past the object's last field
    other = r.random() < 0.25
    seed = branching_seed(r) if same or r.random() < 0.5 else r.getrandbits(32)
    put(obj, 0xa1a8 if other else 0xa198, 'I', seed)
    strings(log, 1, 0, out=1 << 2)
    blob(log, 1, 3, 0x3f)
    fill(log, 1, 2, struct.pack('<8f', *[r.uniform(-100, 100) for _ in range(8)]) if same else struct.pack('<8I', *[r.f() for _ in range(8)]))
    blob(log, 2, 0, QUAD)
    set_return(log, 2, 0 if same else r.choice([1, 1, 1, 1, 2, 0, r.getrandbits(32)]))
    matrix = struct.pack('<16I', *[r.f() for _ in range(16)])
    if same:   # (finite places and width: the fork's second point draws, and the refusal comes before the NaN midpoint's endless recursion)
        put(obj, 0xa16c, 'f', 2.0)
        matrix = struct.pack('<16f', 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
    regions = {LOG: log, OBJECT: obj, MATRIX: matrix, A: struct.pack('<3f', *a), Z: struct.pack('<3f', *b), BATCH: r.bytes(0x10),
               PROJECTION: struct.pack('<I', TABLE), TABLE: vtable({}, 8, cdecl={1: 8}), 0x87b1d8: struct.pack('<I', PROJECTION), 0x87b088: struct.pack('<I', MATRIX),
               0x879ab0: struct.pack('<I', BATCH), 0x8bcbac: struct.pack('<I', OBJECT + 0x10 if other else OBJECT)}
    return Call(0x57e810, [OBJECT, A, Z], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
