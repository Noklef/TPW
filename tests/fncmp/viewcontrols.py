# The view objects of src/game/frontend/viewcontrols against the originals: an object of random bytes, its step (+0x14) and values (+0x48,
# +0x50) now and then ordinary floats. UseSecond / UseFirst: the mode +0x58 random, the dialog [0x7cdb0c] a window whose children (0-2 others)
# now and then hold 0x16007, each child's handler (+0x114) fncmp's four-argument cdecl logger or 0, the hook [0xfaa5fc] the logger or 0.
# usage: viewcontrols.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Lower48,Raise48,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import CDECL_LOG4, LOG, Bench, Call, RAM, log_region, put  # noqa: E402

CALLS = {'Lower48': 0x5dd310, 'Raise48': 0x5dd320, 'Raise50': 0x5dd330, 'Lower50': 0x5dd340, 'UseSecond': 0x5dd350, 'UseFirst': 0x5dd390}
B = Bench([(0x5dd310, 0x5dd3ce), (0x4c57e0, 0x4c5830), (0x658d9f, 0x658e04), (0x659710, 0x659740), (0x65d7bc, 0x65d832), (0x65f697, 0x65f6e1)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
WINDOW, KIDS = RAM + 0x100, RAM + 0x200


def case(r):
    op = r.deal(OPS)
    obj = bytearray(r.bytes(0x60))
    regions = {RAM: obj}
    for off in (0x14, 0x48, 0x50):
        if r.random() < 0.7:
            put(obj, off, 'I', r.f())
    if op.startswith('Use'):
        put(obj, 0x58, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
        regions[LOG] = log_region()
        win = bytearray(r.bytes(0x90))
        ids = [r.choice([0x16006, 0x16008, 1]) for _ in range(r.choice([0, 1, 2]))]
        if r.random() < 0.85:
            ids.insert(r.randrange(len(ids) + 1), 0x16007)
        for i, ident in enumerate(ids):
            kid = bytearray(r.bytes(0x120))
            kid[4] = r.choice([0, 0, 0, 1])
            put(kid, 0x54, 'I', KIDS + 0x120 * (i + 1) if i + 1 < len(ids) else 0)
            put(kid, 0x8c, 'I', ident)
            put(kid, 0x114, 'I', r.choice([CDECL_LOG4, CDECL_LOG4, 0]))
            regions[KIDS + 0x120 * i] = kid
        put(win, 0x5c, 'I', KIDS if ids else 0)
        regions[WINDOW] = win
        regions[0x7cdb0c] = struct.pack('<I', WINDOW)
        regions[0xfaa5fc] = struct.pack('<I', r.choice([0, 0, CDECL_LOG4]))
    return Call(CALLS[op], [RAM], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
