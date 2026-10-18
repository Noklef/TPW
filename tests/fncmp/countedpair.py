# The CountedPair members of src/game/core/countedpair (0x4e6410 the copy constructor, 0x4f5860 the assignment) against the originals: pairs of a
# narrow and a wide std::string (null, empty, short, past 31 characters, shared, frozen, now and then unterminated or cut short by a 0) and a dword;
# the assignment also from a pair sharing the target's blocks, or from the target itself.
# usage: countedpair.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdstring  # noqa: E402
import stdwstring  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

B = Bench([(0x405280, 0x405390), (0x45b380, 0x45b5f0), (0x45b680, 0x45b9b0), (0x479230, 0x479270), (0x4e6410, 0x4e6560), (0x4f23f0, 0x4f2450),
           (0x4f5860, 0x4f5988), (0x4f6080, 0x4f60a0), (0x67acd0, 0x67ad00)])
P, X, NB, WB = RAM, RAM + 0x40, RAM + 0x1000, RAM + 0x2000   # the pair under test, the source, the narrow and the wide strings' blocks
CALLS = {'Copy': 0x4e6410, 'Assign': 0x4f5860}


def strings(r, nb, wb, k):
    """Slot k's narrow and wide strings (as stdstring / stdwstring lay them out, moved to NB / WB): (ptr, len, res) each."""
    n = stdstring.string(r, nb, k)
    w = stdwstring.string(r, wb, k)
    return ((n[0] - stdstring.BLOCKS + NB if n[0] else 0,) + n[1:]), ((w[0] - stdwstring.BLOCKS + WB if w[0] else 0,) + w[1:])


def pair(r, n, w):
    return struct.pack('<I3II3II', r.getrandbits(32), *n, r.getrandbits(32), *w, r.getrandbits(32))


def case(r):
    op = r.deal(OPS)
    nb, wb = bytearray(r.bytes(4 * stdstring.SLOT)), bytearray(r.bytes(4 * stdwstring.SLOT))
    xn, xw = strings(r, nb, wb, 0)
    pn, pw = strings(r, nb, wb, 1)
    if pw[0] and r.random() < 0.2:  # a capacity of exactly 32 (the assignment's _Grow keeps a block of 31 or less)
        pw = (pw[0], min(pw[1], 32), 32)
    if op == 'Assign' and r.random() < 0.2:  # x shares p's blocks (each one now and then), counted or not
        if r.random() < 0.7:
            xn = pn
        if r.random() < 0.7:
            xw = pw
    target = P if op == 'Copy' or r.random() < 0.93 else X  # (now and then the pair assigned to itself)
    regions = {P: pair(r, pn, pw), X: pair(r, xn, xw), NB: nb, WB: wb, HEAP: heap()}
    return Call(CALLS[op], [target, X], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
