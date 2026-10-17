# The 16-texel blends of src/game/terrain/terrain_blend (Lnd_BlendRow16 0x55ee10, Lnd_BlendRow16x3 0x55eaa0) against the originals: random colour rows (the alpha byte is skipped), random weight rows,
# random lookup tables, and the output row (the alpha byte cleared).
# usage: lndblend.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x55eaa0, 0x55ee10), (0x55ee10, 0x55f060)])
A, Bp, C, D, OUT = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
WA, WC, MUL, MIX, TA, TC = RAM + 0x500, RAM + 0x540, RAM + 0x580, RAM + 0x5c0, RAM + 0x600, RAM + 0x800
E, F, WE, MIX2, TE = RAM + 0xa00, RAM + 0xb00, RAM + 0xc00, RAM + 0xc40, RAM + 0xd00


def case(r):
    def row():
        if r.random() < 0.2:
            return r.choice([bytes(0x40), bytes([255]) * 0x40, bytes([0, 255] * 0x20)])
        return r.bytes(0x40)

    def small():
        return r.choice([r.bytes(16), bytes([255]) * 16, bytes(16), bytes(r.choice([0, 128, 255]) for _ in range(16))])

    regions = {A: row(), Bp: row(), C: row(), D: row(), OUT: r.bytes(0x44), WA: small(), WC: small(), MUL: small(), MIX: small(), TA: r.bytes(0x100), TC: r.bytes(0x100)}
    if r.random() < 0.5:
        return Call(0x55ee10, [A, Bp, WA, TA, C, D, WC, TC, MUL, MIX, OUT], regions, ret='v')
    regions.update({E: row(), F: row(), WE: small(), MIX2: small(), TE: r.bytes(0x100)})
    return Call(0x55eaa0, [A, Bp, WA, TA, C, D, WC, TC, E, F, WE, TE, MUL, MIX, MIX2, OUT], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
