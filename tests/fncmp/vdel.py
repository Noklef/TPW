# The scalar deleting destructors over the destructors of src/game/core (vdtor, vobj2, small14) against the originals: each is a wrapper that calls the destructor on the
# object and frees it when bit 0 of its flags is set. The objects are the ones the benches of the destructors build (so the comparison covers what the destructor does),
# the flags are 0, 1 and others; the class 0x702a90 constructor and release are the same objects as those of 0x7029d0.
# usage: vdel.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import small14  # noqa: E402
import small29  # noqa: E402
import small32  # noqa: E402
import vdtor  # noqa: E402
import vobj2  # noqa: E402
from fncmp import Bench  # noqa: E402

# name: (module, the module's op, the wrapper's address, flags argument?)
WRAP = {'W80': (vdtor, 'D80', 0x5b55e0, 1), 'W90': (vdtor, 'D90', 0x5b58a0, 1), 'Wd0': (vdtor, 'Dd0', 0x5bced0, 1), 'Wf8': (vdtor, 'Df8', 0x5bd310, 1),
        'W20': (vdtor, 'D20', 0x5bd720, 1), 'Wda0': (vobj2, 'D2da', 0x5e4000, 1), 'W4f': (vobj2, 'D4f', 0x4f1260, 1), 'Wc2': (small14, 'C2', 0x5ccd80, 1),
        'K2C': (vdtor, 'C9C', 0x5d8b00, 0), 'K2R': (vdtor, 'C9R', 0x5d8b40, 0), 'K2D': (vdtor, 'C9D', 0x5d8b20, 0),
        'Wa': (small29, 'A', 0x5bdbf0, 1), 'Wb': (small29, 'B', 0x5be300, 1), 'Wc': (small29, 'C', 0x5be730, 1), 'W14': (vobj2, 'D14', 0x5cf7c0, 1),
        'Wb08': (small32, 'DtorA', 0x5da270, 1), 'Wcb0': (small32, 'DtorB', 0x5e0330, 1), 'Wd78': (small32, 'DtorC', 0x5e3c90, 1)}
RANGES = [(a, a + n) for (a, _c, _r, n) in vdtor.CALLS.values()] + [(0x67acb0, 0x67acb1), (0x67afa0, 0x67b060), (0x4f0650, 0x4f06a0), (0x5e4020, 0x5e4080),
                                                                       (0x5c9970, 0x5c99f0), (0x5da3c0, 0x5da3c1)]
RANGES += [(a, a + 0x20) for (_m, _o, a, _f) in WRAP.values()] + [(0x5bdc10, 0x5bdcd0), (0x5be320, 0x5be3e0), (0x5be750, 0x5be810), (0x5cea40, 0x5cea90), (0x5f5fc0, 0x5f7f00), (0x5d5700, 0x5d5720), (0x5d5750, 0x5d5770), (0x5e3c00, 0x5e3c20), (0x5da2f0, 0x5da3a0), (0x5e03b0, 0x5e0460), (0x5e3d10, 0x5e3dc0),
                                                                    (0x67a400, 0x690000)]
B = Bench(RANGES, hooks={0x691de0: vobj2.lock, 0x691ea0: lambda e: None})


def case(r):
    name = r.choice(OPS)
    mod, op, address, flags = WRAP[name]
    saved = mod.OPS
    mod.OPS = [op]
    try:
        c = mod.case(r)
    finally:
        mod.OPS = saved
    c.address, c.ret = address, 'a'  # (the wrapper returns the object)
    if flags:
        c.args.append(r.choice([0, 1, 2, 3, 0xffffff00, 0xffffffff]))
    return c


OPS = [k for k in os.environ.get('OPS', ','.join(WRAP)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
