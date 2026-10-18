# An animated object put down on the grid (src/game/motion/placement: 0x467030) against the original: an animator, its holder, model and transform
# block of random content; grid sizes and origins of random floats; angles right and odd, cells and footprints small and wide; flags 2 and 0x20
# each set or not, with random other bits.
# Ground (0x4671f0): an animator whose holder is in holder.py's grid environment (by its rectangle, flag 0x4000, or the slot lists), turned
# by a right angle (now and then another: then at cell (0, 0)), moved to a cell 0-10 with a footprint 1-6 by 1-6 (sometimes 0 or negative)
# on footprint.py's map of heights, with or without its cells and flags 2, 0x10 and 0x20.
# usage: placement.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import holder  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x467030, 0x4671f0), (0x46f650, 0x46f700), (0x4671f0, 0x46730d), (0x454050, 0x4541a0), (0x466d70, 0x466fa0), (0x469930, 0x469950)]
          + holder.RANGES, sines=256, hooks=holder.HOOKS)
# Ground's: clear of holder.py's grid environment (RAM + 0x6100 its holder)
GMODEL, GXF, GMAP, GGRID, GHEIGHTS, GCELLS, GANIM = RAM + 0x6200, RAM + 0x6300, RAM + 0x6400, RAM + 0x6480, RAM + 0x6500, RAM + 0x6d00, RAM + 0x7200
GW = 24
ANIM, HOLDER, MODEL, XF, WORLD, GRID = (RAM + 0x1000 * i for i in range(6))


def small(r):
    return r.choice([0, 1, 2, 3, r.randrange(-5, 300), r.getrandbits(32) - (1 << 31)])


def ground(r):
    c = holder.clean_slots(r, holder.add_to_grid(r))
    regions = dict(c.regions)
    hold = c.args[0]
    turn = r.choice([0, 90, 180, 270] * 4 + [45, 360, -90])
    regions[hold] = put(put(bytearray(regions[hold]), 4, 'I', GMODEL), 0x10, 'i', turn)
    anim = put(bytearray(r.bytes(0x100)), 4, 'I', r.getrandbits(32) | 0x4000 if r.random() < 0.5 else r.getrandbits(32) & ~0x4000)
    put(put(put(anim, 8, 'I', hold), 0xc0, 'I', c.args[1]), 0xc4, 'I', c.args[2])
    grid = put(put(r.floats(r.bytes(0x30), range(8, 0x18, 4)), 0x18, 'I', GW), 0x28, 'I', GHEIGHTS)
    heights = bytearray(4 * (GW + 1) * 20)
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', r.choice([float('nan'), float('inf'), 3e38]) if r.random() < 0.03 else r.uniform(-50, 50))
    cells = bytearray(8 + 8 * 6 + 0xa0 * 6)
    for i in range(len(cells) // 4):
        put(cells, 4 * i, 'I', r.getrandbits(32) & r.choice([0, 0xf0, 1, 2, 4, 8, 0xffffffff]))
    w, d = r.choice([1, 2, 3, 4, 6, r.randrange(1, 7)]), r.choice([1, 2, 3, 5, r.randrange(1, 7)])
    if r.random() < 0.05:
        w, d = r.choice([(0, d), (w, 0), (-1, d), (w, -2)])
    x, y = (r.randrange(0, 11), r.randrange(0, 11)) if turn in (0, 90, 180, 270) else (0, 0)
    flags = r.choice([0, 2, 0x20, 0x22]) | r.choice([0, 0x10]) | (r.getrandbits(32) & ~0x32 if r.random() < 0.3 else 0)
    regions.update({GANIM: anim, GMODEL: put(r.bytes(0xac), 0x78, 'I', GXF), GXF: r.bytes(0x50), GMAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GGRID),
                    GGRID: grid, GHEIGHTS: heights, GCELLS: cells, 0x7a0854: put(bytearray(4), 0, 'I', GMAP)})
    return Call(0x4671f0, [GANIM, x, y, r.choice([GCELLS, GCELLS, 0]), w, d, flags], regions, ret='v')


def case(r):
    if r.deal(OPS) == 'Ground':
        return ground(r)
    anim = put(r.bytes(0xc8), 8, 'I', HOLDER)
    holder = put(r.bytes(0x14), 4, 'I', MODEL)
    put(holder, 0x10, 'i', r.choice([0, 90, 359, r.randrange(-720, 720), r.getrandbits(32) - (1 << 31)]))
    model = put(r.bytes(0xac), 0x78, 'I', XF)
    grid = r.floats(r.bytes(0x18), range(8, 0x18, 4))
    angle = r.choice([0, 0xb4, 90, 270, -90, r.randrange(-1000, 1000), r.getrandbits(32) - (1 << 31)])
    flags = r.choice([0, 2, 0x20, 0x22]) | (r.getrandbits(8) & ~0x22 if r.random() < 0.5 else 0)
    if r.random() < 0.2:
        flags |= r.getrandbits(32) & ~0xff
    args = [ANIM, small(r), small(r), small(r), small(r), angle, r.f(), flags, small(r), small(r)]
    regs = {ANIM: anim, HOLDER: holder, MODEL: model, XF: r.bytes(0x50), WORLD: put(r.bytes(0x70), 0x6c, 'I', GRID), GRID: grid,
            0x7a0854: put(bytearray(4), 0, 'I', WORLD)}
    return Call(0x467030, args, regs)


OPS = [k for k in os.environ.get('OPS', 'Place,Ground').split(',') if k]

if __name__ == '__main__':
    B.main(case)
