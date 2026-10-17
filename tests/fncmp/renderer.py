# The renderer object's accessors and entry lists (src/game/render/renderer: 0x576260, 0x5763f0,
# 0x576400, 0x576410, 0x576540, 0x5765c0, 0x5766e0, 0x576820) against the originals: any object bytes for the
# accessors; for the lists a pool of nodes dealt over the grid cells, the slots and the free list
# (keys from a few values so that removals both find and miss), grids from empty to 6 by 5, tags
# with and without 0x70, rectangles from beyond the grid on every side; for the add (0x576410) grid_masks.py's environment, the object's virtual calls 2 and 3 answering with 2 and a record
# whose packed rectangle bytes run from inside the grid to far outside it.
# usage: renderer.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SetRestoreCallback,...)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import grid_masks  # noqa: E402
import vobj3  # noqa: E402
from fncmp import Bench, Call, RAM, put, set_return  # noqa: E402

B = Bench([(0x576260, 0x57626a), (0x5763f0, 0x5763fa), (0x576400, 0x576404), (0x576540, 0x5765b8),
           (0x5765c0, 0x576616), (0x5766e0, 0x57681a), (0x576820, 0x57690c),
           (0x576410, 0x576550), (0x576270, 0x576320), (0x590730, 0x590750), (0x67acb0, 0x67acb1)],
          hooks={0x691de0: vobj3.lock, 0x691ea0: lambda e: None})
CALLS = {'SetRestoreCallback': 0x576260, 'SetStateFlags': 0x5763f0, 'StateFlags': 0x576400,
         'RemoveFromSlot': 0x576540, 'ClearSlot': 0x5765c0, 'RemoveInRect': 0x5766e0, 'ClearAll': 0x576820, 'Add': 0x576410}
LISTS = ('RemoveFromSlot', 'ClearSlot', 'RemoveInRect', 'ClearAll')
GRID, POOL = RAM + 0x1000, RAM + 0x2000


def lists_case(op, r):
    width, height = r.choice([(0, 0), (1, 1), (3, 2), (6, 5), (4, 4), (2, 5), (-1, 3), (3, -2)])
    cells = max(width, 0) * max(height, 0)
    obj = bytearray(0x100)
    put(obj, 0x38, 'ii', width, height)
    heads = [0] * (cells + 16)
    nodes = bytearray(0x200)
    free = list(range(48))
    r.shuffle(free)
    keys = [r.choice([1, 2, 3, 0x1234, r.getrandbits(32)]) for _ in range(3)]
    for slot in range(cells + 7):
        prev = 0
        for _ in range(r.choice([0, 0, 1, 2, 3])):
            if len(free) < 4:
                break
            i = free.pop()
            put(nodes, 8 * i, 'II', r.choice(keys), prev)
            prev = POOL + 8 * i
        heads[slot] = prev
    prev = 0
    for i in free[:r.randrange(len(free) + 1)]:
        put(nodes, 8 * i, 'II', r.getrandbits(32), prev)
        prev = POOL + 8 * i
    live = 48 - len(free)
    put(obj, 4, 'I', prev)
    put(obj, 0xc, 'I', live + r.choice([0, 0, 5]))
    put(obj, 0x10, 'I', r.choice([GRID, GRID, GRID, 0]))
    for n in range(7):
        put(obj, 0x14 + 4 * n, 'I', heads[cells + n])
    grid = bytearray(0x100)
    for c in range(cells):
        put(grid, 4 * c, 'I', heads[c])
    regions = {RAM: obj, GRID: grid, POOL: nodes, 0x8bd508: r.bytes(4)}
    tag = r.choice([0, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x15, 0x7f, 0x80, 0x90, 0xf0, 0x100])
    key = r.choice(keys + [7])
    if op == 'RemoveFromSlot':
        args = [RAM, key, tag]
    elif op == 'ClearSlot':
        args = [RAM, tag]
    elif op == 'RemoveInRect':
        w, h = max(width, 1), max(height, 1)
        args = [RAM, r.randrange(-2, w + 2), r.randrange(-2, h + 2), r.randrange(-2, w + 2), r.randrange(-2, h + 2), key]
        args = [a & 0xffffffff for a in args]
    else:
        args = [RAM]
    return Call(CALLS[op], args, regions, conv='t', ret='a')


def add_case(r):
    grid_masks.OPS = ['Grid']
    c = grid_masks.case(r)
    regions = dict(c.regions)
    obj = bytearray(regions[RAM])
    obj[0x0c:0x30] = bytes(0x24)           # the slot heads: empty
    regions[RAM] = obj
    record = RAM + 0x700
    byte = lambda: r.choice([0, 0, 1, 2, 3, 4, 0xff, r.getrandbits(8)])
    regions[record] = put(bytearray(r.bytes(0x100)), 0xa8, 'BBBB', byte(), byte(), byte(), byte())
    log = regions[grid_masks.LOG]
    set_return(log, 2, r.choice([2, 2, 2, 1, 0]))
    set_return(log, 3, r.choice([record, record, record, 0]))
    tag = r.choice([1, 1, 3, 0x101, 0, 2, 0x80, 0x10, 0x20, 0x30, 0x70, 0x71, 0x7f, 0xf0])
    return Call(0x576410, [RAM, c.args[5], tag], regions, conv='t', ret='a')


def case(r):
    op = r.choice(OPS)
    if op == 'Add':
        return add_case(r)
    if op in LISTS:
        return lists_case(op, r)
    args = [RAM]
    if op != 'StateFlags':
        args.append(r.choice([0, 0x21, 0x2821, 0x54e2b0, r.getrandbits(32)]))
    return Call(CALLS[op], args, {RAM: r.bytes(0x80)}, conv='t', ret='a' if op == 'StateFlags' else 'v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
