# The complete destructors of src/game/core/small59 (0x41e270, 0x41e2d0, 0x41e330, 0x41e4f0, 0x41e560, 0x41e470) against the originals: the objects of vdtor3.py (the 0x6fd9d8
# class: a member, a list of held objects, two strings) with handles at +0x40 / +0x48 that hold nothing (the release of a held object goes through the import
# the emulator does not give), and an object of two strings and a handle at +0x2c.
# usage: small59.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import vdtor3  # noqa: E402
from fncmp import Bench, HEAP, heap, put  # noqa: E402

CALLS = {'A': 0x41e270, 'B': 0x41e2d0, 'C': 0x41e560, 'D88': 0x41e330, 'Dc8': 0x41e4f0, 'Str': 0x41e470}
B = Bench([(0x41c8b0, 0x41c990), (0x41e270, 0x41e2d0), (0x41e2d0, 0x41e330), (0x41e330, 0x41e3b0), (0x41e470, 0x41e4e0), (0x41e4f0, 0x41e570), (0x41e560, 0x41e5b0),
           (0x5f61e0, 0x5f6210), (0x5f82a0, 0x5f8310), (0x5f8420, 0x5f8460), (0x619800, 0x619870), (0x67a400, 0x690000)])


def case(r):
    name = r.choice(OPS)
    c = vdtor3.case(r)
    c.address, c.ret = CALLS[name], 'v'
    obj = c.regions[vdtor3.OBJ]
    c.regions.setdefault(HEAP, heap())
    if name == 'Str':
        put(obj, 0x30, 'I', 0)  # (the handle's object)
    else:
        put(obj, 0x44, 'I', 0)
        put(obj, 0x4c, 'I', 0)
    return c


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
