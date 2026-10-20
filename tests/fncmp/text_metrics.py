# The text measuring of src/game/gfx/text (0x47f9a0, 0x47fa00; and the constructor 0x47f940) against the originals: wide text (some NULs inside), start and end words either side of the text and of
# each other, widths and glyph widths including negative ones; the glyph width slot is a recorder returning a value of the case (the argument it was given is logged: its
# high half is what the originals left in EAX).
# Lookup (0x47fa40): the key through the ranges [0x78ba80] (methods.py's chains), the inner object's slots 5 and 7 recorders (5 logs the key
# it reaches through its argument; 7 answers a pointer to a word), or no inner object.
# usage: text_metrics.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Width,Fit,Ctor,Lookup)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, LOG, RAM, blob_bytes, log_region, put, set_return, strings, vtable  # noqa: E402

CALLS = {'Width': 0x47f9a0, 'Fit': 0x47fa00}
OTHER = {'Ctor': 0x47f940, 'Lookup': 0x47fa40}   # (its constructor, over any bytes)
B = Bench([(a, a + 0x60) for a in CALLS.values()] + [(0x47f940, 0x47f95a), (0x657d30, 0x657d6c), (0x47fa40, 0x47fa98), (0x6ad8f0, 0x6ad948)])
OBJ, VT, TEXT = RAM, RAM + 0x100, RAM + 0x1000
INNER, VT2, WORD, RANGES, POOL = RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x600


def lookup(r):
    log = log_region()
    blob_bytes(log, 5, 0, 0, (0, 0xf))   # (its argument points at the key, both on the stack)
    strings(log, 7, 0, 1)   # (a stack address)
    set_return(log, 7, WORD + 2 * r.randrange(4))
    if r.random() < 0.7:
        set_return(log, 5, r.getrandbits(32))
    pool = bytearray(0xc0)
    for at_, lo in ((0, 10), (0x40, 3), (0x80, 20)):
        hi = lo + r.randrange(0, 5)
        put(pool, at_, 'HHHH', lo, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0, hi, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0)
        put(pool, at_ + 8, '6H', *[r.getrandbits(16) for _ in range(6)])
    obj = put(bytearray(r.bytes(0x20)), 0x14, 'I', INNER if r.random() < 0.9 else 0)
    regions = {OBJ: obj, INNER: struct.pack('<I', VT2), VT2: vtable({5: 1, 7: 2}), LOG: log, WORD: r.bytes(8), 0x78ba80: struct.pack('<I', RANGES),
               RANGES: put(bytearray(r.bytes(0x10)), 8, 'I', POOL + r.choice([0, 0, 0x40])), POOL: pool}
    key = r.choice([0, 3, 5, 10, 11, 14, 20, 22, 25, r.randrange(0, 40), 0x10000 + r.randrange(0, 30), r.getrandbits(32)])
    return Call(0x47fa40, [OBJ, key], regions, conv='t', ret='a')


def case(r):
    name = r.choice(OPS)
    if name == 'Ctor':
        return Call(OTHER[name], [OBJ, r.getrandbits(32)], {OBJ: r.bytes(0x40)}, conv='t', ret='a')
    if name == 'Lookup':
        return lookup(r)
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
