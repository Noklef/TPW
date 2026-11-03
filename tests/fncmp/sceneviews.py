# The scene object's views made (src/game/frontend/viewcontrols: 0x5e3dc0) against the original: RandomSegments_Ctor (k 20) and Class702bc8_Ctor (k 21) recorders, a heap (any of the five
# allocations now and then failing), the scene [0xf85614] a recorder object (slot 6: the fifth view added), the object's list (+0x18 -> +4) of 0 to 4 nodes, each a recorder object (slot 1).
# usage: sceneviews.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, heap, log_region, put, vtable  # noqa: E402

B = Bench([(0x5e3dc0, 0x5e3f40)], hooks={0x5dfcd0: _recorder(20, 0), 0x5dc580: _recorder(21, 0)})
THIS, SCENE, SCENE_VT, LIST, NODES, ITEMS, ITEM_VT = RAM, RAM + 0x100, RAM + 0x110, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800


def case(r):
    regions = {LOG: log_region(), HEAP: heap(0x1000, r.deal([0, 0, 0, 1, 2, 3, 4, 5])), 0xf85614: struct.pack('<I', SCENE), SCENE: struct.pack('<I', SCENE_VT), SCENE_VT: vtable({6: 1}),
               ITEM_VT: vtable({})}
    n = r.deal([0, 1, 2, 4])
    nodes = bytearray(r.bytes(0x40))
    for k in range(n):
        put(nodes, 8 * k, '2I', ITEMS + 0x10 * k, NODES + 8 * (k + 1) if k + 1 < n else 0)
        regions[ITEMS + 0x10 * k] = struct.pack('<I', ITEM_VT) + r.bytes(0xc)
    regions[NODES] = nodes
    regions[LIST] = put(bytearray(r.bytes(0x10)), 4, 'I', NODES if n else 0)
    regions[THIS] = put(bytearray(r.bytes(0x40)), 0x18, 'I', LIST)
    return Call(0x5e3dc0, [THIS], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
