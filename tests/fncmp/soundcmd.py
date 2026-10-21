# The sound commands' runs (src/game/core/soundcmd) against the originals: a command at CMD (its value random, a float of each kind for
# 0x6b56a0: signalling and quiet NaNs, infinities, denormals), the out dword at OUT, the target at TARGET (a recorder vtable, its +0x24 the
# part at PART, another recorder vtable, or random now and then: a fault in both). The recorders log the stack functor's vtable, not
# its address.
# usage: soundcmd.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, blob_bytes, log_region, vtable  # noqa: E402

CALLS = {'b2c': 0x6b5770, 'b34': 0x6b56a0, 'b48': 0x6b5790, 'b50': 0x6b5840, 'b64': 0x6b5640, 'b68': 0x6b5670, 'b70': 0x6b59f0,
         'b74': 0x6b5a50, 'b78': 0x6b5990, 'ba8': 0x6b58b0}
B = Bench([(a, a + (0x30 if a in (0x6b5640, 0x6b5670) else 0x20)) for a in CALLS.values()] + [(0x6b9220, 0x6b9230), (0x6b9900, 0x6b9930)])
CMD, OUT, TARGET, PART, VT1, VT2 = RAM, RAM + 0x10, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x400
FLOATS = [0x7f800001, 0x7fbfffff, 0xff800001, 0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x00000001, 0x807fffff, 0, 0x80000000,
          0x3f800000]
OPS = [n for n in CALLS if not os.environ.get('OPS') or n in os.environ['OPS'].split(',')]


def case(r):
    name = r.deal(OPS)
    log = log_region()
    blob_bytes(log, 4, 1, 0xf)   # the functor (on the stack): its vtable
    value = r.choice(FLOATS + [r.getrandbits(32)]) if name == 'b34' else r.choice([0, 1, 2, 0xff, 0x100, r.getrandbits(32)])
    target = bytearray(r.bytes(0x40))
    struct.pack_into('<I', target, 0, VT1)
    struct.pack_into('<I', target, 0x24, PART if r.random() < 0.97 else r.getrandbits(32))
    part = bytearray(r.bytes(0x60))
    struct.pack_into('<I', part, 0, VT2)
    slots = {k: 2 for k in (4, 6, 7, 8, 9)}
    regions = {CMD: struct.pack('<II', r.getrandbits(32), value), OUT: r.bytes(4), TARGET: target, PART: part, VT1: vtable(slots),
               VT2: vtable(slots), LOG: log}
    return Call(CALLS[name], [CMD, OUT, TARGET], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
