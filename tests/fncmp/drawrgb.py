# The 16-, 24- and 32-bit drawing primitives (src/game/gfx/drawrgb: the 0x703518 / 0x703558 / 0x703598 tables' own slots) against the
# originals, each called directly on its table's driver as draw8.py's make() sets it up: pixels of 2, 3 or 4 bytes, the context's pixel
# shifts (+0x50) random for the 16-bit packing, the game palette (0xfa71f8) random for the remap and the sprites.
# usage: drawrgb.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import draw8  # noqa: E402
from fncmp import Bench  # noqa: E402

B = Bench([(0x61af40, 0x61af70), (0x61f3a0, 0x62f8a0)])
TABLES = {16: (0x703518, 2, [0x628870, None, None, 0x62d540, None, 0x629c50, 0x62c7d0, 0x628b60, 0x629650, 0x629910, None, 0x62d440, 0x62d840,
                             0x62dc10, 0x62dff0, 0x62ed00]),
          24: (0x703558, 3, [0x61f3a0, None, None, 0x622190, None, 0x61ffa0, 0x621ae0, 0x61f540, 0x61fc20, 0x61fdd0, None, 0x622110, 0x622340,
                             0x622540, 0x622740, 0x622fd0]),
          32: (0x703598, 4, [0x623690, None, None, 0x626f80, None, 0x6242e0, 0x6265a0, 0x623880, 0x623ed0, 0x6240d0, None, 0x626ee0, 0x6271b0,
                             0x627480, 0x627730, 0x6280a0])}
CALLS = {draw8.SLOTS[slot][0] + str(depth): (slot, at, table, size) for depth, (table, size, slots) in TABLES.items()
         for slot, at in enumerate(slots) if at}


def case(r):
    slot, at, table, size = CALLS[r.deal(OPS)]
    return draw8.make(r, slot, at, table, size)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
