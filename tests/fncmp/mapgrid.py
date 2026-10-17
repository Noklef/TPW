# The map grid tables of src/game/world/mapcell (0x52c420 MapGrid_MarkEdges, 0x52c5b0 MapGrid_ClassifyTypes) against the originals: grids of 0 to 16 by 0 to 16 cells in rows
# of 20 (types mostly 4, 9, 0x17, 0xa, 1 or others, and the cells just past the size too), for the second through an object whose +0x18 points at the grid.
# usage: mapgrid.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Edges,Classify)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x52c420, 0x52c5b0), (0x52c5b0, 0x52c6c0)])
GRID, OBJ = RAM, RAM + 0x4000
TYPES = [4, 4, 9, 0x17, 10, 1, 0, 2, 3, 5, 0x16, 0x18, 0xffffffff]


def case(r):
    w, h = r.randrange(0, 17), r.randrange(0, 17)
    grid = bytearray(r.bytes(8 + 8 * 20 * 18))
    put(grid, 0, 'ii', w, h)
    for i in range(20 * 18):
        put(grid, 8 + 8 * i, 'I', r.choice(TYPES))
    regions = {GRID: grid, 0x8186f0: r.bytes(0x110), 0x818800: r.bytes(0x408)}   # both tables and the size words after them
    if r.random() < 0.5:
        return Call(0x52c420, [GRID], regions, conv='c', ret='a')
    regions[OBJ] = put(bytearray(r.bytes(0x40)), 0x18, 'I', GRID)
    return Call(0x52c5b0, [OBJ], regions, conv='c', ret='a')


if __name__ == '__main__':
    B.main(case)
