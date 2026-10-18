# A ride's box in the world (src/game/coaster/ridebounds: 0x466b70) against the original:
# rides (or none) in the 0x7a4610 table, with or without their +0xcc block, tile corners of
# any 16 bits, heights and the root's y as floats; and its centre (0x556af0: the handle 0, or an index whose box there is or not).
# Also a ride part's position and direction (0x556b90): an object with the ride's index or 0, up to 4 model parts with small keys and
# masks (found or not, a negative key for the centre instead), their nodes or none, flipped or not, zero directions, no direction out;
# and a part's direction and up (0x551b30) from the same set-up.
# usage: ridebounds.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x466b70, 0x466c7f), (0x556af0, 0x556b8c), (0x556b90, 0x556ddb), (0x44b220, 0x44b267), (0x5da3c0, 0x5da3c1), (0x551b30, 0x551ca4)])
RIDE, BLOCK, HOLDER, MODEL, ROOT, OUT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x220, RAM + 0x300, RAM + 0x400


def box(r):
    ram = bytearray(r.bytes(0x500))
    put(ram, 0xcc, 'I', r.choice([BLOCK, BLOCK, BLOCK, 0]))
    corner = lambda: r.choice([r.randrange(256), r.getrandbits(16), 0, 0xffff])  # noqa: E731
    put(ram, 0xc0, 'HHHH', corner(), corner(), corner(), corner())
    put(ram, 8, 'I', HOLDER)
    put(ram, HOLDER - RAM + 4, 'I', MODEL)
    put(ram, MODEL - RAM + 0x78, 'I', ROOT)
    scale = r.choice([1, 100, None])
    r.floats(ram, [BLOCK - RAM + 0x1c, BLOCK - RAM + 0x28, ROOT - RAM + 0x44], scale)
    index = r.randrange(8)
    table = bytearray(r.bytes(32))
    put(table, 4 * index, 'I', r.choice([RIDE, RIDE, RIDE, 0]))
    return Call(0x466b70, [index, OUT], {RAM: ram, 0x7a4610: table}, ret='a')


def centre(r):   # (handle, x, y, z): the handle a dword (0: none) holding the index
    c = box(r)
    ram = c.regions[RAM]
    put(ram, 0x480, 'I', c.args[0] if r.random() < 0.9 else 0)
    return Call(0x556af0, [RAM + 0x480, RAM + 0x4c0, RAM + 0x4c4, RAM + 0x4c8], c.regions, ret='a')


def direction(r, ram, off):   # the three floats at off (any, from the caller) now and then made zero ones, or of a length out of float range
    if r.random() < 0.1:
        put(ram, off + r.choice([0, 4, 8]), 'I', 0x80000000 if r.random() < 0.5 else 0)
    if r.random() < 0.08:
        put(ram, off, 'III', 0, 0x80000000, 0)   # a zero direction (0 / 0)
    elif r.random() < 0.3:   # a length past the float range (inf) or under it (denormal), as the float it is stored as
        big = r.random() < 0.5
        put(ram, off, '3f', *(r.uniform(-3.4e38, 3.4e38) if big else r.uniform(-2e-38, 2e-38) for _ in range(3)))


def pose(r):   # (obj, pos, dir, key, mask): the ride at obj +0xc8 (or 0), a model part found by key and mask whose node gives both
    c = box(r)
    index = c.args[0]
    ram = c.regions[RAM] + r.bytes(0x300)
    table = c.regions[0x7a4610]
    if r.random() < 0.95:
        put(table, 4 * index, 'I', RIDE)
    obj, parts, entries, inst, nodes = RAM + 0x500, RAM + 0x600, RAM + 0x680, RAM + 0x6e0, RAM + 0x700
    put(ram, obj - RAM + 0xc8, 'I', index if r.random() < 0.9 else 0)
    n = r.randrange(5)
    put(ram, MODEL - RAM + 0x48, 'H', n)
    put(ram, MODEL - RAM + 0x7c, 'I', parts)
    put(ram, HOLDER - RAM + 0x28, 'I', inst)
    put(ram, inst - RAM + 4, 'I', entries)
    keys = [r.randrange(6) for _ in range(n)]
    for i in range(n):
        put(ram, parts - RAM + 0x14 * i, 'I', r.getrandbits(32) & r.choice([0xffffffff, 0x3da1f83, 0x10, 0]))
        put(ram, parts - RAM + 0x14 * i + 4, 'I', keys[i])
        node = nodes + 0x40 * i
        put(ram, entries - RAM + 0x14 * i, 'I', r.getrandbits(32) & ~0x10 | r.choice([0, 0x10]))
        put(ram, entries - RAM + 0x14 * i + 4, 'I', node if r.random() < 0.85 else 0)
        scale = r.choice([1, 100, None])
        r.floats(ram, [node - RAM + o for o in (0x20, 0x24, 0x28, 0x30, 0x34, 0x38)], scale)
        direction(r, ram, node - RAM + 0x20)
    key = r.choice(keys + [r.randrange(6), -1, -0x10, r.getrandbits(32)]) if keys else r.choice([0, 3, -1])
    mask = r.choice([0xffffffff, 0x10, 1, 0x3da1f82, 0x400000, 0, r.getrandbits(32)])
    out = r.choice([RAM + 0x4d0, RAM + 0x4d0, 0])
    return Call(0x556b90, [obj, RAM + 0x4c0, out, key & 0xffffffff, mask], {RAM: ram, 0x7a4610: table}, ret='a')


def axes(r):   # (index, part, dir, up): a part's node's direction (+0x20, flipped or not) and up (+0x10), each made of length 1
    c = pose(r)
    ram, table = c.regions[RAM], c.regions[0x7a4610]
    index, part = r.randrange(8), r.randrange(4)
    put(table, 4 * index, 'I', RIDE)
    entry, node = RAM + 0x680 + 0x14 * part, RAM + 0x700 + 0x40 * part
    put(ram, entry - RAM, 'I', r.getrandbits(32) & ~0x10 | r.choice([0, 0x10]))
    put(ram, entry - RAM + 4, 'I', node if r.random() < 0.85 else 0)
    r.floats(ram, [node - RAM + o for o in (0x10, 0x14, 0x18, 0x20, 0x24, 0x28)], r.choice([1, 100, None]))
    direction(r, ram, node - RAM + 0x10)
    direction(r, ram, node - RAM + 0x20)
    put(ram, 0x4e0, 'I', part)
    up = r.choice([RAM + 0x4f0, RAM + 0x4f0, 0])
    return Call(0x551b30, [index, RAM + 0x4e0, RAM + 0x4c0, up], c.regions, ret='v')


def case(r):
    op = r.deal([0, 1, 2, 3])
    return box(r) if op == 0 else centre(r) if op == 1 else pose(r) if op == 2 else axes(r)


if __name__ == '__main__':
    B.main(case)
