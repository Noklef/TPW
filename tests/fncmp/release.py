# The instance's released blocks and their statics (src/game/model/release: 0x452540,
# 0x468200, 0x46b3b0-0x46b400), the face texture marks and flag test (src/game/model/faces:
# 0x469930, 0x46a550) and the other small leaves near them -- GlobalMem_FreeAtB0 (0x45baa0),
# IdTable_Init (0x468750) and Highlight_Point (0x45d610) -- against the originals.
# usage: release.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x468200, 0x468246), (0x46a550, 0x46a595), (0x46b3b0, 0x46b413), (0x452540, 0x452560),
           (0x45baa0, 0x45bab4), (0x468750, 0x46875b), (0x45d610, 0x45d616), (0x469930, 0x469940),
           (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30)])
INSTANCE, HOLDER, MODEL, RECORDS, TEXTURES = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x200, RAM + 0x400


def case(r):
    op = r.choice(OPS)
    ram = bytearray(r.bytes(0x600))
    put(ram, 8, 'I', HOLDER)
    put(ram, HOLDER - RAM + 4, 'I', MODEL)
    regions = {RAM: ram, HEAP: heap(0x1000), **global_heap()}
    if op == 'Records':  # up to 0x18 records, a few with bit 8
        n = r.choice([0, 1, 3, r.randrange(0x18)])
        put(ram, MODEL - RAM + 0x30, 'I', r.getrandbits(32) | r.choice([0, 0x8000, 0x8000]))
        put(ram, MODEL - RAM + 0x36, 'H', n)
        put(ram, MODEL - RAM + 0x54, 'I', RECORDS)
        for i in range(0x18):
            put(ram, RECORDS - RAM + 0x10 * i, 'B', r.getrandbits(8) & r.choice([0xf7, 0xf7, 0xff]))
            put(ram, RECORDS - RAM + 0x10 * i + 0xc, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x468200, [INSTANCE], regions)
    if op == 'BlockCC':
        put(ram, 0xcc, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x452540, [INSTANCE], regions)
    if op == 'FreeB0':
        put(ram, 0xb0, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x45baa0, [INSTANCE], regions, ret='a')
    if op == 'IdInit':
        return Call(0x468750, [RAM + 0x100, RAM + 0x104, RAM + 0x108], regions)
    if op == 'Point':
        return Call(0x45d610, [], regions, ret='a')
    if op == 'Low4':
        return Call(0x469930, [r.getrandbits(32) & r.choice([0xfffffff0, 0xffffffff, 0xf, 1, 8])], regions, ret='a')
    if op == 'Holder':
        put(ram, HOLDER - RAM + 0x18, 'I', r.choice([0, HEAP + 0x40, r.getrandbits(32)]))
        return Call(0x46b3b0, [INSTANCE], regions)
    if op == 'Add':
        return Call(0x46b3d0, [r.getrandbits(32), r.getrandbits(32)], regions, ret='a')
    if op in ('One', 'Epsilon'):
        regions[0x7b0468] = bytearray(r.bytes(8))
        return Call(0x46b3e0 if op == 'One' else 0x46b400, [], regions)
    n = r.choice([0, 1, 4, r.randrange(0x10)])  # 'Faces': entries pointing at 8-byte textures
    put(ram, MODEL - RAM, 'I', r.getrandbits(32) & r.choice([~0x200, ~0x200, -1]) & 0xffffffff)
    put(ram, MODEL - RAM + 0x5a, 'H', n)
    put(ram, MODEL - RAM + 0x6c, 'I', RECORDS)
    for i in range(0x10):
        put(ram, RECORDS - RAM + 0x10 * i, 'I', TEXTURES + 8 * r.randrange(0x20))
    return Call(0x46a550, [r.getrandbits(32), MODEL, r.choice([0, 1, r.getrandbits(32)])], regions)


OPS_ALL = ['Records', 'Holder', 'Add', 'One', 'Epsilon', 'Faces', 'Faces', 'BlockCC', 'FreeB0', 'IdInit',
           'Point', 'Low4']
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
