# The destructor of src/game/core/small35 (0x41c8b0) against the original: an object with a member of its own (deleted through slot 3), a list of up to three nodes (some
# without a held object, one with a bad count) whose objects are deleted through slot 11, and two game strings; every virtual call is a recorder.
# usage: vdtor3.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gamestring  # noqa: E402
from fncmp import Bench, Call, HEAP, LOG, RAM, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x41c8b0, 0x41c990), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000)])
OBJ, LST, NODES, OBJS, VT, VTL, VTM, MEMBER = RAM, RAM + 0x1000, RAM + 0x1100, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3300


def case(r):
    obj = bytearray(r.bytes(0x100))
    regions = {OBJ: obj, LOG: log_region(), HEAP: heap()}
    n = r.choice([0, 1, 2, 3])
    nulls = 0.2
    lst = bytearray(r.bytes(0x20))
    put(lst, 0, 'I', VTL)
    put(lst, 4, 'II', NODES if n else 0, NODES + 0x10 * (n - 1) if n else 0)
    put(lst, 0xc, 'I', n)
    nodes = bytearray(r.bytes(0x60))
    for k in range(n):
        held = OBJS + 0x40 * k
        put(nodes, 0x10 * k, 'III', held if r.random() >= nulls else 0, NODES + 0x10 * (k + 1) if k + 1 < n else 0, NODES + 0x10 * (k - 1) if k else 0)
        regions[held] = struct.pack('<I', VT) + r.bytes(0x20)
    regions.update({LST: lst, NODES: nodes, VT: vtable({11: 1}), VTL: vtable({0: 1}), VTM: vtable({3: 1})})
    put(obj, 0x2c, 'I', LST if r.random() < 0.9 else 0)
    put(obj, 0x3c, 'I', MEMBER if r.random() < 0.8 else 0)
    regions[MEMBER] = struct.pack('<I', VTM) + r.bytes(0x20)
    wide = r.choice([0, 1])
    for k, off in enumerate((4, 0x18)):
        gamestring.string(r, wide, RAM + 0x4000 + 0x40 * k, RAM + 0x6000 + 0x80 * k, regions)
        obj[off:off + 0x14] = regions.pop(RAM + 0x4000 + 0x40 * k)[:0x14]
    return Call(0x41c8b0, [OBJ], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
