# Two node pairs tested for a contact (src/game/coaster/contacts: 0x439c40) against the original: one owner or two (the same-owner checks on the types' flag 1 and the first dwords), boxes overlapping or not (a NaN now and then), the
# segments of each side a recorder of 0x43a060 filling its count over the segments in place, the shapes' test 0x43a2b0 a recorder answering in turn, the nodes' contact lists empty, nearly full or full.
# usage: contactspair.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, blob, fill, log_region, put, strings  # noqa: E402

SEGMENTS, TOUCH = 1, 2
CALLS = {0x43a060: (SEGMENTS, 6), 0x43a2b0: (TOUCH, 2)}
B = Bench([(0x439c40, 0x43a055)], hooks={addr: _recorder(k, n, cdecl=True) for addr, (k, n) in CALLS.items()})
OWNER1, OWNER2, TYPE1, TYPE2, SEGS1, SEGS2, ANSWERS = (RAM + 0x200 * k for k in range(7))
NODES = RAM + 0x1000
NAN = 0x7fc00001


def number(r, lo, hi):
    return r.choice([NAN, 0x7f800001]) if r.random() < 0.02 else struct.unpack('<I', struct.pack('<f', r.uniform(lo, hi)))[0]


def fbox(r, buf, at, x, spread):
    for k in range(3):
        put(buf, at + 4 * k, 'I', number(r, x - spread, x))
        put(buf, at + 0xc + 4 * k, 'I', number(r, x, x + spread))
        if x == 0 and r.random() < 0.15:   # limits of a tie that differ only in the zero's sign: which side's the box takes shows
            put(buf, at + r.choice([0, 0xc]) + 4 * k, 'I', r.choice([0, 0x80000000]))


def node(r, x):
    n = bytearray(r.bytes(0x110))
    put(n, 0, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(n, 8, 'I', r.getrandbits(32) & ~0x10000 | r.choice([0] * 9 + [0x10000]))
    fbox(r, n, 0x98, x, 5.0)
    put(n, 0xb0, 'I', r.choice([0, 0, 1, 7, 8, 9]))
    put(n, 0x100, 'I', r.choice([TYPE1, TYPE2]))
    return n


def segments(r, n, x):
    buf = bytearray(r.bytes(0x1c * 4))
    for i in range(n):
        put(buf, 0x1c * i, 'I', r.getrandbits(16))
        fbox(r, buf, 0x1c * i + 4, x, 3.0)
    return buf


def case(r):
    log = log_region()
    same = r.random() < 0.3
    far = r.choice([0.0, 0.0, 0.0, 2.0, 20.0])
    second = OWNER1 if same else OWNER2
    a, a2, b, b2 = (NODES + 0x200 * k for k in range(4))
    n1, n2 = r.choice([0, 1, 1, 2, 3, 4]), r.choice([0, 1, 1, 2, 3, 4])
    regions = {LOG: log, OWNER1: bytearray(r.bytes(0x144)), OWNER2: bytearray(r.bytes(0x144)), TYPE1: r.bytes(8), TYPE2: r.bytes(8), SEGS1: segments(r, n1, 0.0), SEGS2: segments(r, n2, far),
               a: node(r, 0.0), a2: node(r, 0.0), b: node(r, far), b2: node(r, far)}
    if far == 0.0 and r.random() < 0.25:   # (the two boxes' upper limits tying but for the zero's sign: which the common part takes shows)
        limit = 0x98 + 0xc + 4 * r.choice([1, 2])
        put(regions[a], limit, 'I', 0)
        put(regions[b], limit, 'I', 0x80000000)
    globals_ = bytearray(r.bytes(0x18))
    put(globals_, 0, 'I', SEGS2)
    put(globals_, 8, 'I', SEGS1)
    regions[0x792230] = globals_
    regions[0x7922dc] = r.bytes(4)
    strings(log, SEGMENTS, 0, out=1 << 5)
    blob(log, SEGMENTS, 3, 0x3f)
    fill(log, SEGMENTS, 5, struct.pack('<2I', n1, n2), records=2)
    regions[ANSWERS] = struct.pack('<40I', *[r.choice([0, 0, 0, 1]) for _ in range(40)])
    answers(log, TOUCH, ANSWERS)
    return Call(0x439c40, [OWNER1, second, a, a2, b, b2], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
