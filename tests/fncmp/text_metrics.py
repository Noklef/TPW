# The text measuring of src/game/gfx/text (0x47f9a0, 0x47fa00; and the constructor 0x47f940) against the originals: wide text (some NULs inside), start and end words either side of the text and of
# each other, widths and glyph widths including negative ones; the glyph width slot is a recorder returning a value of the case (the argument it was given is logged: its
# high half is what the originals left in EAX).
# usage: text_metrics.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Width,Fit,Ctor)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, log_region, put, set_return, vtable  # noqa: E402

CALLS = {'Width': 0x47f9a0, 'Fit': 0x47fa00}
OTHER = {'Ctor': 0x47f940}   # (its constructor, over any bytes)
B = Bench([(a, a + 0x60) for a in CALLS.values()] + [(0x47f940, 0x47f95a), (0x657d30, 0x657d6c)])
OBJ, VT, TEXT = RAM, RAM + 0x100, RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    if name == 'Ctor':
        return Call(OTHER[name], [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x40)}, conv='t', ret='a')
    log = log_region()
    text = bytearray(r.bytes(0x40))
    for i in range(0, 0x40, 2):
        if r.random() < 0.8:
            put(text, i, 'H', r.choice([0x41, 0x42, 0x100, 0x7fff, 0x8000, 0xffff, r.getrandbits(16)]) or 1)
        else:
            put(text, i, 'H', 0)
    obj = put(bytearray(r.bytes(0x10)), 0, 'I', VT)
    if r.random() < 0.8:
        set_return(log, 3, r.choice([0, 1, 5, 0xffff, 0xfffffffe, 0x12345678, 0x8000, r.getrandbits(32)]))
    regions = {OBJ: obj, VT: vtable({3: 1}), LOG: log, TEXT: text}
    start = r.choice([0, 1, 2, 5, 0x1f, -1, -3, 0xffff, r.randrange(0x20)])
    if name == 'Width':
        end = r.choice([0, 1, 3, 6, 0x20, -1, r.randrange(0x20), start + 3, start + 1])
        return Call(CALLS[name], [OBJ, TEXT, start & 0xffffffff, end & 0xffffffff], regions, conv='t', ret='a')
    width = r.choice([0, 1, 5, 20, 0x7fff, 0x8000, -1, -20, 0x12345, 0xffff0005, r.getrandbits(32)])
    return Call(CALLS[name], [OBJ, TEXT, start & 0xffffffff, width & 0xffffffff], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(list(CALLS) + list(OTHER))).split(',') if k]

if __name__ == '__main__':
    B.main(case)
