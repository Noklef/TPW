# The map find, thing move, queue back and holder sleep of src/game/core/small58 (0x4f2b40, 0x50b6a0, 0x4de130, 0x45bdd0) against the originals:
#   Find: small37.py's wide string trees, searched with the output word given;
#   SetPos: things in a consistent table (thingenv's), a cell grid whose chains hold none or a thing, positions in the same cell or another;
#   Back: an object with a set or unset queue back and a world where the neighbour walk ends or runs on;
#   Sleep: a holder whose part sits in a renderer's slot lists (tags 0x10, 0x20, 0x70) or not.
# usage: small58.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,SetPos,Back,Sleep)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import small37  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import CELLS, POOL, THING, park  # noqa: E402

B = Bench([(0x4f2b40, 0x4f2c00), (0x4f6400, 0x4f6500), (0x50b6a0, 0x50b780), (0x4de130, 0x4de1f0), (0x4de040, 0x4de100), (0x4de670, 0x4de7e0), (0x522850, 0x522860), (0x4d97e0, 0x4d9920),
           (0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d99c0, 0x4d99e0), (0x4d9280, 0x4d93b0), (0x522770, 0x522790), (0x536300, 0x53644c), (0x4d8300, 0x4d8328), (0x4d91f0, 0x4d9280), (0x45bdd0, 0x45be80), (0x576540, 0x5765c0),
           (0x5da3c0, 0x5da3c1)], hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
OUT = RAM + 0x7000


def find(r):
    small37.OPS = ['Lower']
    c = small37.case(r)
    regions = dict(c.regions)
    regions[OUT] = r.bytes(4)
    return Call(0x4f2b40, [c.args[0], OUT, c.args[1]], regions, conv='t', ret='a')


def setpos(r):
    count = r.choice([1, 2, 3, 4])
    regions, handles, things = park(r, count=count, defs=False, cells=True)
    pool = bytearray(regions[POOL])
    grid = bytearray(regions[CELLS])
    for i in range(count):
        put(pool, THING * i + 8, 'HH', r.choice(handles + [0]), r.choice(handles + [0]))
        pool[THING * i + 5], pool[THING * i + 7] = r.choice([0, 1, 127, 128, 255, r.randrange(256)]), r.randrange(4)
    for i in range(len(grid) // 0x44):
        put(grid, 0x44 * i + 0x24, 'H', r.choice(handles + [0]))
    regions[POOL], regions[CELLS] = pool, grid
    me = r.randrange(count)
    x, y = r.getrandbits(32), r.getrandbits(32)
    if r.random() < 0.3:  # the same cell
        x = x & ~0xff00 | pool[THING * me + 5] << 8
        y = y & ~0xff00 | pool[THING * me + 7] << 8
    else:
        x = x & ~0xff00 | r.choice([0, 1, 127, 128, 255, r.randrange(256)]) << 8
        y = y & ~0xff00 | r.randrange(4) << 8
    return Call(0x50b6a0, [things[me], x, y], regions, conv='t', ret='v')


def back(r):
    obj = bytearray(r.bytes(0x60))
    put(obj, 0x3a, 'H', r.choice([0, 0, 0, 5, 0x81]))
    put(obj, 0x36, "H", r.choice([1, 2, 3, 0x81, r.randrange(1, 0x100)]))
    cells = bytearray(r.bytes(0x44 * 0x1c0))
    for idx in range(0x1c0):
        put(cells, 0x44 * idx, "H", min(idx + 1, 0x17f) if r.random() < 0.9 else r.randrange(1, 0x17f))
        put(cells, 0x44 * idx + 8, 'I', r.choice([3, 3, 9, 0, 1, 3, 4, 0xa]))
        cells[0x44 * idx + 0xd] = r.choice([0x10, 1, 0x40, 4, 0x10, 1, 0x40, 4, 0x80, 2])
    world = RAM + 0x10000
    deltas = bytearray(r.bytes(0x80))
    for i in range(0, 0x80, 4):
        put(deltas, i, 'i', r.choice([0, 1, -1, 2, -2, 0x80, 0, 0]))
    regions = {RAM: obj, world + 0x294: cells, 0x8023a0: struct.pack('<I', world + 0x294), 0x7cf83c: struct.pack('<I', world), 0x80239c: struct.pack('<I', world), OUT: r.bytes(4)}
    return Call(0x4de130, [RAM, OUT], regions, conv='t', ret='a')


def sleep(r):
    obj = bytearray(r.bytes(0x400))
    put(obj, 0, 'I', r.getrandbits(32))
    put(obj, 0x1c, 'I', r.choice([0, RAM + 0x100, RAM + 0x100]))
    put(obj, 0x100, 'I', RAM + 0x300)
    offset = r.choice([0, 0x10, 0x20])
    put(obj, 0x304, 'I', offset)
    renderer = bytearray(r.bytes(0x100))
    part = RAM + 0x100 + offset
    nodes = bytearray(0x40)
    for k, slot in enumerate((0x14, 0x18, 0x2c)):
        put(nodes, 8 * k, 'II', part if r.random() < 0.6 else r.getrandbits(32), 0)
        put(renderer, slot, 'I', RAM + 0x1400 + 8 * k if r.random() < 0.8 else 0)
    put(renderer, 4, 'I', 0)
    put(renderer, 0xc, 'I', 5)
    regions = {0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RAM + 0x1000), RAM: obj, RAM + 0x1000: renderer, RAM + 0x1400: nodes, 0x8bd500: r.bytes(0x20)}
    return Call(0x45bdd0, [RAM], regions, conv='c', ret='v')


CASES = {'Find': find, 'SetPos': setpos, 'Back': back, 'Sleep': sleep}


def case(r):
    return CASES[r.choice(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
