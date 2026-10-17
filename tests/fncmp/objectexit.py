# Whether an object's way out is open (src/game/world/object: 0x4de5a0) against the original, with MapCell_Direction, Nibble_Scale,
# Map_StepCellInDirection and MapCell_StepBlocked (and its callees) run on cellstep.py's cells: the exit cell (+0x38, in [0x8023a0]'s cells) the
# entrance (+0x36) or not, facing each way (straight or diagonal, the steps of 0x7cdba0 as set here) without leaving the map; the cell beyond it
# (in [0x80239c]'s cells) naming itself or another cell (about the map's edges and past them); every step out of that cell set up as cellstep.py's.
# usage: objectexit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4de5a0, 0x4de670), (0x522850, 0x522854), (0x4d8c00, 0x4d8c1f), (0x4d97e0, 0x4d9918), (0x4d99c0, 0x4d99d3), (0x4d96f0, 0x4d97d7),
           (0x4d8300, 0x4d8328),
           (0x4d8750, 0x4d8b29), (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc), (0x4dd4e0, 0x4dd510),
           (0x412e90, 0x412f00), (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0),
           (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1)])
OBJ, NEXT = RAM + 0x40000, 0x33000000
# each direction code's (dx, dy) and where 0x7cdba0's table holds it
STEPS = {1: (0x00, (0, -1)), 0x10: (0x08, (0, 1)), 0x20: (0x10, (-1, 1)), 8: (0x18, (1, 1)), 0x80: (0x20, (-1, -1)), 4: (0x28, (1, 0)),
         0x40: (0x30, (-1, 0)), 2: (0x38, (1, -1))}
SWAP = {1: 0x10, 0x10: 1, 4: 0x40, 0x40: 4, 2: 0x20, 0x20: 2, 8: 0x80, 0x80: 8}


def poke(regions, address, data):
    """data written into the page (cellstep.py's) it falls in, made when there is none."""
    for i, b in enumerate(data):
        a = address + i
        page = regions.get(a & ~0xfff)
        if page is None:
            page = regions[a & ~0xfff] = bytearray(0x1000)
        page[a & 0xfff] = b


def case(r):
    x, y = r.choice([0, 1, 0x7e, 0x7f, r.randrange(0x80), r.randrange(0x80)]), r.choice([0, 1, 0x7e, 0x7f, r.randrange(0x80), r.randrange(0x80)])
    same = r.random() < 0.5
    ways = [w for w, (_, (dx, dy)) in STEPS.items() if 0 <= x + dx < 0x80 and 0 <= y + dy < 0x80]
    way = r.choice(ways * 3 + [w for w in ways if w in (1, 4, 0x10, 0x40)] * 3)  # the way out, the cell's direction swapped for the entrance
    facing = SWAP[way] if same else way
    exit_id = (y << 7) + x + 1
    entrance = exit_id if same else r.choice([exit_id + 1, exit_id - 1, r.getrandbits(16)]) & 0xffff
    dx, dy = STEPS[way][1]
    next_id = ((dy << 7) + exit_id + dx) & 0xffff
    named = r.choice([next_id, next_id, next_id, next_id, r.choice([1, 0x80, 0x81, 0x3f80, 0x4000, 0x4001, 0, r.getrandbits(16)])])
    nx, ny = ((named & 0xff) - 1) & 0x7f, (((named - 1) & 0xffff) >> 7) & 0xff
    steps = []
    for d in range(4):
        target, source = cellstep.target_of(nx, ny, d)
        steps.append((source, target, d))
    regions = cellstep.world(r, 0, steps)
    cell = cellstep.ENTRIES + (exit_id - 1) * 0x44
    poke(regions, cell, struct.pack('<H', exit_id))
    poke(regions, cell + 0xd, bytes([facing]))
    pairs = bytearray(0x40)
    for _, (at, d) in STEPS.items():
        put(pairs, at, 'ii', *d)
    regions[0x7cdba0] = bytes(pairs)
    regions[0x80239c] = struct.pack('<I', NEXT)
    regions[NEXT + 0x294 + next_id * 0x44] = struct.pack('<H', named) + r.bytes(0x42)
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x36, 'HH', entrance, exit_id)
    regions[OBJ] = bytes(obj)
    return Call(0x4de5a0, [OBJ], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
