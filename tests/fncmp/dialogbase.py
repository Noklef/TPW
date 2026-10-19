# The dialogs' fallback message handler (src/game/ui/dialogbase: 0x47ee90) against the original: window.py's FocusRestore set-up (a tree of
# windows, the message queues, the focus globals), one of the tree's windows called with its parent (+0x60) another of them or none and its
# default handler (+0x118) fncmp's logger (its result now and then set); message 0x14 mostly, else others.
# usage: dialogbase.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import window  # noqa: E402
from fncmp import CDECL_LOG4, LOG, Bench, put, set_return  # noqa: E402

B = Bench(window.RANGES + [(0x47ee90, 0x47eed0), (0x65d55a, 0x65d57b)])


def case(r):
    window.OPS = ['FocusRestore']
    c = window.case(r)
    wins = sorted(k for k in c.regions if window.TREE <= k < window.RECTS)
    w = r.choice(wins)
    put(c.regions[w], 0x60, 'I', r.choice(wins + [0]))
    put(c.regions[w], 0x118, 'I', CDECL_LOG4)
    if r.random() < 0.5:
        set_return(c.regions[LOG], 0x3d, r.choice([0, 1, -1, r.getrandbits(32)]))
    message = r.choice([0x14] * 6 + [0, 4, 0x15, 0x10014, r.getrandbits(32)])
    c.address, c.args, c.ret = 0x47ee90, [w, message, r.getrandbits(32), r.getrandbits(32)], 'a'
    return c


if __name__ == '__main__':
    B.main(case)
