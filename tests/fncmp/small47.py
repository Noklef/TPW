# The derived class destructor stubs and their scalar deleting destructors of src/game/core/small47 against the originals: the objects of vdtor3.py (the 0x6fd9d8 class: a
# member, a list of held objects, two strings) or small32.py (the 0x702cb0 class: a list of objects); the wrappers get flags either way (the free needs the heap region).
# usage: small47.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import small32  # noqa: E402
import vdtor3  # noqa: E402
from fncmp import Bench, HEAP, heap  # noqa: E402

# name: (module, the module's op, address, flags argument?)
STUBS = {'S48': (vdtor3, None, 0x41e2c0, 0), 'S68': (vdtor3, None, 0x41e320, 0), 'Sa8': (vdtor3, None, 0x41e4d0, 0), 'Sb8': (vdtor3, None, 0x41e4e0, 0),
         'Se8': (vdtor3, None, 0x41e5b0, 0), 'Sf8': (vdtor3, None, 0x41e5c0, 0), 'Sc0': (small32, 'DtorB', 0x5e3f60, 0), 'Sd0': (small32, 'DtorB', 0x5e3f90, 0),
         'W48': (vdtor3, None, 0x41d880, 1), 'W68': (vdtor3, None, 0x41da50, 1), 'Wa8': (vdtor3, None, 0x41ddc0, 1), 'Wb8': (vdtor3, None, 0x41dea0, 1),
         'We8': (vdtor3, None, 0x41e170, 1), 'Wf8': (vdtor3, None, 0x41e250, 1), 'Wc0': (small32, 'DtorB', 0x5e3f40, 1), 'Wd0': (small32, 'DtorB', 0x5e3f70, 1)}
RANGES = [(0x41c8b0, 0x41c990), (0x5f5fc0, 0x5f7f00), (0x67a400, 0x690000), (0x5e03b0, 0x5e0460), (0x5d5700, 0x5d5720), (0x5d5750, 0x5d5770), (0x5e3c00, 0x5e3c20)]
RANGES += [(a, a + 0x20) for (_m, _o, a, _f) in STUBS.values()]
B = Bench(RANGES)


def case(r):
    name = r.choice(OPS)
    mod, op, address, flags = STUBS[name]
    if op:
        saved = mod.OPS
        mod.OPS = [op]
    try:
        c = mod.case(r)
    finally:
        if op:
            mod.OPS = saved
    c.address, c.ret = address, 'a' if flags else 'v'
    c.regions.setdefault(HEAP, heap())
    if flags:
        c.args.append(r.choice([0, 1, 2, 3, 0xffffff00, 0xffffffff]))
    return c


OPS = [k for k in os.environ.get('OPS', ','.join(STUBS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
