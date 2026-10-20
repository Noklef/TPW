# The park analyser's cell set-up (src/game/world/analyser: 0x4c7a90) against the original: the analyser added to the message centre (thing_ctors.py's: made, its maps
# of 0 to 4 keys, or not yet), then its cells' rectangles: rows and columns of either sign about 0 and 6 (a product past 40 cells cut back to a few), cell sizes about 0
# and 64 or negative, the width and height about the grid's and beyond.
# usage: analyserinit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import thing_ctors  # noqa: E402
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

B = Bench([(0x4c7a90, 0x4c7b62), (0x50b350, 0x50b370)] + thing_ctors.CENTRE_RANGES, hooks=thing_ctors.CENTRE_HOOKS)
A = RAM + 0x1000


def case(r):
    while True:
        rows, columns = r.choice([0, 1, 2, 3, 6, -1, -2, r.randrange(-8, 9)]), r.choice([0, 1, 2, 5, 6, -1, -3, r.randrange(-8, 9)])
        count = rows * columns
        if count <= 40:
            break
    size = r.choice([0, 1, 8, 16, 64, -4, r.randrange(-100, 100)])
    obj = bytearray(r.bytes(0x20 + 0x7c * max(count, 0) + 0x10))
    put(obj, 0xc, 'i', size)
    put(obj, 0x10, 'i', columns)
    put(obj, 0x14, 'i', rows)
    for off, n in ((0x18, columns), (0x1c, rows)):
        grid = n * size
        put(obj, off, 'i', r.choice([grid, grid - 1, grid - size // 2, grid + 3, 0, -1, r.randrange(-200, 600)]))
    regions = {A: obj, HEAP: heap()}
    thing_ctors.centre(r, regions)
    return Call(0x4c7a90, [A], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
