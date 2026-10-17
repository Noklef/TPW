# The grid add and the colour mask tables of src/game/core/grid_masks (0x576620, 0x5830e0) against the originals: a pool with up to 6 free nodes and a grid of up to 3 x 3 list
# heads (vobj3.py's Add environment: objects of two classes whose slots 2 and 3 are recorders), rectangles inside, partly outside and outside the grid; masks of every
# size for the 16-bit mode (or another depth, when only the words are stored).
# usage: grid_masks.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Grid,Masks)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import vobj3  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, recorder, set_return, vtable  # noqa: E402

CALLS = {'Grid': 0x576620, 'Masks': 0x5830e0, 'Insert': 0x5e2a80}
B = Bench([(0x576620, 0x5766f0), (0x5830e0, 0x583230), (0x576270, 0x576320), (0x5e2a80, 0x5e2b80), (0x67acb0, 0x67acb1), (0x590730, 0x590750)], hooks={0x691de0: vobj3.lock, 0x691ea0: lambda e: None})
OBJ, VT, VTA, OBJS, NODES, CELLS, FREE = RAM, vobj3.VT, vobj3.VTA, vobj3.OBJS, vobj3.NODES, RAM + 0x600, RAM + 0x5000


def case(r):
    name = r.choice(OPS)
    if name == 'Masks':
        surf = bytearray(r.bytes(0xa00))
        put(surf, 0x1c, 'I', r.choice([0x10, 0x10, 0x10, 0x20, 0x18, 0]))
        words = [r.choice([0, 1, 5, 8, 10, 11, 16, 0x18, 0x1f, 0x20, 0xff, r.getrandbits(32)]) for _ in range(12)]
        return Call(CALLS[name], [OBJ] + words, {OBJ: surf}, conv='t', ret='v')
    if name == 'Insert':
        n = r.choice([0, 1, 2, 3, 4])
        nodes = bytearray(0x20 * 5)
        for k in range(n):
            put(nodes, 0x20 * k, 'III', r.getrandbits(32), NODES + 0x20 * (k + 1) if k + 1 < n else 0, NODES + 0x20 * (k - 1) if k else 0)
        lst = bytearray(r.bytes(0x20))
        put(lst, 4, 'III', NODES if n else 0, NODES + 0x20 * (n - 1) if n else 0, n)
        where = r.choice([0] + [NODES + 0x20 * k for k in range(n)]) if n else 0
        regions = {OBJ: lst, NODES: nodes, RAM + 0x400: put(bytearray(8), 0, 'I', where), HEAP: heap()}
        return Call(CALLS[name], [OBJ, r.getrandbits(32), RAM + 0x400], regions, conv='t', ret='v')
    w, h = r.choice([0, 1, 2, 3]), r.choice([0, 1, 2, 3])
    obj = bytearray(r.bytes(0x500))
    put(obj, 0x38, 'ii', w, h)
    put(obj, 0x10, 'I', CELLS)
    free = bytearray(0x20 * 7)
    nfree = r.choice([0, 1, 2, 4, 6])
    for k in range(nfree):
        put(free, 0x20 * k, 'II', r.getrandbits(32), FREE + 0x20 * (k + 1) if k + 1 < nfree else 0)
    put(obj, 4, 'I', FREE if nfree else 0)
    log = log_region()
    classes = [r.choice([VT, VTA]) for _ in range(5)]
    objs = bytearray(0x40 * 5)
    regions = {OBJ: obj, LOG: log, HEAP: heap(), FREE: free, 0x8bd508: r.bytes(4)}
    for k in range(4):
        regions[OBJS + 0x40 * k] = struct.pack('<I', classes[k]) + r.bytes(0x20)
    regions[OBJS + 0x300] = struct.pack('<I', classes[4]) + r.bytes(0x20)
    cells = bytearray(4 * 9 + 4)
    nodes = bytearray(0x10 * 6)
    n = 0
    for c in range(w * h):
        if r.random() < 0.5 and n < 4:
            put(nodes, 0x10 * n, 'II', OBJS + 0x40 * n, 0)
            put(cells, 4 * c, 'I', NODES + 0x10 * n)
            n += 1
    regions.update({CELLS: cells, NODES: nodes})
    vta = bytearray(vtable({}, size=8))
    struct.pack_into('<I', vta, 8, recorder(3, 0))
    regions.update({VT: vtable({}, size=8), VTA: vta})
    for k, v in {2: r.choice([1, 1, 0, 5]), 3: r.choice([1, 1, 0, 5])}.items():
        if r.random() < 0.7:
            set_return(log, k, v)
    rect = [r.choice([-2, -1, 0, 1, 2, 3, 5]) for _ in range(4)]
    return Call(CALLS[name], [OBJ] + [x & 0xffffffff for x in rect] + [OBJS + 0x300 if r.random() < 0.9 else OBJS, 0x70], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
