# The texture manager's load flags setter and its release (src/game/gfx/texman: 0x575b80, 0x574b10) against the originals: any
# flags over any old ones; for the release the manager at 0x87a268 or none, 0-3 slots in use (or a negative count) whose texture
# arrays (as texturemem.py) are set or not; for a texture's release 0-4 slots (the count theirs, one less, 0 or negative), their arrays
# set, 0 or any bits, their users 1, more, 0 or negative, the array asked for one of theirs or another (src/game/gfx/texman: 0x575ad0).
# usage: texman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLoadFlags,Destroy,Release)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'SetLoadFlags': 0x575b80, 'Destroy': 0x574b10, 'Release': 0x575ad0}
B = Bench([(0x575b80, 0x575b8d), (0x575ad0, 0x575b7a),(0x574b10, 0x574bc6), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e),
           (0x67afa0, 0x67b0d0), (0x67acb0, 0x67acb1)])
MANAGER, ARRAYS = RAM + 0x10000, RAM + 0x5000   # (texturemem.py's regions below RAM + 0x4000)


def destroy(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    manager = bytearray(r.bytes(0x1000c))
    n = r.choice([0, 1, 2, 3])
    put(manager, 0x10000, 'I', r.choice([n, n, n, 0, 0xffffffff]) if n else r.choice([0, 0, 0xffffffff]))
    for k in range(n):
        if r.random() < 0.8:
            m = r.choice([0, 1, 2, 3])
            base = ARRAYS + 0x400 * k
            tex = texturemem.textures(r, regions, base + 4, m + 1)
            regions[base] = struct.pack('<I', m) + tex
            put(manager, 8 + 0x10 * k, 'I', base + 4)
        else:
            put(manager, 8 + 0x10 * k, 'I', 0)
    regions[MANAGER] = manager
    regions[0x87a268] = struct.pack('<I', r.choice([MANAGER, MANAGER, MANAGER, 0]))
    return Call(0x574b10, [], regions, conv='c', ret='a')


def release(r):
    regions = {HEAP: heap(), **global_heap(), LOG: log_region()}
    n = r.choice([0, 1, 2, 3, 4])
    slots = bytearray(r.bytes(0x10 * n + 0x10))
    arrays = []
    for k in range(n):
        if r.random() < 0.85:
            m = r.choice([1, 2, 3])
            base = ARRAYS + 0x400 * k
            regions[base] = struct.pack('<I', m) + texturemem.textures(r, regions, base + 4, m)
            arrays.append(base + 4)
            put(slots, 0x10 * k + 8, 'I', base + 4)
        else:
            put(slots, 0x10 * k + 8, 'I', r.choice([0, r.getrandbits(32)]))
        put(slots, 0x10 * k + 0xc, 'I', r.choice([1, 1, 1, 2, 3, 0, 0xffffffff]))
    regions[MANAGER] = slots
    regions[MANAGER + 0x10000] = struct.pack('<III', r.choice([n, n, n, max(n - 1, 0), 0, 0xffffffff]), r.getrandbits(32), r.getrandbits(32))
    arg = r.choice(arrays) if arrays and r.random() < 0.8 else r.choice([ARRAYS + 0x2004, r.getrandbits(32)])
    return Call(0x575ad0, [MANAGER, arg], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'Destroy':
        return destroy(r)
    if op == 'Release':
        return release(r)
    regions = {RAM + 0x10000: r.bytes(0x10)}
    return Call(0x575b80, [RAM, r.choice([0, 2, 4, 8, r.getrandbits(32)])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
