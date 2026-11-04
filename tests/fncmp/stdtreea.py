# Tree A's other constructor and its _Erase (src/game/core/stdtree: 0x478720 StdTree_CtorFullA, 0x47cde0 StdTree_EraseSubtreeA) against
# the originals. The constructor's bytes +0 and +1 are the comparator's and allocator's stack garbage in the original: left out.
# usage: stdtreea.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, Mask, heap, put  # noqa: E402

B = Bench([(0x478720, 0x4787df), (0x47cde0, 0x47ce61)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
TREE, NIL, NODES = RAM, RAM + 0x100, RAM + 0x1000
OPS = [k for k in os.environ.get('OPS', 'ctor,erase').split(',') if k]


def case(r):
    if r.deal(OPS) == 'ctor':
        regions = {TREE: r.bytes(0x20), NIL: bytes(0x20), 0x7b4a50: struct.pack('<II', r.getrandbits(32), r.choice([0, NIL])), HEAP: heap()}
        return Call(0x478720, [TREE], regions, conv='t', ret='a', masks=[Mask(TREE, [0, 1], 'stack garbage')])
    count = r.randrange(0, 12)
    nodes = [bytearray(r.bytes(0x1c)) for _ in range(count)]
    for m in nodes:
        put(m, 0, 'I', NIL)
        put(m, 8, 'I', NIL)
    for i in range(1, count):                                               # (each hangs on a free link of an earlier one)
        free = [(j, off) for j in range(i) for off in (0, 8) if struct.unpack_from('<I', nodes[j], off)[0] == NIL]
        j, off = r.choice(free)
        put(nodes[j], off, 'I', NODES + 0x1c * i)
    regions = {TREE: r.bytes(0x20), NIL: r.bytes(0x20), 0x7b4a50: struct.pack('<II', r.getrandbits(32), NIL), HEAP: heap(), NODES: b''.join(nodes) + bytes(0x1c)}
    return Call(0x47cde0, [TREE, NODES if count else NIL], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
