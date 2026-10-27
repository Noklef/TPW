# The wide-text writer (src/game/core/widewriter) against the original: its finish (0x6b00e0). A writer at W: the cursor null or in a
# buffer at BUF (cells 0-8, the end at the cursor now and then, else any cell), the full byte 0, 1 or any, a holder at HOLD (a recorder
# vtable, slot 0 no arguments) or null. The buffer starts two cells before BUF (the cell before the cursor).
# usage: widewriter.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, vtable  # noqa: E402

B = Bench([(0x6b00e0, 0x6b0122)])
W, BUF, HOLD, HVT = RAM, RAM + 0x104, RAM + 0x200, RAM + 0x300


def case(r):
    log = log_region()
    k = r.randrange(9)
    cursor = r.choice([BUF + 2 * k] * 6 + [0])
    end = r.choice([cursor, cursor, BUF + 2 * r.randrange(9), r.getrandbits(32)])
    w = bytearray(r.bytes(0x14))
    struct.pack_into('<III', w, 4, cursor, end, r.choice([HOLD, HOLD, 0]))
    w[0x10] = r.choice([0, 0, 1, r.getrandbits(8)])
    regions = {W: w, BUF - 4: r.bytes(0x20), HOLD: struct.pack('<I', HVT) + r.bytes(4), HVT: vtable({}), LOG: log}
    return Call(0x6b00e0, [W], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
