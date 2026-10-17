# The derived class destructor stubs and their scalar deleting destructors of the 0x702cb0 class (0x5e3f60, 0x5e3f90 and 0x5e3f40, 0x5e3f70) against the originals: list_class_dtors.py's objects
# (a list of objects); the wrappers get flags (the free needs the heap region).
# usage: class702cb0_derived.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import list_class_dtors  # noqa: E402
from fncmp import Bench, HEAP, heap  # noqa: E402

# name: (module, the module's op, address, flags argument?)
STUBS = {'Sc0': (list_class_dtors, 'DtorB', 0x5e3f60, 0), 'Sd0': (list_class_dtors, 'DtorB', 0x5e3f90, 0), 'Wc0': (list_class_dtors, 'DtorB', 0x5e3f40, 1), 'Wd0': (list_class_dtors, 'DtorB', 0x5e3f70, 1)}
RANGES = [(0x67a400, 0x690000), (0x5e03b0, 0x5e0460), (0x5d5700, 0x5d5720), (0x5d5750, 0x5d5770), (0x5e3c00, 0x5e3c20)]
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
