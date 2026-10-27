# The sound bank's file (src/game/core/soundbank: 0x6c58f0, and op bankopen the opener 0x6c5a00 over it) against the original: a path String (the game's class, gamestring.py's layout, its
# block from the case and grown from the heap region) of a few chars, hdFirst 0, 1 or random, the opener at OPENER with a recorder vtable whose slot
# 1 answers a pointer to STATUS (negative, zero or positive). The recorder logs the path's text and the info's text (empty), not their addresses.
# usage: soundbank.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import gamestring  # noqa: E402
from fncmp import HEAP, LOG, RAM, Bench, Call, blob_bytes, heap, log_region, set_return, strings, vtable  # noqa: E402

B = Bench(gamestring.B_RANGES + [(0x6c58f0, 0x6c59f4), (0x6c5a00, 0x6c5aca)])
PATH, DATA, OPENER, OVT, STATUS, OUT = RAM, RAM + 0x1000, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
OPEN = 1


def case(r):
    name = r.deal(['open', 'bankopen'])
    log = log_region()
    set_return(log, OPEN, STATUS)
    strings(log, OPEN, 2, out=1)                   # the status (on the stack): 0; the path's text: hashed
    blob_bytes(log, OPEN, 2, 0, inner=(0x20, 1))   # the info (on the stack): the empty text its +0x20 points to
    regions = {HEAP: heap(), gamestring.LOCALE: bytes(4), OPENER: struct.pack('<I', OVT) + r.bytes(12), OVT: vtable({OPEN: 3}),
               STATUS: struct.pack('<i', r.choice([-1, -0x80000000, 0, 1, 0x7fffffff, r.getrandbits(32) - (1 << 31)])), LOG: log}
    gamestring.string(r, 0, PATH, DATA, regions)
    if name == 'bankopen':   # (out; the fourth argument unused)
        regions[OUT] = r.bytes(4)
        return Call(0x6c5a00, [r.getrandbits(32), OUT, OPENER, PATH, r.getrandbits(32), r.choice([0, 1, r.getrandbits(32)])], regions, conv='t',
                    ret='a')
    return Call(0x6c58f0, [r.getrandbits(32), OPENER, PATH, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
