# The handler of the window at 0x7cc61c (src/game/ui/dialog7cc61c: 0x4bb690) against the original: dialogbase.py's set-up (focus.py's FocusRestore tree of windows, queues
# and focus globals; the window's parent and default handler), the frame at 0xf82884 there (its +4 a window of the tree or none) and 0x7cc61c a window or none; messages
# 0x14, 0x15 and 0x100 (a = -1 mostly), else others, which go straight to the fallback handler; the windows' handlers and the hook [0xfaa5fc] fncmp's logger or none.
# usage: dialog7cc61c.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import dialogbase  # noqa: E402
import window  # noqa: E402
from fncmp import CDECL_LOG4, Bench, put  # noqa: E402

B = Bench(window.RANGES + [(0x47ee90, 0x47eed0), (0x65d55a, 0x65d57b), (0x4bb690, 0x4bb720)])


def case(r):
    c = dialogbase.case(r)
    wins = sorted(k for k in c.regions if window.TREE <= k < window.RECTS)
    frame = window.FOCUS + 0x40
    c.regions[0xf82884] = struct.pack('<I', frame)
    c.regions[frame] = put(bytearray(r.bytes(8)), 4, 'I', r.choice(wins + [0]))
    c.regions[0x7cc61c] = struct.pack('<I', r.choice(wins + [0]))
    for w in wins:   # (the windows sent a message: their handlers fncmp's logger or none, the hook [0xfaa5fc] too)
        put(c.regions[w], 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
    c.regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    message = r.choice([0x14, 0x14, 0x15, 0x15, 0x100, 0x100, 0x100, 0, 4, 0x10014, r.getrandbits(32)])
    a = 0xffffffff if message == 0x100 and r.random() < 0.7 else r.getrandbits(32)
    c.address, c.args, c.ret = 0x4bb690, [c.args[0], message, a, r.getrandbits(32)], 'a'
    return c


if __name__ == '__main__':
    B.main(case)
