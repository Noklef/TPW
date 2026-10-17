# The texture manager's load flags setter and its release (src/game/gfx/texman: 0x575b80, 0x574b10) against the originals: any
# flags over any old ones; for the release the manager at 0x87a268 or none, 0-3 slots in use (or a negative count) whose texture
# arrays (as texturemem.py) are set or not.
# usage: texman.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetLoadFlags,Destroy)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import texturemem  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, global_heap, heap, log_region, put  # noqa: E402

CALLS = {'SetLoadFlags': 0x575b80, 'Destroy': 0x574b10}
B = Bench([(0x575b80, 0x575b8d), (0x574b10, 0x574bc6), (0x590a70, 0x590a75), (0x590b90, 0x590c39), (0x579b40, 0x579c13), (0x58dc50, 0x58dd5e),
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


def case(r):
    if r.choice(OPS) == 'Destroy':
        return destroy(r)
    regions = {RAM + 0x10000: r.bytes(0x10)}
    return Call(0x575b80, [RAM, r.choice([0, 2, 4, 8, r.getrandbits(32)])], regions, conv='t')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
